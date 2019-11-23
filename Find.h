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
    QPushButton* getCmdFind ();
    QPushButton* getCancel ();
    QString      getFLineEdit();
    ~Find();

private:
    Ui::Find *ui;
};

#endif // FIND_H
