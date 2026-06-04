#include "game/game_state.hpp"

#include <QJsonArray>
#include <QtGlobal>

namespace {

QString cellTypeToString(CellType type)
{
    switch (type) {
    case CellType::Corner:
        return "corner";
    case CellType::Business:
        return "business";
    case CellType::ExtraBusiness:
        return "extra_business";
    case CellType::Chance:
        return "chance";
    case CellType::CommunityChest:
        return "community_chest";
    }

    return "business";
}

CellType cellTypeFromString(const QString& value)
{
    const QString normalized = value.trimmed().toLower();

    if (normalized == "business") {
        return CellType::Business;
    }

    if (normalized == "extra_business" || normalized == "extra_businesses" || normalized == "extrabusiness") {
        return CellType::ExtraBusiness;
    }

    if (normalized == "chance") {
        return CellType::Chance;
    }

    if (normalized == "community_chest" || normalized == "communitychest" || normalized == "chest") {
        return CellType::CommunityChest;
    }

    // Backward compatibility with older corner-specific names.
    if (normalized == "corner" || normalized == "start" || normalized == "jail"
        || normalized == "free_parking" || normalized == "go_to_jail") {
        return CellType::Corner;
    }

    return CellType::Business;
}

QString businessGroupToString(BusinessGroup group)
{
    switch (group) {
    case BusinessGroup::None:
        return "none";
    case BusinessGroup::Pharmacy:
        return "pharmacy";
    case BusinessGroup::Clothes:
        return "clothes";
    case BusinessGroup::FoodMarket:
        return "foodmarket";
    case BusinessGroup::DarkStore:
        return "darkstore";
    case BusinessGroup::Marketplace:
        return "marketplace";
    case BusinessGroup::Bank:
        return "bank";
    case BusinessGroup::GameStudio:
        return "gamestudio";
    case BusinessGroup::IT:
        return "it";
    case BusinessGroup::ITEducation:
        return "iteducation";
    }

    return "none";
}

BusinessGroup businessGroupFromString(const QString& value)
{
    QString normalized = value.trimmed().toLower();
    normalized.replace("_", "");

    if (normalized == "pharmacy") {
        return BusinessGroup::Pharmacy;
    }

    if (normalized == "clothes" || normalized == "clothers") {
        return BusinessGroup::Clothes;
    }

    if (normalized == "foodmarket") {
        return BusinessGroup::FoodMarket;
    }

    if (normalized == "darkstore") {
        return BusinessGroup::DarkStore;
    }

    if (normalized == "marketplace") {
        return BusinessGroup::Marketplace;
    }

    if (normalized == "bank") {
        return BusinessGroup::Bank;
    }

    if (normalized == "gamestudio") {
        return BusinessGroup::GameStudio;
    }

    if (normalized == "it") {
        return BusinessGroup::IT;
    }

    if (normalized == "iteducation") {
        return BusinessGroup::ITEducation;
    }

    return BusinessGroup::None;
}

QString defaultPlayerColor(int index)
{
    static const QVector<QString> colors = {
        QStringLiteral("#E53935"),
        QStringLiteral("#1E88E5"),
        QStringLiteral("#43A047"),
        QStringLiteral("#42F5F2"),
        QStringLiteral("#8E24AA"),
        QStringLiteral("#FB8C00")
    };

    if (colors.isEmpty()) {
        return QStringLiteral("#E53935");
    }

    return colors[index % colors.size()];
}

int clampBuildingLevel(int value, int maxBuildingLevel)
{
    if (value < 0) {
        return 0;
    }

    if (value > maxBuildingLevel) {
        return maxBuildingLevel;
    }

    return value;
}
int scaledValue(int value, int numerator, int denominator)
{
    if (denominator == 0) {
        return value;
    }

    return (value * numerator + denominator / 2) / denominator;
}

int baseRentForPrice(int price)
{
    return qMax(1, scaledValue(price, 1, 10));
}

QVector<int> rentLevelsForPrice(int price)
{
    const int baseRent = baseRentForPrice(price);

    return {
        baseRent,                         // 0 домов: 10% от цены
        scaledValue(baseRent, 3, 2),      // 1 дом: x1.5
        baseRent * 2,                     // 2 дома: x2
        baseRent * 3,                     // 3 дома: x3
        baseRent * 5,                     // 4 дома: x5
        baseRent * 10                     // отель: x10
    };
}

Cell createSpecialCell(int id, const QString& name, CellType type)
{
    Cell cell;

    cell.id = id;
    cell.name = name;
    cell.type = type;
    cell.group = BusinessGroup::None;
    cell.price = 0;
    cell.rentLevels.clear();
    cell.mortgageValue = 0;
    cell.buildingCost = 0;
    cell.buildingLevel = 0;
    cell.maxBuildingLevel = 0;
    cell.ownerId = NO_OWNER_ID;

    return cell;
}

Cell createBusinessCell(
    int id,
    const QString& name,
    BusinessGroup group,
    int price,
    const QVector<int>& rent,
    int mortgageValue,
    int buildingCost,
    CellType type = CellType::Business,
    int maxBuildingLevel = 5)
{
    Cell cell;
    cell.id = id;
    cell.name = name;
    cell.type = type;
    cell.group = group;
    cell.price = price;
    cell.rentLevels = rent;
    cell.mortgageValue = mortgageValue;
    cell.buildingCost = buildingCost;
    cell.maxBuildingLevel = maxBuildingLevel;
    cell.ownerId = NO_OWNER_ID;

    return cell;
}
Cell createRegularBusinessCell(
    int id,
    const QString& name,
    BusinessGroup group,
    int price)
{
    return createBusinessCell(
        id,
        name,
        group,
        price,
        rentLevelsForPrice(price),
        price / 2,
        price / 2
        );
}
QVector<Cell> buildBoard()
{
    QVector<Cell> board(BOARD_SIZE);

    board[0] = createSpecialCell(0, "START", CellType::Corner);

    board[1] = createRegularBusinessCell(1, "Апрель", BusinessGroup::Pharmacy, 1000);
    board[2] = createRegularBusinessCell(2, "ЕАптека", BusinessGroup::Pharmacy, 1200);
    board[3] = createRegularBusinessCell(3, "Ригла", BusinessGroup::Pharmacy, 1400);

    board[4] = createSpecialCell(4, "Chance", CellType::Chance);

    board[5] = createBusinessCell(
        5,
        "IT Top English",
        BusinessGroup::ITEducation,
        2000,
        {500, 1000, 2000, 4000},
        100,
        0,
        CellType::ExtraBusiness,
        0
        );

    board[6] = createSpecialCell(6, "Community Chest", CellType::CommunityChest);

    board[7] = createRegularBusinessCell(7, "Second Hand", BusinessGroup::Clothes, 1600);
    board[8] = createRegularBusinessCell(8, "Lamoda", BusinessGroup::Clothes, 1800);
    board[9] = createRegularBusinessCell(9, "H&M", BusinessGroup::Clothes, 2000);

    board[10] = createSpecialCell(10, "Jail", CellType::Corner);

    board[11] = createRegularBusinessCell(11, "Дикси", BusinessGroup::FoodMarket, 2200);
    board[12] = createRegularBusinessCell(12, "Магнит", BusinessGroup::FoodMarket, 2400);
    board[13] = createRegularBusinessCell(13, "Пятёрочка", BusinessGroup::FoodMarket, 2600);

    board[14] = createSpecialCell(14, "Chance", CellType::Chance);

    board[15] = createBusinessCell(
        15,
        "IT Top School",
        BusinessGroup::ITEducation,
        2000,
        {500, 1000, 2000, 4000},
        100,
        0,
        CellType::ExtraBusiness,
        0
        );

    board[16] = createSpecialCell(16, "Community Chest", CellType::CommunityChest);

    board[17] = createRegularBusinessCell(17, "ВкусВилл", BusinessGroup::DarkStore, 2800);
    board[18] = createRegularBusinessCell(18, "Яндекс Лавка", BusinessGroup::DarkStore, 3000);
    board[19] = createRegularBusinessCell(19, "Самокат", BusinessGroup::DarkStore, 3200);

    board[20] = createSpecialCell(20, "Free Parking", CellType::Corner);

    board[21] = createRegularBusinessCell(21, "Ozon", BusinessGroup::Marketplace, 3400);
    board[22] = createRegularBusinessCell(22, "Яндекс Маркет", BusinessGroup::Marketplace, 3600);
    board[23] = createRegularBusinessCell(23, "Wildberries", BusinessGroup::Marketplace, 3800);

    board[24] = createSpecialCell(24, "Chance", CellType::Chance);

    board[25] = createBusinessCell(
        25,
        "IT Top University",
        BusinessGroup::ITEducation,
        2000,
        {500, 1000, 2000, 4000},
        100,
        0,
        CellType::ExtraBusiness,
        0
        );

    board[26] = createSpecialCell(26, "Community Chest", CellType::CommunityChest);

    board[27] = createRegularBusinessCell(27, "Ozon Банк", BusinessGroup::Bank, 4000);
    board[28] = createRegularBusinessCell(28, "Альфа-Банк", BusinessGroup::Bank, 4300);
    board[29] = createRegularBusinessCell(29, "Сбербанк", BusinessGroup::Bank, 4600);

    board[30] = createSpecialCell(30, "Go To Jail", CellType::Corner);

    board[31] = createRegularBusinessCell(31, "CD Projekt Red", BusinessGroup::GameStudio, 5000);
    board[32] = createRegularBusinessCell(32, "Rockstar", BusinessGroup::GameStudio, 5500);
    board[33] = createRegularBusinessCell(33, "Valve", BusinessGroup::GameStudio, 6000);

    board[34] = createSpecialCell(34, "Chance", CellType::Chance);

    board[35] = createBusinessCell(
        35,
        "IT Top College",
        BusinessGroup::ITEducation,
        2000,
        {500, 1000, 2000, 4000},
        100,
        0,
        CellType::ExtraBusiness,
        0
        );

    board[36] = createSpecialCell(36, "Community Chest", CellType::CommunityChest);

    board[37] = createRegularBusinessCell(37, "Nvidia", BusinessGroup::IT, 6200);
    board[38] = createRegularBusinessCell(38, "OpenAI", BusinessGroup::IT, 6600);
    board[39] = createRegularBusinessCell(39, "Microsoft", BusinessGroup::IT, 7000);

    return board;
}

QJsonArray rentToJson(const QVector<int>& rent)
{
    QJsonArray json;

    for (int value : rent) {
        json.append(value);
    }

    return json;
}

QVector<int> rentFromJson(const QJsonArray& json)
{
    QVector<int> rent;

    for (const QJsonValue& value : json) {
        rent.append(value.toInt());
    }

    return rent;
}

QJsonArray ownedPropertiesToJson(const QVector<int>& ownedProperties)
{
    QJsonArray json;

    for (int propertyId : ownedProperties) {
        json.append(propertyId);
    }

    return json;
}

QVector<int> ownedPropertiesFromJson(const QJsonArray& json)
{
    QVector<int> ownedProperties;

    for (const QJsonValue& value : json) {
        ownedProperties.append(value.toInt());
    }

    return ownedProperties;
}

} // namespace

