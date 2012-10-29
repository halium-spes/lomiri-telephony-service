import QtQuick 2.0
import Ubuntu.Components 0.1
import "../Widgets" as LocalWidgets
import "../DetailViewKeypad"
import "../"

LocalWidgets.TelephonyPage {
    id: liveCall

    property alias contact: contactWatcher.contact
    property QtObject call: callManager.foregroundCall
    property alias number: contactWatcher.phoneNumber
    property bool onHold: call ? call.held : false
    property bool isSpeaker: call ? call.speaker : false
    property bool isMuted: call ? call.muted : false
    property bool isDtmf: false

    title: "On Call"

    Component.onDestruction: {
        // if this view was destroyed but we still have
        // active calls, then it means it was manually removed
        // from the stack
        if (previousTab != -1 && callManager.hasCalls) {
            telephony.selectedTabIndex = previousTab
        }
    }

    function endCall() {
        if (call) {
            call.endCall();
        }
    }

    ContactWatcher {
        id: contactWatcher
        phoneNumber: call ? call.phoneNumber : ""
    }

    Connections {
        target: callManager
        onCallEnded: {
            if (!callManager.hasCalls) {
                if (liveCall.visible && liveCall.previousTab != -1) {
                    telephony.selectedTabIndex = liveCall.previousTab
                }
                telephony.endCall();
            }
        }
    }

    Rectangle {
        id: background

        anchors.fill: parent
        color: "#3a3c41"
    }

    BackgroundCall {
        id: backgroundCall

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        call: callManager.backgroundCall
        visible: callManager.hasBackgroundCall
    }

    Item {
        id: container

        anchors.top: backgroundCall.visible ? backgroundCall.bottom : parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        Item {
            id: body

            anchors.centerIn: parent
            width: childrenRect.width
            height: childrenRect.height

            LocalWidgets.FramedImage {
                id: picture

                anchors.horizontalCenter: parent.horizontalCenter
                width: units.gu(18)
                height: units.gu(18)
                source: contact ? contact.avatar : fallbackSource
                fallbackSource: "../assets/avatar_incall_rightpane.png"
                darkBorder: true
            }

            TextCustom {
                id: name

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: units.gu(2)
                anchors.top: picture.bottom
                text: contact ? contact.displayLabel : "Unknown Contact"
                color: "white"
                style: Text.Sunken
                styleColor: Qt.rgba(0.0, 0.0, 0.0, 0.5)
                fontSize: "large"
            }

            TextCustom {
                id: number

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: name.bottom
                anchors.topMargin: units.dp(2)
                text: liveCall.number
                color: "#a0a0a2"
                style: Text.Sunken
                styleColor: Qt.rgba(0.0, 0.0, 0.0, 0.5)
                fontSize: "medium"
            }

            TextCustom {
                id: location

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: number.bottom
                anchors.topMargin: units.dp(2)
                // FIXME: add back geo-location information to the ContactEntry model
                text: contact ? contact.geoLocation.label : ""
                color: "#a0a0a2"
                style: Text.Sunken
                styleColor: Qt.rgba(0.0, 0.0, 0.0, 0.5)
                fontSize: "medium"
            }

            TextCustom {
                id: dialing

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: units.gu(2)
                anchors.top: location.text != "" ? location.bottom : number.bottom

                text: "Dialing"
                color: "#a0a0a2"
                style: Text.Sunken
                styleColor: Qt.rgba(0.0, 0.0, 0.0, 0.5)
                fontSize: "medium"
                opacity: (call && call.dialing) ? 1.0 : 0.0
            }

            LocalWidgets.StopWatch {
                id: stopWatch
                time: call ? call.elapsedTime : 0

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: units.gu(2)
                anchors.top: location.text != "" ? location.bottom : number.bottom
                opacity: (call && !call.dialing) ? 1.0 : 0.0
            }

            Keypad {
                id: keypad

                visible: false
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: mainButtonsContainer.top
                anchors.bottomMargin: units.gu(1)
                onKeyPressed: {
                    if (call) {
                        call.sendDTMF(label)
                    }
                }
                z: 1
            }

            Item {
                id: mainButtonsContainer

                anchors.top: stopWatch.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                width: mainButtons.width
                height: mainButtons.height

                BorderImage {
                    id: mainButtonsBackground

                    anchors.fill: parent
                    anchors.topMargin: units.dp(-2)
                    anchors.bottomMargin: units.dp(-2)
                    anchors.leftMargin: units.dp(-3)
                    anchors.rightMargin: units.dp(-2)
                    source: "../assets/incall_keypad_background.png"
                    border {right: units.dp(14); left: units.dp(14); top: units.dp(10); bottom: units.dp(10)}
                    horizontalTileMode: BorderImage.Stretch
                    verticalTileMode: BorderImage.Stretch
                    smooth: true
                }

                Image {
                    id: mainButtonsSeparators

                    anchors.fill: mainButtonsBackground
                    anchors.topMargin: units.dp(1)
                    anchors.bottomMargin: units.dp(2)
                    anchors.leftMargin: units.dp(2)
                    anchors.rightMargin: units.dp(1)
                    source: "../assets/livecall_keypad_div_tile.png"
                    fillMode: Image.Tile
                    verticalAlignment: Image.AlignTop
                    horizontalAlignment: Image.AlignLeft
                }

                Grid {
                    id: mainButtons

                    columns: 2
                    rows: 2

                    LiveCallKeypadButton {
                        corner: Qt.TopLeftCorner
                        iconSource: selected ? "../assets/incall_keypad_dialler_selected.png" : "../assets/incall_keypad_dialler_unselected.png"
                        selected: liveCall.isDtmf
                        onClicked: {
                            liveCall.isDtmf = !liveCall.isDtmf
                            keypad.visible = liveCall.isDtmf
                        }
                    }

                    LiveCallKeypadButton {
                        corner: Qt.TopRightCorner
                        iconSource: selected ? "../assets/incall_keypad_speaker_selected.png" : "../assets/incall_keypad_speaker_unselected.png"
                        selected: liveCall.isSpeaker
                        onClicked: {
                            if (call) {
                                call.speaker = !selected
                            }
                        }
                    }

                    LiveCallKeypadButton {
                        corner: Qt.BottomLeftCorner
                        iconSource: selected ? "../assets/incall_keypad_pause_selected.png" : "../assets/incall_keypad_pause_unselected.png"
                        selected: liveCall.onHold
                        onClicked: {
                            if (call) {
                                call.held = !call.held
                            }
                        }
                    }

                    LiveCallKeypadButton {
                        corner: Qt.BottomRightCorner
                        iconSource: selected ? "../assets/incall_keypad_mute_selected.png" : "../assets/incall_keypad_mute_unselected.png"
                        selected: liveCall.isMuted
                        onClicked: {
                            if (call) {
                                call.muted = !call.muted
                            }
                        }
                    }
                }
            }

            Row {
                anchors.top: mainButtonsContainer.bottom
                anchors.topMargin: units.gu(3)
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: units.gu(0.5)

                Button {
                    id: hangupButton

                    iconSource: "../assets/incall_keypad_endcallbutton_icon.png"
                    width: units.gu(8)
                    color: "#bf400c"
                    darkBorder: true
                    onClicked: endCall()
                }

                Button {
                    id: addToContactsButton

                    iconSource: "../assets/incall_keypad_addcaller_unselected.png"
                    width: units.gu(8)
                    color: "#666666"
                    darkBorder: true
                }
            }
        }
    }
}
