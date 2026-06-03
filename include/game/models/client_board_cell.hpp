#pragma once

#include "game/models/business_group.hpp"
#include "game/models/cell_type.hpp"

#include <QString>
#include <QtTypes>

struct ClientBoardCell
{
    quint8 id = 0;

    CellType type = CellType::Business;
    BusinessGroup group = BusinessGroup::None;

    QString name;
    QString imagePath;

    qint32 price            = 0;
    qint32 rent             = 0;
    qint32 buildingCost     = 0;
    qint32 buildingLevel    = 0;
    qint32 maxBuildingLevel = 0;

    quint16 ownerId = 0;
};

