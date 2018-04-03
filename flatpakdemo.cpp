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

#include "flatpakdemo.h"

#include <KNotification>

FlatpakDemo::FlatpakDemo(QObject *parent)
    : QObject(parent)
{
}

FlatpakDemo::~FlatpakDemo()
{
}

void FlatpakDemo::sendNotification()
{
    KNotification *notify = new KNotification(QLatin1String("notification"), 0);
    notify->setFlags(KNotification::DefaultEvent);
    notify->setTitle(QLatin1String("Flatpak demo"));
    notify->setText(QLatin1String("<html><b>THE FUTURE OF APPLICATION DISTRIBUTION ON LINUX<b><html>"));
    notify->setIconName(QLatin1String("flatpak"));

    notify->sendEvent();
}
