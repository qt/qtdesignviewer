// Hack to force the qml plugins to be linked statically
// TODO: Find out how to do that in .pro or .cpp file

import QtQuick 2.13
import QtQuick.Controls 1.5
import QtQuick.Controls 2.13
import QtQuick.Dialogs 1.3
import QtQuick.Extras 1.4
import QtQuick.Layouts 1.3
import QtQuick.Particles 2.13
import QtQuick.PrivateWidgets 1.1
import QtQuick.Templates 2.13
import QtQuick.VirtualKeyboard 2.2
import QtQuick.VirtualKeyboard.Settings 2.2
import QtQuick.Window 2.13
import QtQuick.XmlListModel 2.13

// QtQuick.Timeline needs to e built from its repo. Will likely be included in Qt 5.14
import QtQuick.Timeline 1.0

// QtQuick.Shapes needs to be separately built from
// qtdeclarative/src/imports/shapes/
// TODO: Find out if that is build system bug
import QtQuick.Shapes 1.0

import Qt.labs.calendar 1.0
import Qt.labs.folderlistmodel 2.13
// import Qt.labs.lottieqt 1.0
import Qt.labs.platform 1.0
import Qt.labs.qmlmodels 1.0
import Qt.labs.settings 1.0
import Qt.labs.wavefrontmesh 1.13

import QtBluetooth 5.13
import QtCanvas3D 1.1
import QtCharts 2.13
import QtDataVisualization 1.13
// import QtFeedback 5.0
import QtGamepad 1.13
import QtGraphicalEffects 1.13
import QtMultimedia 5.13
import QtNfc 5.13
import QtPurchasing 1.13
import QtRemoteObjects 5.13
import QtScxml 5.13
import QtSensors 5.13
import QtTest 1.13
import QtWebChannel 1.13
import QtWebSockets 1.13
import QtWebView 1.13

import QtQml.Models 2.13
import QtQml.RemoteObjects 1.0
import QtQml.StateMachine 1.13

// TODO: Find out how to build that
import Qt.SafeRenderer 1.1

// Will QtStudio3D.OpenGL or its successor support WebAssembly?
import QtStudio3D.OpenGL 2.4

ApplicationWindow {
    visible: true
    width: 640
    height: 480
}
