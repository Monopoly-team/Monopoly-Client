#include "game_widget_components/board_widget.hpp"
#include "game/models/business_group_utils.hpp"
#include "game/models/cell_type.hpp"
#include "ui/player_visuals.hpp"

#include <QAction>
#include <QMessageBox>
#include <QStringList>
#include <QMenu>
#include <QContextMenuEvent>
#include <QPixmap>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QPen>
#include <QColor>
#include <QPainterPath>
#include <QSvgRenderer>

#include <limits>
#include <algorithm>

BoardWidget::BoardWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("boardWidget");
    setMinimumSize(760, 760);

    eventChatArea_ = new QFrame(this);
    eventChatArea_->setObjectName("eventChatArea");

    eventsView_ = new QTextEdit(eventChatArea_);
    eventsView_->setObjectName("eventsView");
    eventsView_->setReadOnly(true);

    chatInput_ = new QLineEdit(eventChatArea_);
    chatInput_->setObjectName("chatInput");
    chatInput_->setPlaceholderText("Написать сообщение...");
    chatInput_->setFixedHeight(46);

    auto* layout = new QVBoxLayout(eventChatArea_);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(12);
    layout->addWidget(eventsView_);
    layout->addWidget(chatInput_);

    connect(chatInput_, &QLineEdit::returnPressed, this, &BoardWidget::sendMessage);
    setCells(createDefaultCells());

    animationTimer_ = new QTimer(this);
    animationTimer_->setInterval(16);

    connect(animationTimer_, &QTimer::timeout,this, &BoardWidget::updateTokenAnimations);
}

BoardWidget::~BoardWidget() = default;

void BoardWidget::addEvent(const QString& event)
{
    QString matchedNickname;

    if (const ClientGamePlayer* turnPlayer = playerInTurnEvent(event, &matchedNickname))
    {
        appendTurnEventLine(event, *turnPlayer, matchedNickname);
        return;
    }

    const ClientGamePlayer* player = playerAtEventStart(event, &matchedNickname);

    if (!player)
    {
        appendSystemLine(event);
        return;
    }

    QString actionText = event.mid(matchedNickname.length()).trimmed();

    if (actionText.startsWith(':') ||
        actionText.startsWith('-') ||
        actionText.startsWith(QChar(0x2014)))
    {
        actionText = actionText.mid(1).trimmed();
    }

    appendPlayerEventLine(*player, actionText);
}

void BoardWidget::addChatMessage(quint16 playerId, const QString& nickname, const QString& text)
{
    const ClientGamePlayer* player = playerById(playerId);

    if (player)
    {
        appendChatLine(*player, text);
        return;
    }

    const QString safeNickname = nickname.toHtmlEscaped();
    const QString safeText = text.toHtmlEscaped();

    eventsView_->append(
        QStringLiteral(
            "<span style=\"color:#FFFFFF; font-weight:800;\">%1:</span> "
            "<span style=\"color:#EDEDED;\">%2</span>"
            )
            .arg(safeNickname)
            .arg(safeText)
        );
}
void BoardWidget::clearEvents()
{
    eventsView_->clear();
}
namespace
{
QString imagePathForCell(quint8 id)
{
    switch (id)
    {
    case 0:  return ":/resources/img/start.svg";
    case 10: return ":/resources/img/jail.svg";
    case 20: return ":/resources/img/parking.svg";
    case 30: return ":/resources/img/policeman.svg";

    case 1:  return ":/resources/img/aprel.svg";
    case 2:  return ":/resources/img/eapteka.png";
    case 3:  return ":/resources/img/rigla.png";

    case 4:  return ":/resources/img/chanceV.png";
    case 5:  return ":/resources/img/it_top_eng.svg";
    case 6:  return ":/resources/img/chestV.png";

    case 7:  return ":/resources/img/secondhand.svg";
    case 8:  return ":/resources/img/lamoda.svg";
    case 9:  return ":/resources/img/hm.png";

    case 11: return ":/resources/img/5erochka.svg";
    case 12: return ":/resources/img/magnit.svg";
    case 13: return ":/resources/img/diksi.svg";

    case 14:  return ":/resources/img/chanceH.png";
    case 15: return ":/resources/img/it_top_sch.svg";
    case 16:  return ":/resources/img/chestH.png";

    case 17: return ":/resources/img/vkusvill.svg";
    case 18: return ":/resources/img/lavka.svg";
    case 19: return ":/resources/img/samokat.svg";

    case 21: return ":/resources/img/ozon.svg";
    case 22: return ":/resources/img/yamarket.svg";
    case 23: return ":/resources/img/wb.svg";

    case 24:  return ":/resources/img/chanceV.png";
    case 25: return ":/resources/img/it_top_uni.svg";
    case 26:  return ":/resources/img/chestV.png";

    case 27: return ":/resources/img/ozonbank.svg";
    case 28: return ":/resources/img/alfa.svg";
    case 29: return ":/resources/img/sber.svg";

    case 31: return ":/resources/img/cd.svg";
    case 32: return ":/resources/img/rockstar.svg";
    case 33: return ":/resources/img/valve.svg";

    case 34:  return ":/resources/img/chanceH.png";
    case 35: return ":/resources/img/it_top_col.svg";
    case 36:  return ":/resources/img/chestH.png";

    case 37: return ":/resources/img/nvidia.svg";
    case 38: return ":/resources/img/openai.svg";
    case 39: return ":/resources/img/microsoft.svg";

    default:
        return {};
    }
}
}
void BoardWidget::setCells(const QVector<ClientBoardCell>& cells)
{
    cells_ = cells;

    for (ClientBoardCell& cell : cells_)
    {
        cell.imagePath = imagePathForCell(cell.id);
    }

    update();
}

void BoardWidget::setPlayers(const QVector<ClientGamePlayer>& players)
{
    for (const ClientGamePlayer& player : players)
    {
        const int newPosition = player.position;

        if (newPosition < 0 || newPosition >= 40)
            continue;

        if (playerPositions_.contains(player.id))
        {
            const int oldPosition = playerPositions_[player.id];

            if (oldPosition != newPosition)
            {
                startTokenAnimation(
                    player.id,
                    oldPosition,
                    newPosition
                    );
            }
        }

        playerPositions_[player.id] = newPosition;
    }

    players_ = players;
    update();
}

