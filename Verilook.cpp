#include "Verilook.h"
#include "CommonTypes.h"

using namespace Bio;

#include <QtDebug>
#include <QImage>
#include <QFileInfo>
#include <QRegExp>
#include <QDir>

#include <NMatcherParams.h>

const char * s_defaultPort = "5000";
const char * s_defaultServer = "/local";
const char * s_licenseList = "SingleComputerLicense:VLExtractor,SingleComputerLicense:VLMatcher";

QHash< QString, int > Verilook::FaceTemplate::s_slotCounts; // how many images per slot?
QHash< QString, QHash< int, Verilook::FaceTemplate::Ptr> > Verilook::FaceTemplate::s_slots;
QDir Verilook::FaceTemplate::s_newFacesDir;

class GrayTable : public QVector<QRgb> {
public:
    GrayTable()
        : QVector<QRgb>(256)
    {
        for(int i=0;i<256;++i) {
            operator[](i) = qRgb(i,i,i);
        }
    }
};

static const GrayTable grayTable;

QImage toGrayScale(const QImage& img)
{
    QImage gray(img.size(), QImage::Format_Indexed8);
    gray.setColorTable(grayTable);
    for(int i=0;i<img.width(); ++i)
        for(int j=0; j<img.height(); ++j)
            gray.setPixel(i,j,qGray(img.pixel(i,j)));
    return gray;
}

Verilook::Verilook(QObject * parent)
    : QThread(parent)
    , m_extractor(0), m_matcher(0)
{
    NBool available;

    Q_ASSERT( isOk( NLicenseObtainA( s_defaultServer, s_defaultPort, s_licenseList, &available),
                    "NLicenseObtain failed") );

    if ( available) {
        Q_ASSERT( isOk(NleCreate(&m_extractor), "No verilook extractor created", "Verilook extractor created") );

        setQualityThreshold(1);

        Q_ASSERT( isOk(NMCreate(&m_matcher), "No verilook matcher created", "Verilook matcher created"));
        // relax matcher parameters...

        NInt v = 1;
        NMSetParameter( m_matcher, NM_PART_NONE, NMP_MATCHING_THRESHOLD, &v );
    } else {
        qFatal("Failed to obtain verilook license");
    }
}

Verilook::~Verilook() {
    if (m_extractor) {
        NleFree(m_extractor);
        m_extractor = 0;
    }
    if (m_matcher) {
        NMFree(m_matcher);
        m_matcher = 0;
    }
    isOk( NLicenseRelease(s_licenseList), "NLicenseRelease failed", "License successfully released" );
}

QString Verilook::errorString(NResult result)
{
    NInt length;
    NChar* message;
    length = NErrorGetDefaultMessage(result, NULL);
    message = (NChar*) malloc(sizeof(NChar) * (length + 1));
    NErrorGetDefaultMessage(result, message);
    QString qmessage((char*)message);
    free(message);
    return qmessage;
}

bool Verilook::isOk(NResult result,
                 QString errorSuffix,
                 QString successMessage) {
    if (NFailed(result)) {
        qCritical()
                << _s(QString("VLERR(%1): %2.%3")
                   .arg(result)
                   .arg(errorString(result))
                   .arg( !errorSuffix.isEmpty()
                        ? (" " + errorSuffix + ".") : ""));
        return false;
    } else {
        if (!successMessage.isEmpty())
            qDebug() << _s(successMessage);
        return true;
    }
}

bool larger(const QRect& a, const QRect& b)
{
    return a.width()*a.height() > b.width()*b.height();
}

void Verilook::findFaces(const QImage& frame, QList<QRect>& faces)
{
    if (m_extractor) {
        HNImage img;

        QImage greyFrame = toGrayScale(frame);

        if ( !isOk( NImageCreateWrapper(
                       npfGrayscale,
                       greyFrame.width(), greyFrame.height(), greyFrame.bytesPerLine(),
                       0.0, 0.0, (void*)greyFrame.bits(), NFalse, &img),
                   "Coudn't wrap matrix for verilook"))
            return;

        /* detect faces */
        int faceCount = 0;
        NleFace * vlFaces;
        NleDetectFaces( m_extractor, img, &faceCount, &vlFaces);

        // convert to rectangles
        for(int i = 0; i<faceCount; ++i) {
            faces.push_back( QRect(vlFaces[i].Rectangle.X,
                                   vlFaces[i].Rectangle.Y,
                                   vlFaces[i].Rectangle.Width,
                                   vlFaces[i].Rectangle.Height));
        }
        NFree(vlFaces);
    }
    qSort(faces.begin(), faces.end(), larger);
}

