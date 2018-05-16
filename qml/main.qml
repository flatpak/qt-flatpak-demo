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

ApplicationWindow {
    id: root

    property variant fontMetrics: TextMetrics { text: "M" }
    property int componentHeight: 40
    property int componentWidth: 250
    property int spacing: Math.floor(fontMetrics.height / 2)
    property string backgroundColor: "#2290d9"
    property string componentBackgroundColor: "#006bb1"
    property string componentHighlightColor: "#eeeeef"

    width: 800
    height: 640
    visible: true
    title: qsTr("Flatpak Developer Demo")

    DemoPage { id: demoPage; anchors.fill: parent }

    function onScreenshotSaved(path) {
        demoPage.notify(qsTr("Screenshot saved to ") + path)
    }
}
