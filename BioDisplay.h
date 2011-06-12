#ifndef BIODISPLAY_H
#define BIODISPLAY_H

#include <QMainWindow>
#include <QList>
#include <QStringList>
#include <QTimer>
#include <QDir>
#include <QHash>

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
    void showRndPic();
    bool showSmallPic(const QString &path, int i);
    void incomingFace(QImage face);
    void addImagePath(QString path);
    void setCaption(const QString& text);
    void setCaptionLoading() { setCaption("Loading database..."); }
    void setCaptionLoaded() { setCaption("Database loaded"); }

    void showMatch(const QString& imgPath);
    void showNoMatch();

protected:
    void setupUI();
    void setupTimers();
    void setupBiometricThread();
    void searchAnimation();
    static QString transformName(const QString& cadabra);

    QGraphicsView * m_view;
    QGraphicsScene * m_scene;
    QGraphicsPixmapItem * m_currentPortrait;
    QGraphicsPixmapItem * m_matchPortrait;
    QGraphicsTextItem * m_caption;
    QList<QGraphicsPixmapItem*> m_smallPortraits;
    int m_faceW, m_faceH, m_vertSpace, m_smallFaceH, m_smallFaceW;
    QHash<QString, QStringList> m_slots;
    QTimer m_rndPicTimer;
    BiometricThread * m_bioThread;
};

#endif // BIODISPLAY_H
