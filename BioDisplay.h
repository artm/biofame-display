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
class QGraphicsItem;
class QGraphicsTextItem;
class QGraphicsSimpleTextItem;
class QGraphicsPixmapItem;
class BiometricThread;

class BioDisplay : public QMainWindow
{
    Q_OBJECT

public:
    BioDisplay(QWidget *parent = 0);
    ~BioDisplay();

signals:
    void requestFakeMatch();
    void requestFakeNoMatch();

public slots:
    void showPic(const QPixmap& pic, QGraphicsPixmapItem * where = 0);
    void showPic(const QString& path, QGraphicsPixmapItem * where = 0);

    void showRndPic();
    bool showSmallPic(const QPixmap &image, int i);
    void incomingFace(QImage face);
    void addImagePath(QString path);
    void setCaption(const QString& text, int delay = 0);
    void clearCaption();
    void setCaptionLoading() { setCaption("Loading database..."); }
    void setCaptionLoaded() { setCaption("Database loaded", 5000); }

    void showMatch(const QString& slot, const QList<Bio::Portrait>& faces, double score);
    void showNoMatch();
    void textBlink(); // blink ... to fake progress

    void hideDisplay();
    void showDisplay();

protected:
    void setupUI();
    void setupTimers();
    void setupBiometricThread();
    void searchAnimation();

    virtual void keyPressEvent(QKeyEvent *);

    QGraphicsItem * m_rootItem;

    QGraphicsView * m_view;
    QGraphicsScene * m_scene;
    QGraphicsPixmapItem * m_currentPortrait, * m_matchPortrait, * m_origPortrait;
    QGraphicsTextItem * m_text[3];
    QGraphicsSimpleTextItem * m_histCaption;
    QList<QGraphicsPixmapItem*> m_smallPortraits;
    int m_faceW, m_faceH, m_vertSpace, m_smallFaceH, m_smallFaceW;
    QList<QString> m_allFiles;
    QTimer m_rndPicTimer, m_textBlinkTimer;
    BiometricThread * m_bioThread;    
};

#endif // BIODISPLAY_H
