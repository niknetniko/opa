/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppMemberFunctionMayBeConst
#include "../src/ai/openai_compatible_service.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTest>
#include <memory>

using namespace Qt::Literals::StringLiterals;

namespace {

// Minimal HTTP server that accepts one request and returns a canned response.
class FakeHttpServer : public QTcpServer {
    Q_OBJECT

public:
    explicit FakeHttpServer(QObject* parent = nullptr) : QTcpServer(parent) {
        listen(QHostAddress::LocalHost, 0);
        connect(this, &QTcpServer::newConnection, this, &FakeHttpServer::handleConnection);
    }

    void respond(int status, const QByteArray& body) {
        nextStatus = status;
        nextBody = body;
    }

    QJsonObject receivedJson() const {
        return QJsonDocument::fromJson(receivedBody).object();
    }

    QString url() const {
        return u"http://127.0.0.1:%1"_s.arg(serverPort());
    }

private Q_SLOTS:
    void handleConnection() {
        auto* socket = nextPendingConnection();
        auto buffer = std::make_shared<QByteArray>();
        connect(socket, &QTcpSocket::readyRead, this, [this, socket, buffer]() {
            *buffer += socket->readAll();
            const int sep = buffer->indexOf("\r\n\r\n");
            if (sep < 0) {
                return;
            }
            int contentLength = 0;
            for (const auto& line: buffer->left(sep).split('\n')) {
                if (line.toLower().startsWith("content-length:")) {
                    contentLength = line.mid(15).trimmed().toInt();
                }
            }
            if (buffer->size() < sep + 4 + contentLength) {
                return;
            }
            receivedBody = buffer->mid(sep + 4, contentLength);
            const QByteArray response = "HTTP/1.1 " + QByteArray::number(nextStatus) +
                                        " OK\r\n"
                                        "Content-Type: application/json\r\n"
                                        "Content-Length: " +
                                        QByteArray::number(nextBody.size()) +
                                        "\r\n"
                                        "Connection: close\r\n"
                                        "\r\n" +
                                        nextBody;
            socket->write(response);
            socket->flush();
            socket->disconnectFromHost();
        });
    }

private:
    int nextStatus = 200;
    QByteArray nextBody;
    QByteArray receivedBody;
};

} // namespace

class TestOpenAiCompatibleService : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testSuccessReturnsMessageContent() {
        FakeHttpServer server;
        server.respond(200, R"({"choices":[{"message":{"role":"assistant","content":"Hello!"}}]})");

        OpenAiCompatibleService service(server.url(), u"test-model"_s);
        auto future = service.ask(u"system"_s, u"user"_s);

        QTRY_VERIFY(future.isFinished());
        try {
            QCOMPARE(future.result(), u"Hello!"_s);
        } catch (const std::exception& e) {
            QFAIL(e.what());
        }
    }

    void testHttpErrorThrows() {
        FakeHttpServer server;
        server.respond(500, R"({"error":"internal server error"})");

        OpenAiCompatibleService service(server.url(), u"test-model"_s);
        auto future = service.ask(u"system"_s, u"user"_s);

        QTRY_VERIFY(future.isFinished());
        bool threw = false;
        try {
            future.result();
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void testSchemaIsIncludedInRequestBody() {
        FakeHttpServer server;
        server.respond(200, R"({"choices":[{"message":{"role":"assistant","content":"{}"}}]})");

        const QJsonObject schema{{u"type"_s, u"object"_s}};
        OpenAiCompatibleService service(server.url(), u"test-model"_s);
        auto future = service.ask(u"system"_s, u"user"_s, schema);

        QTRY_VERIFY(future.isFinished());
        try {
            future.result();
        } catch (const std::exception& e) {
            QFAIL(e.what());
        }
        const auto body = server.receivedJson();
        QVERIFY(body.contains(u"response_format"_s));
        QCOMPARE(body[u"response_format"_s][u"type"_s].toString(), u"json_schema"_s);
    }
};

QTEST_MAIN(TestOpenAiCompatibleService)
#include "openai_compatible_service_test.moc"
