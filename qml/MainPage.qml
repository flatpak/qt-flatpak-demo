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
    id: mainPage

    Text {
        id: hiddenText
        opacity: 0
    }

    background: Rectangle {
        anchors.fill: parent
        color: "#2290d9"
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 4

        Image {
            Layout.alignment: Qt.AlignHCenter
            source: "qrc:/assets/logo"
        }

        Label {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            horizontalAlignment: Text.AlignHCenter
            color: "white"
            font.pointSize: Math.round(hiddenText.font.pointSize * 2.5)
            text: qsTr("THE FUTURE OF APPLICATION\nDISTRIBUTION ON LINUX")
        }

        RowLayout {
            id: buttonLayout
            Layout.alignment: Qt.AlignHCenter
            spacing: 50

            MainButton {
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

            MainButton {
                id: filesystemButton
                text: qsTr("View Filesystem")

               onPressed: {
                    if (down) {
                        down = false
                        filesystemMenu.close()
                    } else {
                        down = true
                        filesystemMenu.open()
                    }
                    repaint()
                }
            }
        }
    }

    MainMenu {
        id: portalsMenu
        x: buttonLayout.x
        y: buttonLayout.y - (count * 40) - 8

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
        }

        Action {
            text: qsTr("Take Screenshot")
        }

        onClosed: {
            portalsButton.down = false
            portalsButton.repaint()
        }
    }

    MainMenu {
        id: filesystemMenu
        x: buttonLayout.x + 250 + 50
        y: buttonLayout.y - (count * 40) - 8

        Action {
            text: qsTr("Application Runtime")
        }

        Action {
            text: qsTr("Application Data")
        }

        onClosed: {
            filesystemButton.down = false
            filesystemButton.repaint()
        }
    }

    FileDialog {
        id: fileDialog
        title: qsTr("Please choose a file")
        folder: shortcuts.home

        onAccepted: {
            console.error("Selected: " + fileDialog.fileUrls)
        }
    }
}



