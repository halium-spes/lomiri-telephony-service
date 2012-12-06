import QtQuick 2.0
import Ubuntu.Components 0.1

AbstractButton {
    id: button
    width: units.gu(20)
    height: units.gu(8)
    // FIXME: waiting on #1072733
    //iconSource: "../assets/dialer_call.png"

    UbuntuShape {
        anchors.fill: parent
        color: button.pressed ? "#aaaaaa" : "#bababa"
        radius: "medium"
    }

    Image {
        anchors.centerIn: parent
        width: units.gu(6)
        height: units.gu(6)
        source: "../assets/incall_hangup.png"
        fillMode: Image.PreserveAspectFit
        z: 1
    }
}
