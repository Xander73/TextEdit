#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Find.h"
#include "highlighter.h"


#include <QActionGroup>
#include <QtAlgorithms>
#include <QClipboard>
#include <QCloseEvent>
#include <QColor>
#include <QCoreApplication>
#include <QDebug>
#include <QDial>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QMessageBox>
#include <QMimeData>
#include <QtPrintSupport/qprinter.h>
#include <QSettings>
#include <QStyle>
#include <QSyntaxHighlighter>
#include <QTextCodec>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QTextDocumentWriter>
#include <QTextStream>
#include <QThread>

#if defined(QT_PRINTSUPPORT_LIB)
#include <QtPrintSupport/qtprintsupportglobal.h>
#if QT_CONFIG(printer)
#if QT_CONFIG(printdialog)
#include <QPrintDialog>
#endif
#include <QPrinter>
#if QT_CONFIG(printpreviewdialog)
#include <QPrintPreviewDialog>
#endif
#endif
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui (new Ui::MainWindow)
{
    ui->setupUi(this);
    mptxt = new QTextEdit(this);
    mpStyleFontCmbx = new QFontComboBox();  //initialize text style
    mpSizeCmbx = new QComboBox();           //initialize combo box

    setCentralWidget(mptxt);

    pal = mptxt->palette();     //highliting the position founded

    //menu and tool bar
    menuFile();
    menuEdit();
    menuText();
    ui->mainToolBar->hide();  //standard tool bar close

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

    connect(mptxt->document(), &QTextDocument::modificationChanged,
            this, &MainWindow::slotChangedDocument);

    connect (mptxt, &QTextEdit::cursorPositionChanged, this, &MainWindow::slotChangeCurrentPosition);

    setWindowModified(mptxt->document()->isModified());

    actionUndo->setEnabled(mptxt->document()->isUndoAvailable());
    actionRedo->setEnabled(mptxt->document()->isRedoAvailable());
    actionSave->setEnabled(mptxt->document()->isModified());

#ifndef NO_CLIP_BOARD
    actionCopy->setEnabled(mptxt->document()->isModified());
    connect(mptxt, &QTextEdit::copyAvailable, actionCopy, &QAction::setEnabled);
    actionCut->setEnabled(mptxt->document()->isModified());
    connect(mptxt, &QTextEdit::copyAvailable, actionCut, &QAction::setEnabled);
#endif

    //context menu
    mptxt->setContextMenuPolicy(Qt::CustomContextMenu);    
    connect(mptxt, &QTextEdit::customContextMenuRequested, this, &MainWindow::slotCustomMenuRequested);

    //Find window
    mFind = new Find ();
    connect (mFind->getCmdFind(), &QPushButton::clicked, this, &MainWindow::slotFind);
    connect (mFind, &Find::signalExit, this, &MainWindow::slotFCancel);



    //the first window title
    if (!currentPath.isEmpty()) {
        slotChangedDocument();
    } else {
        fileName = "New document";
        setWindowTitle(fileName);
    }

    //loadSetting();
}

MainWindow::~MainWindow()
{
    saveSetting();
    delete ui;
}

