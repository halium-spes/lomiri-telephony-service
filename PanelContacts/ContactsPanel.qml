import QtQuick 1.1
import QtMobility.contacts 1.1
import ".."
import "../Widgets"
import "../fontUtils.js" as Font
import "../ContactUtils"

Item {
    id: contactsPanel

    property alias searchQuery : contactsSearchBox.searchQuery

    anchors.fill: parent
    signal contactClicked(variant contact)
    onContactClicked: telephony.showContactDetails(contact)

    SearchEntry {
        id: contactsSearchBox

        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 10

        leftIconSource: text ? "../assets/quick_search_delete_icon.png" : "../assets/search_icon.png"
//        hint: "Search contacts"
        onLeftIconClicked: text = ""
    }

    ContactModel {
        id: contactsModel
        manager: "folks"

        filter: ContactFilters {
            filterText: contactsSearchBox.text
        }

        sortOrders: [
            SortOrder {
               detail: ContactDetail.DisplayLabel
               field: DisplayLabel.label
               direction:Qt.AscendingOrder
            }
        ]
    }

    ListView {
        id: contactsList
        anchors.top: contactsSearchBox.bottom
        anchors.topMargin: 10
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        clip: true
        // FIXME: references to runtime and fake model need to be removed before final release
        model: runtime ? fakeContacts : contactsModel
        delegate: ContactDelegate {
            onClicked: contactsPanel.contactClicked(contact)
        }

        section.property: "display"
        section.criteria: ViewSection.FirstCharacter
        section.delegate: TextCustom {
            width: parent.width
            height: paintedHeight
            text: section
            font.pixelSize: Font.sizeToPixels("medium")
            font.bold: true
            Rectangle {
                height: 1
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                color: "black"
            }
        }
    }
}
