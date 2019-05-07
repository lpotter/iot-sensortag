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
import QtQuick 2.5
import QtCharts 2.1
import SensorTag.DataProvider 1.0

BaseChart {
    property int humiditySeriesIndex: 0
    property int maxNumOfHumiReadings: 30

    antialiasing: true
    title: qsTr("Humidity")

    content: Item {
        anchors.fill: parent

        property real humidityValue: sensor ? sensor.relativeHumidity : 0
        property real maxHumi: 0
        property real minHumi: 100

        onHumidityValueChanged: {
            if (humidityValue > maxHumi)
                maxHumi = humidityValue

            if (humidityValue < minHumi)
                minHumi = humidityValue
        }

        Image {
            id: humidityMainImg

            source: pathPrefix + "Humidity/Hum_combined_all.png"
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.top: parent.top
            anchors.topMargin: -8

            Text {
                id: humidityMainText

                anchors.centerIn: parent
                anchors.horizontalCenterOffset: - 42
                color: "white"
                text: humidityValue.toFixed(0)
                font.pixelSize: 26
            }
        }


        Text {
            anchors.left: humidityMainImg.right
            anchors.top: humidityMainImg.top
            anchors.topMargin: 27
            text: "Max\n" + maxHumi.toFixed(0) + " %"
            lineHeight: 0.8
            color: "white"
            font.pixelSize: 26
        }

        Text {
            anchors.left: humidityMainImg.right
            anchors.bottom: humidityMainImg.bottom
            anchors.bottomMargin: 33
            text: "Min\n" + minHumi.toFixed(0) + " %"
            lineHeight: 0.8
            color: "white"
            font.pixelSize: 26
        }
    }
}
