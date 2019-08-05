// Hack to force the qml plugins to be linked statically
// TODO: Find out how to do that in .pro or .cpp file

import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Window 2.5
import QtQuick.Layouts 1.3
import QtQuick.Extras 1.4
import QtQuick.VirtualKeyboard 2.2
import QtQuick.VirtualKeyboard.Settings 2.2

// QtQuick.Timeline needs to e built from its repo. Will likely be included in Qt 5.14
import QtQuick.Timeline 1.0

// TODO: Find out how to build that
import Qt.SafeRenderer 1.1

// QtQuick.Shapes needs to be separately built from
// qtdeclarative/src/imports/shapes/
// TODO: Find out if that is build system bug
import QtQuick.Shapes 1.0

// Will QtStudio3D.OpenGL or its successor support WebAssembly?
import QtStudio3D.OpenGL 2.4

ApplicationWindow {
    visible: true
    width: 640
    height: 480
}
