#include <QtTest/QtTest>
#include <QJsonObject>

#include "game/game_controller.hpp"
#include "game/game_rules.hpp"

class RegressionTests : public QObject
{
    Q_OBJECT

private slots:
    void fullGroupOwnershipDoublesBaseRent();
    void partialGroupOwnershipScalesBaseRent();
    void cannotBuildUnevenly();
    void nearestBusinessWrapsAroundBoard();
    void dicePayloadCannotOverrideServerRoll();
    void cannotBuyOwnedCell();
    void cannotBuyCellWithoutEnoughMoney();
    void canBuildEvenly();
};

static Cell makeBusinessCell(
    int id,
    BusinessGroup group,
    int ownerId,
    int baseRent = 100,
    int buildingLevel = 0
    )
{
    Cell cell;

    cell.id = id;
    cell.name = QStringLiteral("Business %1").arg(id);
    cell.type = CellType::Business;
    cell.group = group;
    cell.ownerId = ownerId;

    cell.price = 1000;
    cell.rentLevels = { baseRent, baseRent * 2, baseRent * 3 };
    cell.buildingCost = 500;
    cell.buildingLevel = buildingLevel;
    cell.maxBuildingLevel = 3;

    return cell;
}

void RegressionTests::fullGroupOwnershipDoublesBaseRent()
{
    GameState state;
    state.board().clear();

    state.board().push_back(makeBusinessCell(0, BusinessGroup::Pharmacy, 1, 100));
    state.board().push_back(makeBusinessCell(1, BusinessGroup::Pharmacy, 1, 100));
    state.board().push_back(makeBusinessCell(2, BusinessGroup::Pharmacy, 1, 100));

    QCOMPARE(GameRules::calculateRent(state, state.board()[0]), 200);
}

void RegressionTests::partialGroupOwnershipScalesBaseRent()
{
    GameState state;
    state.board().clear();

    state.board().push_back(makeBusinessCell(0, BusinessGroup::Pharmacy, 1, 100));
    state.board().push_back(makeBusinessCell(1, BusinessGroup::Pharmacy, 1, 100));
    state.board().push_back(makeBusinessCell(2, BusinessGroup::Pharmacy, NO_OWNER_ID, 100));

    QCOMPARE(GameRules::calculateRent(state, state.board()[0]), 150);
}

void RegressionTests::cannotBuildUnevenly()
{
    GameState state;
    state.board().clear();

    state.board().push_back(makeBusinessCell(0, BusinessGroup::Pharmacy, 1, 100, 1));
    state.board().push_back(makeBusinessCell(1, BusinessGroup::Pharmacy, 1, 100, 0));
    state.board().push_back(makeBusinessCell(2, BusinessGroup::Pharmacy, 1, 100, 0));

    QVERIFY(!GameRules::canBuildEvenly(state, state.board()[0]));
}

void RegressionTests::nearestBusinessWrapsAroundBoard()
{
    GameState state;
    state.board().clear();

    Cell start;
    start.id = 0;
    start.type = CellType::Corner;
    start.name = QStringLiteral("Start");

    Cell business = makeBusinessCell(1, BusinessGroup::Pharmacy, NO_OWNER_ID);

    Cell chance;
    chance.id = 2;
    chance.type = CellType::Chance;
    chance.name = QStringLiteral("Chance");

    Cell parking;
    parking.id = 3;
    parking.type = CellType::Corner;
    parking.name = QStringLiteral("Parking");

    state.board().push_back(start);
    state.board().push_back(business);
    state.board().push_back(chance);
    state.board().push_back(parking);

    QCOMPARE(GameRules::nearestBusinessCellIdFrom(state, 3), 1);
}

void RegressionTests::dicePayloadCannotOverrideServerRoll()
{
    GameController controller;

    controller.addPlayer(1, QStringLiteral("Player 1"));
    controller.addPlayer(2, QStringLiteral("Player 2"));

    QVERIFY(controller.startGame());

    QJsonObject payload;
    payload["action"] = "roll_dice";

    // Регрессия: раньше клиент мог прислать свои dice/steps.
    // Сервер должен игнорировать эти поля и сам генерировать кубики.
    payload["dice1"] = 99;
    payload["dice2"] = 99;
    payload["steps"] = 2;

    QVERIFY(controller.handlePlayerAction(1, payload));

    const GameState& state = controller.gameState();

    QVERIFY(state.lastDiceFirst() >= 1);
    QVERIFY(state.lastDiceFirst() <= 6);

    QVERIFY(state.lastDiceSecond() >= 1);
    QVERIFY(state.lastDiceSecond() <= 6);

    QCOMPARE(state.lastDiceValue(), state.lastDiceFirst() + state.lastDiceSecond());

    QVERIFY(state.lastDiceValue() >= 2);
    QVERIFY(state.lastDiceValue() <= 12);
}

void RegressionTests::cannotBuyOwnedCell()
{
    Player player;
    player.id = 1;
    player.balance = 5000;

    Cell cell = makeBusinessCell(
        1,
        BusinessGroup::Pharmacy,
        2,
        100
        );

    QVERIFY(!GameRules::canBuyCell(player, cell));
}

void RegressionTests::cannotBuyCellWithoutEnoughMoney()
{
    Player player;
    player.id = 1;
    player.balance = 500;

    Cell cell = makeBusinessCell(
        1,
        BusinessGroup::Pharmacy,
        NO_OWNER_ID,
        100
        );

    cell.price = 1000;

    QVERIFY(!GameRules::canBuyCell(player, cell));
}

void RegressionTests::canBuildEvenly()
{
    GameState state;
    state.board().clear();

    state.board().push_back(makeBusinessCell(0, BusinessGroup::Pharmacy, 1, 100, 0));
    state.board().push_back(makeBusinessCell(1, BusinessGroup::Pharmacy, 1, 100, 0));
    state.board().push_back(makeBusinessCell(2, BusinessGroup::Pharmacy, 1, 100, 0));

    QVERIFY(GameRules::canBuildEvenly(state, state.board()[0]));
}

QTEST_MAIN(RegressionTests)

#include "regression_tests.moc"