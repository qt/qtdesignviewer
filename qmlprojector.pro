QT += \
    quick \
    gui-private

CONFIG += \
    c++11

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp

wasm {
    OTHER_FILES += \
        index.html
}

android {
    QT += widgets
    TARGET = qtdesignviewer

    DISTFILES += \
        android/AndroidManifest.xml

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
}

RESOURCES += \
    $$files(importdummy.qml)


#CONFIG -= import_plugins
#QTPLUGIN += qwasm qjpeg qgif
#wasm:LIBS += -s ASSERTIONS=1 -s DISABLE_EXCEPTION_CATCHING=0
#QMAKE_CXXFLAGS_RELEASE -= -O2
#QMAKE_CXXFLAGS_RELEASE += -Os

#EMCC_THREAD_LFLAGS += -s ASSERTIONS=1 -s TOTAL_MEMORY=33554432

#QMAKE_WASM_TOTAL_MEMORY=33554432

CONFIG(debug, debug|release) {
    wasm:LIBS += -s SAFE_HEAP=1 -s STACK_OVERFLOW_CHECK=1
}
#wasm:LIBS += -s LIBRARY_DEBUG=1
#wasm:LIBS += -s SYSCALL_DEBUG=1
