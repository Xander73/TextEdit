#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QtCore/qglobal.h>
#include <QString>
#include <QObject>
#include <QTextEdit>
#include <QList>
#include <QDebug>

#include <QMetaType>

class Highlighter : public QObject
{
    Q_OBJECT

public:
    Highlighter(QString, QString, QObject *parent = nullptr);    
    QList <QTextEdit::ExtraSelection> extraSelections;

    ~Highlighter ()
    {
        delete mpText;
        qDebug()<<"Hihlighter destructor";
    }

public slots:
    void slotHighlightering ();
    void slotExit();
signals:
    void signalSendHighlighter(QList<QTextEdit::ExtraSelection>);
    void signalFinished(QList<QTextEdit::ExtraSelection>);


private:
    QString mstrSubString;
    QString mstrMainText;
    QTextEdit *mpText;
};

Q_DECLARE_METATYPE(QTextEdit::ExtraSelection)
Q_DECLARE_METATYPE(QTextCharFormat)

#endif // HIGHLIGHTER_H