void MainWindow::slotCustomMenuRequested(QPoint pos)
{
    QMenu* mpcMenu = new QMenu (this);
    mpcMenu->addAction(actionCut);
    mpcMenu->addAction(actionCopy);
    mpcMenu->addAction(actionPaste);
    mpcMenu->addAction(actionRemove);
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

    actionRedo=m->addAction(QIcon(":/resourses/icons/redo.png"), tr("Восстановить"), mptxt, &QTextEdit::redo);
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

    actionPaste=m->addAction(QIcon(":/resourses/icons/paste.png"), tr("&Вставить"), mptxt, &QTextEdit::paste);
    tb->addAction(actionPaste);
    actionPaste->setShortcut(QKeySequence::Paste);
    if (const QMimeData* md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());

#endif

    actionRemove = m->addAction(QIcon(":/resourses/icons/remove.png"), tr("Удалить"), this, &MainWindow::slotRemove);
    tb->addAction(actionRemove);


    m->addSeparator();
    a=m->addAction(QIcon(":/resourses/icons/select_all.png"), tr("Выделить все"), mptxt, &QTextEdit::selectAll);
    a->setShortcut(QKeySequence::SelectAll);

    a=m->addAction(QIcon(":/resourses/icons/search.png"), tr("Найти"), this, &MainWindow::slotSearch);
    a->setShortcut(QKeySequence::Find);

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

    m->addSeparator();
    tb->addSeparator();

    //---Alignment---

    actionAlignCenter = new QAction (QIcon(":/resourses/icons/align-center.png"), tr("Выравние по центру"), this);
    actionAlignCenter->setCheckable(true);
    actionAlignCenter->setPriority(QAction::LowPrioriity);
    actionAlignLeft   = new QAction (QIcon(":/resourses/icons/align-left.png"), tr("Выравнивание по левому краю"), this);
    actionAlignLeft->setCheckable(true);
    actionAlignLeft->setPriority(QAction::LowPrioriity);
    actionAlignRight  = new QAction (QIcon(":/resourses/icons/align-right.png"), tr("Выравние по правому краю"), this);
    actionAlignRight->setCheckable(true);
    actionAlignRight->setPriority(QAction::LowPrioriity);
    actionAlignJustify= new QAction (QIcon(":/resourses/icons/align-justify.png"), tr("Выравнивание по ширине"), this);
    actionAlignJustify->setCheckable(true);
    actionAlignJustify->setPriority(QAction::LowPrioriity);

    QActionGroup* alignGroup = new QActionGroup (this);
    connect (alignGroup, SIGNAL(triggered(QAction*)), this, SLOT(slotAlign(QAction*)));

    alignGroup->addAction(actionAlignLeft);
    alignGroup->addAction(actionAlignCenter);
    alignGroup->addAction(actionAlignRight);
    alignGroup->addAction(actionAlignJustify);

    m->addActions(alignGroup->actions());
    tb->addActions(alignGroup->actions());

    //---Tool bar Font---
    tb = addToolBar(tr("Font"));
    tb->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(tb);

    mpSizeCmbx = new QComboBox (tb);
    mpSizeCmbx->setObjectName("textSize");
    tb->addWidget(mpSizeCmbx);
    mpSizeCmbx->setEditable(true);
    QList<int> standartSizeis = QFontDatabase::standardSizes();
    for (auto a : standartSizeis)
        mpSizeCmbx->addItem(QString::number(a));


    connect (mpSizeCmbx, QOverload<const QString &>::of(&QComboBox::activated), this, &MainWindow::slotSetTextSize);

    mpStyleFontCmbx = new QFontComboBox (tb);
    mpStyleFontCmbx->setObjectName("textStyle");
    tb->addWidget(mpStyleFontCmbx);

    connect (mpStyleFontCmbx, QOverload<const QString &>::of(&QFontComboBox::activated), this, &MainWindow::slotTextStyle);

    //---Codec comboBox---
    mpCodecCmbx = new QComboBox (tb);
    mpCodecCmbx->setObjectName("codec");
    tb->addWidget(mpCodecCmbx);

// Some codecs
    QStringList listCodec;
    listCodec<< "Apple Roman"
             << "Big5"
             << "Big5-HKSCS"
             << "CP949"
             << "EUC-JP"
             << "EUC-KR"
             << "KOI8-R"
             << "KOI8-U"
             << "ROMAN8"
             << "Shift-JIS"
             << "TIS-620"
             << "TSCII"
             << "UTF-8"
             << "UTF-16"
             << "UTF-16BE"
             << "Windows-1250" // to 1258
             << "Windows-1251"; // Русский

       mpCodecCmbx->addItems(listCodec);
       connect(mpCodecCmbx,SIGNAL(activated(QString)),this,SLOT(setCodec(QString)));
       mpCodecCmbx->setCurrentIndex(listCodec.indexOf("UTF-8"));
}