void BoardWidget::updateTokenAnimations()
{
    constexpr float durationMs = 180.0f;

    const float progress =
        std::min(
            1.0f,
            static_cast<float>(animationClock_.elapsed()) / durationMs
            );

    for (auto it = tokenAnimations_.begin(); it != tokenAnimations_.end(); )
    {
        TokenAnimation& animation = it.value();
        animation.progress = progress;

        if (progress >= 1.0f)
        {
            animation.currentStep++;

            if (animation.currentStep >= animation.path.size())
            {
                it = tokenAnimations_.erase(it);
                continue;
            }

            const int fromCell = animation.path[animation.currentStep - 1];
            const int toCell = animation.path[animation.currentStep];

            animation.from = tokenPositionForCell(
                fromCell,
                0,
                1
                );

            animation.to = tokenPositionForCell(
                toCell,
                0,
                1
                );

            animation.progress = 0.0f;
            animationClock_.restart();
        }

        ++it;
    }

    if (tokenAnimations_.isEmpty())
        animationTimer_->stop();

    update();
}

QPointF BoardWidget::currentTokenPosition(
    const ClientGamePlayer& player,
    int tokenIndex,
    int tokenCount
    ) const
{
    if (tokenAnimations_.contains(player.id))
    {
        const TokenAnimation& animation =
            tokenAnimations_[player.id];

        const float t = animation.progress;
        const float eased = 1.0f - (1.0f - t) * (1.0f - t);

        return animation.from + (animation.to - animation.from) * eased;
    }

    return tokenPositionForCell(
        player.position,
        tokenIndex,
        tokenCount
        );
}

QVector<int> BoardWidget::buildMovePath(int from, int to) const
{
    QVector<int> path;

    if (from == to)
        return path;

    int current = from;

    while (current != to)
    {
        current = (current + 1) % 40;
        path.push_back(current);
    }

    return path;
}
void BoardWidget::startTokenAnimation(
    quint16 playerId,
    int fromPosition,
    int toPosition
    )
{
    QVector<int> path = buildMovePath(fromPosition, toPosition);

    if (path.isEmpty())
        return;

    TokenAnimation animation;

    animation.path = path;
    animation.currentStep = 0;
    animation.progress = 0.0f;

    animation.from = tokenPositionForCell(
        fromPosition,
        0,
        1
        );

    animation.to = tokenPositionForCell(
        path[0],
        0,
        1
        );

    tokenAnimations_[playerId] = animation;

    if (!animationTimer_->isActive())
    {
        animationClock_.restart();
        animationTimer_->start();
    }
}

void BoardWidget::sendMessage()
{
    const QString message = chatInput_->text().trimmed();

    if (message.isEmpty())
        return;

    emit messageSent(message);
    chatInput_->clear();
}

void BoardWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    pixmapCache_.clear();

    updateChatGeometry();
}
void BoardWidget::contextMenuEvent(QContextMenuEvent* event)
{
    const ClientBoardCell* cell = cellAtPoint(event->pos());

    if (!cell)
    {
        QWidget::contextMenuEvent(event);
        return;
    }

    showCellContextMenu(event->globalPos(), *cell);
}

const ClientBoardCell* BoardWidget::cellAtPoint(const QPoint& point) const
{
    for (int i = 0; i < cells_.size(); ++i)
    {
        if (cellRectByIndex(i).contains(point))
            return &cells_[i];
    }

    return nullptr;
}
QString BoardWidget::ownerNameById(quint16 playerId) const
{
    for (const ClientGamePlayer& player : players_)
    {
        if (player.id == playerId)
            return player.nickname;
    }

    return QStringLiteral("Игрок %1").arg(playerId);
}

const ClientGamePlayer* BoardWidget::playerById(quint16 playerId) const
{
    for (const ClientGamePlayer& player : players_)
    {
        if (player.id == playerId)
            return &player;
    }

    return nullptr;
}

const ClientGamePlayer* BoardWidget::playerAtEventStart(const QString& eventText, QString* matchedNickname) const
{
    const ClientGamePlayer* matchedPlayer = nullptr;
    QString matchedName;

    for (const ClientGamePlayer& player : players_)
    {
        const QString nickname = player.nickname.trimmed();

        if (nickname.isEmpty())
            continue;

        const bool startsWithName =
            eventText == nickname ||
            eventText.startsWith(nickname + " ") ||
            eventText.startsWith(nickname + ":") ||
            eventText.startsWith(nickname + "-") ||
            eventText.startsWith(nickname + QChar(0x2014));

        if (!startsWithName)
            continue;

        if (!matchedPlayer || nickname.length() > matchedName.length())
        {
            matchedPlayer = &player;
            matchedName = nickname;
        }
    }

    if (matchedNickname)
        *matchedNickname = matchedName;

    return matchedPlayer;
}

const ClientGamePlayer* BoardWidget::playerInTurnEvent(const QString& eventText, QString* matchedNickname) const
{
    const QString trimmedText = eventText.trimmed();

    const QStringList prefixes = {
        QStringLiteral("Ход игрока "),
        QStringLiteral("Первым ходит ")
    };

    for (const QString& prefix : prefixes)
    {
        if (!trimmedText.startsWith(prefix))
            continue;

        QString namePart = trimmedText.mid(prefix.length()).trimmed();

        if (namePart.endsWith('.'))
            namePart.chop(1);

        namePart = namePart.trimmed();

        const ClientGamePlayer* matchedPlayer = nullptr;
        QString matchedName;

        for (const ClientGamePlayer& player : players_)
        {
            const QString nickname = player.nickname.trimmed();

            if (nickname.isEmpty())
                continue;

            if (namePart != nickname)
                continue;

            if (!matchedPlayer || nickname.length() > matchedName.length())
            {
                matchedPlayer = &player;
                matchedName = nickname;
            }
        }

        if (matchedNickname)
            *matchedNickname = matchedName;

        return matchedPlayer;
    }

    if (matchedNickname)
        matchedNickname->clear();

    return nullptr;
}

