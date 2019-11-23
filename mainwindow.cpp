#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Find.h"

#include <QClipboard>
#include <QTextStream>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QDebug>
#include <QTextCodec>
#include <QTextCursor>
#include <QColor>
#include <QDial>
#include <QList>
#include <QCoreApplication>
#include <QStyle>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QEvent>
#include <QSettings>
#include <QTextDocumentWriter>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui (new Ui::MainWindow)
{
    ui->setupUi(this);
    mptxt = new QTextEdit(this);
    mpStyleCmbx = new QComboBox(); //text style
    mpSizeCmbx = new QComboBox();
    sizeText = 12;
    mFind = new Find;
    setCentralWidget(mptxt);
    pal = mptxt->palette();


    //menu and tool bar
    menuFile();
    menuEdit();
    menuText();
    ui->mainToolBar->hide();

    QMenu* help = new QMenu(tr("Помощь"));
    ui->menuBar->addMenu(help);    

    connect(mptxt->document(), &QTextDocument::modificationChanged,
            actionSave, &QAction::setEnabled);
    connect(mptxt->document(), &QTextDocument::modificationChanged,
            this, &QWidget::setWindowModified);
    connect(mptxt->document(), &QTextDocument::undoAvailable,
            actionUndo, &QAction::setEnabled);
    connect(mptxt->document(), &QTextDocument::redoAvailable,
            actionRedo, &QAction::setEnabled);

    actionUndo->setEnabled(mptxt->document()->isUndoAvailable());
    actionRedo->setEnabled(mptxt->document()->isRedoAvailable());
    setWindowModified(mptxt->document()->isModified());
    actionSave->setEnabled(mptxt->document()->isModified());

#ifndef NO_CLIP_BOARD
    actionCopy->setEnabled(mptxt->document()->isModified());
    connect(mptxt, &QTextEdit::copyAvailable, actionCopy, &QAction::setEnabled);
    actionCut->setEnabled(mptxt->document()->isModified());
    connect(mptxt, &QTextEdit::copyAvailable, actionCut, &QAction::setEnabled);
#endif

    //context menu
    mptxt->setContextMenuPolicy(Qt::CustomContextMenu);    
    connect(mptxt, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotCustomMenuRequested(QPoint)));

    //connects
    connect (mFind->getCancel(), SIGNAL (clicked()), this, SLOT (slotFCancel()));
    connect (mFind->getCmdFind(), SIGNAL (clicked()), this, SLOT (slotFind()));

    if (!currentPath.isEmpty()) {
        QFileInfo f (currentPath);
        setWindowTitle(f.baseName());
    } else {
        setWindowTitle("New document");
    }

    slotOpen();
    //load old settings
    loadSetting();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotCustomMenuRequested(QPoint pos)
{
    QMenu* mpcMenu = new QMenu (this);
    mpcMenu->addAction(QIcon("://resourses//icons//cut.png"),tr("Вырезать"), mptxt, SLOT(cut()));
    mpcMenu->addAction(QIcon("://resourses//icons//copy.png"),tr("Копировать"), mptxt, SLOT(copy()));
    mpcMenu->addAction(QIcon("://resourses//icons//paste.png"),tr("Вставить"), mptxt, SLOT(paste()));
    mpcMenu->addSeparator();

    mpcMenu->popup(mptxt->viewport()->mapToGlobal(pos));
}


void MainWindow::menuFile()
{
    QMenu* m = menuBar()->addMenu(tr("Файл"));
    QToolBar* tb =addToolBar(tr("File")) ;
    QAction* a = m->addAction(QIcon("://resourses////icons//new.png"), tr("Новый файл"), this, SLOT(slotNewFile()));
    tb->addAction(a);
    a->setPriority(QAction::LowPrioriity);
    a->setShortcut(QKeySequence::New);

    a = m->addAction(QIcon(":/resourses/icons/open.png"), tr("Открыть"), this, SLOT(slotOpen()));
    tb->addAction(a);
    a->setPriority(QAction::LowPrioriity);
    a->setShortcut(QKeySequence::Open);

    m->addSeparator();

    actionSave =m->addAction(QIcon(":/resourses/icons/save.png"), tr("&Сохранить"), this, SLOT (slotSave()));
    tb->addAction(actionSave);
    //a->setPriority(QAction::LowPrioriity);
    actionSave->setShortcut(QKeySequence::Save);

    a=m->addAction(QIcon(":/resourses/icons/save_ass.png"), tr("Сохранить &как..."), this, SLOT(slotSaveAs()));
    a->setPriority(QAction::LowPrioriity);

    m->addSeparator();

    a=m->addAction(QIcon(":/resourses/icons/exit.png"), tr("&Закрыть"), this, &MainWindow::close);
    tb->addAction(a);
}

