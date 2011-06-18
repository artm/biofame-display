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
#include <QRegExp>

template<class X>
X random(QList<X> lst)
{
    return lst[ random() % lst.size() ];
}

template<class Key, class X>
X random(QHash<Key, X> map)
{
    return map[random( map.keys() )];
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

    // small portraits
    m_smallFaceH = m_vertSpace*3/4, m_smallFaceW = m_smallFaceH*2/3;
    int smallFaceMargin = (m_vertSpace-m_smallFaceH)/2;
    int nSmallFaces = (w - smallFaceMargin) / (m_smallFaceW+smallFaceMargin);
    int offs = smallFaceMargin+m_smallFaceW/2;
    for(int i = 0; i<nSmallFaces; i++) {
        QGraphicsRectItem * smallPortrait = m_scene->addRect( -m_smallFaceW/2, -m_smallFaceH/2, m_smallFaceW, m_smallFaceH,
                                                          QPen(QColor(50,50,50)) );
        smallPortrait->setPos(offs+i*(smallFaceMargin+m_smallFaceW),m_vertSpace/2);
        QGraphicsPixmapItem * item = new QGraphicsPixmapItem(smallPortrait);
        item->setOffset( -m_smallFaceW/2, -m_smallFaceH/2 );
        m_smallPortraits << item;
    }

    // text
    QFont fnt;
    fnt.setPixelSize( m_vertSpace/4 );
    m_caption = m_scene->addText( "", fnt );
    m_caption->setDefaultTextColor(QColor(100,100,100));
    m_caption->setTextWidth(w*2/3);
    setCaption("Fifteen Minutes of Biometric Fame");

    fnt.setPixelSize( m_vertSpace/8 );
    m_currentStamp = m_scene->addText("", fnt);
    m_currentStamp->setX( m_currentPortrait->parentItem()->x() );
    m_matchStamp = m_scene->addText("", fnt);
    m_matchStamp->setX( m_matchPortrait->parentItem()->x() );
    QList<QGraphicsTextItem*> lst = QList<QGraphicsTextItem*>() << m_currentStamp << m_matchStamp;
    foreach(QGraphicsTextItem* item, lst) {
        item->setDefaultTextColor(QColor(150,150,150));
        item->setTextWidth( m_faceW * 3 / 2 );
        item->setX( item->x() - item->textWidth()/2 );
        item->setY( m_currentPortrait->parentItem()->y() + m_faceH*2/5  );
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

void BioDisplay::showMatch( const QString& slot, const QList<Bio::Portrait>& faces )
{
    QString timeFormat = "yyyy-MM-dd hh:mm";

    setCaption(slot);
    m_currentStamp->setHtml(QString("<center>%1</center>").arg(QDateTime::currentDateTime().toString((timeFormat))));

    Q_ASSERT(faces.size()>1);
    showPic(QPixmap::fromImage(faces[1].image));
    m_matchStamp->setHtml(QString("<center>%1</center>").arg(faces[1].timestamp.toString(timeFormat)));

    // show the rest of the slot...
    int i = 0;
    foreach(Bio::Portrait face, faces) {
        showSmallPic(QPixmap::fromImage(face.image), i++);
    }

    m_rndPicTimer.stop();
}


void BioDisplay::setupTimers()
{
    m_rndPicTimer.setInterval(50);
    Q_ASSERT( connect(&m_rndPicTimer, SIGNAL(timeout()), SLOT(showRndPic())) );

    m_textBlinkTimer.setInterval(250);
    Q_ASSERT( connect(&m_textBlinkTimer, SIGNAL(timeout()), SLOT(textBlink())) );
}

void BioDisplay::showRndPic()
{
    showPic( random( m_allFiles ) );
}

void BioDisplay::showPic(const QPixmap& pic)
{
    m_matchPortrait->setPixmap(pic.scaled(m_faceW,m_faceH));
}

void BioDisplay::showPic(const QString &path)
{
    showPic(QPixmap( path ));
}

bool BioDisplay::showSmallPic(const QPixmap &image, int i)
{
    if (i >= m_smallPortraits.size()) return false;
    m_smallPortraits[i]->setPixmap( image.scaled(m_smallFaceW,m_smallFaceH) );
    return true;
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
    // Q_ASSERT( connect( m_bioThread, SIGNAL(loadDbStarted()), SLOT(setCaptionLoading())) );
    Q_ASSERT( connect( m_bioThread, SIGNAL(dirReadStarted()), SLOT(setCaptionLoading())) );
    Q_ASSERT( connect( m_bioThread, SIGNAL(dirReadStarted()), &m_textBlinkTimer, SLOT(start())) );
    Q_ASSERT( connect( m_bioThread, SIGNAL(loadDbFinished()), SLOT(setCaptionLoaded())) );
    Q_ASSERT( connect( m_bioThread, SIGNAL(dirReadDone()), &m_textBlinkTimer, SLOT(stop())) );
    Q_ASSERT( connect( m_bioThread, SIGNAL(print(QString)), SLOT(setCaption(QString))));

    m_bioThread->start();
}

void BioDisplay::addImagePath(QString path)
{
    m_allFiles << path;
}

void BioDisplay::textBlink()
{
    QString caption = m_caption->toPlainText();
    QRegExp dots("\\.*$");
    Q_ASSERT( dots.indexIn(caption) > -1 );
    int numDots = (dots.matchedLength() + 1) % 4;

    // this doesn't do what I expect...
    // caption.replace(dots, QString(numDots, '.'));
    // ... which is:
    caption.remove(dots);
    caption += QString(numDots, '.');

    setCaption(caption);
}