QColor BoardWidget::displayColorForPlayer(const ClientGamePlayer& player) const
{
    return PlayerVisuals::displayColor(player, winnerId_);
}

QColor BoardWidget::ownerColorForCell(const ClientBoardCell& cell) const
{
    if (cell.ownerId == 0)
        return QColor();

    const ClientGamePlayer* owner = playerById(cell.ownerId);

    if (!owner)
        return QColor();

    return displayColorForPlayer(*owner);
}

QColor BoardWidget::backgroundColorForCell(const ClientBoardCell* cell) const
{
    const QColor defaultBackground("#EBEBEB");

    if (!cell)
        return defaultBackground;

    const bool isBusiness =
        cell->type == CellType::Business ||
        cell->type == CellType::ExtraBusiness;

    if (!isBusiness)
        return defaultBackground;

    const QColor ownerColor = ownerColorForCell(*cell);

    if (!ownerColor.isValid())
        return defaultBackground;

    constexpr int ownerPart = 70;
    constexpr int basePart = 30;

    return QColor(
        (ownerColor.red() * ownerPart + defaultBackground.red() * basePart) / 100,
        (ownerColor.green() * ownerPart + defaultBackground.green() * basePart) / 100,
        (ownerColor.blue() * ownerPart + defaultBackground.blue() * basePart) / 100
        );
}

QString BoardWidget::iconForEventText(const QString& text) const
{
    const QString lowerText = text.toLower();

    if (lowerText.contains(QStringLiteral("взял карточку")) ||
        lowerText.contains(QStringLiteral("карточку chance")) ||
        lowerText.contains(QStringLiteral("карточку community chest")))
    {
        return QString::fromUtf8("🃏");
    }

    if (lowerText.startsWith(QStringLiteral("ход игрока")) ||
        lowerText.contains(QStringLiteral("следующий ход")) ||
        lowerText.contains(QStringLiteral("ходит")))
    {
        return QString::fromUtf8("🎯");
    }

    if (lowerText.contains(QStringLiteral("остановился на клетке")))
    {
        return QString::fromUtf8("📍");
    }
    if (lowerText.contains(QStringLiteral("кикнут")) ||
        lowerText.contains(QStringLiteral("кик")) ||
        lowerText.contains(QStringLiteral("заблокирован")))
    {
        return QString::fromUtf8("🚫");
    }

    if (lowerText.contains(QStringLiteral("штраф")) ||
        lowerText.contains(QStringLiteral("оштрафован")))
    {
        return QString::fromUtf8("💸");
    }

    if (lowerText.contains(QStringLiteral("бонус")) ||
        lowerText.contains(QStringLiteral("награду")))
    {
        return QString::fromUtf8("🎁");
    }

    if (lowerText.contains(QStringLiteral("изменение баланса")) ||
        lowerText.contains(QStringLiteral("баланс измен")))
    {
        return QString::fromUtf8("🧾");
    }
    if (lowerText.contains(QStringLiteral("побед")) ||
        lowerText.contains(QStringLiteral("выиграл игру")) ||
        lowerText.contains(QStringLiteral("игра окончена")))
    {
        return QString::fromUtf8("🏆");
    }

    if (lowerText.contains(QStringLiteral("банкрот")))
    {
        return QString::fromUtf8("⚠️");
    }

    if (lowerText.contains(QStringLiteral("тюрьм")))
    {
        return QString::fromUtf8("🚔");
    }

    if (lowerText.contains(QStringLiteral("торг")) ||
        lowerText.contains(QStringLiteral("ставк")) ||
        lowerText.contains(QStringLiteral("аукцион")))
    {
        return QString::fromUtf8("🔨");
    }

    if (lowerText.contains(QStringLiteral("улучш")) ||
        lowerText.contains(QStringLiteral("постро")))
    {
        return QString::fromUtf8("🏠");
    }

    if (lowerText.contains(QStringLiteral("аренд")) ||
        lowerText.contains(QStringLiteral("заплатил")) ||
        lowerText.contains(QStringLiteral("получил аренду")))
    {
        return QString::fromUtf8("🏦");
    }

    if (lowerText.contains(QStringLiteral("купил")) ||
        lowerText.contains(QStringLiteral("купить")) ||
        lowerText.contains(QStringLiteral("покуп")))
    {
        return QString::fromUtf8("💰");
    }

    if (lowerText.contains(QStringLiteral("выбросил")) ||
        lowerText.contains(QStringLiteral("кубик")) ||
        lowerText.contains(QStringLiteral("кубики")))
    {
        return QString::fromUtf8("🎲");
    }

    return QString();
}
void BoardWidget::appendChatLine(const ClientGamePlayer& player, const QString& text)
{
    const QString safeNickname = player.nickname.toHtmlEscaped();
    const QString safeText = text.toHtmlEscaped();
    const QString colorName = displayColorForPlayer(player).name();

    eventsView_->append(
        QStringLiteral(
            "<span style=\"color:%1; font-weight:800;\">%2:</span> "
            "<span style=\"color:#EDEDED; font-weight:600;\">%3</span>"
            )
            .arg(colorName)
            .arg(safeNickname)
            .arg(safeText)
        );
}

void BoardWidget::appendPlayerEventLine(const ClientGamePlayer& player, const QString& text)
{
    const QString safeNickname = player.nickname.toHtmlEscaped();
    const QString safeText = text.toHtmlEscaped();
    const QString colorName = displayColorForPlayer(player).name();

    QString iconHtml;
    const QString icon = iconForEventText(text);

    if (!icon.isEmpty())
    {
        iconHtml = QStringLiteral(
                       "<span style=\"font-size:16px; font-weight:800;\">%1</span> "
                       ).arg(icon.toHtmlEscaped());
    }

    eventsView_->append(
        QStringLiteral(
            "%1"
            "<span style=\"color:%2; font-weight:900;\">%3</span> "
            "<span style=\"color:#EDEDED; font-weight:600;\">%4</span>"
            )
            .arg(iconHtml)
            .arg(colorName)
            .arg(safeNickname)
            .arg(safeText)
        );
}

