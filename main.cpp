/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Design Viewer of the Qt Toolkit.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include <QtGui/private/qzipreader_p.h>

#include <QGuiApplication>
#include <QDebug>
#include <QQuickView>
#include <QQmlEngine>
#include <QDirIterator>
#include <QBuffer>
#include <QRegularExpression>
#include <QQmlComponent>
#include <QQuickItem>
#include <QQuickWindow>
#include <QPointer>
#include <QResource>
#include <QFileInfoList>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QFontDatabase>
#include <QFontInfo>
#include <QDir>
#if defined(Q_OS_ANDROID)
#include <QApplication>
#include <QMessageBox>
#endif

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
    emscripten_run_script("alert('" + escaped.toUtf8() + "');"
                          "location.hash = '';"
                          "location.reload();");
}

void setScreenSize(const QSize &size)
{
    const QString command =
            QString::fromLatin1("setScreenSize(%1, %2);").arg(size.width()).arg(size.height());
    emscripten_run_script(command.toUtf8());
}
#elif defined(Q_OS_ANDROID)
std::function<int(QStringList)> showFatalMessageAndDie;

void fetchProject(QByteArray *data, QString *fileName)
{
    *fileName = QCoreApplication::arguments().at(1);
    QFile file(*fileName);
    if (file.open(QIODevice::ReadOnly))
        *data = file.readAll();
}

void printError(const QString &error)
{
    qDebug() << error;
}

void setScreenSize(const QSize &size)
{
    qDebug() << "Screen size: " << size.width() << "x" << size.height();
}
#else

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

void setScreenSize(const QSize &size)
{
    fprintf(stderr, "Screen size: %d x %d\n", size.width(), size.height());
}
#endif // Q_OS_WASM

QString unpackProject(const QByteArray &project, const QString &targetDir, bool skipExtract = false)
{
    QDir().mkpath(targetDir);

    QBuffer buffer;
    buffer.setData(project);
    buffer.open(QIODevice::ReadOnly);

    if (!skipExtract) {
        QZipReader reader(&buffer);
        reader.extractAll(targetDir);
    }

    QDir projectLocationDir(targetDir);
    // maybe it was not a zip file so try it as resource binary
    if (projectLocationDir.isEmpty()) {
        if (!skipExtract)
            qDebug() << "File could not be extracted. Trying to open it as a resource file.";
        const uchar* data = reinterpret_cast<const uchar*>(project.data());
        const QString resourcePath("/qmlprojector");
        const QFileInfo sourceInfo(resourcePath);
        const QDir sourceDir(sourceInfo.dir());
        if (QResource::registerResource(data, resourcePath)) {
            return ":" + resourcePath;
        } else {
            printError("Can not load the resource data.");
        }
    }
    return targetDir;
}

QString findFile(const QString &dir, const QString &filter)
{
    QDirIterator it(dir, {filter}, QDir::NoFilter, QDirIterator::Subdirectories);
    return it.next();
}

void parseQmlprojectFile(const QString &fileName, QString *mainFile, QStringList *importPaths)
{
    /* if filename comes from a resource qml need qrc:/ at the mainfile and importPaths,
     * but all other c++ call like QFileInfo::exists do not understand that, there we
     * need to keep the : only at the beginning
     */
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        printError("Could not open " + fileName + ": " + file.errorString());
        return;
    }
    const QString text = QString::fromUtf8(file.readAll());
    const QRegularExpression mainFileRegExp("mainFile:\\s*\"(.*)\"");
    const QRegularExpressionMatch mainFileMatch = mainFileRegExp.match(text);
    if (mainFileMatch.hasMatch()) {
        QString basePath = QFileInfo(fileName).path() + "/";

        *mainFile = basePath + mainFileMatch.captured(1);
        if (mainFile->startsWith(QLatin1String(":/")))
#if QT_VERSION_MAJOR < 6
            *mainFile = "qrc:" + mainFile->midRef(1);
#else
            *mainFile = "qrc:" + mainFile->mid(1);
#endif
        const QRegularExpression importPathsRegExp("importPaths:\\s*\\[\\s*(.*)\\s*\\]");
        const QRegularExpressionMatch importPathsMatch = importPathsRegExp.match(text);
        if (importPathsMatch.hasMatch()) {
            for (const QString &path : importPathsMatch.captured(1).split(",")) {
                QString cleanedPath = path.trimmed();
                cleanedPath = basePath + cleanedPath.mid(1, cleanedPath.length() - 2);
                if (QFileInfo::exists(cleanedPath)) {
                    if (cleanedPath.startsWith(QLatin1String(":/")))
#if QT_VERSION_MAJOR < 6
                        cleanedPath = "qrc:" + cleanedPath.midRef(1);
#else
                        cleanedPath = "qrc:" + cleanedPath.mid(1);
#endif
                    importPaths->append(cleanedPath);
                }
            }
        }
    } else {
        return;
    }
}