void MainWindow::slotNewFile()
{
   // slotSave();
    mptxt->clear();
    currentPath = "";
    ui->statusBar->showMessage(tr("New file"));
    fileName = "New document";
    setWindowTitle(fileName);
}

void MainWindow::slotOpen()
{
    QString strTemp = QFileDialog::getOpenFileName(nullptr, tr("Open File"),"", "*.txt;; *.html;; *.*");
    if (!strTemp.isEmpty()) {
        slotLoad(currentPath = strTemp);
        slotChangedDocument();
    }
}

void MainWindow::slotLoad(const QString &currentPath)
{
    if (!currentPath.isEmpty()) {
        QFile file (currentPath);
        if (!file.open(QIODevice::ReadOnly)) {
            statusBar()->showMessage(tr("File is not open for read"), 2500);
        } else {
            QString formatOpenedFile = QFileInfo (currentPath).suffix();
            if (formatOpenedFile == "html")
                slotOpenHtml (file);
            else
                slotOpenPlainText (file);
        }
        file.close();
    }
    else
        ui->statusBar->showMessage(tr("No file to load"), 2500);
}

void MainWindow::slotOpenHtml(QFile &file)
{
    QByteArray bArray = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(bArray);
    QString str = codec->toUnicode(bArray);
    mptxt->setHtml(str);
    statusBar()->showMessage(tr ("File open %1").arg(QDir::toNativeSeparators(currentPath)), 2500);
}

void MainWindow::slotOpenPlainText(QFile &file)
{
    QByteArray bArray = file.readAll();
    QTextCodec *codec = QTextCodec::codecForName ("UTF-8");
    QString str = codec->toUnicode(bArray);
    mptxt->setPlainText(str);

    statusBar()->showMessage(tr ("File open %1").arg(QDir::toNativeSeparators(currentPath)), 2500);
}

bool MainWindow::slotSave()
{
    if (currentPath.isEmpty())
       return slotSaveAs();
    QTextDocumentWriter writer (currentPath);
    bool bSave {false};
    if (QFileInfo (currentPath).suffix() == "pdf")
        slotSavePdf();
    else {
        bSave = writer.write(mptxt->document());
        if (bSave) {
            mptxt->document()->setModified(false);
            statusBar()->showMessage(tr ("Wrote %1").arg(QDir::toNativeSeparators(currentPath)), 2500);
            slotChangedDocument();  //if the saved file has a new name, change the window name to the new name

        }
        else
            statusBar()->showMessage(tr ("Could not write file %1").arg(QDir::toNativeSeparators(currentPath)), 2500);

    }
    return bSave;
}

bool MainWindow::slotSaveAs()
{
    currentPath = QFileDialog::getSaveFileName(nullptr, tr("Save file"), "","*.html;; *.odt;; *.pdf;; *.txt");
    if (currentPath.isEmpty())
        return false;
    return slotSave();
}

void MainWindow::slotSavePdf()
{
    QPrinter pdf;
    pdf.setOutputFormat(QPrinter::PdfFormat);
    pdf.setOutputFileName(currentPath);
    mptxt->document()->print(&pdf);

    setWindowTitle(fileName = QFileInfo (currentPath).fileName());
    mptxt->document()->setModified(false);
}

bool MainWindow::slotSaveHTML()
{
    QFile file (QApplication::applicationDirPath()+"/temp.html");
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("Error decoding"), tr("File is not open."));
        return false;
    }
    QTextStream outStream (&file);
    outStream<<mptxt->document()->toHtml();           //write to open file

    file.close();
    return true;
}
//-------------------------------------------
void MainWindow::slotSearch()
{
    mFind->show();
}

