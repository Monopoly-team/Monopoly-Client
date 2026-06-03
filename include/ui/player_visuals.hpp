#pragma once

#include "network/client/tcp_client_controller.hpp"

#include <QColor>
#include <QString>

namespace PlayerVisuals
{

inline QColor normalPlayerColor(const QString& color)
{
    const QColor parsedColor(color);

    if (parsedColor.isValid())
        return parsedColor;

    return QColor("#FFFFFF");
}

inline QColor displayColor(const ClientGamePlayer& player, quint16 winnerId)
{
    if (winnerId != 0 && player.id == winnerId)
        return QColor("#FFD166"); // победитель

    if (player.isBankrupt)
        return QColor("#8A8A8A"); // банкрот

    if (!player.active)
        return QColor("#A45A5A"); // отключился

    return normalPlayerColor(player.color);
}

inline QString statusText(const ClientGamePlayer& player, quint16 winnerId)
{
    if (winnerId != 0 && player.id == winnerId)
        return QStringLiteral("Победитель");

    if (player.isBankrupt)
        return QStringLiteral("Банкрот");

    if (!player.active)
        return QStringLiteral("Отключился");

    return QString();
}

} // namespace PlayerVisuals