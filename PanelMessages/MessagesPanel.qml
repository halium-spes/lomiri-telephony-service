import QtQuick 2.0
import TelephonyApp 0.1
import Ubuntu.Components.ListItems 0.1 as ListItem
import Ubuntu.Components 0.1

Item {
    id: messageList

    anchors.fill: parent

    TextField {
        id: search

        anchors.top: parent.top
        anchors.topMargin: units.dp(10)
        anchors.left: parent.left
        anchors.leftMargin: units.dp(10)
        anchors.right: parent.right
        anchors.rightMargin: units.dp(10)

        placeholderText: "Search messages"
        Keys.onEscapePressed: text = ""

        primaryItem: AbstractButton {
            width: units.dp(20)
            Image {
                anchors.left: parent.left
                anchors.leftMargin: units.dp(5)
                anchors.verticalCenter: parent.verticalCenter
                source: search.text ? "../assets/quick_search_delete_icon.png" : "../assets/search_icon.png"
            }
            onClicked: search.text = ""
        }
    }

    Column {
        id: buttons
        anchors.top: search.bottom
        anchors.topMargin: units.dp(10)
        anchors.left: parent.left
        anchors.right: parent.right

        ListItem.ThinDivider {}

        ListItem.Standard {
            id: newMessage
            anchors.left: parent.left
            anchors.right: parent.right
            height: units.dp(30)
            __leftIconMargin: units.dp(19)
            __rightIconMargin: units.dp(14)

            icon: Qt.resolvedUrl("../assets/add_new_message_icon.png")
            iconFrame: false
            text: "New Message"
            // FIXME: maybe use a signal and handle in the instance
            onClicked: telephony.startNewMessage()
            selected: telephony.messages.loaded && telephony.view.newMessage
        }
    }

    MessagesProxyModel {
        id: messagesProxyModel
        messagesModel: messageLogModel
        searchString: search.text
        ascending: false
        onlyLatest: true
    }

    MessagesList {
        anchors.top: buttons.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        // FIXME: reference to fake model needs to be removed before final release
        model: typeof(runtime) != "undefined" ? fakeMessages : messagesProxyModel
    }
}