void BoardWidget::appendTurnEventLine(
    const QString& eventText,
    const ClientGamePlayer& player,
    const QString& nickname
    )
{
    const QString icon = iconForEventText(eventText);
    const QString colorName = displayColorForPlayer(player).name();

    QString iconHtml;

    if (!icon.isEmpty())
    {
        iconHtml = QStringLiteral(
                       "<span style=\"font-size:16px; font-weight:800;\">%1</span> "
                       ).arg(icon.toHtmlEscaped());
    }

    const int nicknameIndex = eventText.indexOf(nickname);

    if (nicknameIndex < 0)
    {
        appendSystemLine(eventText);
        return;
    }

    const QString beforeNickname = eventText.left(nicknameIndex).toHtmlEscaped();
    const QString safeNickname = nickname.toHtmlEscaped();
    const QString afterNickname = eventText.mid(nicknameIndex + nickname.length()).toHtmlEscaped();

    eventsView_->append(
        QStringLiteral(
            "%1"
            "<span style=\"color:#EDEDED; font-weight:600;\">%2</span>"
            "<span style=\"color:%3; font-weight:900;\">%4</span>"
            "<span style=\"color:#EDEDED; font-weight:600;\">%5</span>"
            )
            .arg(iconHtml)
            .arg(beforeNickname)
            .arg(colorName)
            .arg(safeNickname)
            .arg(afterNickname)
        );
}

void BoardWidget::appendSystemLine(const QString& text, bool withEventIcon)
{
    const QString safeText = text.toHtmlEscaped();

    QString iconHtml;

    if (withEventIcon)
    {
        const QString icon = iconForEventText(text);

        if (!icon.isEmpty())
        {
            iconHtml = QStringLiteral(
                           "<span style=\"font-size:16px; font-weight:800;\">%1</span> "
                           ).arg(icon.toHtmlEscaped());
        }
    }

    eventsView_->append(
        QStringLiteral(
            "%1"
            "<span style=\"color:rgba(255,255,255,0.74); font-weight:600;\">%2</span>"
            )
            .arg(iconHtml)
            .arg(safeText)
        );
}

QString BoardWidget::pixmapCacheKey(const QString& path, const QSize& targetSize) const
{
    return path
           + "|"
           + QString::number(targetSize.width())
           + "x"
           + QString::number(targetSize.height());
}

QPixmap BoardWidget::cachedCellPixmap(const QString& path, const QSize& targetSize)
{
    if (path.isEmpty() || targetSize.isEmpty())
        return {};

    const QString key = pixmapCacheKey(path, targetSize);

    const auto cached = pixmapCache_.constFind(key);

    if (cached != pixmapCache_.constEnd())
        return cached.value();

    QPixmap result;

    if (path.endsWith(".svg", Qt::CaseInsensitive))
    {
        QSvgRenderer renderer(path);

        if (!renderer.isValid())
            return {};

        QSize renderSize = renderer.defaultSize();

        if (renderSize.isEmpty())
            renderSize = targetSize;

        renderSize.scale(targetSize, Qt::KeepAspectRatio);

        result = QPixmap(renderSize);
        result.fill(Qt::transparent);

        QPainter svgPainter(&result);
        svgPainter.setRenderHint(QPainter::Antialiasing, true);
        svgPainter.setRenderHint(QPainter::SmoothPixmapTransform, true);

        renderer.render(
            &svgPainter,
            QRect(QPoint(0, 0), renderSize)
            );
    }
    else
    {
        const QPixmap source(path);

        if (source.isNull())
            return {};

        result = source.scaled(
            targetSize,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            );
    }

    if (!result.isNull())
        pixmapCache_.insert(key, result);

    return result;
}

bool BoardWidget::canBuildOnCellEvenly(const ClientBoardCell& targetCell) const
{
    if (targetCell.type != CellType::Business)
        return false;

    if (targetCell.group == BusinessGroup::None)
        return false;

    if (targetCell.ownerId == 0)
        return false;

    if (targetCell.buildingLevel >= targetCell.maxBuildingLevel)
        return false;

    int minBuildingLevel = std::numeric_limits<int>::max();
    int maxBuildingLevel = std::numeric_limits<int>::min();

    bool hasGroupCells = false;

    for (const ClientBoardCell& cell : cells_)
    {
        if (cell.type != CellType::Business || cell.group != targetCell.group)
            continue;

        if (cell.ownerId != targetCell.ownerId)
            return false;

        hasGroupCells = true;

        const int nextLevel =
            cell.id == targetCell.id
                ? cell.buildingLevel + 1
                : cell.buildingLevel;

        minBuildingLevel = std::min(minBuildingLevel, nextLevel);
        maxBuildingLevel = std::max(maxBuildingLevel, nextLevel);
    }

    if (!hasGroupCells)
        return false;

    return maxBuildingLevel - minBuildingLevel <= 1;
}
QString BoardWidget::cellInfoText(const ClientBoardCell& cell) const
{
    QStringList lines;

    lines << QStringLiteral("Клетка #%1").arg(cell.id);
    lines << QStringLiteral("Название: %1").arg(cell.name.isEmpty() ? QStringLiteral("без названия") : cell.name);
    lines << QStringLiteral("Цена: %1").arg(cell.price);
    lines << QStringLiteral("Текущая аренда: %1").arg(cell.rent);

    if (cell.ownerId == 0)
    {
        lines << QStringLiteral("Владелец: нет");
    }
    else
    {
        lines << QStringLiteral("Владелец: %1").arg(ownerNameById(cell.ownerId));
    }

    if (cell.type == CellType::Business)
    {
        lines << QStringLiteral("Улучшение: %1/%2").arg(cell.buildingLevel).arg(cell.maxBuildingLevel);
        lines << QStringLiteral("Стоимость улучшения: %1").arg(cell.buildingCost);
    }

    return lines.join(QLatin1Char('\n'));
}

