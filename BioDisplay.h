#ifndef BIODISPLAY_H
#define BIODISPLAY_H

#include <QMainWindow>
#include <QList>
#include <QStringList>
#include <QTimer>
#include <QDir>

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
    void showRndPic();
    void incomingFace(QImage face);

protected:
    void setupUI();
    void setupTimers();
    void setupBiometricThread();
    void loadDB();
    void setCaption(const QString& text);
    void searchAnimation();

    QGraphicsView * m_view;
    QGraphicsScene * m_scene;
    QGraphicsPixmapItem * m_currentPortrait;
    QGraphicsPixmapItem * m_matchPortrait;
    QGraphicsTextItem * m_caption;
    QList<QGraphicsPixmapItem*> m_smallPortraits;
    int m_faceW, m_faceH, m_vertSpace;
    QStringList m_pics;
    QTimer m_rndPicTimer;
    BiometricThread * m_bioThread;
};

#endif // BIODISPLAY_H
