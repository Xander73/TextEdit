#include <QtGlobal>


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#if QT_VERSION >= 0x050000
#include <QtWidgets/QMainWindow>
#else
#include <QtGui/QMainWindow>
#endif

#include <QContextMenuEvent>
#include <QMenu>
#include <QTextEdit>
#include <QMenuBar>
#include <QPoint>
#include <QString>
#include <QComboBox>
#include <QAction>

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

    QString currentPath {"C:\\Users\\Саша\\Desktop\\51244651.txt"};    //open file path
    QPalette pal;           //palette to highlight the found string
    QComboBox* mpStyleCmbx; //text style
    QComboBox* mpSizeCmbx;
    int sizeText;
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

    //===============
//
//    void slotTextSize(QString&);
//    void slotTextStyle(int);
//    void slotSaveTxt ();
//    void slotSavePdf ();
//    void slotQuit();
 //   void slotRemove();

};

#endif // MAINWINDOW_H
