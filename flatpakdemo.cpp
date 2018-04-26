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
#include <QDesktopServices>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusUnixFileDescriptor>
#include <QDesktopServices>
#include <QFileInfo>
#include <QPainter>
#include <QPdfWriter>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryFile>

#include <fcntl.h>
#include <QtCore/private/qcore_unix_p.h>

#include <QDebug>

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
    notify->setTitle(QLatin1String("Flatpak Demo"));
    notify->setText(QLatin1String("This notification was sent from a Flatpak sandbox."));
    notify->setIconName(QLatin1String("flatpak"));

    notify->sendEvent();
}

void FlatpakDemo::printFile()
{
    QDBusMessage message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.portal.Desktop"),
                                                          QLatin1String("/org/freedesktop/portal/desktop"),
                                                          QLatin1String("org.freedesktop.portal.Print"),
                                                          QLatin1String("PreparePrint"));
    // TODO add some default configuration to verify it's read/parsed properly
    message << QLatin1String("x11:") << QLatin1String("Prepare print") << QVariantMap() << QVariantMap() << QVariantMap{{QLatin1String("handle_token"), getRequestToken()}};

    QDBusPendingCall pendingCall = QDBusConnection::sessionBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall);
    connect(watcher, &QDBusPendingCallWatcher::finished, [this] (QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<QDBusObjectPath> reply = *watcher;
        if (reply.isError()) {
            qWarning() << "Couldn't get reply";
            qWarning() << "Error: " << reply.error().message();
        } else {
            QDBusConnection::sessionBus().connect(QString(),
                                                  reply.value().path(),
                                                  QLatin1String("org.freedesktop.portal.Request"),
                                                  QLatin1String("Response"),
                                                  this,
                                                  SLOT(gotPreparePrintResponse(uint,QVariantMap)));
        }
    });
}

void FlatpakDemo::takeScreenshot()
{
    QDBusMessage message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.portal.Desktop"),
                                                          QLatin1String("/org/freedesktop/portal/desktop"),
                                                          QLatin1String("org.freedesktop.portal.Screenshot"),
                                                          QLatin1String("Screenshot"));
    // TODO add some default configuration to verify it's read/parsed properly
    message << QLatin1String("x11:") << QVariantMap{{QLatin1String("interactive"), true}, {QLatin1String("handle_token"), getRequestToken()}};

    QDBusPendingCall pendingCall = QDBusConnection::sessionBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall);
    connect(watcher, &QDBusPendingCallWatcher::finished, [this] (QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<QDBusObjectPath> reply = *watcher;
        if (reply.isError()) {
            qWarning() << "Couldn't get reply";
            qWarning() << "Error: " << reply.error().message();
        } else {
            QDBusConnection::sessionBus().connect(QString(),
                                                  reply.value().path(),
                                                  QLatin1String("org.freedesktop.portal.Request"),
                                                  QLatin1String("Response"),
                                                  this,
                                                  SLOT(gotScreenshotResponse(uint,QVariantMap)));
        }
    });
}

void FlatpakDemo::openApplicationData()
{
    QSettings setting(QLatin1String("/.flatpak-info"), QSettings::IniFormat);
    QDesktopServices::openUrl(QUrl(QStringLiteral("file://") +
                                   setting.value(QLatin1String("Instance/instance-path"),
                                                 QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first() + QStringLiteral("/.var/app/org.flatpak.demo/")).toString()));
}

void FlatpakDemo::openFile(const QUrl &fileName)
{
    QDesktopServices::openUrl(fileName);
}

void FlatpakDemo::gotPrintResponse(uint response, const QVariantMap &results)
{
    // qWarning() << "Print response: " << response << results;
    if (response) {
        qWarning() << "Failed to print Flatpak Cheat Sheet";
    }
}

void FlatpakDemo::gotPreparePrintResponse(uint response, const QVariantMap &results)
{
    if (!response) {
        QDBusUnixFileDescriptor descriptor;
        const int fd = qt_safe_open("/app/share/org.flatpak.demo/flatpak-print-cheatsheet.pdf", O_RDONLY | O_CLOEXEC);
        if (fd == -1) {
            qWarning() << "Failed to open flatpak cheatsheet pdf file";
            return;
        }

        descriptor.setFileDescriptor(fd);
        qt_safe_close(fd);

        QDBusMessage message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.portal.Desktop"),
                                                              QLatin1String("/org/freedesktop/portal/desktop"),
                                                              QLatin1String("org.freedesktop.portal.Print"),
                                                              QLatin1String("Print"));

        message << QLatin1String("x11:") << QLatin1String("Print dialog") << QVariant::fromValue<QDBusUnixFileDescriptor>(descriptor) << QVariantMap{{QLatin1String("handle_token"), getRequestToken()}, {QLatin1String("token"), results.value(QLatin1String("token")).toUInt()}};

        QDBusPendingCall pendingCall = QDBusConnection::sessionBus().asyncCall(message);
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall);
        connect(watcher, &QDBusPendingCallWatcher::finished, [this] (QDBusPendingCallWatcher *watcher) {
            QDBusPendingReply<QDBusObjectPath> reply = *watcher;
            if (reply.isError()) {
                qWarning() << "Couldn't get reply";
                qWarning() << "Error: " << reply.error().message();
            } else {
                QDBusConnection::sessionBus().connect(QString(),
                                                    reply.value().path(),
                                                    QLatin1String("org.freedesktop.portal.Request"),
                                                    QLatin1String("Response"),
                                                    this,
                                                    SLOT(gotPrintResponse(uint,QVariantMap)));
            }
        });
    } else {
        qWarning() << "Failed to print selected document";
    }
}

void FlatpakDemo::gotScreenshotResponse(uint response, const QVariantMap &results)
{
    // qWarning() << "Screenshot response: " << response << results;
    if (!response) {
        if (results.contains(QLatin1String("uri"))) {
            Q_EMIT screenshotSaved(results.value(QLatin1String("uri")).toString());
        }
    } else {
        qWarning() << "Failed to take screenshot";
    }
}

QString FlatpakDemo::getRequestToken()
{
    m_requestTokenCounter += 1;
    return QString("u%1").arg(m_requestTokenCounter);
}
