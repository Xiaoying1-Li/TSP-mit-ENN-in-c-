#include "mywidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MyWidget w;
    w.setWindowTitle(QString("TSP-ENN"));
    w.show();
    return a.exec();
}
