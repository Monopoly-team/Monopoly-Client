#include "network/network_message.hpp"

#include <QJsonDocument>

QByteArray NetworkMessage::serialize(const QJsonObject& message)
{
    QByteArray data = QJsonDocument(message).toJson(QJsonDocument::Compact);
    data.append('\n');

    return data;
}

QJsonObject NetworkMessage::deserialize(const QByteArray& data)
{
    QJsonParseError error;

    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError || !doc.isObject())
        return {};

    return doc.object();
}

QJsonObject NetworkMessage::create(
    const QString& type,
    qint64 senderId,
    const QJsonObject& payload
    )
{
    QJsonObject message;

    message["type"] = type;
    message["senderId"] = senderId;
    message["payload"] = payload;

    return message;
}

bool NetworkMessage::isValid(const QJsonObject& message)
{
    return message.contains("type")
    && message["type"].isString()
        && message.contains("senderId")
        && message["senderId"].isDouble()
        && message.contains("payload")
        && message["payload"].isObject();
}