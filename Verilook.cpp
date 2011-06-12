#include "Verilook.h"
#include "CommonTypes.h"

#include <QtDebug>
#include <QImage>
#include <QFileInfo>
#include <QRegExp>
#include <QDir>

#include <NMatcherParams.h>

const char * s_defaultPort = "5000";
const char * s_defaultServer = "/local";
const char * s_licenseList = "SingleComputerLicense:VLExtractor,SingleComputerLicense:VLMatcher";

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

    if ( isOk( NLicenseObtainA( s_defaultServer, s_defaultPort, s_licenseList, &available),
               "NLicenseObtain failed")
         && available) {
        Q_ASSERT( isOk(NleCreate(&m_extractor), "No verilook extractor created", "Verilook extractor created") );

        setQualityThreshold(1);

        Q_ASSERT( isOk(NMCreate(&m_matcher), "No verilook matcher created", "Verilook matcher created"));
        // relax matcher parameters...

        NInt v = 1;
        NMSetParameter( m_matcher, NM_PART_NONE, NMP_MATCHING_THRESHOLD, &v );
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
                << QString("VLERR(%1): %2.%3")
                   .arg(result)
                   .arg(errorString(result))
                   .arg( !errorSuffix.isEmpty()
                        ? (" " + errorSuffix + ".") : "");
        return false;
    } else {
        if (!successMessage.isEmpty())
            qDebug() << successMessage;
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
        m_templates.push_back( FaceTemplatePtr( new FaceTemplate(imgPath, tplPath, tplFile.readAll())));
        markSlot(imgPath);
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
                    saveTemplate(imgPath, tplPath, tpl);
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

Verilook::FaceTemplate::FaceTemplate(const QString &imgPath, const QString &tplPath, const QByteArray &data)
    : m_imgPath(imgPath)
    , m_tplPath(tplPath)
    , m_data(data)
{
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

    FaceTemplatePtr best;
    int bestScore = 0;
    QList< FaceTemplatePtr >::iterator iter = m_templates.begin();
    for(;iter != m_templates.end(); ++iter) {
        NInt score;
        FaceTemplatePtr dbFace(*iter);
        Q_ASSERT( isOk( NMIdentifyNext(m_matcher, dbFace->m_data.data(), dbFace->m_data.size(), m_details, &score)));
        // update best match if any...
        if (score > bestScore) {
            bestScore = score;
            best = dbFace;
        }
    }
    Q_ASSERT( isOk(NMIdentifyEnd(m_matcher)) );

    if (bestScore > 0) {
        emit identified( best->m_imgPath );
        saveToSlot( slotName(best->m_imgPath), cropped, tpl );
    } else
        emit noMatchFound();

    if (m_details)
        NMMatchDetailsFree(m_details);

    if (tpl)
        NLTemplateFree(tpl);
}

QString Verilook::slotName(const QString &path, int * num)
{
    QRegExp unnum("\\d+$");

    if (num && unnum.indexIn(path) > -1)
        *num = unnum.cap(0).toInt();

    return QFileInfo(path).baseName().remove(unnum);
}

void Verilook::markSlot(const QString &imgPath)
{
    int num = 0;
    QString slot = slotName(QFileInfo(imgPath).baseName(),&num);
    if (!m_slotCounts.contains(slot) || (m_slotCounts[slot] < num))
        m_slotCounts[slot]=num;
}

void Verilook::saveToSlot(const QString &slot, QImage image, HNLTemplate tpl )
{
    QString basepath = m_newFacesDir.filePath( QString("%1%2").arg(slot).arg(++m_slotCounts[slot]) );
    QString imgPath = basepath + ".jpg", tplPath = basepath + ".tpl";
    image.save( imgPath );
    // save the template
    // can't rely on loading here - too much cropping breaks the extraction.
    saveTemplate( imgPath, tplPath, tpl );
}

void Verilook::setNewFacesDir(const QString &path)
{
    m_newFacesDir = QDir(path);
    Q_ASSERT(m_newFacesDir.exists());
}

QImage Verilook::cropAroundFace(const QImage &orig, const QRect &face)
{
    int w = Bio::CROP_WIDTH_SCALE * face.width(), h = Bio::CROP_RATIO * w;
    int dw = w - face.width(), dh = h - face.height();

    return orig.copy( face.x()-dw/2, face.y()-dh/2, w, h );
}

void Verilook::saveTemplate(const QString &imgPath, const QString &tplPath, HNLTemplate tpl)
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
    // save compressed template to file
    QFile tplFile(tplPath);
    tplFile.open(QFile::WriteOnly);
    tplFile.write( bytes );

    // add to the database in memory as well
    m_templates.push_back( FaceTemplatePtr( new FaceTemplate( imgPath, tplPath, bytes)) );
    markSlot(imgPath);
    emit faceAdded(imgPath);
}

