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
    KNotification *notify = new KNotification(QStringLiteral("notification"));
    notify->setFlags(KNotification::DefaultEvent);
    notify->setTitle(QStringLiteral("Flatpak Developer Demo"));
    notify->setText(QStringLiteral("This notification was sent from a Flatpak sandbox."));
    notify->setIconName(QStringLiteral("flatpak"));

    notify->sendEvent();
}

void FlatpakDemo::printFile()
{
    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.portal.Desktop"),
                                                          QStringLiteral("/org/freedesktop/portal/desktop"),
                                                          QStringLiteral("org.freedesktop.portal.Print"),
                                                          QStringLiteral("PreparePrint"));
    // TODO add some default configuration to verify it's read/parsed properly
    message << QStringLiteral("x11:") << QStringLiteral("Prepare print") << QVariantMap() << QVariantMap() << QVariantMap{{QStringLiteral("handle_token"), getRequestToken()}};

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
                                                  QStringLiteral("org.freedesktop.portal.Request"),
                                                  QStringLiteral("Response"),
                                                  this,
                                                  SLOT(gotPreparePrintResponse(uint,QVariantMap)));
        }
    });
}

void FlatpakDemo::takeScreenshot()
{
    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.portal.Desktop"),
                                                          QStringLiteral("/org/freedesktop/portal/desktop"),
                                                          QStringLiteral("org.freedesktop.portal.Screenshot"),
                                                          QStringLiteral("Screenshot"));
    // TODO add some default configuration to verify it's read/parsed properly
    message << QStringLiteral("x11:") << QVariantMap{{QStringLiteral("interactive"), true}, {QStringLiteral("handle_token"), getRequestToken()}};

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
                                                  QStringLiteral("org.freedesktop.portal.Request"),
                                                  QStringLiteral("Response"),
                                                  this,
                                                  SLOT(gotScreenshotResponse(uint,QVariantMap)));
        }
    });
}

void FlatpakDemo::openApplicationData()
{
    QSettings setting(QStringLiteral("/.flatpak-info"), QSettings::IniFormat);
    QDesktopServices::openUrl(QUrl(QStringLiteral("file://") +
                                   setting.value(QStringLiteral("Instance/instance-path"),
                                                 QStringLiteral("%1%2").arg(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first())
                                                                       .arg(QStringLiteral("/.var/app/org.flatpak.qtdemo/"))).toString()));
}

void FlatpakDemo::openFile(const QUrl &fileName)
{
    QDesktopServices::openUrl(fileName);
}

void FlatpakDemo::gotPrintResponse(uint response, const QVariantMap &results)
{
    Q_UNUSED(results);
    // qWarning() << "Print response: " << response << results;
    if (response) {
        qWarning() << "Failed to print Flatpak Cheat Sheet";
    }
}

void FlatpakDemo::gotPreparePrintResponse(uint response, const QVariantMap &results)
{
    if (!response) {
        QDBusUnixFileDescriptor descriptor;
        const int fd = qt_safe_open("/app/share/org.flatpak.qtdemo/flatpak-print-cheatsheet.pdf", O_RDONLY | O_CLOEXEC);
        if (fd == -1) {
            qWarning() << "Failed to open flatpak cheatsheet pdf file";
            return;
        }

        descriptor.setFileDescriptor(fd);
        qt_safe_close(fd);

        QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.portal.Desktop"),
                                                              QStringLiteral("/org/freedesktop/portal/desktop"),
                                                              QStringLiteral("org.freedesktop.portal.Print"),
                                                              QStringLiteral("Print"));

        message << QStringLiteral("x11:") << QStringLiteral("Print dialog") << QVariant::fromValue<QDBusUnixFileDescriptor>(descriptor) << QVariantMap{{QStringLiteral("handle_token"), getRequestToken()}, {QStringLiteral("token"), results.value(QStringLiteral("token")).toUInt()}};

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
                                                    QStringLiteral("org.freedesktop.portal.Request"),
                                                    QStringLiteral("Response"),
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
        if (results.contains(QStringLiteral("uri"))) {
            Q_EMIT screenshotSaved(results.value(QStringLiteral("uri")).toString());
        }
    } else {
        qWarning() << "Failed to take screenshot";
    }
}

QString FlatpakDemo::getRequestToken()
{
    m_requestTokenCounter += 1;
    return QStringLiteral("u%1").arg(m_requestTokenCounter);
}
