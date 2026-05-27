#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QString>

class NetworkMessage
{
public:
    static QByteArray serialize(const QJsonObject& message);
    static QJsonObject deserialize(const QByteArray& data);

    static QJsonObject create(
        const QString& type,
        qint64 senderId,
        const QJsonObject& payload = {}
        );

    static bool isValid(const QJsonObject& message);
};