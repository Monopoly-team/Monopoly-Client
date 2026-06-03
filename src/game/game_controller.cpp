#include "game/game_controller.hpp"
#include "game/game_rules.hpp"

#include <QRandomGenerator>
#include <QJsonArray>

namespace {

constexpr int START_BONUS = 200;
constexpr int JAIL_POSITION = 10;

constexpr int MAX_DICE_VALUE = 6;

constexpr int CHANCE_CARD_COUNT = 6;
constexpr int CHANCE_REWARD = 100;
constexpr int CHANCE_FINE = 50;
constexpr int CHANCE_DEAL_REWARD = 75;

constexpr int COMMUNITY_CHEST_CARD_COUNT = 6;
constexpr int COMMUNITY_CHEST_REWARD = 150;
constexpr int COMMUNITY_CHEST_FINE = 75;
constexpr int COMMUNITY_CHEST_GRANT = 100;
constexpr int COMMUNITY_CHEST_COLLECT_FROM_PLAYER = 50;
constexpr int PROPERTY_MAINTENANCE_COST = 25;

} // namespace

GameController::GameController(QObject* parent)
    : QObject(parent)
{
}

void GameController::addPlayer(int playerId, const QString& nickname, const QString& color)
{
    GameState& state = session_.state();

    if (session_.isPlaying()) {
        appendEvent(QStringLiteral("Нельзя добавить игрока: игра уже началась."));
        return;
    }

    if (session_.isFinished()) {
        appendEvent(QStringLiteral("Нельзя добавить игрока: партия уже завершена."));
        return;
    }

    const bool hadPlayer = session_.hasPlayer(playerId);

    if (!session_.addPlayer(playerId, nickname, color)) {
        appendEvent(QStringLiteral("Не удалось добавить игрока."));
        return;
    }

    const Player* player = state.playerById(playerId);

    if (player == nullptr) {
        appendEvent(QStringLiteral("Игрок добавлен, но не найден в состоянии игры."));
        emit stateChanged();
        return;
    }

    if (hadPlayer) {
        appendEvent(QStringLiteral("%1 обновил имя.").arg(playerName(*player)));
    } else {
        appendEvent(QStringLiteral("%1 присоединился к игре.").arg(playerName(*player)));
    }

    emit stateChanged();
}

bool GameController::removePlayer(int playerId)
{
    const Player* player = session_.state().playerById(playerId);
    const QString name = player != nullptr ? playerName(*player) : QStringLiteral("Игрок %1").arg(playerId);

    if (!session_.removePlayer(playerId)) {
        appendEvent(QStringLiteral("Не удалось удалить игрока %1.").arg(name));
        return false;
    }

    hasRolledThisTurn_ = false;
    appendEvent(QStringLiteral("%1 отключился от игры.").arg(name));
    updateGameOver();
    emit stateChanged();

    return true;
}

bool GameController::startGame()
{
    if (session_.state().players().isEmpty()) {
        appendEvent(QStringLiteral("Нельзя начать игру без игроков."));
        return false;
    }

    if (session_.isPlaying()) {
        appendEvent(QStringLiteral("Игра уже запущена."));
        return false;
    }

    if (session_.isFinished()) {
        appendEvent(QStringLiteral("Нельзя начать игру: партия уже завершена."));
        return false;
    }

    if (!session_.start()) {
        appendEvent(QStringLiteral("Не удалось начать игру."));
        return false;
    }

    hasRolledThisTurn_ = false;

    appendEvent(QStringLiteral("Игра началась."));

    if (const Player* player = session_.currentPlayer()) {
        appendEvent(QStringLiteral("Первым ходит %1.").arg(playerName(*player)));
    }

    emit stateChanged();

    return true;
}