void BoardWidget::showCellContextMenu(const QPoint& globalPosition, const ClientBoardCell& cell)
{
    QMenu menu(this);

    QAction* infoAction = menu.addAction(QStringLiteral("Информация о клетке"));
    QAction* buildAction = menu.addAction(QStringLiteral("Построить улучшение"));

    const bool canRequestBuild =
        cell.type == CellType::Business &&
        cell.ownerId == localPlayerId_ &&
        cell.buildingCost > 0 &&
        cell.maxBuildingLevel > 0 &&
        cell.buildingLevel < cell.maxBuildingLevel &&
        canBuildOnCellEvenly(cell);

    buildAction->setEnabled(canRequestBuild);

    QAction* selectedAction = menu.exec(globalPosition);

    if (selectedAction == infoAction)
    {
        QMessageBox::information(
            this,
            QStringLiteral("Информация о клетке"),
            cellInfoText(cell)
            );

        return;
    }

    if (selectedAction == buildAction)
        emit buildBusinessRequested(cell.id);
}
void BoardWidget::updateChatGeometry()
{
    const int padding = 24;
    const int boardSize = std::min(width(), height()) - padding * 2;

    const int cornerSize = boardSize / 6;
    const int cellWidth = (boardSize - cornerSize * 2) / 9;

    const int startX = (width() - boardSize) / 2;
    const int startY = (height() - boardSize) / 2;

    QRect centerRect(
        startX + cornerSize,
        startY + cornerSize,
        cellWidth * 9,
        cellWidth * 9
        );
    centerRect.adjust(24, 24, -24, -24);

    eventChatArea_->setGeometry(centerRect);
}

void BoardWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QLinearGradient backgroundGradient(
        rect().topLeft(),
        rect().bottomRight()
        );

    backgroundGradient.setColorAt(0.0, QColor("#0F2E2D"));
    backgroundGradient.setColorAt(0.5, QColor("#184645"));
    backgroundGradient.setColorAt(1.0, QColor("#0A1D1D"));

    painter.fillRect(
        rect(),
        backgroundGradient
        );

    drawCells(painter);
    drawPlayerTokens(painter);
}

void BoardWidget::drawCells(QPainter& painter)
{
    const int padding       = 24;
    const int boardSize     = std::min(width(), height()) - padding * 2;

    const int cornerSize    = boardSize / 6;
    const int cellWidth     = (boardSize - cornerSize * 2) / 9;

    const int startX        = (width() - boardSize) / 2;
    const int startY        = (height() - boardSize) / 2;

    // углы
    drawCellByIndex(painter, QRect(startX + cornerSize + cellWidth * 9,
                                   startY + cornerSize + cellWidth * 9,
                                   cornerSize, cornerSize), 0, BoardCellSide::Corner); // bottom-right

    drawCellByIndex(painter, QRect(startX,
                                   startY + cornerSize + cellWidth * 9,
                                   cornerSize, cornerSize), 10, BoardCellSide::Corner); // bottom-left

    drawCellByIndex(painter, QRect(startX,
                                   startY,
                                   cornerSize, cornerSize), 20, BoardCellSide::Corner); // top-left

    drawCellByIndex(painter, QRect(startX + cornerSize + cellWidth * 9,
                                   startY,
                                   cornerSize, cornerSize), 30, BoardCellSide::Corner); // top-right

    // низ: 1..9, справа налево
    for (int i = 0; i < 9; ++i)
    {
        QRect rect(
            startX + cornerSize + (8 - i) * cellWidth,
            startY + cornerSize + cellWidth * 9,
            cellWidth,
            cornerSize
            );

        drawCellByIndex(painter, rect, i + 1, BoardCellSide::Bottom);
    }

    // лево: 11..19, снизу вверх
    for (int i = 0; i < 9; ++i)
    {
        QRect rect(
            startX,
            startY + cornerSize + (8 - i) * cellWidth,
            cornerSize,
            cellWidth
            );

        drawCellByIndex(painter, rect, 11 + i, BoardCellSide::Left);
    }

    // верх: 21..29, слева направо
    for (int i = 0; i < 9; ++i)
    {
        QRect rect(
            startX + cornerSize + i * cellWidth,
            startY,
            cellWidth,
            cornerSize
            );

        drawCellByIndex(painter, rect, 21 + i, BoardCellSide::Top);
    }

    // право: 31..39, сверху вниз
    for (int i = 0; i < 9; ++i)
    {
        QRect rect(
            startX + cornerSize + cellWidth * 9,
            startY + cornerSize + i * cellWidth,
            cornerSize,
            cellWidth
            );

        drawCellByIndex(painter, rect, 31 + i, BoardCellSide::Right);
    }
}