void MainWindow::menuEdit()
{
    QMenu* m = menuBar()->addMenu(tr("&Правка"));
    QToolBar* tb = addToolBar(tr("Edit"));
    QAction* a;
    actionUndo= m->addAction(QIcon(":/resourses/icons/undo.png"), tr("Отменить"), mptxt, &QTextEdit::undo);
    tb->addAction(actionUndo);
    actionUndo->setShortcut(QKeySequence::Undo);

    actionRedo=m->addAction(QIcon(":/resourses/icons/redo.png"), tr("Повторить"), mptxt, &QTextEdit::redo);
    tb->addAction(actionRedo);
    actionRedo->setShortcut(QKeySequence::Redo);
    m->addSeparator();

#ifndef QT_NO_CLIPBOARD
    actionCut = m->addAction(QIcon(":/resourses/icons/cut.png"), tr("Вырезать"), mptxt, &QTextEdit::cut);
    tb->addAction(actionCut);
    actionCut->setShortcut(QKeySequence::Cut);

    actionCopy=m->addAction(QIcon(":/resourses/icons/copy.png"), tr("&Копировать"), mptxt, &QTextEdit::copy);
    tb->addAction(actionCopy);
    actionCopy->setShortcut(QKeySequence::Copy);

    a=m->addAction(QIcon(":/resourses/icons/paste.png"), tr("&Вставить"), mptxt, &QTextEdit::paste);
    tb->addAction(a);
    a->setShortcut(QKeySequence::Paste);
#endif

    a=m->addAction(QIcon(":/resourses/icons/select_all.png"), tr("Выделить все"), mptxt, &QTextEdit::selectAll);
    a->setShortcut(QKeySequence::SelectAll);

    a=m->addAction(QIcon(":/resourses/icons/search.png"), tr("Найти"), this, &MainWindow::slotSearch);

}

void MainWindow::menuText()
{
    QMenu* m = menuBar()->addMenu(tr("Формат"));
    QToolBar* tb = addToolBar(tr("Format"));
    actionBold = m->addAction(QIcon(":/resourses/icons/bold.png"), tr("Жирный"), this, &MainWindow::slotBold);
    tb->addAction(actionBold);
    actionBold->setCheckable(true);
    QFont bold;
    bold.setBold(true);
    actionBold->setFont(bold);
    actionBold->setShortcut(QKeySequence::Bold);

    actionItalic=m->addAction(QIcon(":/resourses/icons/italic.png"), tr("Курсив"), this, &MainWindow::slotItalic);
    QFont italic;
    italic.setItalic(true);
    actionItalic->setFont(italic);
    tb->addAction(actionItalic);
    actionItalic->setCheckable(true);
    actionItalic->setShortcut(QKeySequence::Italic);

    actionUnderLine=m->addAction(QIcon(":/resourses/icons/underline.png"), tr("Подчеркнутый"), this, &MainWindow::slotUnderLine);
    tb->addAction(actionUnderLine);
    actionUnderLine->setCheckable(true);
    QFont underLine;
    underLine.setUnderline(true);
    actionUnderLine->setFont(underLine);
    actionUnderLine->setShortcut(QKeySequence::Underline);

}

void MainWindow::slotNewFile()
{
    slotSave();
    mptxt->clear();
    currentPath = "";
    ui->statusBar->showMessage(tr("New file"));
}

void MainWindow::slotOpen()
{
    //currentPath = QFileDialog::getOpenFileName(nullptr, tr("Open File"),"C:\\Users\\Саша\\Desktop", "");
    QFile file (currentPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::WriteOnly)) {
        qDebug()<<tr("File is not open for read");
    } else {
        QTextStream stream (&file);
        stream.setCodec(QTextCodec::codecForName("Window-1251"));
        mptxt->setText(stream.readAll());
        ui->statusBar->showMessage(tr("File open"));
    }
    file.close();

}

