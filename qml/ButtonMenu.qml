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

import QtQuick 2.10
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

Menu {
    id: menu

    background: Rectangle {
        implicitHeight: menu.count * root.componentHeight
        implicitWidth: root.componentWidth
        color: "#eeeeef"
        radius: 5
    }

    delegate: MenuItem {
        id: menuItem
        implicitWidth: root.componentWidth
        implicitHeight: root.componentHeight

        arrow: Canvas {
            x: parent.width - width
            implicitWidth: root.componentHeight
            implicitHeight: root.componentHeight
            visible: menuItem.subMenu
            onPaint: {
                var ctx = getContext("2d")
                ctx.fillStyle = menuItem.highlighted ? "#ffffff" : "#21be2b"
                ctx.moveTo(15, 15)
                ctx.lineTo(width - 15, height / 2)
                ctx.lineTo(15, height - 15)
                ctx.closePath()
                ctx.fill()
            }
        }

        contentItem: Text {
            leftPadding: menuItem.indicator.width
            rightPadding: menuItem.arrow.width
            text: menuItem.text
            font: menuItem.font
            opacity: enabled ? 1.0 : 0.3
            color: menuItem.highlighted ? "#ffffff" : "#000000"
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        background: Item {
                implicitWidth: root.componentWidth
                implicitHeight: root.componentHeight
                Rectangle {
                    anchors.top: parent.top
                    implicitWidth: root.componentWidth
                    implicitHeight: root.componentHeight - 5
                    color: menuItem.highlighted ? "#006bb1" : "transparent"
                    opacity: enabled ? 1 : 0.3
                    radius: menu.currentIndex == 0 ? 5 : 0
                }

                Rectangle {
                    anchors.bottom: parent.bottom
                    implicitWidth: root.componentWidth
                    implicitHeight: root.componentHeight - 5
                    color: menuItem.highlighted ? "#006bb1" : "transparent"
                    opacity: enabled ? 1 : 0.3
                    radius: menu.currentIndex == menu.count - 1 ? 5 : 0
                }
        }
    }

    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
    }

    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1.0; to: 0.0 }
    }
}



