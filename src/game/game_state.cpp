#include "game/game_state.hpp"

#include <QJsonArray>
#include <QtGlobal>

namespace {

QString cellTypeToString(CellType type)
{
    switch (type) {
    case CellType::Start:
        return "start";
    case CellType::Business:
        return "business";
    case CellType::ExtraBusiness:
        return "extra_business";
    case CellType::Jail:
        return "jail";
    case CellType::FreeParking:
        return "free_parking";
    case CellType::GoToJail:
        return "go_to_jail";
    case CellType::Chance:
        return "chance";
    case CellType::CommunityChest:
        return "community_chest";
    }

    return "business";
}

CellType cellTypeFromString(const QString& value)
{
    if (value == "start") {
        return CellType::Start;
    }

    if (value == "business") {
        return CellType::Business;
    }

    if (value == "extra_business") {
        return CellType::ExtraBusiness;
    }

    if (value == "jail") {
        return CellType::Jail;
    }

    if (value == "free_parking") {
        return CellType::FreeParking;
    }

    if (value == "go_to_jail") {
        return CellType::GoToJail;
    }

    if (value == "chance") {
        return CellType::Chance;
    }

    if (value == "community_chest") {
        return CellType::CommunityChest;
    }

    return CellType::Business;
}

QString propertyGroupToString(PropertyGroup group)
{
    switch (group) {
    case PropertyGroup::None:
        return "NONE";
    case PropertyGroup::Pharmacy:
        return "PHARMACY";
    case PropertyGroup::Clothes:
        return "CLOTHERS";
    case PropertyGroup::FoodMarket:
        return "FOODMARKET";
    case PropertyGroup::DarkStore:
        return "DARKSTORE";
    case PropertyGroup::Marketplace:
        return "MARKETPLACE";
    case PropertyGroup::Bank:
        return "BANK";
    case PropertyGroup::GameStudio:
        return "GAMESTUDIO";
    case PropertyGroup::It:
        return "IT";
    case PropertyGroup::ItEducation:
        return "ITEDUCATION";
    }

    return "NONE";
}

PropertyGroup propertyGroupFromString(const QString& value)
{
    if (value == "PHARMACY") {
        return PropertyGroup::Pharmacy;
    }

    if (value == "CLOTHERS") {
        return PropertyGroup::Clothes;
    }

    if (value == "FOODMARKET") {
        return PropertyGroup::FoodMarket;
    }

    if (value == "DARKSTORE") {
        return PropertyGroup::DarkStore;
    }

    if (value == "MARKETPLACE") {
        return PropertyGroup::Marketplace;
    }

    if (value == "BANK") {
        return PropertyGroup::Bank;
    }

    if (value == "GAMESTUDIO") {
        return PropertyGroup::GameStudio;
    }

    if (value == "IT") {
        return PropertyGroup::It;
    }

    if (value == "ITEDUCATION") {
        return PropertyGroup::ItEducation;
    }

    return PropertyGroup::None;
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

Cell createSpecialCell(int id, const QString& name, CellType type)
{
    Cell cell;
    cell.id = id;
    cell.name = name;
    cell.type = type;
    cell.group = PropertyGroup::None;
    cell.ownerId = NO_OWNER_ID;

    return cell;
}

Cell createBusinessCell(
    int id,
    const QString& name,
    PropertyGroup group,
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
    cell.rent = rent;
    cell.mortgageValue = mortgageValue;
    cell.buildingCost = buildingCost;
    cell.maxBuildingLevel = maxBuildingLevel;
    cell.ownerId = NO_OWNER_ID;

    return cell;
}

QVector<Cell> buildBoard()
{
    QVector<Cell> board(BOARD_SIZE);

    board[0] = createSpecialCell(0, "Start", CellType::Start);

    board[1] = createBusinessCell(
        1,
        "Pharmacy Point",
        PropertyGroup::Pharmacy,
        60,
        {2, 10, 30, 90, 160, 250},
        30,
        50);

    board[2] = createSpecialCell(2, "Chance", CellType::Chance);

    board[3] = createBusinessCell(
        3,
        "Pharmacy Network",
        PropertyGroup::Pharmacy,
        60,
        {4, 20, 60, 180, 320, 450},
        30,
        50);

    board[4] = createSpecialCell(4, "Community Chest", CellType::CommunityChest);

    board[5] = createBusinessCell(
        5,
        "IT Education School",
        PropertyGroup::ItEducation,
        200,
        {25, 50, 100, 200},
        100,
        0,
        CellType::ExtraBusiness,
        0);

    board[6] = createBusinessCell(
        6,
        "Pharmacy Warehouse",
        PropertyGroup::Pharmacy,
        100,
        {6, 30, 90, 270, 400, 550},
        50,
        50);

    board[7] = createBusinessCell(
        7,
        "Clothers Studio",
        PropertyGroup::Clothes,
        100,
        {6, 30, 90, 270, 400, 550},
        50,
        50);

    board[8] = createBusinessCell(
        8,
        "Clothers Brand",
        PropertyGroup::Clothes,
        120,
        {8, 40, 100, 300, 450, 600},
        60,
        50);

    board[9] = createBusinessCell(
        9,
        "Clothers Outlet",
        PropertyGroup::Clothes,
        120,
        {8, 40, 100, 300, 450, 600},
        60,
        50);

    board[10] = createSpecialCell(10, "Jail", CellType::Jail);

    board[11] = createBusinessCell(
        11,
        "FoodMarket Local",
        PropertyGroup::FoodMarket,
        140,
        {10, 50, 150, 450, 625, 750},
        70,
        100);

    board[12] = createSpecialCell(12, "Community Chest", CellType::CommunityChest);

    board[13] = createBusinessCell(
        13,
        "FoodMarket Delivery",
        PropertyGroup::FoodMarket,
        140,
        {10, 50, 150, 450, 625, 750},
        70,
        100);

    board[14] = createBusinessCell(
        14,
        "FoodMarket Hyper",
        PropertyGroup::FoodMarket,
        160,
        {12, 60, 180, 500, 700, 900},
        80,
        100);

    board[15] = createBusinessCell(
        15,
        "IT Education Bootcamp",
        PropertyGroup::ItEducation,
        200,
        {25, 50, 100, 200},
        100,
        0,
        CellType::ExtraBusiness,
        0);

    board[16] = createBusinessCell(
        16,
        "DarkStore Mini",
        PropertyGroup::DarkStore,
        180,
        {14, 70, 200, 550, 750, 950},
        90,
        100);

    board[17] = createSpecialCell(17, "Chance", CellType::Chance);

    board[18] = createBusinessCell(
        18,
        "DarkStore Express",
        PropertyGroup::DarkStore,
        180,
        {14, 70, 200, 550, 750, 950},
        90,
        100);

    board[19] = createBusinessCell(
        19,
        "DarkStore Logistics",
        PropertyGroup::DarkStore,
        200,
        {16, 80, 220, 600, 800, 1000},
        100,
        100);

    board[20] = createSpecialCell(20, "Free Parking", CellType::FreeParking);

    board[21] = createBusinessCell(
        21,
        "Marketplace Local",
        PropertyGroup::Marketplace,
        220,
        {18, 90, 250, 700, 875, 1050},
        110,
        150);

    board[22] = createSpecialCell(22, "Chance", CellType::Chance);

    board[23] = createBusinessCell(
        23,
        "Marketplace App",
        PropertyGroup::Marketplace,
        220,
        {18, 90, 250, 700, 875, 1050},
        110,
        150);

    board[24] = createBusinessCell(
        24,
        "Marketplace Global",
        PropertyGroup::Marketplace,
        240,
        {20, 100, 300, 750, 925, 1100},
        120,
        150);

    board[25] = createBusinessCell(
        25,
        "IT Education Academy",
        PropertyGroup::ItEducation,
        200,
        {25, 50, 100, 200},
        100,
        0,
        CellType::ExtraBusiness,
        0);

    board[26] = createBusinessCell(
        26,
        "Bank Branch",
        PropertyGroup::Bank,
        260,
        {22, 110, 330, 800, 975, 1150},
        130,
        150);

    board[27] = createBusinessCell(
        27,
        "Bank Online",
        PropertyGroup::Bank,
        260,
        {22, 110, 330, 800, 975, 1150},
        130,
        150);

    board[28] = createSpecialCell(28, "Community Chest", CellType::CommunityChest);

    board[29] = createBusinessCell(
        29,
        "Bank Holding",
        PropertyGroup::Bank,
        280,
        {24, 120, 360, 850, 1025, 1200},
        140,
        150);

    board[30] = createSpecialCell(30, "Go To Jail", CellType::GoToJail);

    board[31] = createBusinessCell(
        31,
        "GameStudio Indie",
        PropertyGroup::GameStudio,
        300,
        {26, 130, 390, 900, 1100, 1275},
        150,
        200);

    board[32] = createBusinessCell(
        32,
        "GameStudio Mobile",
        PropertyGroup::GameStudio,
        300,
        {26, 130, 390, 900, 1100, 1275},
        150,
        200);

    board[33] = createSpecialCell(33, "Community Chest", CellType::CommunityChest);

    board[34] = createBusinessCell(
        34,
        "GameStudio AAA",
        PropertyGroup::GameStudio,
        320,
        {28, 150, 450, 1000, 1200, 1400},
        160,
        200);

    board[35] = createBusinessCell(
        35,
        "IT Education Platform",
        PropertyGroup::ItEducation,
        200,
        {25, 50, 100, 200},
        100,
        0,
        CellType::ExtraBusiness,
        0);

    board[36] = createSpecialCell(36, "Chance", CellType::Chance);

    board[37] = createBusinessCell(
        37,
        "IT Support",
        PropertyGroup::It,
        350,
        {35, 175, 500, 1100, 1300, 1500},
        175,
        200);

    board[38] = createBusinessCell(
        38,
        "IT Cloud",
        PropertyGroup::It,
        350,
        {35, 175, 500, 1100, 1300, 1500},
        175,
        200);

    board[39] = createBusinessCell(
        39,
        "IT Enterprise",
        PropertyGroup::It,
        400,
        {50, 200, 600, 1400, 1700, 2000},
        200,
        200);

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
    root["gameStatus"] = gameStatus_;
    root["currentPlayerId"] = currentPlayerId_;
    root["lastDiceValue"] = lastDiceValue_;
    root["isGameOver"] = isGameOver_;
    root["winnerId"] = winnerId_;

    QJsonArray playersJson;

    for (const Player& player : players_) {
        QJsonObject playerJson;
        playerJson["id"] = player.id;
        playerJson["nickname"] = player.nickname;
        playerJson["position"] = player.position;
        playerJson["balance"] = player.balance;
        playerJson["isBankrupt"] = player.isBankrupt;
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
        cellJson["group"] = propertyGroupToString(cell.group);
        cellJson["price"] = cell.price;
        cellJson["rent"] = rentToJson(cell.rent);
        cellJson["mortgageValue"] = cell.mortgageValue;
        cellJson["buildingCost"] = cell.buildingCost;
        cellJson["buildingLevel"] = cell.buildingLevel;
        cellJson["maxBuildingLevel"] = cell.maxBuildingLevel;
        cellJson["ownerId"] = cell.ownerId;

        boardJson.append(cellJson);
    }

    root["board"] = boardJson;

    return root;
}

GameState GameState::fromJson(const QJsonObject& json)
{
    GameState state;
    state.gameStatus_ = json["gameStatus"].toString("waiting");
    state.currentPlayerId_ = json["currentPlayerId"].toInt(NO_OWNER_ID);
    state.lastDiceValue_ = json["lastDiceValue"].toInt(0);
    state.isGameOver_ = json["isGameOver"].toBool(false);
    state.winnerId_ = json["winnerId"].toInt(NO_WINNER_ID);

    state.players_.clear();

    const QJsonArray playersJson = json["players"].toArray();

    for (const QJsonValue& playerValue : playersJson) {
        const QJsonObject playerJson = playerValue.toObject();

        Player player;
        player.id = playerJson["id"].toInt();
        player.nickname = playerJson["nickname"].toString();
        player.position = playerJson["position"].toInt();
        player.balance = playerJson["balance"].toInt(PLAYER_START_BALANCE);
        player.isBankrupt = playerJson["isBankrupt"].toBool(false);
        player.isInJail = playerJson["isInJail"].toBool(false);
        player.roundsInJail = playerJson["roundsInJail"].toInt();

        const QJsonArray ownedPropertiesJson = playerJson.contains("ownedProperties")
                                                   ? playerJson["ownedProperties"].toArray()
                                                   : playerJson["ownedCells"].toArray();

        player.ownedProperties = ownedPropertiesFromJson(ownedPropertiesJson);

        state.players_.append(player);
    }

    const QJsonArray boardJson = json["board"].toArray();
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
            cell.group = propertyGroupFromString(cellJson["group"].toString());
        }

        cell.price = cellJson["price"].toInt(cell.price);

        const QJsonArray rentJson = cellJson["rent"].toArray();
        if (!rentJson.isEmpty()) {
            cell.rent = rentFromJson(rentJson);
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

void GameState::addPlayer(int id, const QString& nickname)
{
    if (Player* existingPlayer = playerById(id)) {
        existingPlayer->nickname = nickname;
        return;
    }

    Player player;
    player.id = id;
    player.nickname = nickname;

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