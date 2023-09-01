#include "mainwindow.h"

#include <QtWidgets>
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
    MainWindow window;

    QTcpSocket socket;
    socket.connectToHost("localhost", 44444);

    if (!socket.waitForConnected()) {
        QLabel* label = new QLabel("Can't connect to the server.\nRetry later!", &window);
        label->setAlignment(Qt::AlignCenter);
        window.setCentralWidget(label);
        window.show();
    }
    else{
        QByteArray block;
        QDataStream in(&block, QIODevice::ReadOnly);
        in.setVersion(QDataStream::Qt_5_15);

        socket.waitForReadyRead();
        block = socket.readAll();
        QString infoFromServer;
        in >> infoFromServer;

        socket.disconnectFromHost();

        nlohmann::json data = nlohmann::json::parse(infoFromServer.toStdString());

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

        QLabel* label = new QLabel("Successfully connected to the server.", &window);
        label->setAlignment(Qt::AlignCenter);

        QWidget* centralWidget = new QWidget(&window);
        QVBoxLayout* layout = new QVBoxLayout(centralWidget);
        layout->addWidget(treeWidget);
        layout->addWidget(label);
        window.setCentralWidget(centralWidget);
        window.show();
    }

    return app.exec();
}
