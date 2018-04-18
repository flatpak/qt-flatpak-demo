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
#include <QPainter>
#include <QPdfWriter>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryFile>

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
    notify->setTitle(QLatin1String("Flatpak demo"));
    notify->setText(QLatin1String("This notification was sent from a Flatpak sandbox."));
    notify->setIconName(QLatin1String("flatpak"));

    notify->sendEvent();
}

void FlatpakDemo::printFile(const QUrl &file)
{
    m_fileToPrint = file.toLocalFile();

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

void FlatpakDemo::gotPrintResponse(uint response, const QVariantMap &results)
{
    // qWarning() << "Print response: " << response << results;
    if (response) {
        qWarning() << "Failed to print " << m_fileToPrint;
    }
}

void FlatpakDemo::gotPreparePrintResponse(uint response, const QVariantMap &results)
{
    if (!response) {
        QVariantMap settings;
        QVariantMap pageSetup;

        QDBusArgument dbusArgument = results.value(QLatin1String("settings")).value<QDBusArgument>();
        dbusArgument >> settings;

        QDBusArgument dbusArgument1 = results.value(QLatin1String("page-setup")).value<QDBusArgument>();
        dbusArgument1 >> pageSetup;

        QTemporaryFile tempFile;
        tempFile.setAutoRemove(false);
        if (!tempFile.open()) {
            qWarning() << "Couldn't generate pdf file";
            return;
        }

        QPdfWriter writer(tempFile.fileName());
        writer.setResolution(150);

        QPainter painter(&writer);

        if (pageSetup.contains(QLatin1String("Orientation"))) {
            const QString orientation = pageSetup.value(QLatin1String("Orientation")).toString();
            if (orientation == QLatin1String("portrait") || orientation == QLatin1String("revers-portrait")) {
                writer.setPageOrientation(QPageLayout::Portrait);
            } else if (orientation == QLatin1String("landscape") || orientation == QLatin1String("reverse-landscape")) {
                writer.setPageOrientation(QPageLayout::Landscape);
            }
        }

        if (pageSetup.contains(QLatin1String("MarginTop")) &&
            pageSetup.contains(QLatin1String("MarginBottom")) &&
            pageSetup.contains(QLatin1String("MarginLeft")) &&
            pageSetup.contains(QLatin1String("MarginRight"))) {
            const int marginTop = pageSetup.value(QLatin1String("MarginTop")).toInt();
            const int marginBottom = pageSetup.value(QLatin1String("MarginBottom")).toInt();
            const int marginLeft = pageSetup.value(QLatin1String("MarginLeft")).toInt();
            const int marginRight = pageSetup.value(QLatin1String("MarginRight")).toInt();
            writer.setPageMargins(QMarginsF(marginLeft, marginTop, marginRight, marginBottom), QPageLayout::Millimeter);
        }

        // TODO num-copies, pages

        writer.setPageSize(QPagedPaintDevice::A4);

        QPixmap pixmap(m_fileToPrint);
        painter.setViewport(0, 0, pixmap.width(), pixmap.height());
        painter.setWindow(pixmap.rect());
        painter.drawPixmap(QPoint(0,0), pixmap);
        painter.end();

        // Send it back for printing
        QDBusUnixFileDescriptor descriptor(tempFile.handle());

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
