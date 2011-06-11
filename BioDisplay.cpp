#include "BioDisplay.h"
#include "BiometricThread.h"
#include "CommonTypes.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsPixmapItem>
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QTimer>
#include <QFileSystemWatcher>

template<class X>
X random(QList<X> lst)
{
    return lst[ random() % lst.size() ];
}

BioDisplay::BioDisplay(QWidget *parent)
    : QMainWindow(parent)
{
    setupTimers();
    setupUI();
    QApplication::processEvents();
    setupBiometricThread();
}

BioDisplay::~BioDisplay()
{
    if (m_bioThread->isRunning()) {
        // proper thread destruction
        m_bioThread->quit();
        if (m_bioThread->wait( 2000 ))
            delete m_bioThread;
    }
}

void BioDisplay::setupUI()
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
    m_faceH = h/2, m_faceW = m_faceH / Bio::CROP_RATIO;
    m_vertSpace = h/2 - m_faceH/2;
    int hgap = (w - 2*m_faceW)/3;

    // big portraits
    QGraphicsRectItem * rectItem = m_scene->addRect( -m_faceW/2,-m_faceH/2,m_faceW,m_faceH,QPen(QColor(50,50,50)) );
    rectItem->setPos( hgap+m_faceW/2 ,h/2);
    m_currentPortrait = new QGraphicsPixmapItem(rectItem);
    m_currentPortrait->setOffset( -m_faceW/2, -m_faceH/2 );
    rectItem =  m_scene->addRect( -m_faceW/2,-m_faceH/2,m_faceW,m_faceH,QPen(QColor(50,50,50)) );
    rectItem->setPos(w - (hgap+m_faceW/2),h/2);
    m_matchPortrait = new QGraphicsPixmapItem(rectItem);
    m_matchPortrait->setOffset( -m_faceW/2, -m_faceH/2 );

    // text
    QFont fnt;
    fnt.setPixelSize( h/12 );
    m_caption = m_scene->addText( "", fnt );
    m_caption->setDefaultTextColor(QColor(100,100,100));
    m_caption->setTextWidth(w*2/3);
    setCaption("Fifteen Minutes of Biometric Fame");

    // small portraits
    int smallFaceH = m_vertSpace*3/4, smallFaceW = smallFaceH*2/3;
    int smallFaceMargin = (m_vertSpace-smallFaceH)/2;
    int nSmallFaces = (w - smallFaceMargin) / (smallFaceW+smallFaceMargin);
    int offs = smallFaceMargin+smallFaceW/2;
    for(int i = 0; i<nSmallFaces; i++) {
        QGraphicsRectItem * smallPortrait = m_scene->addRect( -smallFaceW/2, -smallFaceH/2, smallFaceW, smallFaceH,
                                                          QPen(QColor(50,50,50)) );
        smallPortrait->setPos(offs+i*(smallFaceMargin+smallFaceW),m_vertSpace/2);
        m_smallPortraits << new QGraphicsPixmapItem(smallPortrait);
    }
}

void BioDisplay::setCaption(const QString &text)
{
    m_caption->setHtml(QString("<center>%1</center>").arg(text));
    QSizeF tsz = m_caption->boundingRect().size();
    // vertically center under the portrait...
    m_caption->setPos(m_scene->width()/2 - tsz.width()/2, m_scene->height() - m_vertSpace/2 - tsz.height()/2);
}

void BioDisplay::searchAnimation()
{
    setCaption("searching...");
    m_rndPicTimer.start();
}

void BioDisplay::showNoMatch()
{
    setCaption("No match found");
    m_rndPicTimer.stop();
}

void BioDisplay::showMatch(const QString &imgPath)
{
    setCaption("Identified as: " + transformName(QFileInfo(imgPath).baseName()));
    showPic(imgPath);

    m_rndPicTimer.stop();
}


void BioDisplay::setupTimers()
{
    m_rndPicTimer.setInterval(50);
    Q_ASSERT( connect(&m_rndPicTimer, SIGNAL(timeout()), this, SLOT(showRndPic())) );
}

void BioDisplay::showRndPic()
{
    showPic( random( m_pics ) );
}

void BioDisplay::showPic(const QString &path)
{
    QPixmap pic( path );
    m_matchPortrait->setPixmap(pic.scaled(m_faceW,m_faceH));
}


void BioDisplay::incomingFace(QImage face)
{
    QPixmap pic = QPixmap::fromImage( face );
    m_currentPortrait->setPixmap( pic.scaled(m_faceW,m_faceH) );
    searchAnimation();
}

void BioDisplay::setupBiometricThread()
{
    m_bioThread = new BiometricThread(this);

    Q_ASSERT( connect( m_bioThread, SIGNAL(incomingFace(QImage)), SLOT(incomingFace(QImage)) ) );
    Q_ASSERT( connect( m_bioThread, SIGNAL(newImagePath(QString)), SLOT(addImagePath(QString)) ) );
    Q_ASSERT( connect( m_bioThread, SIGNAL(loadDbStarted()), SLOT(setCaptionLoading())) );
    Q_ASSERT( connect( m_bioThread, SIGNAL(loadDbFinished()), SLOT(setCaptionLoaded())) );

    m_bioThread->start();
}

void BioDisplay::addImagePath(QString path)
{
    m_pics << path;
}

QString BioDisplay::transformName(const QString &cadabra)
{
    QRegExp pattern("(^\\d+|\\d+$|_+)");
    return QString(cadabra).remove(pattern).replace("-"," ");
}