void BoardWidget::drawCell(
    QPainter& painter,
    const QRect& rect,
    const ClientBoardCell* cell,
    BoardCellSide side
    )
{
    painter.save();

    painter.setPen(QPen(QColor("#202020"), 2));
    painter.setBrush(backgroundColorForCell(cell));
    painter.drawRect(rect);

    if (!cell)
    {
        painter.restore();
        return;
    }

    const bool isBusiness =
        cell->type == CellType::Business ||
        cell->type == CellType::ExtraBusiness;

    const int stripSize = 12;
    const int priceSize = 28;

    QRect groupRect;
    QRect priceRect;
    QRect contentRect = rect.adjusted(6, 6, -6, -6);

    if (isBusiness)
    {
        if (side == BoardCellSide::Bottom)
        {
            groupRect = QRect(rect.left(), rect.top(), rect.width(), stripSize);
            priceRect = QRect(rect.left(), rect.bottom() - priceSize + 1, rect.width(), priceSize);
            contentRect = rect.adjusted(4, stripSize + 4, -4, -priceSize - 4);
        }
        else if (side == BoardCellSide::Top)
        {
            priceRect = QRect(rect.left(), rect.top(), rect.width(), priceSize);
            groupRect = QRect(rect.left(), rect.bottom() - stripSize + 1, rect.width(), stripSize);
            contentRect = rect.adjusted(4, priceSize + 4, -4, -stripSize - 4);
        }
        else if (side == BoardCellSide::Left)
        {
            priceRect = QRect(rect.left(), rect.top(), priceSize, rect.height());
            groupRect = QRect(rect.right() - stripSize + 1, rect.top(), stripSize, rect.height());
            contentRect = rect.adjusted(priceSize + 4, 4, -stripSize - 4, -4);
        }
        else if (side == BoardCellSide::Right)
        {
            groupRect = QRect(rect.left(), rect.top(), stripSize, rect.height());
            priceRect = QRect(rect.right() - priceSize + 1, rect.top(), priceSize, rect.height());
            contentRect = rect.adjusted(stripSize + 4, 4, -priceSize - 4, -4);
        }

        painter.fillRect(groupRect, BusinessGroupUtils::color(cell->group));
        painter.fillRect(priceRect, QColor("#333333"));
    }

    painter.setPen(QColor("#151515"));
    painter.setFont(QFont("Segoe UI", 8, QFont::Bold));

    if (cell->type == CellType::Corner)
    {
        if (!cell->imagePath.isEmpty())
        {
            const QPixmap pixmap = cachedCellPixmap(cell->imagePath, rect.size());

            if (!pixmap.isNull())
            {
                const QPixmap pixmap = cachedCellPixmap(cell->imagePath, rect.size());

                if (!pixmap.isNull())
                {
                    const QPoint pos(
                        rect.center().x() - pixmap.width() / 2,
                        rect.center().y() - pixmap.height() / 2
                        );

                    painter.drawPixmap(pos, pixmap);
                }
            }
        }
        else
        {
            painter.drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, cell->name);
        }
    }

    else if (cell->type == CellType::Chance)
    {
        if (!cell->imagePath.isEmpty())
        {
            const QRect imageRect = rect.adjusted(8, 8, -8, -8);
            const QPixmap pixmap = cachedCellPixmap(cell->imagePath, imageRect.size());

            if (!pixmap.isNull())
            {
                const QPoint pos(
                    imageRect.center().x() - pixmap.width() / 2,
                    imageRect.center().y() - pixmap.height() / 2
                    );

                painter.drawPixmap(pos, pixmap);
            }
        }
        else
        {
            painter.setPen(QColor("#B56CFF"));
            painter.setFont(QFont("Segoe UI", 18, QFont::Bold));
            painter.drawText(rect, Qt::AlignCenter, "?");
        }
    }
    else if (cell->type == CellType::CommunityChest)
    {
        if (!cell->imagePath.isEmpty())
        {
            const QRect imageRect = rect.adjusted(8, 8, -8, -8);
            const QPixmap pixmap = cachedCellPixmap(cell->imagePath, imageRect.size());

            if (!pixmap.isNull())
            {
                const QPoint pos(
                    imageRect.center().x() - pixmap.width() / 2,
                    imageRect.center().y() - pixmap.height() / 2
                    );

                painter.drawPixmap(pos, pixmap);
            }
        }
        else
        {
            painter.setPen(QColor("#50C878"));
            painter.setFont(QFont("Segoe UI", 16, QFont::Bold));
            painter.drawText(rect, Qt::AlignCenter, "!");
        }
    }
    else
    {

        if (!cell->imagePath.isEmpty())
        {
            const QRect imageRect = contentRect.adjusted(2, 2, -2, -2);

            painter.save();

            if (side == BoardCellSide::Top || side == BoardCellSide::Bottom)
            {
                painter.translate(imageRect.center());
                painter.rotate(-90);

                const QRect rotatedRect(
                    -imageRect.height() / 2,
                    -imageRect.width() / 2,
                    imageRect.height(),
                    imageRect.width()
                    );

                const QPixmap pixmap = cachedCellPixmap(
                    cell->imagePath,
                    rotatedRect.size()
                    );

                if (!pixmap.isNull())
                {
                    const QPoint pos(
                        rotatedRect.center().x() - pixmap.width() / 2,
                        rotatedRect.center().y() - pixmap.height() / 2
                        );

                    painter.drawPixmap(pos, pixmap);
                }
            }
            else
            {
                const QPixmap pixmap = cachedCellPixmap(
                    cell->imagePath,
                    imageRect.size()
                    );

                if (!pixmap.isNull())
                {
                    const QPoint pos(
                        imageRect.center().x() - pixmap.width() / 2,
                        imageRect.center().y() - pixmap.height() / 2
                        );

                    painter.drawPixmap(pos, pixmap);
                }
            }

            painter.restore();
        }
        else
        {
            painter.drawText(
                contentRect,
                Qt::AlignCenter | Qt::TextWordWrap,
                cell->name
                );
        }

    }

    if (isBusiness)
    {
        const QString moneyText =
            cell->ownerId == 0
                ? QString("$%1").arg(cell->price)
                : QString("$%1").arg(cell->rent);

        const bool isSideCell =
            side == BoardCellSide::Left ||
            side == BoardCellSide::Right;

        QFont priceFont("Segoe UI", isSideCell ? 9 : 8, QFont::Black);
        priceFont.setHintingPreference(QFont::PreferFullHinting);

        painter.setPen(Qt::white);
        painter.setFont(priceFont);

        if (isSideCell)
        {
            painter.save();

            painter.setRenderHint(QPainter::TextAntialiasing, true);
            painter.translate(priceRect.center());
            painter.rotate(side == BoardCellSide::Right ? 90 : -90);

            QRect rotatedRect(
                -priceRect.height() / 2,
                -priceRect.width() / 2,
                priceRect.height(),
                priceRect.width()
                );

            painter.drawText(
                rotatedRect.adjusted(1, 0, -1, 0),
                Qt::AlignCenter,
                moneyText
                );

            painter.restore();
        }
        else
        {
            painter.drawText(priceRect, Qt::AlignCenter, moneyText);
        }
    }

    painter.restore();
    if (isBusiness)
        drawBuildingMarker(painter, rect, *cell, side);
}
void BoardWidget::drawCellByIndex(QPainter& painter, const QRect& rect, int index, BoardCellSide side)
{
    const ClientBoardCell* cell = nullptr;

    if (index >= 0 && index < cells_.size())
        cell = &cells_[index];

    drawCell(painter, rect, cell, side);
}

