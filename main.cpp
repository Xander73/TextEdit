#include "mainwindow.h"
#include "Find.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setApplicationName("Text Editor");
    QApplication::setApplicationVersion("1");
    QApplication::setOrganizationName("No organization name");

    MainWindow txt;

    txt.resize(700,400);
    txt.show();

    int res = a.exec();

    txt.saveSetting();


    return res;
}
