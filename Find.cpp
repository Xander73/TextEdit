#include "Find.h"
#include "ui_Find.h"

Find::Find(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Find)
{
    ui->setupUi(this);
    ui->mpFLabel->setBuddy(ui->mpFLineEdit);

    setFixedSize(315, 74);
    setWindowTitle(tr("Найти"));
    setWindowFlag(Qt::WindowStaysOnTopHint);
    setWindowModality(Qt::ApplicationModal);
    setWindowFlag(Qt::WindowDoesNotAcceptFocus);

    connect (ui->cmdCancel, &QPushButton::clicked, this, &Find::slotExit);
}

Find::~Find()
{
    delete ui;
}

void Find::closeEvent(QCloseEvent *event)
{
    emit signalExit();
}

void Find::slotExit()
{
    this->close();
    ui->mpFLineEdit->clear();
}

QPushButton* Find::getCmdFind ()
{
    return ui->cmdFind;
}
//void Find::getCancel ()
//{
//    if (ui->cmdCancel->clicked())
//        emit signalExit();
//}
QLineEdit* Find::getFLineEdit()
{
    return ui->mpFLineEdit;
}
