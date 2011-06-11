#include "Verilook.h"

#include <QtDebug>
#include <QImage>
#include <QFileInfo>

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
        Q_ASSERT( isOk(NMCreate(&m_matcher), "No verilook matcher created", "Verilook matcher created"));
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
        m_templates.push_back(
                    FaceTemplatePtr(
                        new FaceTemplate(imgPath, tplPath,
                                         tplFile.readAll())));
    } else {
        HNImage image, greyscale;
        NleDetectionDetails details;
        HNLTemplate tpl;
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
                    // compress
                    HNLTemplate compTemplate;

                    if (isOk(NleCompressEx(tpl, nletsSmall, &compTemplate))) {

                        // free uncompressed template
                        NLTemplateFree(tpl);

                        // get the size of the template
                        NSizeType maxSize;
                        if (isOk(NLTemplateGetSize(compTemplate, 0, &maxSize))) {

                            // transform to byte array
                            NSizeType size;
                            QByteArray bytes(maxSize, 0);

                            if (isOk(NLTemplateSaveToMemory(compTemplate,
                                                            bytes.data(), maxSize,
                                                            0, &size))) {
                                bytes.truncate(size);
                                // save compressed template to file
                                QFile tplFile(tplPath);
                                tplFile.open(QFile::WriteOnly);
                                tplFile.write( bytes );

                                m_templates.push_back(
                                            FaceTemplatePtr(
                                                new FaceTemplate(
                                                    imgPath,
                                                    tplPath,
                                                    bytes)) );
                            }
                        }
                    }

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
    QList<QRect> faces;
    findFaces(image, faces);
    if (faces.size() > 0) {
        emit incomingFace(image, faces[0]);
    }
}
