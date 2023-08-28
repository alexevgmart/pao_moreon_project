#include "mainwindow.h"

#include <QApplication>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QtNetwork>
#include <QTcpSocket>
#include "json.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTcpSocket socket;
    socket.connectToHost("localhost", 44444);

    if (!socket.waitForConnected()) {
        qWarning() << "Failed to connect to the server:" << socket.errorString();
        return 1;
    }

    QByteArray block;
    QDataStream in(&block, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_5_15);

    socket.waitForReadyRead();
    block = socket.readAll();
    QString infoFromServer;
    in >> infoFromServer;

    socket.disconnectFromHost();

    nlohmann::json data = nlohmann::json::parse(infoFromServer.toStdString());

    MainWindow window;

    QTreeWidget* treeWidget = new QTreeWidget();
    window.setWindowTitle("Client");
    treeWidget->setColumnCount(1);
    treeWidget->setHeaderLabels({"Info from database"});

    for (nlohmann::json block : data["blocks"]){
        QTreeWidgetItem* blockItem = new QTreeWidgetItem(treeWidget);
        blockItem->setText(0, QString::fromStdString((std::string)block[1]));
        for (nlohmann::json board : data["boards"]){
            if (block[0] == board[0]){
                QTreeWidgetItem* boardItem = new QTreeWidgetItem(blockItem);
                boardItem->setText(0, QString::fromStdString((std::string)board[2]));
                for (nlohmann::json port : data["ports"]){
                    if (board[1] == port[0]){
                        QTreeWidgetItem* portItem = new QTreeWidgetItem(boardItem);
                        portItem->setText(0, QString::fromStdString((std::string)port[1]));
                        treeWidget->addTopLevelItem(blockItem);
                    }
                }
            }
        }
    }

    window.setCentralWidget(treeWidget);
    window.show();
    return app.exec();
}