QVector<ClientBoardCell> BoardWidget::createDefaultCells() const
{
    QVector<ClientBoardCell> cells(40);

    auto corner = [&](int id, const QString& name,const QString& imagePath = "")
    {
        cells[id].id        = id;
        cells[id].type      = CellType::Corner;
        cells[id].group     = BusinessGroup::None;
        cells[id].name      = name;
        cells[id].imagePath = imagePath;
    };

    auto business = [&](int id, const QString& name, BusinessGroup group, int price, int rent, const QString& imagePath = "")
    {
        cells[id].id        = id;
        cells[id].type      = CellType::Business;
        cells[id].group     = group;
        cells[id].name      = name;
        cells[id].price     = price;
        cells[id].rent      = rent;
        cells[id].ownerId   = 0;
        cells[id].imagePath = imagePath;
    };

    auto extraBusiness = [&](int id, const QString& name, BusinessGroup group, int price, int rent, const QString& imagePath = "")
    {
        cells[id].id        = id;
        cells[id].type      = CellType::ExtraBusiness;
        cells[id].group     = group;
        cells[id].name      = name;
        cells[id].price     = price;
        cells[id].rent      = rent;
        cells[id].ownerId   = 0;
        cells[id].imagePath = imagePath;
    };

    auto chance = [&](int id)
    {
        cells[id].id    = id;
        cells[id].type  = CellType::Chance;
        cells[id].group = BusinessGroup::None;
        cells[id].name  = "ШАНС";
    };

    auto chest = [&](int id)
    {
        cells[id].id    = id;
        cells[id].type  = CellType::CommunityChest;
        cells[id].group = BusinessGroup::None;
        cells[id].name  = "КАЗНА";
    };

    corner(0,  "СТАРТ",     ":/resources/img/start.svg");
    corner(10, "ТЮРЬМА",    ":/resources/img/jail.svg");
    corner(20, "ПАРКОВКА",  ":/resources/img/parking.svg");
    corner(30, "ПОЛИЦИЯ",   ":/resources/img/policeman.svg");

    // НИЗ: справа налево
    business(1, "Апрель",       BusinessGroup::Pharmacy, 100, 10, ":/resources/img/aprel.svg");
    business(2, "ЕАптека",      BusinessGroup::Pharmacy, 120, 12, ":/resources/img/eapteka.png");
    business(3, "Ригла",        BusinessGroup::Pharmacy, 140, 14, ":/resources/img/rigla.png");

    chance(4);

    extraBusiness(5, "",        BusinessGroup::ITEducation, 200, 25,":/resources/img/it_top_eng.svg");

    chest(6);

    business(7, "lamoda",       BusinessGroup::Clothes, 160, 16,":/resources/img/secondhand.svg");
    business(8, "h&m",          BusinessGroup::Clothes, 180, 18,":/resources/img/lamoda.svg");
    business(9, "versace",      BusinessGroup::Clothes, 200, 20,":/resources/img/hm.png");

    // ЛЕВО: снизу вверх
    business(11, "Дикси",       BusinessGroup::FoodMarket, 220, 22,":/resources/img/diksi.svg");
    business(12, "Магнит",      BusinessGroup::FoodMarket, 240, 24,":/resources/img/magnit.svg");
    business(13, "Пятерочка",   BusinessGroup::FoodMarket, 260, 26,":/resources/img/5erochka.svg");

    chance(14);

    extraBusiness(15, "",       BusinessGroup::ITEducation, 200, 25,":/resources/img/it_top_sch.svg");

    chest(16);

    business(17, "ВкусВилл",    BusinessGroup::DarkStore, 280, 28,":/resources/img/vkusvill.svg");
    business(18, "Яндекс Лавка",BusinessGroup::DarkStore, 300, 30,":/resources/img/lavka.svg");
    business(19, "Самокат",     BusinessGroup::DarkStore, 320, 32,":/resources/img/samokat.svg");

    // ВЕРХ: слева направо
    business(21, "Ozon",        BusinessGroup::Marketplace, 340, 34,":/resources/img/ozon.svg");
    business(22,"Яндекс Маркет",BusinessGroup::Marketplace, 360, 36,":/resources/img/yamarket.svg");
    business(23, "Wildberries", BusinessGroup::Marketplace, 380, 38,":/resources/img/wb.svg");

    chance(24);

    extraBusiness(25, "",       BusinessGroup::ITEducation, 200, 25,":/resources/img/it_top_uni.svg");

    chest(26);

    business(27, "Озон банк",   BusinessGroup::Bank, 400, 40,":/resources/img/ozonbank.svg");
    business(28, "Альфа-банк",  BusinessGroup::Bank, 430, 42,":/resources/img/alfa.svg");
    business(29, "Сбербанк",    BusinessGroup::Bank, 460, 44,":/resources/img/sber.svg");

    // ПРАВО: сверху вниз
    business(31, "Cd project",  BusinessGroup::GameStudio, 500, 46,":/resources/img/cd.svg");
    business(32, "Rockstar",    BusinessGroup::GameStudio, 550, 48,":/resources/img/rockstar.svg");
    business(33, "Valve",       BusinessGroup::GameStudio, 600, 50,":/resources/img/valve.svg");

    chance(34);

    extraBusiness(35, "",       BusinessGroup::ITEducation, 200, 25,":/resources/img/it_top_col.svg");

    chest(36);

    business(37, "Nvidia",      BusinessGroup::IT, 620, 52,":/resources/img/nvidia.svg");
    business(38, "OpenAI",      BusinessGroup::IT, 660, 54,":/resources/img/openai.svg");
    business(39, "Microsoft",   BusinessGroup::IT, 700, 56,":/resources/img/microsoft.svg");

    return cells;
}
void BoardWidget::drawBuildingMarker(
    QPainter& painter,
    const QRect& rect,
    const ClientBoardCell& cell,
    BoardCellSide side
    )
{
    if (cell.type != CellType::Business || cell.buildingLevel <= 0)
        return;

    const QString house = QString::fromUtf8("🏠");
    const QString hotel = QString::fromUtf8("🏨");

    QString marker;

    if (cell.maxBuildingLevel > 0 && cell.buildingLevel >= cell.maxBuildingLevel)
    {
        marker = hotel;
    }
    else
    {
        for (int i = 0; i < cell.buildingLevel; ++i)
            marker += house;
    }

    if (marker.isEmpty())
        return;

    constexpr int markerSize = 18;
    constexpr int margin = 2;

    QRect markerRect;

    if (side == BoardCellSide::Top)
    {
        markerRect = QRect(
            rect.left(),
            rect.top() - markerSize - margin,
            rect.width(),
            markerSize
            );
    }
    else if (side == BoardCellSide::Bottom)
    {
        markerRect = QRect(
            rect.left(),
            rect.bottom() + margin,
            rect.width(),
            markerSize
            );
    }
    else if (side == BoardCellSide::Left)
    {
        markerRect = QRect(
            rect.left() - markerSize - margin,
            rect.top(),
            markerSize,
            rect.height()
            );
    }
    else if (side == BoardCellSide::Right)
    {
        markerRect = QRect(
            rect.right() + margin,
            rect.top(),
            markerSize,
            rect.height()
            );
    }
    else
    {
        return;
    }

    painter.save();

    painter.setFont(QFont("Segoe UI Emoji", 10, QFont::Bold));
    painter.setPen(Qt::white);

    if (side == BoardCellSide::Left || side == BoardCellSide::Right)
    {
        painter.translate(markerRect.center());
        painter.rotate(side == BoardCellSide::Right ? 90 : -90);

        QRect rotatedRect(
            -markerRect.height() / 2,
            -markerRect.width() / 2,
            markerRect.height(),
            markerRect.width()
            );

        painter.drawText(rotatedRect, Qt::AlignCenter, marker);
    }
    else
    {
        painter.drawText(markerRect, Qt::AlignCenter, marker);
    }

    painter.restore();
}
QRect BoardWidget::cellRectByIndex(int index) const
{
    const int padding = 24;
    const int boardSize = std::min(width(), height()) - padding * 2;

    const int cornerSize = boardSize / 6;
    const int cellWidth = (boardSize - cornerSize * 2) / 9;

    const int startX = (width() - boardSize) / 2;
    const int startY = (height() - boardSize) / 2;

    if (index == 0)
        return QRect(startX + cornerSize + cellWidth * 9,
                     startY + cornerSize + cellWidth * 9,
                     cornerSize,
                     cornerSize);

    if (index == 10)
        return QRect(startX,
                     startY + cornerSize + cellWidth * 9,
                     cornerSize,
                     cornerSize);

    if (index == 20)
        return QRect(startX,
                     startY,
                     cornerSize,
                     cornerSize);

    if (index == 30)
        return QRect(startX + cornerSize + cellWidth * 9,
                     startY,
                     cornerSize,
                     cornerSize);

    if (index >= 1 && index <= 9)
    {
        const int i = index - 1;

        return QRect(startX + cornerSize + (8 - i) * cellWidth,
                     startY + cornerSize + cellWidth * 9,
                     cellWidth,
                     cornerSize);
    }

    if (index >= 11 && index <= 19)
    {
        const int i = index - 11;

        return QRect(startX,
                     startY + cornerSize + (8 - i) * cellWidth,
                     cornerSize,
                     cellWidth);
    }

    if (index >= 21 && index <= 29)
    {
        const int i = index - 21;

        return QRect(startX + cornerSize + i * cellWidth,
                     startY,
                     cellWidth,
                     cornerSize);
    }

    if (index >= 31 && index <= 39)
    {
        const int i = index - 31;

        return QRect(startX + cornerSize + cellWidth * 9,
                     startY + cornerSize + i * cellWidth,
                     cornerSize,
                     cellWidth);
    }

    return {};
}

