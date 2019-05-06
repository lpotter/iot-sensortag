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

#include "sensortagdataproviderpool.h"
#ifndef Q_OS_WASM
#include "bluetoothdataprovider.h"
#endif

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(boot2QtDemos)

SensorTagDataProviderPool::SensorTagDataProviderPool(QObject *parent)
    : SensorTagDataProviderPool("SensorTag", parent)
{
}

SensorTagDataProviderPool::SensorTagDataProviderPool(QString poolName, QObject *parent)
    : DataProviderPool(poolName, parent)
{
#ifndef Q_OS_WASM
    m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent();
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &SensorTagDataProviderPool::btDeviceFound);
    connect(m_discoveryAgent, QOverload<QBluetoothDeviceDiscoveryAgent::Error>::of(&QBluetoothDeviceDiscoveryAgent::error),
            this, &SensorTagDataProviderPool::deviceScanError);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &SensorTagDataProviderPool::deviceDiscoveryFinished);
#endif
}

void SensorTagDataProviderPool::startScanning()
{
#ifndef Q_OS_WASM
    m_discoveryAgent->setLowEnergyDiscoveryTimeout(0);
    m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

    if (m_discoveryAgent->isActive()) {
        emit scanStarted();
    }
#endif
}

void SensorTagDataProviderPool::disconnectProvider(const QString &id)
{
    SensorTagDataProvider *p = findProvider(id);
#ifndef Q_OS_WASM
    if (BluetoothDataProvider *btp = qobject_cast<BluetoothDataProvider*>(findProvider(id)))
        btp->unbindDevice();
    else
#endif
        if (p)
        p->setState(SensorTagDataProvider::Disconnected);
}

SensorTagDataProvider *SensorTagDataProviderPool::providerForCloud() const
{
    return m_providerForCloud;
}

// Currently, return simply the first provider found.
// It might be better to somehow collect the results from
// all providers and combine them
void SensorTagDataProviderPool::updateProviderForCloud()
{
    if (m_dataProviders.length())
        m_providerForCloud = m_dataProviders.at(0);
    else
        m_providerForCloud = 0;
    emit providerForCloudChanged();
}

void SensorTagDataProviderPool::deviceDiscoveryFinished()
{
    finishScanning();
    emit scanFinished();
}

void SensorTagDataProviderPool::finishScanning()
{
    updateProviderForCloud();
}

#ifndef Q_OS_WASM
void SensorTagDataProviderPool::btDeviceFound(const QBluetoothDeviceInfo &info)
{
    if (info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        // all tags by TI have this naming convention
        if (info.name().contains(QLatin1String("SensorTag")) && !m_knownDevices.contains(info)) {
            qCDebug(boot2QtDemos) << "Found a new device:" << info.name();
            m_knownDevices.append(info);
            BluetoothDataProvider *p = new BluetoothDataProvider(info, this);
            m_dataProviders.push_back(p);
            connect(p, &SensorTagDataProvider::stateChanged,
                    this, &SensorTagDataProviderPool::handleStateChange);

            emit dataProvidersChanged();

            // Set initial state to Scanning for UI to be
            // able to show "Connecting.." information
            p->setState(SensorTagDataProvider::Scanning);
        }
    }
}
#endif

void SensorTagDataProviderPool::handleStateChange()
{
    SensorTagDataProvider *provider = static_cast<SensorTagDataProvider *>(sender());

    switch (provider->state()) {
    case SensorTagDataProvider::Disconnected:
        updateProviderForCloud();
        emit providerDisconnected(provider->id());
        break;
    case SensorTagDataProvider::Connected:
        updateProviderForCloud();
        emit providerConnected(provider->id());
        break;
    case SensorTagDataProvider::Error:
        emit providerInError(provider->id());
        break;
    default:
        break;
    }
}

#ifndef Q_OS_WASM
void SensorTagDataProviderPool::deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError)
        qCDebug(boot2QtDemos) << "The Bluetooth adaptor is powered off, power it on before doing discovery.";
    else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError)
        qCDebug(boot2QtDemos) << "Writing or reading from the device resulted in an error.";
    else
        qCDebug(boot2QtDemos) << "An unknown error has occurred.";

    emit scanFinished();
}
#endif

SensorTagDataProvider *SensorTagDataProviderPool::findProvider(QString id) const
{
    for (SensorTagDataProvider *p : m_dataProviders) {
        if (id == p->id())
            return p;
    }
    return 0;
}