bool GameController::handlePlayerAction(int playerId, const QJsonObject& payload)
{
    GameState& state = session_.state();

    if (session_.isFinished() || state.isGameOver()) {
        appendEvent(QStringLiteral("Действие отклонено: игра уже завершена."));
        return false;
    }

    if (!session_.isPlaying()) {
        appendEvent(QStringLiteral("Действие отклонено: игра ещё не началась."));
        return false;
    }

    Player* player = state.playerById(playerId);

    if (player == nullptr) {
        appendEvent(QStringLiteral("Действие отклонено: игрок не найден."));
        return false;
    }

    if (player->isBankrupt || !player->active) {
        appendEvent(QStringLiteral("%1 не может ходить: игрок банкрот.").arg(playerName(*player)));
        return false;
    }

    if (!isCurrentPlayersTurn(playerId)) {
        appendEvent(QStringLiteral("Сейчас не ход игрока %1.").arg(playerName(*player)));
        return false;
    }

    const QString actionType = actionTypeFromJson(payload);

    if (actionType.isEmpty()) {
        appendEvent(QStringLiteral("Действие отклонено: пустой тип действия."));
        return false;
    }

    bool isHandled = false;

    if (actionType == "roll_dice" || actionType == "roll") {
        isHandled = handleRollDiceAction(*player, payload);
    } else if (actionType == "buy_business" || actionType == "buy_property" || actionType == "buy_cell") {
        isHandled = handleBuyBusinessAction(*player);
    } else if (actionType == "build_business" || actionType == "build") {
        isHandled = handleBuildBusinessAction(*player, payload);
    } else if (actionType == "end_turn") {
        isHandled = handleEndTurnAction(*player);
    } else {
        appendEvent(QStringLiteral("Неизвестное действие: %1.").arg(actionType));
        return false;
    }

    if (isHandled) {
        updateGameOver();
        emit stateChanged();
    }

    return isHandled;
}

QJsonObject GameController::gameStateToJson() const
{
    QJsonObject stateJson = session_.state().toJson();
    QJsonArray cellsJson = stateJson["cells"].toArray();

    for (int i = 0; i < cellsJson.size(); ++i)
    {
        QJsonObject cellJson = cellsJson[i].toObject();

        const int cellId = cellJson["id"].toInt(-1);
        const Cell* cell = session_.state().cellAt(cellId);

        if (cell != nullptr)
            cellJson["rent"] = GameRules::calculateRent(session_.state(), *cell);

        cellsJson[i] = cellJson;
    }

    stateJson["cells"] = cellsJson;

    return stateJson;
}

QStringList GameController::takeEvents()
{
    QStringList result = events_;
    events_.clear();

    return result;
}

bool GameController::buyAuctionBusiness(int playerId, int cellId, int price)
{
    GameState& state = session_.state();

    Player* player = state.playerById(playerId);
    Cell* cell = state.cellAt(cellId);

    if (!player || !cell)
        return false;

    if (!GameRules::isBusinessCell(*cell))
        return false;

    if (cell->ownerId != NO_OWNER_ID)
        return false;

    if (price <= 0 || player->balance < price)
        return false;

    player->balance -= price;
    cell->ownerId = player->id;

    if (!player->ownedProperties.contains(cell->id))
        player->ownedProperties.append(cell->id);

    appendEvent(
        QStringLiteral("%1 выиграл торги и купил \"%2\" за %3.")
            .arg(playerName(*player))
            .arg(cell->name)
            .arg(price)
        );

    updateGameOver();
    emit stateChanged();

    return true;
}

void GameController::finishCurrentTurn()
{
    finishTurn();
    emit stateChanged();
}

const GameState& GameController::gameState() const
{
    return session_.state();
}

QString GameController::actionTypeFromJson(const QJsonObject& payload) const
{
    QString actionType = payload["action"].toString().trimmed();

    if (actionType.isEmpty()) {
        actionType = payload["type"].toString().trimmed();
    }

    return actionType.toLower();
}

