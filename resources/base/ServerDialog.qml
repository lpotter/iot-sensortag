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
import QtQuick 2.0
import SensorTag.DataProvider 1.0
import Style 1.0
import QtQuick.Controls 2.3

Dialog {
    id: serverDialog
    width: 400
    height: 400
    standardButtons: Dialog.Ok | Dialog.Cancel
    Image {
        source: "images/bg_blue.jpg"
        anchors.fill: parent

        Text {
            id: text1
            x: 83
            y: 88
            width: 116
            height: 17
            color: "#ffffff"
            text: qsTr("Server address:")
            font.pixelSize: 12
        }

        Text {
            id: text2
            x: 83
            y: 131
            width: 69
            height: 17
            color: "#ffffff"
            text: qsTr("Server port:")
            font.pixelSize: 12
        }

        Text {
            id: text3
            x: 83
            y: 223
            width: 101
            height: 17
            color: "#ffffff"
            text: qsTr("Username")
            font.pixelSize: 12
        }

        Text {
            id: text4
            x: 83
            y: 274
            width: 95
            height: 17
            color: "#f9f9f9"
            text: qsTr("Password:")
            font.pixelSize: 12
        }
    }

    TextInput {
        id: serverNameTextInput
        x: 256
        y: 87
        width: 80
        height: 20
        color: "#ffffff"
        text: "10.0.0.65"
        font.pixelSize: 12
    }

    TextInput {
        id: serverPortInput
        x: 256
        y: 129
        width: 80
        height: 20
        color: "#ffffff"
        text: qsTr("8000")
        passwordCharacter: qsTr("●")
        font.pixelSize: 12
    }

    TextInput {
        id: passwordInput
        x: 256
        y: 267
        width: 80
        height: 20
        color: "#ffffff"
        text: qsTr("")
        echoMode: TextInput.PasswordEchoOnEdit
        inputMask: qsTr("")
        passwordCharacter: qsTr("●")
        font.pixelSize: 12
    }

    TextInput {
        id: usernameInput
        x: 256
        y: 220
        width: 80
        height: 20
        color: "#ffffff"
        text: ""
        font.pixelSize: 12

    }
        onAccepted: {
            console.log(serverNameTextInput.text + ":" + serverPortInput)
            mainWindow.remoteProviderPool.serverName = serverNameTextInput.text + ":" + serverPortInput.text
            mainWindow.remoteProviderPool.serverUserName = usernameInput.text
            mainWindow.remoteProviderPool.serverPassword = passwordInput.text
            remoteProviderPool.startScanning()
        }
        onRejected: {
            close()
        }

    //    Column {
    //        id: column
    //        x: 35
    //        y: 8
    //        width: 357
    //        height: 384
    //    }
}
