QT += \
    quick \
    gui-private

CONFIG += \
    c++11

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp

OTHER_FILES += \
    index.html

RESOURCES += \
    $$files(importdummy.qml)