void Verilook::setMinIOD(int value)
{
    if (!m_extractor) return;
    NInt v = (NInt)value;
    NleSetParameter( m_extractor, NLEP_MIN_IOD, (const void *)&v );
}

void Verilook::setMaxIOD(int value)
{
    if (!m_extractor) return;
    NInt v = (NInt)value;
    NleSetParameter( m_extractor, NLEP_MAX_IOD, (const void *)&v );
}

void Verilook::setConfidenceThreshold(double value)
{
    if (!m_extractor) return;
    NDouble v = (NDouble)value;
    NleSetParameter( m_extractor, NLEP_FACE_CONFIDENCE_THRESHOLD, (const void *)&v );
}

void Verilook::setQualityThreshold(int value)
{
    if (!m_extractor) return;
    NByte v = (NByte)value;
    NleSetParameter( m_extractor, NLEP_FACE_CONFIDENCE_THRESHOLD, (const void *)&v );
}


void Verilook::addDbFace(const QString& imgPath)
{
    QFileInfo fi(imgPath);
    Q_ASSERT( fi.exists() );
    QString tplPath = fi.path() + "/" + fi.baseName() + ".tpl";
    if (QFileInfo(tplPath).exists()) {
        // load from file
        QFile tplFile(tplPath);
        tplFile.open(QFile::ReadOnly);
        m_templates.push_front(FaceTemplate::Ptr(new FaceTemplate(imgPath, tplFile.readAll())));
        emit faceAdded(imgPath);
    } else {
        HNImage image, greyscale;
        NleDetectionDetails details;
        HNLTemplate tpl = 0;
        NleExtractionStatus extrStatus;

        if (isOk(NImageCreateFromFile(imgPath.toLocal8Bit(), NULL, &image))) {

            if (isOk(NImageCreateFromImage(npfGrayscale, 0, image, &greyscale))) {

                NResult result;

                result = NleExtract(
                            m_extractor,
                            greyscale,
                            &details,
                            &extrStatus,
                            &tpl);

                if (isOk(result) && (extrStatus == nleesTemplateCreated)) {
                    FaceTemplate::Ptr face( new FaceTemplate( imgPath, compressTemplate(tpl)) );
                    saveTemplate(face);
                    // free uncompressed template
                    NLTemplateFree(tpl);
                } else {
                    if (tpl != 0)
                        qWarning("Leaking a templ that allegedly wasn't loaded");
                }
                NImageFree(greyscale);
            }
            NImageFree(image);
        }
    }
}

void Verilook::scrutinize(const QImage &image)
{
    HNImage img;

    QImage cropped;
    QImage greyFrame = toGrayScale(image);

    if ( !isOk( NImageCreateWrapper(
                   npfGrayscale,
                   greyFrame.width(), greyFrame.height(), greyFrame.bytesPerLine(),
                   0.0, 0.0, (void*)greyFrame.bits(), NFalse, &img),
               "Coudn't wrap QImage in verilook's HNImage"))
        return;

    /* detect a face */
    NleFace face;
    NBool detected = false;
    isOk( NleDetectFace( m_extractor, img, &detected, &face), "Error during face detection" );

    if (!detected) return;

    cropped = cropAroundFace(image, QRect(face.Rectangle.X, face.Rectangle.Y, face.Rectangle.Width, face.Rectangle.Height));
    emit incomingFace(cropped);

    NleDetectionDetails details;
    NleExtractionStatus status;
    HNLTemplate tpl = 0;

    Q_ASSERT( isOk( NleExtract(m_extractor, img, &details, &status, &tpl)));
    if (status != nleesTemplateCreated) {
        emit noMatchFound();
        return;
    }

    // extract template data
    NSizeType maxSize, size;
    QByteArray memTpl;
    Q_ASSERT( isOk(NLTemplateGetSize(tpl, 0, &maxSize)) );
    memTpl.resize(maxSize);
    Q_ASSERT( isOk(NLTemplateSaveToMemory( tpl, memTpl.data(), memTpl.size(), 0, &size)));

    NMMatchDetails * m_details = 0;
    Q_ASSERT( isOk(NMIdentifyStart( m_matcher, memTpl.data(), memTpl.size(), &m_details),
                   "Couldn't start matching") );

    FaceTemplate::Ptr best;
    int bestScore = 0;
    QList< FaceTemplate::Ptr >::iterator iter = m_templates.begin();
    for(;iter != m_templates.end(); ++iter) {
        NInt score;
        FaceTemplate::Ptr dbFace(*iter);
        Q_ASSERT( isOk( NMIdentifyNext(m_matcher, dbFace->data().data(), dbFace->data().size(), m_details, &score)));
        // update best match if any...
        if (score > bestScore) {
            bestScore = score;
            best = dbFace;
        }
    }
    Q_ASSERT( isOk(NMIdentifyEnd(m_matcher)) );

    if (bestScore > 0) {
        FaceTemplate::Ptr face(new FaceTemplate( compressTemplate(tpl), best ));
        cropped.save( face->imgPath() );
        saveTemplate( face );
        emit identified( QString(face->slot()).replace('-',' '), face->ancestors() );
    } else
        emit noMatchFound();

    if (m_details)
        NMMatchDetailsFree(m_details);

    if (tpl)
        NLTemplateFree(tpl);
}