bool MainWindow::slotSave()
{
    QFile file(currentPath);
    if (!file.open(QIODevice::WriteOnly)) {
        slotSaveAs();
    } else {
        QTextStream stream (&file);
        stream<<mptxt->toPlainText();
    }
    file.close();
    ui->statusBar->showMessage(tr("File saved"));



//    if (fileName.isEmpty())
//        return slotSaveAs();
//    if (fileName.startsWith(QStringLiteral(":/")))
//        return slotSaveAs();

//    QTextDocumentWriter writer(fileName);
//    bool success = writer.write(mptxt->document());
//    if (success) {
//        mptxt->document()->setModified(false);
//        statusBar()->showMessage(tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName)));
//    } else {
//        statusBar()->showMessage(tr("Could not write to file \"%1\"")
//                                 .arg(QDir::toNativeSeparators(fileName)));
//    }
    return true;
}

bool MainWindow::slotSaveAs()
{
    QStringList strLst {"*.txt", "*.pdf"};
    QString fileSave = QFileDialog::getSaveFileName(nullptr, tr("Save file"), "","*.txt;; *.pdf");
    QFile file (fileSave);
    if(!file.open(QIODevice::WriteOnly)) {
            qDebug()<<tr("File is not open to Save ass");
    } else {
        QTextStream stream (&file);
        stream<<mptxt->toPlainText();
    }
    file.close();
    ui->statusBar->showMessage(tr("File saved"));
    return true;
}

//void MainWindow::saveTxt()
//{

//}

//void MainWindow::savePdf()
//{
//    QTextEdit::fi
//}



void MainWindow::slotSearch()
{
    mFind->show();
}

void MainWindow::slotFind()
{
//    QList <QTextEdit::ExtraSelection> extraSelections;
//    while(mptxt->find(mFind->getLEText()))
//            {
//                QTextEdit::ExtraSelection extra;
//                extra.format.setBackground(QColor(Qt::cyan).lighter(80));
//                mptxt->setTextColor(Qt::white);
//                extra.cursor = mptxt->textCursor();
//                extraSelections.append(extra);
//                mptxt->setExtraSelections(extraSelections);

//            }
//    mptxt->setExtraSelections(extraSelections);
//    mptxt->moveCursor(QTextCursor::Start);

    //mptxt->find(mFind->getFLineEdit());
   // mptxt->setPalette()

    mptxt->find(mFind->getFLineEdit());
    pal = mptxt->palette();
    for (int colorRole=0; colorRole<QPalette::NColorRoles; ++colorRole)
        pal.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(colorRole) , pal.color(QPalette::Active, static_cast<QPalette::ColorRole>(colorRole)));
    qDebug()<<mFind->getFLineEdit();
    mptxt->setPalette(pal);
    //QWidget::activateWindow();
    //setWindowFlag(Qt::WindowStaysOnTopHint);


}

void MainWindow::slotFCancel()
{
    mFind->close();
}

void MainWindow::saveSetting()
{
    QSettings settings ("settings.conf", QSettings::IniFormat);
    settings.beginGroup("textSettings");
    settings.setValue ("text", currentPath);
    settings.endGroup();
    ui->statusBar->showMessage("Settings save");
}

void MainWindow:: loadSetting()
{
    QSettings settings ("settings.conf", QSettings::IniFormat);
    QFile file (currentPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::WriteOnly)) {

        qDebug()<<tr("File is not open for read");
    } else {
        QTextStream stream (&file);
        stream.setCodec(QTextCodec::codecForName("Window-1251"));
        mptxt->setText(stream.readAll());
        ui->statusBar->showMessage(tr("File open"));
    }
    file.close();
}

void MainWindow::slotItalic()
{
    QTextCharFormat fmt;
    fmt.setFontItalic(actionItalic->isChecked());
    setCharFormat(fmt);

}

void MainWindow::slotBold()
{
//    mptxt->setF
}

void MainWindow::slotUnderLine()
{
    mptxt->fontUnderline() ? mptxt->setFontUnderline(false) :
                             mptxt->setFontUnderline(true);
}

void MainWindow::setCharFormat(const QTextCharFormat &fmt)
{
    QTextCursor cursor = mptxt->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(fmt);
    mptxt->mergeCurrentCharFormat(fmt);

}

 void MainWindow::positionChanged (const QFont &font)
 {
    actionItalic->setChecked(font.italic());
    actionBold->setChecked(font.bold());
    actionUndo->setChecked(font.underline());
 }


