#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QList>
#include <QString>
#include <QTextEdit>
#include <QThread>

#include "mainwindow.h"



class MyThread : public QThread
{
    Q_OBJECT
public:
    explicit MyThread();



public slots:
    void run() override;
    void slotThHighlighting(QList<QString>);

signals:
    void highLighting (QList <QTextEdit::ExtraSelection>);

private:
    QString strReceiveMainText;     //text from main TextEdit (List[0])
    QString strReceivedSearchText;     //text from Find->LineEdit (List[1])
    QTextEdit mTextEdit;

};

#endif // MYTHREAD_H