bool GameController::handleRollDiceAction(Player& player, const QJsonObject& payload)
{
    if (hasRolledThisTurn_) {
        appendEvent(QStringLiteral("%1 уже бросал кубики в этом ходу.").arg(playerName(player)));
        return false;
    }

    if (player.isInJail) {
        player.roundsInJail -= 1;
        session_.state().setLastDiceValues(1, 1);

        if (player.roundsInJail <= 0) {
            player.roundsInJail = 0;
            player.isInJail = false;

            appendEvent(QStringLiteral("%1 пропустил ход и выходит из тюрьмы.").arg(playerName(player)));
        } else {
            appendEvent(QStringLiteral("%1 пропускает ход в тюрьме.").arg(playerName(player)));
        }

        finishTurn();
        return true;
    }

    int dice1 = payload["dice1"].toInt(0);
    int dice2 = payload["dice2"].toInt(0);
    int steps = payload["steps"].toInt(0);

    if (steps <= 0) {
        if (dice1 < 1 || dice1 > MAX_DICE_VALUE) {
            dice1 = QRandomGenerator::global()->bounded(MAX_DICE_VALUE) + 1;
        }

        if (dice2 < 1 || dice2 > MAX_DICE_VALUE) {
            dice2 = QRandomGenerator::global()->bounded(MAX_DICE_VALUE) + 1;
        }

        steps = dice1 + dice2;

        appendEvent(
            QStringLiteral("%1 выбросил %4. Кубики: %2 и %3.")
                .arg(playerName(player))
                .arg(dice1)
                .arg(dice2)
                .arg(steps));
    } else {
        if (steps < 1 || steps > MAX_DICE_VALUE * 2) {
            appendEvent(QStringLiteral("Некорректное количество шагов: %1.").arg(steps));
            return false;
        }

        appendEvent(
            QStringLiteral("%1 выбросил %2.")
                .arg(playerName(player))
                .arg(steps));
    }

    session_.state().setLastDiceValues(dice1, dice2);

    hasRolledThisTurn_ = true;

    movePlayer(player, steps);

    if (Cell* cell = session_.state().cellAt(player.position)) {
        handleLandingCell(player, *cell);
    }
    if (session_.state().currentPlayerId() == player.id
        && !shouldWaitForPurchaseDecision(player)
        && !player.isBankrupt)
    {
        finishTurn();
    }

    if (session_.state().currentPlayerId() == player.id && player.isBankrupt) {
        finishTurn();
    }

    return true;
}

bool GameController::handleBuyBusinessAction(Player& player)
{
    if (!hasRolledThisTurn_) {
        appendEvent(QStringLiteral("%1 сначала должен бросить кубики.").arg(playerName(player)));
        return false;
    }

    Cell* cell = session_.state().cellAt(player.position);

    if (cell == nullptr) {
        appendEvent(QStringLiteral("Покупка невозможна: клетка не найдена."));
        return false;
    }

    return buyBusiness(player, *cell);
}

bool GameController::handleBuildBusinessAction(Player& player, const QJsonObject& payload)
{
    const int cellId = payload.contains("cellId")
    ? payload["cellId"].toInt(-1)
    : player.position;

    Cell* cell = session_.state().cellAt(cellId);

    if (cell == nullptr) {
        appendEvent(QStringLiteral("Строительство невозможно: клетка не найдена."));
        return false;
    }

    return buildBusiness(player, *cell);
}

bool GameController::handleEndTurnAction(Player& player)
{
    if (!hasRolledThisTurn_) {
        appendEvent(QStringLiteral("%1 сначала должен бросить кубики.").arg(playerName(player)));
        return false;
    }

    appendEvent(QStringLiteral("%1 завершил ход.").arg(playerName(player)));

    finishTurn();

    return true;
}

void GameController::movePlayer(Player& player, int steps)
{
    const int oldPosition = player.position;
    const int rawPosition = oldPosition + steps;

    if (rawPosition >= BOARD_SIZE) {
        player.balance += START_BONUS;

        appendEvent(
            QStringLiteral("%1 прошёл Start и получил %2.")
                .arg(playerName(player))
                .arg(START_BONUS));
    }

    player.position = rawPosition % BOARD_SIZE;

    if (const Cell* cell = session_.state().cellAt(player.position)) {
        appendEvent(
            QStringLiteral("%1 остановился на клетке \"%2\".")
                .arg(playerName(player))
                .arg(cell->name));
    }
}

