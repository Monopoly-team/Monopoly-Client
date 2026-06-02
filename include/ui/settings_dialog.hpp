#pragma once

#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QPushButton>

class SettingsDialog : public QDialog
{
    Q_OBJECT

signals:
    void musicVolumeChanged(float volume);

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    void setMusicVolume(float volume);

private:
    QSlider* musicVolumeSlider_;
    QLabel* musicVolumeValue_;
};