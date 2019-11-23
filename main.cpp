#include "mainwindow.h"
#include "Find.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow txt;

    txt.resize(700,400);
    txt.show();

    int res = a.exec();

    txt.saveSetting();


    return res;
}