bool GameController::movePlayerToCell(Player& player, int cellId, bool shouldCollectStartBonus)
{
    Cell* targetCell = session_.state().cellAt(cellId);

    if (targetCell == nullptr) {
        appendEvent(QStringLiteral("Перемещение невозможно: клетка %1 не найдена.").arg(cellId));
        return false;
    }

    const int oldPosition = player.position;

    if (shouldCollectStartBonus && cellId < oldPosition) {
        player.balance += START_BONUS;

        appendEvent(
            QStringLiteral("%1 прошёл Start и получил %2.")
                .arg(playerName(player))
                .arg(START_BONUS));
    }

    player.position = cellId;

    appendEvent(
        QStringLiteral("%1 переместился на клетку \"%2\".")
            .arg(playerName(player))
            .arg(targetCell->name));

    handleLandingCell(player, *targetCell);

    return true;
}

bool GameController::movePlayerToNearestBusiness(Player& player)
{
    const int cellId = GameRules::nearestBusinessCellIdFrom(session_.state(), player.position);

    if (cellId < 0) {
        appendEvent(QStringLiteral("Ближайший бизнес не найден."));
        return false;
    }

    return movePlayerToCell(player, cellId, true);
}

void GameController::handleLandingCell(Player& player, Cell& cell)
{
    switch (cell.type) {
    case CellType::Corner:
        handleCornerCell(player, cell);
        break;

    case CellType::Business:
    case CellType::ExtraBusiness:
        handleBusinessCell(player, cell);
        break;

    case CellType::Chance:
        handleChanceCell(player);
        break;

    case CellType::CommunityChest:
        handleCommunityChestCell(player);
        break;
    }
}

void GameController::handleCornerCell(Player& player, const Cell& cell)
{
    switch (cell.id) {
    case 0:
        appendEvent(QStringLiteral("%1 остановился на Start.").arg(playerName(player)));
        break;

    case 10:
        appendEvent(QStringLiteral("%1 просто посетил тюрьму.").arg(playerName(player)));
        break;

    case 20:
        appendEvent(QStringLiteral("%1 попал на свободную парковку.").arg(playerName(player)));
        break;

    case 30:
        sendPlayerToJail(player);
        break;

    default:
        appendEvent(QStringLiteral("%1 попал на угловую клетку.").arg(playerName(player)));
        break;
    }
}

void GameController::handleChanceCell(Player& player)
{
    appendEvent(QStringLiteral("%1 взял карточку Chance.").arg(playerName(player)));

    const int cardIndex = QRandomGenerator::global()->bounded(CHANCE_CARD_COUNT);

    switch (cardIndex) {
    case 0:
        giveMoney(
            player,
            CHANCE_REWARD,
            QStringLiteral("Chance: инвестиционный бонус"));
        break;

    case 1:
        payToBank(
            player,
            CHANCE_FINE,
            QStringLiteral("Chance: штраф за рекламу"));
        break;

    case 2:
        appendEvent(QStringLiteral("Chance: перейти на Start."));
        movePlayerToCell(player, 0, true);
        break;

    case 3:
        appendEvent(QStringLiteral("Chance: отправиться в тюрьму."));
        sendPlayerToJail(player);
        break;

    case 4:
        appendEvent(QStringLiteral("Chance: перейти к ближайшему бизнесу."));
        movePlayerToNearestBusiness(player);
        break;

    case 5:
        giveMoney(
            player,
            CHANCE_DEAL_REWARD,
            QStringLiteral("Chance: удачная сделка"));
        break;

    default:
        break;
    }
}

