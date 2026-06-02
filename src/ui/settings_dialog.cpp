#include "ui/settings_dialog.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setObjectName("settingsDialog");
    setWindowTitle("Настройки");
    setModal(true);
    setFixedSize(420, 240);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(28, 28, 28, 28);
    rootLayout->setSpacing(20);

    auto* title = new QLabel("Настройки", this);
    title->setObjectName("settingsTitle");

    auto* musicLabel = new QLabel("Громкость музыки", this);
    musicLabel->setObjectName("settingsLabel");

    musicVolumeSlider_ = new QSlider(Qt::Horizontal, this);
    musicVolumeSlider_->setObjectName("settingsSlider");
    musicVolumeSlider_->setRange(0, 100);
    musicVolumeSlider_->setValue(15);

    musicVolumeValue_ = new QLabel("15%", this);
    musicVolumeValue_->setObjectName("settingsValue");

    auto* volumeLayout = new QHBoxLayout();
    volumeLayout->addWidget(musicVolumeSlider_);
    volumeLayout->addWidget(musicVolumeValue_);

    auto* closeButton = new QPushButton("Закрыть", this);
    closeButton->setObjectName("settingsCloseButton");

    rootLayout->addWidget(title);
    rootLayout->addWidget(musicLabel);
    rootLayout->addLayout(volumeLayout);
    rootLayout->addStretch();
    rootLayout->addWidget(closeButton);

    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    connect(musicVolumeSlider_, &QSlider::valueChanged, this,
            [this](int value)
            {
                musicVolumeValue_->setText(QString("%1%").arg(value));
                emit musicVolumeChanged(value / 100.0f);
            });
}

void SettingsDialog::setMusicVolume(float volume)
{
    const int value = static_cast<int>(volume * 100.0f);
    musicVolumeSlider_->setValue(value);
    musicVolumeValue_->setText(QString("%1%").arg(value));
}