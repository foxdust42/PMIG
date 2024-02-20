#include "pmig.h"
#include "./ui_pmig.h"
#include "basefilter.h"
#include "functionalfilters.h"

#include <QGraphicsPixmapItem>
#include <QDebug>
#include <QTextStream>

PMIG::PMIG(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PMIG)
{
    ui->setupUi(this);

    filters.push_back(new Filters::InversionFilter());
    filters.push_back(new FunctionalFilters::BrightnessCorrectionFilter());

    //ui->listWidget_Functional->addItem();

    //ui->graphicsViewLeft->setScene(&this->original_scene);
    QObject::connect(ui->actionOpen, &QAction::triggered,
                     this, &PMIG::slot_loadImage);
    QObject::connect(ui->actionApply_Inversion, &QAction::triggered,
                     this, &PMIG::slot_applyInv);
    QObject::connect(ui->actionUp_Brightness, &QAction::triggered,
                     this, &PMIG::slot_applyInv);
    QObject::connect(ui->listWidget_Functional, &QListWidget::itemDoubleClicked,
                     this, &PMIG::slot_listFunctional);

    QTextStream(stdout) << "Setup Done\n" ;

    //this->loadImage();
}

void PMIG::loadImage() {
    if (original_scene != NULL) {
        delete original_scene;
    }
    original_scene = new QGraphicsScene(this);
    ui->graphicsViewLeft->setScene(original_scene);

    //QBrush blueBrush(Qt::blue);

    image = QImage(":/debug/amongkill.jpg");
    modified_image = QImage(image);
    //image = image.scaledToHeight(50);
    this->item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
    this->item->setFlag(QGraphicsItem::ItemIsMovable);
    original_scene->addItem(this->item);
    text = original_scene->addText("ABCD", QFont("Arial", 20));
    text->setFlag(QGraphicsItem::ItemIsMovable);

    loadRightImage();

}

void PMIG::slot_loadImage(){
    this->loadImage();
    QTextStream(stdout) << "AAABBB\n";
}

void PMIG::slot_applyInv(){
    if (modified_image.isNull()) {
        QTextStream(stdout) << "Nothing to invert\n";
        return;
    }
    //modified_image = QImage(image);

    filters[1]->applyFilter(&modified_image);

    loadRightImage();

    QTextStream(stdout) << "Inv\n";

}

void PMIG::loadRightImage(){
    if (new_scene != NULL){
        delete new_scene;
    }
    modified_item = new QGraphicsPixmapItem(QPixmap::fromImage(modified_image));
    new_scene = new QGraphicsScene(this);
    new_scene->addItem(modified_item);
    this->modified_item->setFlag(QGraphicsItem::ItemIsMovable);
    ui->graphicsViewRight->setScene(new_scene);
}

PMIG::~PMIG()
{
    for (int i=0; i<filters.size(); i++){
        delete filters[i];
    }
    delete ui;
}

