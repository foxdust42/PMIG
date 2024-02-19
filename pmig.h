#ifndef PMIG_H
#define PMIG_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class PMIG; }
QT_END_NAMESPACE

class PMIG : public QMainWindow
{
    Q_OBJECT

public:
    PMIG(QWidget *parent = nullptr);
    ~PMIG();

private:
    Ui::PMIG *ui;
};
#endif // PMIG_H