void MainWindow::slotFind()
{    
    //thread to highlighter
highlighter = new Highlighter(mptxt->toHtml(),
                      mFind->getFLineEdit()->text(), mptxt);
thread = new QThread;
highlighter->moveToThread(thread);

connect (highlighter, &Highlighter::signalSendHighlighter, this, &MainWindow::slotSetHighlight);
connect (thread, &QThread::started, highlighter, &Highlighter::slotHighlightering);
connect (mFind, &Find::signalExit, highlighter, &Highlighter::slotExit);

    qDebug()<<"Main Window Find";
    slotHighlight();
    //if the text finished go back to the beginning and finde the first overlap
    if (!mptxt->find(mFind->getFLineEdit()->text())) {
        mptxt->moveCursor(QTextCursor::Start);
        mptxt->find(mFind->getFLineEdit()->text());
    }

    //change gray highlighter to blue
    pal = mptxt->palette();
    for (int colorRole=0; colorRole<QPalette::NColorRoles; ++colorRole)
        pal.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(colorRole) , pal.color(QPalette::Active, static_cast<QPalette::ColorRole>(colorRole)));
    mptxt->setPalette(pal);

    //QWidget::activateWindow();
    //setWindowFlag(Qt::WindowStaysOnTopHint);


}

void MainWindow::slotHighlight()
{


    if (mFind->getFLineEdit()->isModified()) {
        mFind->getFLineEdit()->setModified(false);  //reset line change QLineEdit in Find
        mptxt->moveCursor(QTextCursor::Start);
        thread->start();
        qDebug()<<"Thread start";
    }




}

void MainWindow::slotSetHighlight(QList<QTextEdit::ExtraSelection> extraSelections)
{
    mptxt->setExtraSelections(extraSelections);
}

void MainWindow::slotFCancel()
{        
    mFind->getFLineEdit()->clear();
    mFind->close();
}

void MainWindow::saveSetting()
{
    QSettings settings ("settings.conf", QSettings::IniFormat);

    settings.setValue("currentPath", QDir::toNativeSeparators(currentPath));
    qDebug()<<"Save settings - " <<settings.value("currentPath").toString();
    settings.setValue("mpStyleFontCmbx", mpStyleFontCmbx->currentText());
    settings.setValue("mpSizeCmbx", mpSizeCmbx->currentText());
    settings.setValue("mpCodecCmbx", mpCodecCmbx->currentText());

    ui->statusBar->showMessage(tr ("Settings save"), 2500);
}

void MainWindow::loadSetting()
{
    QSettings settings ("settings.conf", QSettings::IniFormat);
    currentPath = settings.value("currenPath", "").toString();

    //mptxt->setPlainText(settings.value("mptxText", "").toString());
    mpStyleFontCmbx->setCurrentText(settings.value("mpStyleFontCmbx", "Times New Roman").toString());
    mpSizeCmbx->setCurrentText(settings.value("mpSizeCmbx", "10").toString());
    mpCodecCmbx->setCurrentText(settings.value("mpCodecCmbx").toString());

    slotSetTextSize (mpSizeCmbx->currentText());
    slotTextStyle(mpStyleFontCmbx->currentText());


    slotLoad(currentPath);
//    void slotChangeCurrentPosition();
}

void MainWindow::slotRemove()
{
    QTextCursor cursor = mptxt->textCursor();
    cursor.removeSelectedText();
}

void MainWindow::slotItalic()
{
    QTextCharFormat fmt;
    fmt.setFontItalic(actionItalic->isChecked());
    setCharFormat(fmt);
}

void MainWindow::slotBold()
{
    QTextCharFormat fmt;
    fmt.setFontWeight(actionBold->isChecked() ? QFont::Bold : QFont::Normal);
    setCharFormat(fmt);
}

void MainWindow::slotUnderLine()
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(actionUnderLine->isChecked());
    setCharFormat(fmt);
}

