#ifndef BIODISPLAY_H
#define BIODISPLAY_H

#include <QMainWindow>
#include <QList>

class QGraphicsView;
class QGraphicsScene;
class QGraphicsRectItem;
class QGraphicsTextItem;

class BioDisplay : public QMainWindow
{
    Q_OBJECT

public:
    BioDisplay(QWidget *parent = 0);
    ~BioDisplay();

protected:
    void createUI();

    QGraphicsView * m_view;
    QGraphicsScene * m_scene;
    QGraphicsRectItem * m_mainPortrait;
    QGraphicsRectItem * m_matchPortrait;
    QGraphicsTextItem * m_caption;
    QList<QGraphicsRectItem*> m_smallPortraits;
};

#endif // BIODISPLAY_H
