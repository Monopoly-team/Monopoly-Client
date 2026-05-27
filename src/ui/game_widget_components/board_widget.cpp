#include "game_widget_components/board_widget.hpp"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QPen>
#include <QColor>
#include <QPainterPath>

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

    connect(chatInput_, &QLineEdit::returnPressed,
            this, &BoardWidget::sendMessage);

    addEvent("Игра началась");
    addEvent("Polad подключился");
    addEvent("Ilja бросил кубики");
}

BoardWidget::~BoardWidget() = default;

void BoardWidget::addEvent(const QString& event)
{
    eventsView_->append(event);
}

void BoardWidget::clearEvents()
{
    eventsView_->clear();
}

void BoardWidget::sendMessage()
{
    const QString message = chatInput_->text().trimmed();

    if (message.isEmpty())
        return;

    emit messageSent(message);

    addEvent("Вы: " + message);

    chatInput_->clear();
}

void BoardWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateChatGeometry();
}

void BoardWidget::updateChatGeometry()
{
    const int padding = 24;
    const int boardSize = std::min(width(), height()) - padding * 2;

    const int cornerSize = boardSize / 7;
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
}

void BoardWidget::drawCells(QPainter& painter)
{
    const int padding = 24;
    const int boardSize = std::min(width(), height()) - padding * 2;

    const int cornerSize = boardSize / 7;
    const int cellWidth = (boardSize - cornerSize * 2) / 9;

    const int startX = (width() - boardSize) / 2;
    const int startY = (height() - boardSize) / 2;

    painter.setPen(QPen(QColor("#202020"), 2));
    painter.setBrush(QColor("#ebebeb"));

    // Углы
    const int radius = 18;

    // TOP LEFT
    {
        QPainterPath path;

        path.moveTo(startX + radius, startY);

        path.lineTo(startX + cornerSize, startY);
        path.lineTo(startX + cornerSize, startY + cornerSize);
        path.lineTo(startX, startY + cornerSize);
        path.lineTo(startX, startY + radius);

        path.quadTo(
            startX,
            startY,
            startX + radius,
            startY
            );

        painter.drawPath(path);
    }

    // TOP RIGHT
    {
        const int x =
            startX + cornerSize + cellWidth * 9;

        const int y = startY;

        QPainterPath path;

        path.moveTo(x, y);

        path.lineTo(x + cornerSize - radius, y);

        path.quadTo(
            x + cornerSize,
            y,
            x + cornerSize,
            y + radius
            );

        path.lineTo(x + cornerSize, y + cornerSize);
        path.lineTo(x, y + cornerSize);

        painter.drawPath(path);
    }

    // BOTTOM LEFT
    {
        const int x = startX;

        const int y =
            startY + cornerSize + cellWidth * 9;

        QPainterPath path;

        path.moveTo(x, y);

        path.lineTo(x + cornerSize, y);
        path.lineTo(x + cornerSize, y + cornerSize);

        path.lineTo(x + radius, y + cornerSize);

        path.quadTo(
            x,
            y + cornerSize,
            x,
            y + cornerSize - radius
            );

        path.lineTo(x, y);

        painter.drawPath(path);
    }

    // BOTTOM RIGHT
    {
        const int x =
            startX + cornerSize + cellWidth * 9;

        const int y =
            startY + cornerSize + cellWidth * 9;

        QPainterPath path;

        path.moveTo(x, y);

        path.lineTo(x + cornerSize, y);
        path.lineTo(
            x + cornerSize,
            y + cornerSize - radius
            );

        path.quadTo(
            x + cornerSize,
            y + cornerSize,
            x + cornerSize - radius,
            y + cornerSize
            );

        path.lineTo(x, y + cornerSize);
        path.lineTo(x, y);

        painter.drawPath(path);
    }

    // Верхняя сторона
    for (int i = 0; i < 9; ++i)
    {
        QRect cell(
            startX + cornerSize + i * cellWidth,
            startY,
            cellWidth,
            cornerSize
            );

        painter.drawRect(cell);
    }

    // Правая сторона
    for (int i = 0; i < 9; ++i)
    {
        QRect cell(
            startX + cornerSize + cellWidth * 9,
            startY + cornerSize + i * cellWidth,
            cornerSize,
            cellWidth
            );

        painter.drawRect(cell);
    }

    // Нижняя сторона
    for (int i = 0; i < 9; ++i)
    {
        QRect cell(
            startX + cornerSize + i * cellWidth,
            startY + cornerSize + cellWidth * 9,
            cellWidth,
            cornerSize
            );

        painter.drawRect(cell);
    }

    // Левая сторона
    for (int i = 0; i < 9; ++i)
    {
        QRect cell(
            startX,
            startY + cornerSize + i * cellWidth,
            cornerSize,
            cellWidth
            );

        painter.drawRect(cell);
    }
}