void GameController::handleCommunityChestCell(Player& player)
{
    appendEvent(QStringLiteral("%1 взял карточку Community Chest.").arg(playerName(player)));

    const int cardIndex = QRandomGenerator::global()->bounded(COMMUNITY_CHEST_CARD_COUNT);

    switch (cardIndex) {
    case 0:
        giveMoney(
            player,
            COMMUNITY_CHEST_REWARD,
            QStringLiteral("Community Chest: поддержка от банка"));
        break;

    case 1:
        payToBank(
            player,
            COMMUNITY_CHEST_FINE,
            QStringLiteral("Community Chest: налог"));
        break;

    case 2:
        giveMoney(
            player,
            COMMUNITY_CHEST_GRANT,
            QStringLiteral("Community Chest: грант на развитие"));
        break;

    case 3:
        collectFromOtherPlayers(
            player,
            COMMUNITY_CHEST_COLLECT_FROM_PLAYER,
            QStringLiteral("Community Chest: сбор с игроков"));
        break;

    case 4: {
        const int payment = GameRules::propertyMaintenanceCost(
            session_.state(),
            player,
            PROPERTY_MAINTENANCE_COST);

        if (payment <= 0) {
            appendEvent(
                QStringLiteral("Community Chest: у %1 нет расходов на обслуживание бизнеса.")
                    .arg(playerName(player)));
            break;
        }

        payToBank(
            player,
            payment,
            QStringLiteral("Community Chest: обслуживание бизнесов"));
        break;
    }

    case 5:
        appendEvent(QStringLiteral("Community Chest: отправиться в тюрьму."));
        sendPlayerToJail(player);
        break;

    default:
        break;
    }
}

void GameController::handleBusinessCell(Player& player, Cell& cell)
{
    if (cell.ownerId == NO_OWNER_ID) {
        appendEvent(
            QStringLiteral("%1 может купить \"%2\" за %3.")
                .arg(playerName(player))
                .arg(cell.name)
                .arg(cell.price));

        return;
    }

    if (cell.ownerId == player.id) {
        appendEvent(
            QStringLiteral("%1 попал на свой бизнес \"%2\".")
                .arg(playerName(player))
                .arg(cell.name));
        finishTurn();
        return;
    }

    Player* owner = session_.state().playerById(cell.ownerId);

    if (owner == nullptr || owner->isBankrupt) {
        cell.ownerId = NO_OWNER_ID;
        cell.buildingLevel = 0;

        appendEvent(QStringLiteral("Бизнес \"%1\" снова свободен.").arg(cell.name));

        appendEvent(
            QStringLiteral("%1 может купить \"%2\" за %3.")
                .arg(playerName(player))
                .arg(cell.name)
                .arg(cell.price));

        return;
    }

    payRent(player, *owner, cell);
    finishTurn();
}

bool GameController::buyBusiness(Player& player, Cell& cell)
{
    if (!GameRules::isBusinessCell(cell)) {
        appendEvent(QStringLiteral("Клетку \"%1\" нельзя купить.").arg(cell.name));
        return false;
    }

    if (cell.ownerId != NO_OWNER_ID) {
        appendEvent(QStringLiteral("Бизнес \"%1\" уже куплен.").arg(cell.name));
        return false;
    }

    if (cell.price <= 0) {
        appendEvent(QStringLiteral("У бизнеса \"%1\" некорректная цена.").arg(cell.name));
        return false;
    }

    if (!GameRules::canBuyBusiness(player, cell)) {
        appendEvent(
            QStringLiteral("%1 не хватает денег на покупку \"%2\".")
                .arg(playerName(player))
                .arg(cell.name));

        return false;
    }

    player.balance -= cell.price;
    cell.ownerId = player.id;

    if (!player.ownedProperties.contains(cell.id)) {
        player.ownedProperties.append(cell.id);
    }

    appendEvent(
        QStringLiteral("%1 купил \"%2\" за %3.")
            .arg(playerName(player))
            .arg(cell.name)
            .arg(cell.price));

    finishTurn();

    return true;
}