QPoint BoardWidget::tokenPositionForCell(int cellIndex, int tokenIndex, int tokenCount) const
{
    const QRect rect = cellRectByIndex(cellIndex);

    if (!rect.isValid())
        return {};

    const QPoint center = rect.center();

    const int xOffset = 11;
    const int yOffset = 16;

    QVector<QPoint> offsets;

    if (tokenCount <= 1)
    {
        offsets = { QPoint(0, 0) };
    }
    else if (tokenCount == 2)
    {
        offsets =
            {
                QPoint(-xOffset, 0),
                QPoint( xOffset, 0)
            };
    }
    else if (tokenCount == 3)
    {
        offsets =
            {
                QPoint(-xOffset, -yOffset),
                QPoint( xOffset, -yOffset),
                QPoint(0, yOffset)
            };
    }
    else if (tokenCount == 4)
    {
        offsets =
            {
                QPoint(-xOffset, -yOffset),
                QPoint( xOffset, -yOffset),
                QPoint(-xOffset,  yOffset),
                QPoint( xOffset,  yOffset)
            };
    }
    else
    {
        offsets =
            {
                QPoint(-xOffset, -yOffset),
                QPoint( xOffset, -yOffset),
                QPoint(-xOffset, 0),
                QPoint( xOffset, 0),
                QPoint(-xOffset, yOffset),
                QPoint( xOffset, yOffset)
            };
    }

    const int safeIndex =
        std::min(tokenIndex, static_cast<int>(offsets.size()) - 1);

    return center + offsets[safeIndex];
}

void BoardWidget::drawPlayerTokens(QPainter& painter)
{
    QHash<int, QVector<ClientGamePlayer>> playersByPosition;

    for (const ClientGamePlayer& player : players_)
    {
        if (player.position < 0 || player.position >= 40)
            continue;

        playersByPosition[player.position].push_back(player);
    }

    const int tokenSize = 14;

    for (auto it = playersByPosition.begin(); it != playersByPosition.end(); ++it)
    {
        const QVector<ClientGamePlayer>& cellPlayers = it.value();

        for (int i = 0; i < cellPlayers.size(); ++i)
        {
            const ClientGamePlayer& player = cellPlayers[i];

            QPointF center =
                currentTokenPosition(player, i, cellPlayers.size());

            QRectF tokenRect(
                center.x() - tokenSize / 2.0,
                center.y() - tokenSize / 2.0,
                tokenSize,
                tokenSize
                );

            painter.setBrush(displayColorForPlayer(player));
            painter.setPen(QPen(QColor("#202020"), 2));
            painter.drawEllipse(tokenRect);
        }
    }
}

void BoardWidget::setLocalPlayerId(quint16 playerId)
{
    localPlayerId_ = playerId;
}

void BoardWidget::setWinnerId(quint16 winnerId)
{
    winnerId_ = winnerId;
    update();
}