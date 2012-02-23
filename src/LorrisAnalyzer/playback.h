#ifndef PLAYBACK_H
#define PLAYBACK_H

#include <QFrame>

class QTimer;

namespace Ui {
    class Playback;
}

class Playback : public QFrame

{
    Q_OBJECT
    
Q_SIGNALS:
    void enablePosSet(bool enable);
    void setPos(int pos);

public:
    explicit Playback(QWidget *parent = 0);
    ~Playback();
    
public slots:
    void rangeChanged(int min, int max);
    void valChanged(int val);

private slots:
    void startBtn();
    void timeout();
    void delayChanged(int val);

    void stopPlayback();

private:
    void enableUi(bool enable);
    bool updateIndex();

    bool m_playing;
    bool m_pause;
    int m_index;

    QTimer *m_timer;
    Ui::Playback *ui;
};

#endif // PLAYBACK_H
