#include <QtGlobal>

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#if QT_VERSION >= 0x050000
#include <QtWidgets/QMainWindow>
#else
#include <QtGui/QMainWindow>
#endif

#include <QAction>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QFontComboBox>
#include <QMenuBar>
#include <QMenu>
#include <QPoint>
#include <QPrinter>
#include <QString>
#include <QTextEdit>

#include "Find.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);    
    QTextEdit* mptxt;
    Find* mFind;

    void saveSetting();

    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QString currentPath;    //open file path
    QPalette pal;           //palette to highlight the found string
    QFontComboBox*  mpStyleFontCmbx; //text style
    QComboBox* mpSizeCmbx;
    QComboBox* mpCodecCmbx;
    QString fileName;

        // menu bar and tool bar
    void menuFile();
    void menuEdit();
    void menuText();

    void setCharFormat(const QTextCharFormat& fmt);

    void loadSetting();


    //actions
    QAction* actionUndo;
    QAction* actionRedo;
    QAction* actionSave;
    QAction* actionCut;
    QAction* actionCopy;
    QAction* actionPaste;
    QAction* actionRemove;
    QAction* actionBold;
    QAction* actionItalic;
    QAction* actionUnderLine;
    QAction* actionAlignCenter;
    QAction* actionAlignLeft;
    QAction* actionAlignRight;
    QAction* actionAlignJustify;



private slots:
    void slotCustomMenuRequested(QPoint pos);
    void slotNewFile();
    void slotOpen();
    void slotLoad(const QString &path);
    bool slotSave();
    bool slotSaveAs();
    void slotSearch();
    void slotFind();
    void slotFCancel();
    void slotBold();
    void slotItalic();    
    void slotUnderLine();
    void slotChangeCurrentText (const QFont &font);  //set the format of the current position
    void slotChangeCurrentPosition();
    void slotAlign(QAction*);
    void slotSetTextSize(const QString&);
    void slotTextStyle(const QString &);
    void slotSaveTxt ();
    void slotSavePdf ();
    void slotSaveODT ();
    void slotSaveHTML ();
    void setCodec(QString newCodec);
    void slotChangedDocument();
    void closeEvent(QCloseEvent* e) override;
    void slotRemove();
    void slotOpenHtml ();
    void slotOpenPlainText ();
    //-------------
//
//



};

#endif // MAINWINDOW_H
