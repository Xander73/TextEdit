#include "Find.h"
#include "ui_Find.h"

Find::Find(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Find)
{
    ui->setupUi(this);
    ui->mpFLabel->setBuddy(ui->mpFLineEdit);

    setFixedSize(315, 74);
    setWindowFlag(Qt::WindowStaysOnTopHint);
    setWindowModality(Qt::ApplicationModal);
    setWindowFlag(Qt::WindowDoesNotAcceptFocus);
}

Find::~Find()
{
    delete ui;
}

QPushButton* Find::getCmdFind ()
{
    return ui->cmdFind;
}
QPushButton* Find::getCancel ()
{
    return ui->cmdCancel;
}
QLineEdit* Find::getFLineEdit()
{
    return ui->mpFLineEdit;
}