bool GameController::buildBusiness(Player& player, Cell& cell)
{
    if (cell.type != CellType::Business) {
        appendEvent(QStringLiteral("На клетке \"%1\" нельзя строить улучшения.").arg(cell.name));
        return false;
    }

    if (cell.ownerId != player.id) {
        appendEvent(
            QStringLiteral("%1 не владеет бизнесом \"%2\".")
                .arg(playerName(player))
                .arg(cell.name));
        return false;
    }

    if (!GameRules::ownsFullGroup(session_.state(), player.id, cell.group)) {
        appendEvent(
            QStringLiteral("Для строительства нужно владеть всей группой бизнеса \"%1\".")
                .arg(cell.name));

        return false;
    }

    if (cell.maxBuildingLevel <= 0 || cell.buildingCost <= 0) {
        appendEvent(QStringLiteral("У бизнеса \"%1\" нет доступных улучшений.").arg(cell.name));
        return false;
    }

    if (cell.buildingLevel >= cell.maxBuildingLevel) {
        appendEvent(QStringLiteral("Бизнес \"%1\" уже улучшен до максимума.").arg(cell.name));
        return false;
    }

    if (!GameRules::canBuildEvenly(session_.state(), cell))
    {
        appendEvent(
            QStringLiteral("Улучшения в группе нужно строить равномерно. Сначала улучши остальные бизнесы этой группы.")
            );

        return false;
    }

    if (player.balance < cell.buildingCost)
    {
        appendEvent(
            QStringLiteral("%1 не хватает денег на улучшение \"%2\".")
                .arg(playerName(player))
                .arg(cell.name)
            );

        return false;
    }

    if (!GameRules::canBuildBusiness(session_.state(), player, cell))
    {
        appendEvent(
            QStringLiteral("Строительство улучшения для \"%1\" невозможно.")
                .arg(cell.name)
            );

        return false;
    }

    player.balance -= cell.buildingCost;
    cell.buildingLevel += 1;

    appendEvent(
        QStringLiteral("%1 улучшил \"%2\" до уровня %3 за %4.")
            .arg(playerName(player))
            .arg(cell.name)
            .arg(cell.buildingLevel)
            .arg(cell.buildingCost));

    return true;
}

bool GameController::payRent(Player& player, Player& owner, const Cell& cell)
{
    const int rentAmount = GameRules::calculateRent(session_.state(), cell);

    if (rentAmount <= 0) {
        appendEvent(QStringLiteral("Аренда за \"%1\" равна 0.").arg(cell.name));
        return true;
    }

    int paidAmount = player.balance;

    if (paidAmount < 0) {
        paidAmount = 0;
    }

    if (paidAmount > rentAmount) {
        paidAmount = rentAmount;
    }

    player.balance -= paidAmount;
    owner.balance += paidAmount;

    appendEvent(
        QStringLiteral("%1 заплатил аренду %2 игроку %3 за \"%4\".")
            .arg(playerName(player))
            .arg(paidAmount)
            .arg(playerName(owner))
            .arg(cell.name));

    if (paidAmount < rentAmount) {
        appendEvent(
            QStringLiteral("%1 не смог оплатить полную аренду %2.")
                .arg(playerName(player))
                .arg(rentAmount));

        declareBankrupt(player);
        return false;
    }

    return true;
}

bool GameController::payToBank(Player& player, int amount, const QString& reason)
{
    if (amount <= 0) {
        return true;
    }

    int paidAmount = player.balance;

    if (paidAmount < 0) {
        paidAmount = 0;
    }

    if (paidAmount > amount) {
        paidAmount = amount;
    }

    player.balance -= paidAmount;

    appendEvent(
        QStringLiteral("%1 заплатил банку %2. Причина: %3.")
            .arg(playerName(player))
            .arg(paidAmount)
            .arg(reason));

    if (paidAmount < amount) {
        appendEvent(
            QStringLiteral("%1 не смог оплатить полный платёж %2.")
                .arg(playerName(player))
                .arg(amount));

        declareBankrupt(player);
        return false;
    }

    return true;
}

void GameController::giveMoney(Player& player, int amount, const QString& reason)
{
    if (amount <= 0) {
        return;
    }

    player.balance += amount;

    appendEvent(
        QStringLiteral("%1 получил %2. Причина: %3.")
            .arg(playerName(player))
            .arg(amount)
            .arg(reason));
}

