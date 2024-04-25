import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.3

Window {
    id: mainWindow
    visible: true
    width: 400
    height: 600
    minimumWidth: 400
    minimumHeight: 600
    title: qsTr(".bmp handler")


    ColumnLayout{
        anchors.fill: parent
        ListView{
            id : listView
            Layout.fillWidth: true
            Layout.fillHeight: true

            model : fileModel
            delegate: Rectangle {
                height: 90
                radius: 10
                color : "gray"
                border.color: "cyan"
                width: parent.width
                RowLayout{
                    anchors.fill: parent
                    anchors.margins: 20

                    Text{
                        clip : true
                        text : filename
                        Layout.fillWidth: true
                    }

                    Text{
                        clip : true
                        text : status
                    }

                    Text{
                        clip : true
                        text : size
                    }
                }

                MouseArea{
                    anchors.fill: parent
                    onClicked: imageHandler.onClickFile(index)
                }
            }
        }
    }

    Popup {
        id: errorDialog
        property string errorText: ""
        width: 300
        height: 100
        anchors.centerIn: parent
        modal: true
        closePolicy: Popup.NoAutoClose

        Column {
            id: column
            spacing: 20
            anchors.fill: parent

            Label {
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
                text: errorDialog.errorText
                width: parent.width
                wrapMode: Label.Wrap
            }
            Button{
                width:70
                anchors.horizontalCenter: parent.horizontalCenter
                id: okBtn

                property color buttonColor: "white"

                text: "Ok"
                background: Rectangle{
                    anchors.fill: parent
                    color: okBtn.buttonColor
                }

                onPressed: {
                    okBtn.buttonColor = "red"
                }
                onCanceled: {
                    okBtn.buttonColor = "white"
                }
                onReleased: {
                    okBtn.buttonColor = "white"
                    errorDialog.close()
                }
            }
        }
    }

    Connections{
        target: imageHandler
        onError : {
            errorDialog.errorText = error
            errorDialog.open()
        }
    }

}
