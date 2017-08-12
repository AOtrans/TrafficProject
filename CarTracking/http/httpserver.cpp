#include "httpserver.h"
#include "httprequest.h"
#include "httpresponse.h"
#include "httpconnection.h"
#include "iostream"
#include <QTcpServer>
#include "funcs.h"

QHash<int, QString> STATUS_CODES;

HttpServer::HttpServer(QObject *parent)
    : QObject(parent)
    , m_tcpServer(0)
{
#define STATUS_CODE(num, reason) STATUS_CODES.insert(num, reason);
    // {{{
    STATUS_CODE(100, "Continue")
            STATUS_CODE(101, "Switching Protocols")
            STATUS_CODE(102, "Processing")                 // RFC 2518) obsoleted by RFC 4918
            STATUS_CODE(200, "OK")
            STATUS_CODE(201, "Created")
            STATUS_CODE(202, "Accepted")
            STATUS_CODE(203, "Non-Authoritative Information")
            STATUS_CODE(204, "No Content")
            STATUS_CODE(205, "Reset Content")
            STATUS_CODE(206, "Partial Content")
            STATUS_CODE(207, "Multi-Status")               // RFC 4918
            STATUS_CODE(300, "Multiple Choices")
            STATUS_CODE(301, "Moved Permanently")
            STATUS_CODE(302, "Moved Temporarily")
            STATUS_CODE(303, "See Other")
            STATUS_CODE(304, "Not Modified")
            STATUS_CODE(305, "Use Proxy")
            STATUS_CODE(307, "Temporary Redirect")
            STATUS_CODE(400, "Bad Request")
            STATUS_CODE(401, "Unauthorized")
            STATUS_CODE(402, "Payment Required")
            STATUS_CODE(403, "Forbidden")
            STATUS_CODE(404, "Not Found")
            STATUS_CODE(405, "Method Not Allowed")
            STATUS_CODE(406, "Not Acceptable")
            STATUS_CODE(407, "Proxy Authentication Required")
            STATUS_CODE(408, "Request Time-out")
            STATUS_CODE(409, "Conflict")
            STATUS_CODE(410, "Gone")
            STATUS_CODE(411, "Length Required")
            STATUS_CODE(412, "Precondition Failed")
            STATUS_CODE(413, "Request Entity Too Large")
            STATUS_CODE(414, "Request-URI Too Large")
            STATUS_CODE(415, "Unsupported Media Type")
            STATUS_CODE(416, "Requested Range Not Satisfiable")
            STATUS_CODE(417, "Expectation Failed")
            STATUS_CODE(418, "I\"m a teapot")              // RFC 2324
            STATUS_CODE(422, "Unprocessable Entity")       // RFC 4918
            STATUS_CODE(423, "Locked")                     // RFC 4918
            STATUS_CODE(424, "Failed Dependency")          // RFC 4918
            STATUS_CODE(425, "Unordered Collection")       // RFC 4918
            STATUS_CODE(426, "Upgrade Required")           // RFC 2817
            STATUS_CODE(500, "Internal Server Error")
            STATUS_CODE(501, "Not Implemented")
            STATUS_CODE(502, "Bad Gateway")
            STATUS_CODE(503, "Service Unavailable")
            STATUS_CODE(504, "Gateway Time-out")
            STATUS_CODE(505, "HTTP Version not supported")
            STATUS_CODE(506, "Variant Also Negotiates")    // RFC 2295
            STATUS_CODE(507, "Insufficient Storage")       // RFC 4918
            STATUS_CODE(509, "Bandwidth Limit Exceeded")
            STATUS_CODE(510, "Not Extended")                // RFC 2774
            // }}}
}

HttpServer::~HttpServer()
{
}

void HttpServer::newConnection()
{
    Q_ASSERT(m_tcpServer);
    while(m_tcpServer->hasPendingConnections()) {
        HttpConnection *connection = new HttpConnection(m_tcpServer->nextPendingConnection(), this);
        // connect(connection, SIGNAL(newRequest(HttpRequest*, HttpResponse*)),
        //   this, SIGNAL(newRequest(HttpRequest*, HttpResponse*)));
        connect(connection, SIGNAL(newRequest(HttpRequest*, HttpResponse*)),
                this, SLOT(onRequest(HttpRequest*, HttpResponse*)));
    }
}

bool HttpServer::listen(const QHostAddress &address, quint16 port)
{
    m_tcpServer = new QTcpServer;

    connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
    return m_tcpServer->listen(address, port);
}

bool HttpServer::listen(quint16 port)
{
    return listen(QHostAddress::Any, port);
}


void HttpServer::onRequest(HttpRequest* req, HttpResponse* resp)
{
    qDebug() << "---http in---";

    QString path = req->path();
    qDebug() << "path:" << path;

    string result;
    QStringList list = path.mid(1).split("/");

    if(list.size() == 2)
    {

        QStringList paras = list.at(0).split("_");
        QString videoPath = list.at(1);
        videoPath = videoPath.replace("+","/").replace("I:","/home/zg/1T");
        QString tag = paras.at(0);

        if(tag == "car")
        {
            carTrack(videoPath, paras.at(1), paras.at(2), paras.at(3));
        }
        else if(tag == "moto")
        {
            motoTrack(videoPath, paras.at(1), paras.at(2));
        }
        else if(tag == "truck")
        {
            truckDetect(videoPath, paras.at(1), paras.at(2));
        }
        else if(tag == "taxi")
        {
            taxiDetect(videoPath, paras.at(1), paras.at(2));
        }
        else if(tag == "areacar")
        {
            areaCarDetect(videoPath, paras.at(1), paras.at(2), paras.at(3));
        }
        else if(tag == "shotcut")
        {
            saveShotcut(videoPath, paras.at(1));
        }
        else if(tag == "traffic")
        {
            trafficStatistics(videoPath, paras.at(1), paras.at(2));
        }
        else
        {
            result = "bad tag";
        }

        result = "success";
    }
    else
    {
        result = "bad parameter";
    }

    qDebug() << "result:" << result.c_str();
    QString reply  = tr(("<html><head><title>Rokh Server Test</title></head><body><h1>"
                         + result
                         + "</h1></body></html>").c_str()).toLatin1();
    resp->setHeader("Content-Type", "text/html");
    resp->setStatus(200);
    resp->end(reply);

    return;
}
