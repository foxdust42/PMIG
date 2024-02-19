#include "pmig.h"
#include "./ui_pmig.h"

PMIG::PMIG(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PMIG)
{
    ui->setupUi(this);
}

PMIG::~PMIG()
{
    delete ui;
}

