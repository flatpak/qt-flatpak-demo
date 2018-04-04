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

#ifndef FLATPAK_DEMO_H
#define FLATPAK_DEMO_H

#include <QObject>

class FlatpakDemo : public QObject
{
    Q_OBJECT
public:
    explicit FlatpakDemo(QObject *parent = 0);
    virtual ~FlatpakDemo();

    Q_INVOKABLE void sendNotification();
    Q_INVOKABLE void printFile(const QUrl &file);

private Q_SLOTS:
    void gotPrintResponse(uint response, const QVariantMap &results);
    void gotPreparePrintResponse(uint response, const QVariantMap &results);

    QString getRequestToken();

private:
    QString m_fileToPrint;
    uint m_requestTokenCounter = 0;
};

#endif // FLATPAK_DEMO_H
