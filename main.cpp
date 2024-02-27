#include "photo_comparator.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Photo_Comparator w;
    w.show();
    return a.exec();
}
