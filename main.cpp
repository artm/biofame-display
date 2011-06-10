#include <QtGui/QApplication>
#include "BioDisplay.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BioDisplay w;
    w.show();

    return a.exec();
}
