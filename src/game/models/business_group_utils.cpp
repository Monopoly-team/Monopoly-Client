#include "game/models/business_group_utils.hpp"

QColor BusinessGroupUtils::color(BusinessGroup group)
{
    switch (group)
    {
        case BusinessGroup::Pharmacy:     return QColor("#50C878");
        case BusinessGroup::Clothes:      return QColor("#FF8C42");
        case BusinessGroup::FoodMarket:   return QColor("#FFD166");
        case BusinessGroup::DarkStore:    return QColor("#7B61FF");
        case BusinessGroup::Marketplace:  return QColor("#4DDBFF");
        case BusinessGroup::Bank:         return QColor("#6EE7A8");
        case BusinessGroup::GameStudio:   return QColor("#FF6B6B");
        case BusinessGroup::IT:           return QColor("#6C7BFF");
        case BusinessGroup::ITEducation:  return QColor("#B56CFF");
        case BusinessGroup::None:         return QColor("#555555");
    }

    return QColor("#555555");
}
