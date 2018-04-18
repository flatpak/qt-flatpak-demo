/*
 * Copyright © 2018 Red Hat, Inc
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *       Jan Grulich <jgrulich@redhat.com>
 */

import QtQuick 2.6
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

Page {
    id: demoPage

    background: Rectangle {
        anchors.fill: parent
        color: root.backgroundColor
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.bottomMargin: root.spacing

        ColumnLayout {
            spacing: root.spacing * 4
            Layout.alignment: Qt.AlignHCenter

            Image {
                Layout.alignment: Qt.AlignHCenter
                source: "qrc:/assets/logo"
            }

            Label {
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                horizontalAlignment: Text.AlignHCenter
                color: "white"
                font.pointSize: Math.round(root.fontMetrics.font.pointSize * 2.5)
                text: qsTr("THE FUTURE OF APPLICATION\nDISTRIBUTION ON LINUX")
            }
        }

        RowLayout {
            id: buttonLayout
            Layout.alignment: Qt.AlignHCenter
            spacing: root.spacing * 4

            MenuButton {
                id: portalsButton
                text: qsTr("Portals")

                onPressed: {
                    if (down) {
                        down = false
                        portalsMenu.close()
                    } else {
                        down = true
                        portalsMenu.open()
                    }
                    repaint()
                }
            }

            BaseButton {
                id: filesystemButton
                text: qsTr("Application Data")

               onPressed: {
                    flatpakDemo.openApplicationData();
                }
            }
        }
    }

    ButtonMenu {
        id: portalsMenu
        x: buttonLayout.x
        y: buttonLayout.y - (count * root.componentHeight) - root.spacing

        Action {
            text: qsTr("Open a File...")
            onTriggered: fileDialog.open()
        }

        Action {
            text: qsTr("Show Notification")
            onTriggered: flatpakDemo.sendNotification()
        }

        Action {
            text: qsTr("Print...")
            onTriggered: printFileDialog.open()
        }

        Action {
            text: qsTr("Take Screenshot")
            onTriggered: flatpakDemo.takeScreenshot()
        }

        onClosed: {
            portalsButton.down = false
            portalsButton.repaint()
        }
    }

    FileDialog {
        id: fileDialog
        folder: shortcuts.home
        selectMultiple: false
        title: qsTr("Please choose a file")

        onAccepted: {
            demoPage.notify(fileUrl)
        }
    }

    FileDialog {
        id: printFileDialog
        folder: shortcuts.pictures
        nameFilters: [ "Image files (*.jpg *.png)"]
        selectMultiple: false
        title: qsTr("Please choose image file to print")

        onAccepted: {
            flatpakDemo.printFile(fileUrl)
        }
    }

    footer: Rectangle {
        id: footer
        implicitHeight: root.componentHeight
        implicitWidth: parent.width
        opacity: timer.running ? 1 : 0
        color: root.componentHighlightColor

        Behavior on opacity { PropertyAnimation { } }

        Text {
            id: footerText
            anchors.fill: parent
            color: "black"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        Timer {
            id: timer
            interval: 5000
        }
    }

    function notify(text) {
        footerText.text = text
        timer.start()
    }
}
