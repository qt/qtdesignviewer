// Hack to force the qml plugins to be linked statically
// TODO: Find out how to do that in .pro or .cpp file

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Extras
import QtQuick.Layouts
import QtQuick.Particles
import QtQuick.PrivateWidgets
import QtQuick.Templates
import QtQuick.VirtualKeyboard
import QtQuick.VirtualKeyboard.Settings
import QtQuick.Window
import QtQuick.XmlListModel
import QtQuick.Timeline
import QtQuick.Shapes

import Qt.labs.calendar
import Qt.labs.folderlistmodel
// import Qt.labs.lottieqt
import Qt.labs.platform
import Qt.labs.qmlmodels
import Qt.labs.settings
import Qt.labs.wavefrontmesh
import Qt.labs.sharedimage

import QtBluetooth
import QtCanvas3D
import QtCharts
import QtDataVisualization
// import QtFeedback
import QtGamepad
import QtGraphicalEffects
import QtLocation
import QtMultimedia
import QtNfc
import QtPositioning
import QtPurchasing
import QtRemoteObjects
import QtScxml
import QtSensors
import QtTest
import QtWebChannel
import QtWebSockets
import QtWebView

import QtQml
import QtQml.Models
import QtQml.RemoteObjects
import QtQml.StateMachine

import QtQuick.Scene3D
import QtQuick.Scene2D
import QtQuick.LocalStorage

import QtQuick.Controls.Styles
import QtQuick.Controls.Material
import QtQuick.Controls.Universal
import QtQuick.Controls.Imagine

import Qt3D.Extras
import Qt3D.Input
import Qt3D.Logic
import Qt3D.Core
import Qt3D.Render
import Qt3D.Animation

// TODO: Find out how to build that
import Qt.SafeRenderer

// Will QtStudio3D.OpenGL or its successor support WebAssembly?
import QtStudio3D.OpenGL

import Qt5Compat.GraphicalEffects

import FlowView

import Components
import QtQuick.Studio.Components
import QtQuick.Studio.Effects
import QtQuick.Studio.EventSimulator
import QtQuick.Studio.EventSystem
import QtQuick.Studio.LogicHelper
import QtQuick.Studio.MultiText

import QtQuick3D
import QtQuick3D.Effects
import QtQuick3D.Particles3D

ApplicationWindow {
    visible: true
    width: 640
    height: 480
}
