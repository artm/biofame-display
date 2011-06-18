#ifndef BIODISPLAY_H
#define BIODISPLAY_H

#include <QMainWindow>
#include <QList>
#include <QStringList>
#include <QTimer>
#include <QDir>
#include <QHash>

#include "CommonTypes.h"

class QGraphicsView;
class QGraphicsScene;
class QGraphicsTextItem;
class QGraphicsPixmapItem;
class BiometricThread;

class BioDisplay : public QMainWindow
{
    Q_OBJECT

public:
    BioDisplay(QWidget *parent = 0);
    ~BioDisplay();

public slots:
    void showPic(const QString& path);
    void showPic(const QPixmap& pic);

    void showRndPic();
    bool showSmallPic(const QPixmap &image, int i);
    void incomingFace(QImage face);
    void addImagePath(QString path);
    void setCaption(const QString& text);
    void setCaptionLoading() { setCaption("Loading database..."); }
    void setCaptionLoaded() { setCaption("Database loaded"); }

    void showMatch(const QString& slot, const QList<Bio::Portrait>& faces);
    void showNoMatch();
    void textBlink(); // blink ... to fake progress

protected:
    void setupUI();
    void setupTimers();
    void setupBiometricThread();
    void searchAnimation();

    QGraphicsView * m_view;
    QGraphicsScene * m_scene;
    QGraphicsPixmapItem * m_currentPortrait;
    QGraphicsPixmapItem * m_matchPortrait;
    QGraphicsTextItem * m_caption, * m_currentStamp, * m_matchStamp;
    QList<QGraphicsPixmapItem*> m_smallPortraits;
    int m_faceW, m_faceH, m_vertSpace, m_smallFaceH, m_smallFaceW;
    QList<QString> m_allFiles;
    QTimer m_rndPicTimer, m_textBlinkTimer;
    BiometricThread * m_bioThread;
};

#endif // BIODISPLAY_H
