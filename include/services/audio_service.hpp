#pragma once

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSoundEffect>

class AudioService : public QObject
{
    Q_OBJECT

public:
    explicit AudioService(QObject* parent = nullptr);

    void startBackgroundMusic();
    void stopBackgroundMusic();

    void playReadyClick();
    void playGameStart();

    void playMenuClick();
    void playJoinSound();
    void playDiceSound();
    void setMusicVolume(float volume);
    float musicVolume() const;
    void playBuySound();
    void playAuctionStartSound();

private:
    QMediaPlayer* musicPlayer_;
    QAudioOutput* musicOutput_;

    QSoundEffect* readyClickSound_;
    QSoundEffect* menuClickSound_;
    QSoundEffect* gameStartSound_;
    QSoundEffect* joinSound_;
    QSoundEffect* diceSound_;
    QSoundEffect* buySound_;
    QSoundEffect* auctionStartSound_;
};