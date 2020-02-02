#ifndef FIND_H
#define FIND_H

#include <QtGlobal>

#if QT_VERSION >= 0x050000
#include <QtWidgets/QWidget>
#else
#include <QtGui/QWidget>
#endif

#include <QPushButton>
#include <QLineEdit>

namespace Ui {
class Find;
}

class Find : public QWidget
{
    Q_OBJECT

public:
    explicit Find(QWidget *parent = nullptr);
    QPushButton *getCmdFind ();
//    void getCancel ();
    QLineEdit *getFLineEdit();
    ~Find();

private:
    Ui::Find *ui;
public slots:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void slotExit();
signals:
    void signalExit();
};

#endif // FIND_H