GameState::GameState()
    : board_(buildBoard())
{
}

QJsonObject GameState::toJson() const
{
    QJsonObject root;
    root["status"] = gameStatus_;
    root["currentPlayerId"] = currentPlayerId_;
    root["lastDiceValue"] = lastDiceValue_;
    root["lastDiceFirst"] = lastDiceFirst_;
    root["lastDiceSecond"] = lastDiceSecond_;
    root["isGameOver"] = isGameOver_;
    root["winnerId"] = winnerId_;

    QJsonArray playersJson;

    for (const Player& player : players_) {
        QJsonObject playerJson;
        playerJson["id"] = player.id;
        playerJson["nickname"] = player.nickname;
        playerJson["color"] = player.color;
        playerJson["position"] = player.position;
        playerJson["balance"] = player.balance;
        playerJson["isBankrupt"] = player.isBankrupt;
        playerJson["active"] = player.active;
        playerJson["isInJail"] = player.isInJail;
        playerJson["roundsInJail"] = player.roundsInJail;
        playerJson["ownedProperties"] = ownedPropertiesToJson(player.ownedProperties);

        playersJson.append(playerJson);
    }

    root["players"] = playersJson;

    QJsonArray boardJson;

    for (const Cell& cell : board_) {
        QJsonObject cellJson;
        cellJson["id"] = cell.id;
        cellJson["name"] = cell.name;
        cellJson["type"] = cellTypeToString(cell.type);
        cellJson["group"] = businessGroupToString(cell.group);
        cellJson["price"] = cell.price;
        cellJson["rentLevels"] = rentToJson(cell.rentLevels);
        cellJson["rent"] = cell.rentLevels.isEmpty() ? 0 : cell.rentLevels.first();
        cellJson["mortgageValue"] = cell.mortgageValue;
        cellJson["buildingCost"] = cell.buildingCost;
        cellJson["buildingLevel"] = cell.buildingLevel;
        cellJson["maxBuildingLevel"] = cell.maxBuildingLevel;
        cellJson["ownerId"] = cell.ownerId;

        boardJson.append(cellJson);
    }

    root["cells"] = boardJson;

    return root;
}

