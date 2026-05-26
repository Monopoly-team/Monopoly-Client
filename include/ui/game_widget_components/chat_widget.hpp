#pragma once
#include <QWidget>

class ChatWidget : public QWidget
{
public:
    explicit ChatWidget(QWidget* parent = nullptr);
    ~ChatWidget() override;
};
