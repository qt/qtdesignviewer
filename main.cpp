#include <QGuiApplication>
#include <QQuickView>
#include <QQmlEngine>
#include <QDirIterator>
#include <QTemporaryDir>
#include <QBuffer>
#include <QRegularExpression>

#include <QtGui/private/qzipreader_p.h>

#include <QQmlComponent>
#include <QQuickItem>
#include <QQuickWindow>
#include <QPointer>

#ifdef Q_OS_WASM
#include <emscripten.h>

std::function<void(char *, size_t, char *)> g_setFileDataCallback;
extern "C" EMSCRIPTEN_KEEPALIVE void qt_callSetFileData(char *content, size_t contentSize,
                                                        char *fileName)
{
    if (g_setFileDataCallback == nullptr)
        return;

    g_setFileDataCallback(content, contentSize, fileName);
    g_setFileDataCallback = nullptr;
}

void fetchProject(QByteArray *data, QString *fileName)
{
    // Call qt_callSetFileData to make sure the emscripten linker does not
    // optimize it away, which may happen if the function is called from JavaScript
    // only. Set g_setFileDataCallback to null to make it a no-op.
    ::g_setFileDataCallback = nullptr;
    ::qt_callSetFileData(nullptr, 0, nullptr);

    auto setFileDataCallback = [data, fileName](char *content, size_t contentSize, char *projectFilename){
        *data->setRawData(content, contentSize);
        *fileName = QString::fromUtf8(projectFilename);
    };
    g_setFileDataCallback = setFileDataCallback;

    EM_ASM_({
        // Copy the file file content to the C++ heap.
        // Note: this could be simplified by passing the content as an
        // "array" type to ccall and then let it copy to C++ memory.
        // However, this built-in solution does not handle files larger
        // than ~15M (Chrome). Instead, allocate memory manually and
        // pass a pointer to the C++ side (which will free() it when done).

        // TODO: consider slice()ing the file to read it picewise and
        // then assembling it in a QByteArray on the C++ side.

        const contentSize = contentArray.byteLength;
        const heapPointer = _malloc(contentSize);
        const heapBytes = new Uint8Array(Module.HEAPU8.buffer, heapPointer, contentSize);
        heapBytes.set(contentArray);

        // Null out the first data copy to enable GC
        reader = null;
        contentArray = null;

        // Call the C++ file data ready callback
        ccall("qt_callSetFileData", null,
            ["number", "number", "string"], [heapPointer, contentSize, projectfileName]);
    });
}

void printError(const QString &error)
{
    QString escaped = error;
    escaped.replace("'", "\'");
    escaped.replace("\n", "\\n");
    emscripten_run_script("alert('" + escaped.toUtf8() + "');");
}
#else // Q_OS_WASM

void fetchProject(QByteArray *data, QString *fileName)
{
    if (QCoreApplication::arguments().count() < 2) {
        qWarning() << "Pass a package file name as argument";
        return;
    }

    *fileName = QCoreApplication::arguments().at(1);
    QFile file(*fileName);
    if (file.open(QIODevice::ReadOnly))
        *data = file.readAll();
}

void printError(const QString &error)
{
    fprintf(stderr, "%s\n", qPrintable(error));
}
#endif // Q_OS_WASM

void unpackProject(const QByteArray &project, const QString &targetDir)
{
    QDir().mkpath(targetDir);
    QBuffer buffer;
    buffer.setData(project);
    buffer.open(QIODevice::ReadOnly);
    QZipReader reader(&buffer);
    reader.extractAll(targetDir);
}

QString findFile(const QString &dir, const QString &filter)
{
    QDirIterator it(dir, {filter}, QDir::NoFilter, QDirIterator::Subdirectories);
    return it.next();
}

void parseQmlprojectFile(const QString &fileName, QString *mainFile, QStringList *importPaths)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return;
    const QString text = QString::fromUtf8(file.readAll());
    const QRegularExpression mainFileRegExp("mainFile:\\s*\"(.*)\"");
    const QRegularExpressionMatch mainFileMatch = mainFileRegExp.match(text);
    if (mainFileMatch.hasMatch()) {
        const QString basePath = QFileInfo(fileName).path() + "/";
        *mainFile = basePath + mainFileMatch.captured(1);

        const QRegularExpression importPathsRegExp("importPaths:\\s*\\[\\s*(.*)\\s*\\]");
        const QRegularExpressionMatch importPathsMatch = importPathsRegExp.match(text);
        if (importPathsMatch.hasMatch()) {
            for (const QString path : importPathsMatch.captured(1).split(",")) {
                QString cleanedPath = path.trimmed();
                cleanedPath = basePath + cleanedPath.mid(1, cleanedPath.length() - 2);
                if (QFileInfo::exists(cleanedPath))
                    *importPaths << cleanedPath;
            }
        }
    } else {
        return;
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QString projectFileName;
#ifdef Q_OS_WASM
    QString root = "/home/web_user/";
#else // Q_OS_WASM
    QTemporaryDir tempDir("qmlprojector");
    const QString root = tempDir.path();
#endif // Q_OS_WASM
    {
        QByteArray projectData;
        fetchProject(&projectData, &projectFileName);
        unpackProject(projectData, root);
    }

    QString mainQmlFile;
    QStringList importPaths;
    const QString qmlProjectFile = findFile(root, "*.qmlproject");
    if (!qmlProjectFile.isEmpty()) {
        parseQmlprojectFile(qmlProjectFile, &mainQmlFile, &importPaths);
    } else {
        mainQmlFile = findFile(root, "main.qml");
        if (mainQmlFile.isEmpty())
            mainQmlFile = findFile(root, QFileInfo(projectFileName).baseName() + ".qml");
    }
    const QUrl mainQmlUrl = QUrl::fromUserInput(mainQmlFile);

    const QString qtquickcontrols2File = findFile(root, "qtquickcontrols2.conf");
    if (!qtquickcontrols2File.isEmpty())
        qputenv("QT_QUICK_CONTROLS_CONF", qtquickcontrols2File.toLatin1());

    QQmlEngine engine;
    for (const QString &importPath : importPaths)
        engine.addImportPath(importPath);
    QPointer<QQmlComponent> component = new QQmlComponent(&engine);
    component->loadUrl(mainQmlUrl);
    while (component->isLoading())
        QCoreApplication::processEvents();
    if (!component->isReady()) {
        printError(component->errorString());
        return -1;
    }
    QObject *topLevel = component->create();
    if (!topLevel && component->isError()) {
        printError(component->errorString());
        return -1;
    }
    QScopedPointer<QQuickWindow> window(qobject_cast<QQuickWindow *>(topLevel));
    if (window) {
        engine.setIncubationController(window->incubationController());
    } else {
        QQuickItem *contentItem = qobject_cast<QQuickItem *>(topLevel);
        if (contentItem) {
            QQuickView* view = new QQuickView(&engine, nullptr);
            window.reset(view);
            view->setContent(mainQmlUrl, component, contentItem);
            view->setResizeMode(QQuickView::SizeViewToRootObject);
        }
    }
    window->showMaximized();

    const int exitcode = app.exec();
    delete component;
    return exitcode;
}