GameState GameState::fromJson(const QJsonObject& json)
{
    GameState state;
    state.gameStatus_ = json.contains("status") ? json["status"].toString("waiting") : json["gameStatus"].toString("waiting");
    state.currentPlayerId_ = json["currentPlayerId"].toInt(NO_OWNER_ID);
    state.lastDiceValue_ = json["lastDiceValue"].toInt(0);
    state.lastDiceFirst_ = json["lastDiceFirst"].toInt(1);
    state.lastDiceSecond_ = json["lastDiceSecond"].toInt(1);
    state.isGameOver_ = json["isGameOver"].toBool(false);
    state.winnerId_ = json["winnerId"].toInt(NO_WINNER_ID);

    state.players_.clear();

    const QJsonArray playersJson = json["players"].toArray();

    for (const QJsonValue& playerValue : playersJson) {
        const QJsonObject playerJson = playerValue.toObject();

        Player player;
        player.id = playerJson["id"].toInt();
        player.nickname = playerJson["nickname"].toString();
        player.color = playerJson["color"].toString(defaultPlayerColor(state.players_.size()));
        player.position = playerJson["position"].toInt();
        player.balance = playerJson["balance"].toInt(PLAYER_START_BALANCE);
        player.isBankrupt = playerJson["isBankrupt"].toBool(false);
        player.active = playerJson.contains("active") ? playerJson["active"].toBool(!player.isBankrupt) : !player.isBankrupt;
        player.isInJail = playerJson["isInJail"].toBool(false);
        player.roundsInJail = playerJson["roundsInJail"].toInt();

        const QJsonArray ownedPropertiesJson = playerJson.contains("ownedProperties")
                                                   ? playerJson["ownedProperties"].toArray()
                                                   : playerJson["ownedCells"].toArray();

        player.ownedProperties = ownedPropertiesFromJson(ownedPropertiesJson);

        state.players_.append(player);
    }

    const QJsonArray boardJson = json.contains("cells") ? json["cells"].toArray() : json["board"].toArray();
    const int cellsCount = qMin(boardJson.size(), state.board_.size());

    for (int i = 0; i < cellsCount; ++i) {
        const QJsonObject cellJson = boardJson[i].toObject();
        Cell& cell = state.board_[i];

        cell.id = cellJson["id"].toInt(cell.id);
        cell.name = cellJson["name"].toString(cell.name);

        if (cellJson.contains("type")) {
            cell.type = cellTypeFromString(cellJson["type"].toString());
        }

        if (cellJson.contains("group")) {
            cell.group = businessGroupFromString(cellJson["group"].toString());
        }

        cell.price = cellJson["price"].toInt(cell.price);

        if (cellJson.contains("rentLevels") && cellJson["rentLevels"].isArray()) {
            const QJsonArray rentJson = cellJson["rentLevels"].toArray();
            if (!rentJson.isEmpty()) {
                cell.rentLevels = rentFromJson(rentJson);
            }
        } else if (cellJson.contains("rent") && cellJson["rent"].isArray()) {
            const QJsonArray rentJson = cellJson["rent"].toArray();
            if (!rentJson.isEmpty()) {
                cell.rentLevels = rentFromJson(rentJson);
            }
        } else if (cellJson.contains("rent")) {
            const int rent = cellJson["rent"].toInt(0);
            if (rent > 0) {
                cell.rentLevels = {rent};
            }
        }

        cell.mortgageValue = cellJson["mortgageValue"].toInt(cell.mortgageValue);
        cell.buildingCost = cellJson["buildingCost"].toInt(cell.buildingCost);
        cell.maxBuildingLevel = cellJson["maxBuildingLevel"].toInt(cell.maxBuildingLevel);

        const int buildingLevel = cellJson.contains("buildingLevel")
                                      ? cellJson["buildingLevel"].toInt(cell.buildingLevel)
                                      : cellJson["houses"].toInt(cell.buildingLevel);

        cell.buildingLevel = clampBuildingLevel(buildingLevel, cell.maxBuildingLevel);
        cell.ownerId = cellJson["ownerId"].toInt(NO_OWNER_ID);
    }

    if (state.currentPlayerId_ == NO_OWNER_ID && !state.players_.isEmpty()) {
        state.currentPlayerId_ = state.players_.first().id;
    }

    return state;
}

