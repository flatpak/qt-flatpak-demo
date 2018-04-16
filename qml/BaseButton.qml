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
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

Button {
    id: control

    implicitWidth: root.componentWidth
    implicitHeight: root.componentHeight

    background: Rectangle {
        opacity: enabled ? 1 : 0.3
        color: control.down ? root.componentHighlightColor : root.componentBackgroundColor
        radius: 5
    }

    contentItem: Text {
        anchors.fill: parent
        text: control.text
        font: control.font
        color: control.down ? "black" : "white"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
