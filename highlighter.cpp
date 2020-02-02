#include <QDebug>

#include "highlighter.h"
#include "Find.h"

Highlighter::Highlighter(QString strMain, QString strSub, QObject *parent) : QObject (parent), mstrSubString(strSub), mstrMainText(strMain)
{
    qRegisterMetaType<QList <QTextEdit::ExtraSelection>>();
    qRegisterMetaType<QTextCharFormat>();
    mpText = new QTextEdit ();
    mpText->setHtml(mstrMainText);
    qDebug()<<"highligh constructor";
}

void Highlighter::slotHighlightering()
{
    qDebug()<<"highlighter slot highlightering";
    QList <QTextEdit::ExtraSelection> extraSelections;
    qDebug()<<"highligh make extra selections";
    while(mpText->find(mstrSubString))
    {
        QTextEdit::ExtraSelection extra;
        extra.format.setBackground(QColor(Qt::yellow).lighter(140));    //backgroun color
        //mptxt->setTextColor(Qt::white);                               //text color
        extra.cursor = mpText->textCursor();
        extraSelections.append(extra);
    }
    qDebug()<<"highligh while ended";
    emit signalSendHighlighter(extraSelections);
    qDebug()<<"highligh emit";
    emit signalFinished();
}

void Highlighter::slotHighlighterFinish()
{
    mstrSubString = "";
    mstrMainText = "";
    mpText->close();
}