void GameState::addPlayer(int id, const QString& nickname, const QString& color)
{
    if (Player* existingPlayer = playerById(id)) {
        existingPlayer->nickname = nickname;
        if (!color.trimmed().isEmpty()) {
            existingPlayer->color = color;
        }
        return;
    }

    Player player;
    player.id = id;
    player.nickname = nickname;
    player.color = color.trimmed().isEmpty() ? defaultPlayerColor(players_.size()) : color;
    player.active = true;

    if (players_.isEmpty()) {
        currentPlayerId_ = id;
    }

    players_.append(player);
}

const QVector<Cell>& GameState::board() const
{
    return board_;
}

QVector<Cell>& GameState::board()
{
    return board_;
}

const QVector<Player>& GameState::players() const
{
    return players_;
}

QVector<Player>& GameState::players()
{
    return players_;
}

QString GameState::gameStatus() const
{
    return gameStatus_;
}

void GameState::setGameStatus(const QString& gameStatus)
{
    gameStatus_ = gameStatus;
}

int GameState::currentPlayerId() const
{
    return currentPlayerId_;
}

void GameState::setCurrentPlayerId(int playerId)
{
    currentPlayerId_ = playerId;
}

int GameState::lastDiceValue() const
{
    return lastDiceValue_;
}

void GameState::setLastDiceValue(int value)
{
    if (value < 0) {
        lastDiceValue_ = 0;
        return;
    }

    lastDiceValue_ = value;
}

