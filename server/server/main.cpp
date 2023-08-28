#include <vector>
#include "json.hpp"
#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QXmlStreamReader>
#include <filesystem>
#include <QtSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>

void insertIntoDbFromXml(std::string fileName){
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("database.db");

    if(db.open()) {
        QSqlQuery query;

        QFile file(const_cast<char*>(("files_xml/" + fileName).c_str()));

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug() << "Error opening file";
            return;
        }

        QXmlStreamReader xml(&file);

        while (!xml.atEnd() && !xml.hasError())
        {
            QXmlStreamReader::TokenType block = xml.readNext();
            if (block == QXmlStreamReader::StartElement){
                if (xml.name().toString() == "block")
                {
                    QXmlStreamAttributes attributes = xml.attributes();
                    QString block_id = xml.attributes().value("id").toString();
                    QString block_info = "block: ";
                    for (int i = 0; i < attributes.size(); i++) {
                        block_info += attributes[i].name().toString() + "=" + attributes[i].value().toString() + " ";
                    }
                    QString insertBlocks = "INSERT INTO blocks (block_id, block_info) VALUES ('" + block_id + "', '" + block_info + "');";
                    if (query.exec(insertBlocks)) qDebug() << "blocks insert fine";
                    else qDebug() << "blocks insert not fine";
                    while(!xml.atEnd() && !xml.hasError()){
                        QXmlStreamReader::TokenType board = xml.readNext();
                        if (board == QXmlStreamReader::StartElement){
                            if (xml.name().toString() == "board")
                            {
                                QXmlStreamAttributes attributes = xml.attributes();
                                QString board_id = xml.attributes().value("id").toString();
                                QString board_info = "board: ";
                                for (int i = 0; i < attributes.size(); i++) {
                                    board_info += attributes[i].name().toString() + "=" + attributes[i].value().toString() + " ";
                                }
                                QString insertBoards = "INSERT INTO boards (block_id, board_id, board_info) VALUES ('" + block_id + "', '" + board_id + "', '" + board_info + "');";
                                if (query.exec(insertBoards)) qDebug() << "boards insert fine";
                                else qDebug() << "boards insert not fine";
                                while(!xml.atEnd() && !xml.hasError()){
                                    QXmlStreamReader::TokenType port = xml.readNext();
                                    if (port == QXmlStreamReader::StartElement){
                                        if (xml.name().toString() == "port")
                                        {
                                            QXmlStreamAttributes attributes = xml.attributes();
                                            QString port_info = "port: ";
                                            for (int i = 0; i < attributes.size(); i++) {
                                                port_info += attributes[i].name().toString() + "=" + attributes[i].value().toString() + " ";
                                            }
                                            QString insertPorts = "INSERT INTO ports (board_id, port_info) VALUES ('" + board_id + "', '" + port_info + "');";
                                            if (query.exec(insertPorts)) qDebug() << "ports insert fine";
                                            else qDebug() << "ports insert not fine";
                                        }
                                    }
                                    else if (port == QXmlStreamReader::EndElement && xml.name().toString() == "board"){
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (xml.hasError())
        {
            qDebug() << "XML Error:" << xml.errorString();
        }

        file.close();

        db.close();
    } else qDebug() << "db not open";
}

void insertInfoFromXml(){
    std::string directoryPath = "files_xml";

    for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
        if (entry.is_regular_file()) {
            insertIntoDbFromXml(entry.path().filename().string());
        }
    }
}

void createTablesDb(){
    QString queryCreateBlocks = "CREATE TABLE blocks (block_id TEXT, block_info TEXT);";
    QString queryCreateBoards = "CREATE TABLE boards (block_id TEXT, board_id TEXT, board_info TEXT);";
    QString queryCreatePorts = "CREATE TABLE ports (board_id TEXT, port_info TEXT);";

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("database.db");

    if (db.open()) {
        QSqlQuery query;
        if (query.exec(queryCreateBlocks)) qDebug() << "create blocks fine";
        else qDebug() << "create blocks not fine";

        if (query.exec(queryCreateBoards)) qDebug() << "create boards fine";
        else qDebug() << "create boards not fine";

        if (query.exec(queryCreatePorts)) qDebug() << "create ports fine";
        else qDebug() << "create ports not fine";

        db.close();
    } else qDebug() << "db not open";
}

void deleteAllFromDb(){
    QString queryDeleteBlocks = "DELETE FROM blocks;";
    QString queryDeleteBoards = "DELETE FROM boards;";
    QString queryDeletePorts = "DELETE FROM ports;";

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("database.db");

    if (db.open()) {
        QSqlQuery query;
        if (query.exec(queryDeleteBlocks)) qDebug() << "delete blocks fine";
        else qDebug() << "delete blocks not fine";

        if (query.exec(queryDeleteBoards)) qDebug() << "delete boards fine";
        else qDebug() << "delete boards not fine";

        if (query.exec(queryDeletePorts)) qDebug() << "delete ports fine";
        else qDebug() << "delete ports not fine";

        db.close();
    } else qDebug() << "db not open";
}

nlohmann::json getInfoFromDb(){
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("database.db");
    if(db.open()){
        QSqlQuery query;
        nlohmann::json returnData;
        QString queryStr = "SELECT * FROM blocks";
        std::vector<std::string> blocks;
        if(query.exec(queryStr)){
            while (query.next()){
                QString block_id = query.value(0).toString();
                QString block_info = query.value(1).toString();
                blocks.push_back(block_id.toStdString());
                blocks.push_back(block_info.toStdString());
                returnData["blocks"].push_back((nlohmann::json)blocks);
                blocks.clear();
            }
        }
        QString queryStr1 = "SELECT * FROM boards";
        std::vector<std::string> boards;
        if(query.exec(queryStr1)){
            while (query.next()){
                QString block_id = query.value(0).toString();
                QString board_id = query.value(1).toString();
                QString board_info = query.value(2).toString();
                boards.push_back(block_id.toStdString());
                boards.push_back(board_id.toStdString());
                boards.push_back(board_info.toStdString());
                returnData["boards"].push_back((nlohmann::json)boards);
                boards.clear();
            }
        }
        QString queryStr2 = "SELECT * FROM ports";
        std::vector<std::string> ports;
        if(query.exec(queryStr2)){
            while (query.next()){
                QString board_id = query.value(0).toString();
                QString port_info = query.value(1).toString();
                ports.push_back(board_id.toStdString());
                ports.push_back(port_info.toStdString());
                returnData["ports"].push_back((nlohmann::json)ports);
                ports.clear();
            }
        }
        db.close();

        return returnData;
    }
    else qDebug() << "db not open";
    return (nlohmann::json)"error";
}

// void showInfoFromDb(){
//     nlohmann::json dbData = getInfoFromDb();
//     for (nlohmann::json block : dbData["blocks"]){
//         qDebug() << (std::string)block[1];
//         for (nlohmann::json board : dbData["boards"]){
//             if (block[0] == board[0]){
//                 qDebug() << "\t" << (std::string)board[2];
//                 for (nlohmann::json port : dbData["ports"]){
//                     if (board[1] == port[0]){
//                         qDebug() << "\t\t" << (std::string)port[1];
//                     }
//                 }
//             }
//         }
//     }
// }

int main(int argc, char *argv[]){
    QCoreApplication app(argc, argv);

    deleteAllFromDb();
    insertInfoFromXml();

    QTcpServer server;
    if (!server.listen(QHostAddress::Any, 1234)) {
        qWarning() << "Failed to start the server:" << server.errorString();
        return 1;
    }

    qDebug() << "Server started, listening on" << server.serverAddress().toString() << server.serverPort();

    std::string dbInfoString = getInfoFromDb().dump();
    QString dbInfo = QString::fromStdString(dbInfoString);
    QObject::connect(&server, &QTcpServer::newConnection, [&]() {
        QTcpSocket *socket = server.nextPendingConnection();

        qDebug() << "client connected:" << socket->peerAddress().toString() << socket->peerPort();

        // Отправка данных клиенту
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_15);
        out << dbInfo;

        socket->write(block);
        socket->flush();
        socket->waitForBytesWritten();

        socket->disconnectFromHost();
    });

    return app.exec();
}
