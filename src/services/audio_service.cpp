#include "services/audio_service.hpp"

#include "services/audio_service.hpp"

AudioService::AudioService(QObject* parent)
    : QObject(parent)
{
    musicOutput_ = new QAudioOutput(this);
    musicOutput_->setVolume(0.04f);

    musicPlayer_ = new QMediaPlayer(this);
    musicPlayer_->setAudioOutput(musicOutput_);
    musicPlayer_->setSource(QUrl("qrc:/resources/audio/lofi.mp3"));
    musicPlayer_->setLoops(QMediaPlayer::Infinite);


    readyClickSound_ = new QSoundEffect(this);
    readyClickSound_->setSource(QUrl("qrc:/resources/audio/ready_click.wav") );
    readyClickSound_->setVolume(0.2f);

    menuClickSound_ = new QSoundEffect(this);
    menuClickSound_->setSource(QUrl("qrc:/resources/audio/menu_click.wav") );
    menuClickSound_->setVolume(0.5f);

    joinSound_ = new QSoundEffect(this);
    joinSound_->setSource(QUrl("qrc:/resources/audio/join_sound.wav") );
    joinSound_->setVolume(0.5f);

    diceSound_ = new QSoundEffect(this);
    diceSound_->setSource(QUrl("qrc:/resources/audio/dice_sound.wav") );
    diceSound_->setVolume(0.5f);

    gameStartSound_ = new QSoundEffect(this);
    gameStartSound_->setSource(QUrl("qrc:/resources/audio/game_start.wav"));
    gameStartSound_->setVolume(0.7f);
}

void AudioService::startBackgroundMusic()
{
    musicPlayer_->play();
}

void AudioService::stopBackgroundMusic()
{
    musicPlayer_->stop();
}

void AudioService::playReadyClick()
{
    readyClickSound_->play();
}
void AudioService::playMenuClick()
{
    menuClickSound_->play();
}

void AudioService::playGameStart()
{
    gameStartSound_->play();
}

void AudioService::playJoinSound()
{
    joinSound_->play();
}

void AudioService::playDiceSound()
{
    diceSound_->play();
}

void AudioService::setMusicVolume(float volume)
{
    musicOutput_->setVolume(volume);
}

float AudioService::musicVolume() const
{
    return musicOutput_->volume();
}