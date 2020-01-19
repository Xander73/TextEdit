#include <QList>

#include "MyThread.h"

MyThread::MyThread()
{

}

void MyThread::run()
{
    mTextEdit.moveCursor(QTextCursor::Start)
    QList <QTextEdit::ExtraSelection> extraSelections;

    //thread start

    while(mTextEdit.find(strReceivedSearchText))
    {
        QTextEdit::ExtraSelection extra;
        extra.format.setBackground(QColor(Qt::yellow).lighter(140));
        //mptxt->setTextColor(Qt::white);
        extra.cursor = mTextEdit.textCursor();
        extraSelections.append(extra);
        emit highLighting (extraSelections);
    }
   // mptxt->setExtraSelections(extraSelections);
}

void MyThread::slotThHighlighting (QList<QString> listTexts)
{
    mTextEdit.setText((strReceiveMainText = listTexts[0]));
    strReceivedSearchText = listTexts[1];



}
