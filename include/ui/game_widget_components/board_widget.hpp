#pragma once

#include <QWidget>
#include <QFrame>
#include <QTextEdit>
#include <QLineEdit>

class BoardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BoardWidget(QWidget* parent = nullptr);
    ~BoardWidget() override;

    void addEvent(const QString& event);
    void clearEvents();

signals:
    void messageSent(const QString& message);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void updateChatGeometry();
    void sendMessage();

    void drawCells(QPainter& painter);

private:
    QFrame*    eventChatArea_;
    QTextEdit* eventsView_;
    QLineEdit* chatInput_;
};