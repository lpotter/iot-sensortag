/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of Qt for Device Creation.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mqttdataproviderpool.h"
#include "mqttdataprovider.h"

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>

#include <QObject>
#include <QCoreApplication>

#ifdef Q_USE_WEBSOCKETS
#include "websocketiodevice.h"
#endif

MqttDataProviderPool::MqttDataProviderPool(QObject *parent)
    : DataProviderPool(parent)
    , m_client(new QMqttClient(this))
#ifdef Q_USE_WEBSOCKETS
    , m_device(new WebSocketIODevice(this))
    , m_version(4)
#endif
{

    m_poolName = "Mqtt";

//    mqttBroker = QString::fromLocal8Bit(MQTT_BROKER);
//    mqttPort = MQTT_PORT;

    connect(this,&DataProviderPool::serverNameChanged,this,&MqttDataProviderPool::serverChanged);
}

void MqttDataProviderPool::startScanning()
{
    if (mqttBroker.isEmpty())
        return;
#ifdef Q_USE_WEBSOCKETS

    QUrl url;
    url.setHost(mqttBroker);
    url.setPort(mqttPort);
    url.setScheme(QLatin1String("ws"));
    url.setPath(QLatin1String("/mqtt"));
    if (!mqttUsername.isEmpty())
            url.setUserName(mqttUsername);
    if (!mqttPassword.isEmpty())
            url.setPassword(mqttPassword);

    m_device->setUrl(url);
    m_device->setProtocol(m_version == 3 ? "mqttv3.1" : "mqtt");

    qDebug() << "Connecting to broker at " << url;

    connect(m_device, &WebSocketIODevice::socketConnected, [this]() {

        qDebug() << "<<<<<< WebSocket connected, initializing MQTT connection.";

        m_client->setProtocolVersion(m_version == 3 ? QMqttClient::MQTT_3_1 : QMqttClient::MQTT_3_1_1);
        m_client->setTransport(m_device, QMqttClient::IODevice);

        connect(m_client, &QMqttClient::connected, [this]() {

            qDebug() << "<<<<<< MQTT connection established";
            QMqttSubscription *sub = m_client->subscribe(QLatin1String("sensors/active"));
            connect(sub, &QMqttSubscription::messageReceived, this, &MqttDataProviderPool::deviceUpdate);
        });

        connect(m_client, &QMqttClient::disconnected, [this]() {
            qDebug() << "Pool client disconnected";
        });

        m_client->connectToHost();
    });
#else

    m_client->setHostname(MqttCredentials::getBroker());
    m_client->setPort((quint16)MqttCredentials::getPort());
    m_client->setUsername(MqttCredentials::getUsername());
    m_client->setPassword(MqttCredentials::getPassword());

    connect(m_client, &QMqttClient::connected, [this]() {
        QSharedPointer<QMqttSubscription> sub(m_client->subscribe(QLatin1String("sensors")));
        connect(sub.data(), &QMqttSubscription::messageReceived, this, &MqttDataProviderPool::deviceUpdate);
    });
    connect(m_client, &QMqttClient::disconnected, [this]() {
        qDebug() << "Pool client disconnected";
    });

    m_client->connectToHost();
#endif
#ifdef Q_USE_WEBSOCKETS
    if (!m_device->open(QIODevice::ReadWrite))
        qDebug() << "Could not open socket device";
#endif

    emit providerConnected("MQTT_CLOUD");
    emit providersUpdated();
    emit dataProvidersChanged();
    emit scanFinished();
}

void MqttDataProviderPool::deviceUpdate(const QMqttMessage &msg)
{
    static QSet<QString> knownDevices;

    bool updateRequired = false;
    // Registration is: deviceName>Online
    const QByteArrayList payload = msg.payload().split('>');

    const QString deviceName = payload.first();
    const QString deviceStatus = payload.at(1);
    const QString subName = QString::fromLocal8Bit("sensors/%1/#").arg(deviceName);

    if (deviceStatus == QLatin1String("Online")) { // new device
        // Skip local items
        if (deviceName.startsWith(QSysInfo::machineHostName()))
            return;

        if (!knownDevices.contains(deviceName)) {
            auto prov = new MqttDataProvider(deviceName, m_client, this);
            prov->setState(SensorTagDataProvider::Connected);
            m_dataProviders.push_back(prov);
            if (m_currentProvider == nullptr)
                setCurrentProviderIndex(m_dataProviders.size() - 1);
            knownDevices.insert(deviceName);
            updateRequired = true;
        }
    } else if (deviceStatus == QLatin1String("Offline")) { // device died
        qDebug() << Q_FUNC_INFO << "device died";
        knownDevices.remove(deviceName);
        updateRequired = true;
        for (auto prov : m_dataProviders) {
            if (prov->id() == deviceName) {
                m_dataProviders.removeAll(prov);
                break;
            }
        }
    }

    if (updateRequired) {
        emit providersUpdated();
        emit dataProvidersChanged();
    }
}

void MqttDataProviderPool::serverChanged(const QString &name)
{
    QStringList token = name.split(":");
    if (token.count() > 0)
        mqttBroker = token.at(0);
    if (token.count() > 1)
        mqttPort = token.at(1).toInt();
}

void MqttDataProviderPool::serverUserChanged(const QString &user)
{
    mqttUsername = user;
}

void MqttDataProviderPool::serverPasswordChanged(const QString &pass)
{
    mqttPassword = pass;
}

namespace {
    static QString gBroker;
    static int gPort{0};
    static QString gUser;
    static QString gPassword;
    bool resolveCredentials() {
        static bool resolved = false;
        if (resolved)
            return true;

        QFile f(QLatin1String("broker.json"));
        if (!f.open(QIODevice::ReadOnly)) {
            qDebug() << "Could not find or open broker.json";
            return false;
        }
        const QByteArray data = f.readAll();
        if (data.isEmpty()) {
            qDebug() << "broker.json file empty";
            return false;
        }
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull()) {
            qDebug() << "broker.json does not contain valid data";
            return false;
        }
        gBroker = doc["broker"].toString();
        gPort = doc["port"].toInt();
        gUser = doc["user"].toString();
        gPassword = doc["password"].toString();
        qDebug() << "broker.json parsed. Using broker:" << gBroker << ":" << gPort;
        resolved = true;
        return true;
    }
}

QString MqttCredentials::getBroker()
{
    resolveCredentials();
    return gBroker;
}

int MqttCredentials::getPort()
{
    resolveCredentials();
    return gPort;
}

QString MqttCredentials::getUsername()
{
    resolveCredentials();
    return gUser;
}

QString MqttCredentials::getPassword()
{
    resolveCredentials();
    return gPassword;
}
