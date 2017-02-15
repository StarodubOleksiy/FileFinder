#include "filefinder.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FileFinder w;
    w.show();

    return a.exec();
}
