#include <QtGui/QApplication>
#include "BioDisplay.h"
#include "LogHub.h"

int main(int argc, char *argv[])
{
    LogHub lh("BioFame");

    QApplication a(argc, argv);
    BioDisplay w;
    w.show();
    return a.exec();
}
