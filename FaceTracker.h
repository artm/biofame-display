#ifndef FACETRACKER_H
#define FACETRACKER_H

#include <QObject>
#include <QImage>

#include <NCore.h>
#include <NLExtractor.h>
#include <NLicensing.h>

class FaceTracker : public QObject {
    Q_OBJECT
public:
    explicit FaceTracker(QObject * parent);
    virtual ~FaceTracker();
    void findFaces(const QImage& frame, QList<QRect>& faces);
    bool usesVerilook() const { return m_extractor != 0; }

public slots:
    /* verilook specific */
    void setMinIOD(int value);
    void setMaxIOD(int value);
    void setConfidenceThreshold(double value);
    void setQualityThreshold(int value);

private:
    HNLExtractor m_extractor;

    static QString errorString(NResult result);
    static bool isOk(NResult result,
                     QString errorSuffix = QString(),
                     QString successMessage = QString());
};


#endif // FACETRACKER_H
