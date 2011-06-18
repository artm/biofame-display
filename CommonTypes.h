#ifndef COMMONTYPES_H
#define COMMONTYPES_H

#include <QString>
#include <QDateTime>
#include <QImage>

namespace Bio {

const double CROP_RATIO = 3.0/2.0;
const double CROP_WIDTH_SCALE = 3.0/2.0;

inline const char * _s(const QString& s) { return s.toLocal8Bit().constData(); }

struct Portrait {
    QDateTime timestamp;
    QImage image;
};

}

#endif // COMMONTYPES_H
