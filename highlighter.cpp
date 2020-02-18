#include <QDebug>

#include "highlighter.h"
#include "Find.h"

Highlighter::Highlighter(QString strMain, QString strSub, QObject *parent) : QObject (parent), mstrSubString(strSub), mstrMainText(strMain)
{
    qRegisterMetaType<QList <QTextEdit::ExtraSelection>>();
    qRegisterMetaType<QTextCharFormat>();
    mpText = new QTextEdit ();
    mpText->setHtml(mstrMainText);
    qDebug()<<"Highlighter constructor";

}

void Highlighter::slotHighlightering()
{
    while(mpText->find(mstrSubString))
    {
        QTextEdit::ExtraSelection extra;
        extra.format.setBackground(QColor(Qt::yellow).lighter(140));    //backgroun color
        //mptxt->setTextColor(Qt::white);                               //text color
        extra.cursor = mpText->textCursor();
        extraSelections.append(extra);
    }
    emit signalSendHighlighter(extraSelections);

}

void Highlighter::slotExit()
{
    for (QList<QTextEdit::ExtraSelection>::iterator  a = extraSelections.begin(); a!=extraSelections.end(); a++)
    {
        a->format.setBackground(QColor(Qt::white));
    }

    emit signalFinished(extraSelections);  //cancel higlighting
    mstrSubString = "";
    mstrMainText = "";
    mpText->close();
    extraSelections.clear();


}