bool GameState::isGameOver() const
{
    return isGameOver_;
}

void GameState::setGameOver(bool isGameOver)
{
    isGameOver_ = isGameOver;
}

int GameState::winnerId() const
{
    return winnerId_;
}

void GameState::setWinnerId(int id)
{
    winnerId_ = id;
}

Player* GameState::playerById(int id)
{
    for (Player& player : players_) {
        if (player.id == id) {
            return &player;
        }
    }

    return nullptr;
}

const Player* GameState::playerById(int id) const
{
    for (const Player& player : players_) {
        if (player.id == id) {
            return &player;
        }
    }

    return nullptr;
}

Cell* GameState::cellAt(int index)
{
    if (index < 0 || index >= board_.size()) {
        return nullptr;
    }

    return &board_[index];
}

const Cell* GameState::cellAt(int index) const
{
    if (index < 0 || index >= board_.size()) {
        return nullptr;
    }

    return &board_[index];
}

int GameState::lastDiceFirst() const
{
    return lastDiceFirst_;
}

int GameState::lastDiceSecond() const
{
    return lastDiceSecond_;
}

void GameState::setLastDiceValues(int first, int second)
{
    if (first < 1 || first > 6 || second < 1 || second > 6)
    {
        lastDiceFirst_ = 1;
        lastDiceSecond_ = 1;
        lastDiceValue_ = 0;
        return;
    }

    lastDiceFirst_ = first;
    lastDiceSecond_ = second;
    lastDiceValue_ = first + second;
}