void GameController::collectFromOtherPlayers(Player& player, int amountPerPlayer, const QString& reason)
{
    if (amountPerPlayer <= 0) {
        return;
    }

    int totalAmount = 0;

    for (Player& otherPlayer : session_.state().players()) {
        if (otherPlayer.id == player.id || otherPlayer.isBankrupt || !otherPlayer.active) {
            continue;
        }

        int paidAmount = otherPlayer.balance;

        if (paidAmount < 0) {
            paidAmount = 0;
        }

        if (paidAmount > amountPerPlayer) {
            paidAmount = amountPerPlayer;
        }

        otherPlayer.balance -= paidAmount;
        player.balance += paidAmount;
        totalAmount += paidAmount;

        appendEvent(
            QStringLiteral("%1 заплатил %2 игроку %3.")
                .arg(playerName(otherPlayer))
                .arg(paidAmount)
                .arg(playerName(player)));

        if (paidAmount < amountPerPlayer) {
            appendEvent(
                QStringLiteral("%1 не смог оплатить полный платёж %2.")
                    .arg(playerName(otherPlayer))
                    .arg(amountPerPlayer));

            declareBankrupt(otherPlayer);
        }
    }

    appendEvent(
        QStringLiteral("%1 получил всего %2. Причина: %3.")
            .arg(playerName(player))
            .arg(totalAmount)
            .arg(reason));
}

void GameController::sendPlayerToJail(Player& player)
{
    player.position = JAIL_POSITION;
    player.isInJail = true;
    player.roundsInJail = 1;

    appendEvent(QStringLiteral("%1 отправлен в тюрьму.").arg(playerName(player)));

    if (session_.state().currentPlayerId() == player.id) {
        finishTurn();
    }
}

bool GameController::isCurrentPlayersTurn(int playerId) const
{
    const Player* player = session_.currentPlayer();

    return player != nullptr && player->id == playerId;
}

void GameController::finishTurn()
{
    hasRolledThisTurn_ = false;

    if (session_.activePlayersCount() <= 1) {
        updateGameOver();

        if (!session_.state().isGameOver()) {
            if (const Player* player = session_.currentPlayer()) {
                appendEvent(QStringLiteral("Ход игрока %1.").arg(playerName(*player)));
            }
        }

        return;
    }

    Player* nextPlayer = session_.nextActivePlayerAfter(session_.state().currentPlayerId());

    if (nextPlayer == nullptr) {
        appendEvent(QStringLiteral("Следующий игрок не найден."));
        return;
    }

    session_.state().setCurrentPlayerId(nextPlayer->id);

    appendEvent(QStringLiteral("Ход игрока %1.").arg(playerName(*nextPlayer)));
}

void GameController::updateGameOver()
{
    GameState& state = session_.state();

    if (state.isGameOver()) {
        return;
    }

    const int activePlayersCount = session_.activePlayersCount();

    if (activePlayersCount > 1) {
        return;
    }

    if (activePlayersCount == 1 && state.players().size() < 2) {
        return;
    }

    const Player* winner = session_.firstActivePlayer();

    if (winner != nullptr) {
        session_.finish(winner->id);

        appendEvent(
            QStringLiteral("Игра окончена. Победитель: %1.")
                .arg(playerName(*winner)));
    } else {
        session_.finish(NO_WINNER_ID);

        appendEvent(QStringLiteral("Игра окончена без победителя."));
    }
}

void GameController::declareBankrupt(Player& player)
{
    if (player.isBankrupt) {
        return;
    }

    player.isBankrupt = true;
    player.active = false;
    player.balance = 0;

    releaseOwnedProperties(player);

    appendEvent(QStringLiteral("%1 стал банкротом.").arg(playerName(player)));
}

void GameController::releaseOwnedProperties(Player& player)
{
    for (Cell& cell : session_.state().board()) {
        if (cell.ownerId != player.id) {
            continue;
        }

        cell.ownerId = NO_OWNER_ID;
        cell.buildingLevel = 0;
    }

    player.ownedProperties.clear();
}

QString GameController::playerName(const Player& player) const
{
    const QString nickname = player.nickname.trimmed();

    if (!nickname.isEmpty()) {
        return nickname;
    }

    return QStringLiteral("Игрок %1").arg(player.id);
}

void GameController::appendEvent(const QString& eventText)
{
    if (!eventText.trimmed().isEmpty()) {
        events_.append(eventText);
    }
}

bool GameController::shouldWaitForPurchaseDecision(const Player& player) const
{
    const Cell* cell = session_.state().cellAt(player.position);

    if (cell == nullptr) {
        return false;
    }

    if (!GameRules::isBusinessCell(*cell)) {
        return false;
    }

    return cell->ownerId == NO_OWNER_ID;
}