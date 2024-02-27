#include "pmig.h"
#include "./ui_pmig.h"
#include "basefilter.h"
#include "functionalfilters.h"
#include "convolutionfilters.h"

#include <QGraphicsPixmapItem>
#include <QDebug>
#include <QTextStream>
#include <QFileDialog>
#include <qscreen.h>

PMIG::PMIG(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PMIG)
{
    ui->setupUi(this);

    filters.push_back(new Filters::InversionFilter());
    filters.push_back(new FunctionalFilters::BrightnessCorrectionFilter());
    filters.push_back(new FunctionalFilters::ContrastEnchancementFilter());
    filters.push_back(new FunctionalFilters::GammaCorectionFilter());
    filters.push_back(new ConvolutionFilters::BlurFilter());
    filters.push_back(new ConvolutionFilters::GaussBlurFilter());
    filters.push_back(new ConvolutionFilters::SharpenFilter());
    filters.push_back(new ConvolutionFilters::EdgeDetectionDiag());
    filters.push_back(new ConvolutionFilters::SouthEmoss());

    initFilterLists();

    original_scene = new QGraphicsScene(this);
    new_scene = new QGraphicsScene(this);

    QObject::connect(ui->actionOpen, &QAction::triggered,
                     this, &PMIG::loadImage);
    QObject::connect(ui->listWidget_Functional, &QListWidget::itemDoubleClicked,
                     this, &PMIG::slot_listDClick);
    QObject::connect(ui->listWidget_Convolutional, &QListWidget::itemDoubleClicked,
                     this, &PMIG::slot_listDClick);
    QObject::connect(ui->actionReset_Image, &QAction::triggered,
                     this, &PMIG::slot_resetImage);

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

    image = QImage(fileName);

    if (image.isNull()){
        QTextStream(stderr) << "Failed to load image";
        return;
    }


    original_scene->clear();
    //original_scene = new QGraphicsScene(this);
    ui->graphicsViewLeft->setScene(original_scene);

    modified_image = QImage(image);
    this->item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
    this->item->setFlag(QGraphicsItem::ItemIsMovable);
    original_scene->addItem(this->item);

    loadRightImage();

}

void PMIG::slot_loadImage(){
    this->loadImage();
    QTextStream(stdout) << "AAABBB\n";
}

void PMIG::slot_resetImage(){
    if (this->image.isNull()){
        QTextStream(stdout) << "Nothing to reset\n";
        return;
    }
    modified_image = QImage(image);
    this->loadRightImage();
}

void PMIG::slot_listDClick(QListWidgetItem *item){
    int filter_index = item->data(Qt::UserRole + 0x1).toInt();
    if (modified_image.isNull()){
        QTextStream(stdout) << "No Image found\n";
        return;
    }
    QTextStream(stdout) << filters[filter_index]->getName() << "\n";
    filters[filter_index]->applyFilter(&modified_image);
    loadRightImage();
}


void PMIG::loadRightImage(){
    new_scene->clear();
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
    delete original_scene;
    delete new_scene;
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