Verilook::FaceTemplate::FaceTemplate(const QString& imgPath, const QByteArray& data)
    : m_imgPath(imgPath)
    , m_data(data)
{

    QRegExp origRe("([^_]+)_(\\d+)"), newRe("([^_]+)_(\\d+)_(\\d+)_(\\d+)");

    QString fname = QFileInfo(m_imgPath).baseName();

    if (origRe.exactMatch( fname )) {
        m_slot = origRe.cap(1);
        m_id = origRe.cap(2).toInt();
        m_parentId = 0;
        m_gen = 0;
    } else if (newRe.exactMatch( fname )) {
        m_slot = newRe.cap(1);
        m_parentId = newRe.cap(2).toInt();
        m_gen = newRe.cap(3).toInt();
        m_id = newRe.cap(4).toInt();
    } else {
        qFatal("Unparseable filename: %s", m_imgPath.toLocal8Bit().constData());
    }

    if (!s_slotCounts.contains(m_slot) || (s_slotCounts[m_slot] < m_id))
        s_slotCounts[m_slot]=m_id;

    s_slots[m_slot][m_id] = Ptr(this);
}

Verilook::FaceTemplate::FaceTemplate( const QByteArray& data, const Ptr parent )
    : m_data(data)
{
    m_slot = parent->m_slot;
    m_parentId = parent->m_id;
    m_gen = parent->m_gen + 1;
    m_id = ++s_slotCounts[m_slot];
    m_imgPath = s_newFacesDir.filePath( QString("%1_%2_%3_%4.jpg").arg(m_slot).arg(m_parentId).arg(m_gen).arg(m_id) );
    s_slots[m_slot][m_id] = Ptr(this);
}


void Verilook::setNewFacesDir(const QString &path)
{
    FaceTemplate::s_newFacesDir = QDir(path);
    Q_ASSERT( FaceTemplate::s_newFacesDir.exists() );
}

QImage Verilook::cropAroundFace(const QImage &orig, const QRect &face)
{
    int w = Bio::CROP_WIDTH_SCALE * face.width(), h = Bio::CROP_RATIO * w;
    int dw = w - face.width(), dh = h - face.height();

    return orig.copy( face.x()-dw/2, face.y()-dh/2, w, h );
}

void Verilook::saveTemplate( const FaceTemplate::Ptr face )
{
    face->save();
    m_templates.push_front( face );
    emit faceAdded( face->imgPath() );
}

QByteArray Verilook::compressTemplate(HNLTemplate tpl)
{
    // compress
    HNLTemplate compTemplate;

    Q_ASSERT( isOk(NleCompressEx(tpl, nletsSmall, &compTemplate)) );

    // get the size of the template
    NSizeType maxSize;
    Q_ASSERT( isOk(NLTemplateGetSize(compTemplate, 0, &maxSize)) );

    // transform to byte array
    NSizeType size;
    QByteArray bytes(maxSize, 0);

    Q_ASSERT( isOk(NLTemplateSaveToMemory(compTemplate, bytes.data(), maxSize, 0, &size)));
    bytes.truncate(size);

    return bytes;
}

void Verilook::FaceTemplate::save() const
{
    QFileInfo fi(m_imgPath);
    QString tplPath = QDir(fi.path()).filePath( fi.baseName() + ".tpl" );
    QFile tplFile(tplPath);
    tplFile.open(QFile::WriteOnly);
    tplFile.write( m_data );
}

QStringList Verilook::FaceTemplate::ancestors() const
{
    QStringList lst;
    lst << m_imgPath;

    // now follow the ancestors tail...
    if (s_slots.contains(m_slot) && s_slots[m_slot].contains(m_parentId))
        lst.append( s_slots[m_slot][m_parentId]->ancestors() );
    return lst;
}

