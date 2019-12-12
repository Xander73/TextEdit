#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Find.h"

#include <QActionGroup>
#include <QtAlgorithms>
#include <QClipboard>
#include <QCloseEvent>
#include <QColor>
#include <QCoreApplication>
#include <QDebug>
#include <QDial>
#include <QEvent>
#include <QList>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QMessageBox>
#include <QtPrintSupport/qprinter.h>
#include <QSettings>
#include <QStyle>
#include <QSyntaxHighlighter>
#include <QTextCodec>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QTextDocumentWriter>
#include <QTextStream>

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

    connect (mptxt, SIGNAL (cursorPositionChanged()), this, SLOT (slotChangeCurrentPosition()));

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
    connect(mptxt, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotCustomMenuRequested(QPoint)));

    mFind = new Find ();
    //connects window Find
    connect (mFind->getCancel(), SIGNAL (clicked()), this, SLOT (slotFCancel()));
    connect (mFind->getCmdFind(), SIGNAL (clicked()), this, SLOT (slotFind()));


    if (!currentPath.isEmpty()) {
        QFileInfo f (currentPath);
        setWindowTitle(f.baseName());
    } else {
        setWindowTitle("New document");
    }

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

    m->addSeparator();
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
// Вот некоторые кодеки из доступных в Qt
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


        // Добавим из в comboBox
       mpCodecCmbx->addItems(listCodec);

        // Подключим сигнал изменеия comboBox
        connect(mpCodecCmbx,SIGNAL(activated(QString)),this,SLOT(setCodec(QString)));

        // Установим comboBox на нашу кодировку
        mpCodecCmbx->setCurrentIndex(listCodec.indexOf("UTF-8"));


}

void MainWindow::slotNewFile()
{
   // slotSave();
    mptxt->clear();
    currentPath = "";
    ui->statusBar->showMessage(tr("New file"));
    setWindowTitle("New document");
}

void MainWindow::slotOpen()
{
    currentPath = QFileDialog::getOpenFileName(nullptr, tr("Open File"),"C:\\Users\\Саша\\Desktop", "*.txt");
    slotLoad(currentPath);

    setWindowTitle(QFileInfo (currentPath).fileName());
}

void MainWindow::slotLoad(const QString &currentPath)
{
    if (!currentPath.isEmpty()) {
        QFile file (currentPath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::WriteOnly)) {
            qDebug()<<tr("File is not open for read");
        } else {
            QTextStream stream (&file);
            stream.setCodec(QTextCodec::codecForName("UTF-8"));
            mptxt->setText(stream.readAll());
            ui->statusBar->showMessage(tr("File open"), 1000);
        }
        file.close();
    }
    else
        ui->statusBar->showMessage(tr("No file to load"), 5000);
}

bool MainWindow::slotSave()
{
    if (currentPath.isEmpty())
       return slotSaveAs();
    slotSaveTxt();

    setWindowTitle(QFileInfo (currentPath).fileName());  //if the saved file has a new name, change the window name to the new name

    return true;
}

bool MainWindow::slotSaveAs()
{
    currentPath = QFileDialog::getSaveFileName(nullptr, tr("Save file"), "","*.txt;; *.pdf");
    QString fileName = QFileInfo (currentPath).fileName();    
    QString format = QFileInfo (currentPath).suffix();

    if (format!="txt" || format != "pdf") {
        QMessageBox::warning (this, tr("Attention"), tr("Undefined format"), QMessageBox::Ok);
        ui->statusBar->showMessage(tr("Format is not available"));
        return false;
    }
    else {
        if (format=="pdf") {
            slotSavePdf();
            ui->statusBar->showMessage(tr("File saved %1").arg(currentPath));
        }
        else if (format=="txt") {
            slotSaveTxt();
            ui->statusBar->showMessage(tr("File saved %1").arg(currentPath));
        }
    }

    setWindowTitle(QFileInfo (currentPath).fileName());

    return true;
}

void MainWindow::slotSaveTxt ()
{
    if (currentPath.isEmpty()) {
        slotSaveAs();
        return;
    }
    QTextDocumentWriter text {currentPath};
    if (text.write(mptxt->document())) {
        mptxt->document()->setModified(false);
        statusBar()->showMessage(tr("Wrote %1").arg(QDir::toNativeSeparators(currentPath)), 5);
    } else {
        statusBar()->showMessage(tr("Could not write a file %1").arg(currentPath));
    }
}

void MainWindow::slotSavePdf()
{
    QPrinter pdf;
    pdf.setOutputFormat(QPrinter::PdfFormat);
    pdf.setOutputFileName(currentPath);
    mptxt->document()->print(&pdf);

    setWindowTitle(QFileInfo (currentPath).fileName());
}

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
    QWidget::activateWindow();
    setWindowFlag(Qt::WindowStaysOnTopHint);


}

void MainWindow::slotFCancel()
{
    mFind->close();
}

void MainWindow::saveSetting()
{
    QSettings settings ("settings.conf", QSettings::IniFormat);

    settings.beginGroup("positionSettings");

    settings.endGroup();

    settings.beginGroup("parametersSetting");
    settings.setValue("currentPath", currentPath);
    settings.setValue("mptxtText", mptxt->toPlainText());
    settings.setValue("mpStyleFontCmbx", mpSizeCmbx->currentText());
    settings.setValue("mpSizeCmbx", mpSizeCmbx->currentText());
    settings.setValue("mpCodecCmbx", mpCodecCmbx->currentText());

    settings.endGroup();

    ui->statusBar->showMessage("Settings save");
}

void MainWindow::loadSetting()
{
    QSettings settings ("settings.conf", QSettings::IniFormat);
    currentPath = settings.value("currenPath").toString();
    mptxt->setPlainText(settings.value("mptxText").toString());
    mpStyleFontCmbx->setCurrentText(settings.value("mpStyleFontCmbx").toString());
    mpSizeCmbx->setCurrentText(settings.value("mpSizeCmbx").toString());
    mpCodecCmbx->setCurrentText(settings.value("mpCodecCmbx").toString());


    slotLoad(currentPath);
    void slotChangeCurrentPosition();
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

void MainWindow::setCodec(QString newCodec)
{
    // Codec selection
     QTextCodec *codec = QTextCodec::codecForName(newCodec.toUtf8());

     // Change codec and set new encoded text
     mptxt->setText(codec->fromUnicode(mptxt->document()->toPlainText()));
}

void MainWindow::slotChangedDocument()
{
    QString fileName = QFileInfo (currentPath).fileName();
    currentPath.isEmpty() ? setWindowTitle("New document*") : setWindowTitle(fileName+'*');
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