int main(int argc, char *argv[])
{
    qDebug().noquote() << QString("Built on %1 %2\n").arg(__DATE__, __TIME__);

#if defined(Q_OS_ANDROID)
    QString mainQml;

    showFatalMessageAndDie = [&argc,&argv](QStringList messages) {
        QApplication *app = static_cast<QApplication*>(QApplication::instance());
        const bool createdLocally = (app == nullptr);
        if (createdLocally)
            app = new QApplication(argc, argv);
        messages.append("\nThis app is supposed to be run from Qt Design Studio.");
        QMessageBox msg(QMessageBox::Critical, "Fatal error", messages.join("\n"), QMessageBox::Ok);
        QObject::connect(&msg, &QMessageBox::finished, [&app](int){app->exit(-1);});
        msg.show();
        int retVal = app->exec();
        if (createdLocally) {
            delete app;
        }
        return retVal;
    };
    if (argc < 2) {
        return showFatalMessageAndDie({QString("Qml project to show has not been defined.")});
    }
#endif
#if QT_VERSION_MAJOR < 6
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

#if defined(Q_OS_ANDROID)
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Qt Design Viewer"));
#else
    QGuiApplication app(argc, argv);
#endif
    QString projectFileName;
    QByteArray projectData;
    fetchProject(&projectData, &projectFileName);

#ifdef Q_OS_WASM
    QString projectLocation = "/home/web_user/";
#else // Q_OS_WASM
    QTemporaryDir tempDir("qmlprojector");
    QString projectLocation = tempDir.path();
#endif // Q_OS_WASM

#if defined(Q_OS_ANDROID)
    const bool skipZipOpenAttempt = projectFileName.endsWith(".qmlrc");
    projectLocation = unpackProject(projectData, projectLocation, skipZipOpenAttempt);
#else
    projectLocation = unpackProject(projectData, projectLocation);
#endif
    QString mainQmlFile;
    QStringList importPaths;
    const QString qmlProjectFile = findFile(projectLocation, "*.qmlproject");
    if (!qmlProjectFile.isEmpty()) {
        parseQmlprojectFile(qmlProjectFile, &mainQmlFile, &importPaths);
    } else {
        mainQmlFile = findFile(projectLocation, "main.qml");
        if (mainQmlFile.isEmpty())
            mainQmlFile = findFile(projectLocation, QFileInfo(projectFileName).baseName() + ".qml");
    }
    if (mainQmlFile.isEmpty()) {
        printError("No \"*.qmlproject\", \"main.qml\" or \"" + QFileInfo(projectFileName).baseName()
                   + ".qml\" found in \"" + projectFileName + "\".");
        return -1;
    }
    QUrl mainQmlUrl = QUrl::fromUserInput(mainQmlFile);

    const QString qtquickcontrols2File = findFile(projectLocation, "qtquickcontrols2.conf");
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
#if defined(Q_OS_ANDROID)
        showFatalMessageAndDie({"Error while loading Qml component.", component->errorString()});
#endif
        return -1;
    }
    QObject *topLevel = component->create();
    if (!topLevel && component->isError()) {
        printError("Create error");
#if defined(Q_OS_ANDROID)
        showFatalMessageAndDie({"Error while creating Qml component.", component->errorString()});
#endif
        return -1;
    }
    QScopedPointer<QQuickWindow> window(qobject_cast<QQuickWindow *>(topLevel));
    if (window) {
        engine.setIncubationController(window->incubationController());
        setScreenSize(QSize()); // Full browser size. TODO: Find out initial ApplicationWindow size
    } else {
        QQuickItem *contentItem = qobject_cast<QQuickItem *>(topLevel);
        if (contentItem) {
            QQuickView* view = new QQuickView(&engine, nullptr);
            window.reset(view);
            view->setContent(mainQmlUrl, component, contentItem);
            view->setResizeMode(QQuickView::SizeViewToRootObject);
            setScreenSize(QSize(contentItem->width(), contentItem->height()));
        }
    }

    window->show();

    const int exitcode = app.exec();
    delete component;
    return exitcode;
}
