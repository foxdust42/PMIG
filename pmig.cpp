#include "pmig.h"
#include "./ui_pmig.h"
#include "basefilter.h"
#include "functionalfilters.h"

#include <QGraphicsPixmapItem>
#include <QDebug>
#include <QTextStream>
#include <QFileDialog>

PMIG::PMIG(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PMIG)
{
    ui->setupUi(this);

    filters.push_back(new Filters::InversionFilter());
    filters.push_back(new FunctionalFilters::BrightnessCorrectionFilter());

    initFilterLists();

    //ui->listWidget_Functional->addItem();
    //ui->graphicsViewLeft->setScene(&this->original_scene);
    QObject::connect(ui->actionOpen, &QAction::triggered,
                     this, &PMIG::loadImage);
    //QObject::connect(ui->actionApply_Inversion, &QAction::triggered,
    //                 this, &PMIG::slot_applyInv);
    //QObject::connect(ui->actionUp_Brightness, &QAction::triggered,
    //                 this, &PMIG::slot_applyInv);
    QObject::connect(ui->listWidget_Functional, &QListWidget::itemDoubleClicked,
                     this, &PMIG::slot_listFunctional);
    //QObject::connect(ui->graphicsViewLeft, &QWidget::)

    QTextStream(stdout) << "Setup Done\n" ;

    //this->loadImage();
}

void PMIG::loadImage() {
    QString fileName =
        QFileDialog::getOpenFileName(this,
            tr("Open Image"),
            "c",
            tr("Image Files (*.png *.jpg *.bmp)"));

    if (fileName.isNull()){
        QTextStream(stdout) << "No file selected\n";
        return;
    }

    //image = QImage(":/debug/amongkill.jpg");

    image = QImage(fileName);

    if (image.isNull()){
        QTextStream(stderr) << "Failed to load image";
        return;
    }

    if (original_scene != NULL) {
        delete original_scene;
    }
    original_scene = new QGraphicsScene(this);
    ui->graphicsViewLeft->setScene(original_scene);

    modified_image = QImage(image);
    this->item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
    original_scene->addItem(this->item);

    loadRightImage();

}

void PMIG::slot_loadImage(){
    this->loadImage();
    QTextStream(stdout) << "AAABBB\n";
}

void PMIG::slot_listFunctional(QListWidgetItem *item){
    int filter_index = item->data(Qt::UserRole + 0x1).toInt();
    if (modified_image.isNull()){
        QTextStream(stdout) << "No Image found";
        return;
    }
    filters[filter_index]->applyFilter(&modified_image);
    loadRightImage();
}

void PMIG::slot_applyInv(){
    if (modified_image.isNull()) {
        QTextStream(stdout) << "Nothing to invert\n";
        return;
    }
    //modified_image = QImage(image);

    filters[0]->applyFilter(&modified_image);

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
    for (int i=0; i<listEntries.size(); i++){
        delete listEntries[i];
    }
    delete ui;
}

void PMIG::initFilterLists(){
    QListWidgetItem *item;
    //QListWidget *list;
    for (int i = 0; i < filters.size(); i++){
        switch (filters[i]->getClass()){
        case Filters::FilterClass::FILTER_FUNCTIONAL:
            item = new QListWidgetItem(QString(filters[i]->getName()),
                                       ui->listWidget_Functional,
                                       QListWidgetItem::ItemType::UserType);
            //ui->listWidget_Functional->addItem(QString(filters[i]->getName()));
            item->setData(Qt::UserRole + 0x1, QVariant(i));
            ui->listWidget_Functional->addItem(item);
            break;
        case Filters::FilterClass::FILTER_CONVOLUTION:
            item = new QListWidgetItem(QString(filters[i]->getName()),
                                       ui->listWidget_Convolutional,
                                       QListWidgetItem::ItemType::UserType);
            item->setData(Qt::UserRole + 0x1, QVariant(i));
            ui->listWidget_Convolutional->addItem(item);
            break;
        default:
            //skip
            continue;
            break;
        }
        listEntries.push_back(item);
        //QTextStream(stdout) << ui->listWidget_Functional->count() << "\n";
    }
}