void MainWindow::setCharFormat(const QTextCharFormat &fmt)
{
    QTextCursor cursor = mptxt->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(fmt);
    mptxt->mergeCurrentCharFormat(fmt);

}

void MainWindow::slotChangeCurrentPosition()
{
    QFont font = mptxt->currentFont();
    slotChangeCurrentText(font);
}

 void MainWindow::slotChangeCurrentText (const QFont &font)
 {
    actionItalic->setChecked(font.italic());
    actionBold->setChecked(font.bold());
    actionUnderLine->setChecked(font.underline());

    //---Alignment---

    if (mptxt->alignment() & Qt::AlignLeft)
        actionAlignLeft->setChecked(true);
    else if (mptxt->alignment() & Qt::AlignHCenter)
        actionAlignCenter->setChecked(true);
    else if (mptxt->alignment() & Qt::AlignRight)
        actionAlignRight->setChecked(true);
    else if (mptxt->alignment() & Qt::AlignJustify)
        actionAlignJustify->setChecked(true);

    //---Text size---
    mpSizeCmbx->setCurrentText(QString::number(font.pointSize()));

    //---Text style---
    mpStyleFontCmbx->setCurrentIndex(mpStyleFontCmbx->findText(QFontInfo(font).family()));

 }

void MainWindow::slotAlign(QAction* action)
{
    if (action == actionAlignLeft){
        mptxt->setAlignment(Qt::AlignLeft);
    } else if (action == actionAlignCenter) {
        mptxt->setAlignment(Qt::AlignCenter);
    } else if (action == actionAlignRight) {
        mptxt->setAlignment(Qt::AlignRight);
    } else if (action == actionAlignJustify) {
        mptxt->setAlignment(Qt::AlignJustify);
    }

}

void MainWindow::slotSetTextSize (const QString& s)
{
    if (s.toFloat()>0) {
        QTextCharFormat format;
        format.setFontPointSize(qreal (s.toFloat()));
        setCharFormat(format);
    }
}

void MainWindow::slotTextStyle(const QString &style)
{
    QTextCharFormat format;
    format.setFontFamily(style);
    setCharFormat(format);
}

void MainWindow::slotChangedDocument()
{
    if (!currentPath.isEmpty())
        fileName = QFileInfo (currentPath).fileName();
    else
        fileName = "New document";
    mptxt->document()->isModified() ? setWindowTitle(fileName + '*') :
                                      setWindowTitle(fileName);

}

void MainWindow::closeEvent(QCloseEvent* e)
{
    if (mptxt->document()->isModified()) {
        QMessageBox::StandardButton stb = QMessageBox::warning(this, QCoreApplication::applicationName(),
                                                                tr("The document has been modified.\n"
                                                                   "Do you want to save your changes?"),
                                                                    QMessageBox::Save| QMessageBox::Discard |QMessageBox::Cancel);
        if (stb == QMessageBox::Save) {
            slotSave();
            e->accept();
        }
        else {
            e->ignore();
            QApplication::quit();
        }
    }

}

void MainWindow::setCodec(QString newCodec)
{
    QStringList strBufText;
     strBufText << mptxt->document()->toHtml() << mpCodecCmbx->currentText();
    // Codec selection
    QTextCodec *codec = QTextCodec::codecForName(newCodec.toUtf8());

     // Change codec and set new encoded text
    mptxt->setHtml(codec->fromUnicode(mptxt->document()->toHtml()));
    if (
            QMessageBox::information(this, tr ("Codec change"), tr ("Accept the codec?"), QMessageBox::Ok | QMessageBox::No)== QMessageBox::No
            ) {
        mptxt->setHtml(strBufText[0]);      //returns the previous codec
        mpCodecCmbx->setCurrentIndex (mpCodecCmbx->findText(strBufText[1]));    //sets the name of the previous codec
    }

}


