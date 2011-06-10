#ifndef BIODISPLAY_H
#define BIODISPLAY_H

#include <QMainWindow>

class QGraphicsView;
class QGraphicsScene;

class BioDisplay : public QMainWindow
{
    Q_OBJECT

public:
    BioDisplay(QWidget *parent = 0);
    ~BioDisplay();

protected:
    QGraphicsView * m_view;
    QGraphicsScene * m_scene;
};

#endif // BIODISPLAY_H
