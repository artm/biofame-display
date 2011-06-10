#include "BioDisplay.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QDebug>

BioDisplay::BioDisplay(QWidget *parent)
    : QMainWindow(parent)
{
    createUI();
}

BioDisplay::~BioDisplay()
{

}

void BioDisplay::createUI()
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
    m_mainPortrait = m_scene->addRect( -faceW/2,-faceH/2,faceW,faceH,QPen(QColor(50,50,50)) );
    m_mainPortrait->setPos( hgap+faceW/2 ,h/2);
    m_matchPortrait = m_scene->addRect( -faceW/2,-faceH/2,faceW,faceH,QPen(QColor(50,50,50)) );
    m_matchPortrait->setPos(w - (hgap+faceW/2),h/2);

    // text
    QFont fnt;
    fnt.setPixelSize( h/12 );
    m_caption = m_scene->addText( "", fnt );
    m_caption->setDefaultTextColor(QColor(100,100,100));
    m_caption->setTextWidth(w*2/3);
    m_caption->setHtml(QString("<center>%1</center>").arg("Fifteen Minutes of Biometric Fame"));
    QSizeF tsz = m_caption->boundingRect().size();
    // vertically center under the portrait...
    m_caption->setPos(w/2 - tsz.width()/2, h - vspace/2 - tsz.height()/2);

    // small portraits
    int smallFaceH = vspace*3/4, smallFaceW = smallFaceH*2/3;
    int smallFaceMargin = (vspace-smallFaceH)/2;
    int nSmallFaces = (w - smallFaceMargin) / (smallFaceW+smallFaceMargin);
    int offs = smallFaceMargin+smallFaceW/2;
    for(int i = 0; i<nSmallFaces; i++) {
        QGraphicsRectItem * smallPortrait = m_scene->addRect( -smallFaceW/2, -smallFaceH/2, smallFaceW, smallFaceH,
                                                          QPen(QColor(50,50,50)) );
        smallPortrait->setPos(offs+i*(smallFaceMargin+smallFaceW),vspace/2);
        m_smallPortraits << smallPortrait;
    }
}
