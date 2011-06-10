#include "BioDisplay.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QDebug>

BioDisplay::BioDisplay(QWidget *parent)
    : QMainWindow(parent)
{
    m_view = new QGraphicsView(this);
    setCentralWidget(m_view);

    m_scene = new QGraphicsScene(this);
    Q_ASSERT(m_scene);
    m_scene->setBackgroundBrush( QBrush(Qt::black) );
    m_view->setScene(m_scene);

    showFullScreen();
    m_scene->setSceneRect(0,0,width(),height());
    m_view->fitInView( m_scene->sceneRect() );

    int w = width(), h = height();
    int faceH = h/2, faceW = faceH*2/3;
    int vspace = h/2 - faceH/2;
    int hgap = (w - 2*faceW)/3;

    // big portraits
    QGraphicsRectItem * mainPortrait = m_scene->addRect( -faceW/2,-faceH/2,faceW,faceH,QPen(QColor(50,50,50)) );
    mainPortrait->setPos( hgap+faceW/2 ,h/2);
    QGraphicsRectItem * matchPortrait = m_scene->addRect( -faceW/2,-faceH/2,faceW,faceH,QPen(QColor(50,50,50)) );
    matchPortrait->setPos(w - (hgap+faceW/2),h/2);

    // text
    QFont fnt;
    fnt.setPixelSize( h/12 );
    QGraphicsTextItem * text = m_scene->addText( "", fnt );
    text->setDefaultTextColor(QColor(100,100,100));
    text->setTextWidth(w*2/3);
    text->setHtml(QString("<center>%1</center>").arg("Fifteen Minutes of Biometric Fame"));
    QSizeF tsz = text->boundingRect().size();
    // vertically center under the portrait...
    text->setPos(w/2 - tsz.width()/2, h - vspace/2 - tsz.height()/2);

    // small portraits
    int smallFaceH = vspace*3/4, smallFaceW = smallFaceH*2/3;
    int smallFaceMargin = (vspace-smallFaceH)/2;
    int nSmallFaces = (w - smallFaceMargin) / (smallFaceW+smallFaceMargin);
    int offs = smallFaceMargin+smallFaceW/2;
    for(int i = 0; i<nSmallFaces; i++) {
        QGraphicsItem * smallPortrait = m_scene->addRect( -smallFaceW/2, -smallFaceH/2, smallFaceW, smallFaceH,
                                                          QPen(QColor(50,50,50)) );
        smallPortrait->setPos(offs+i*(smallFaceMargin+smallFaceW),vspace/2);
    }
}

BioDisplay::~BioDisplay()
{

}
