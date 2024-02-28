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
#include <QMessageBox>

PMIG::PMIG(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PMIG)
{
    ui->setupUi(this);

    //ui->doubleSpinBox->setDecimals(8);

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

    initCustomConv();

    original_scene = new QGraphicsScene(this);
    new_scene = new QGraphicsScene(this);

    QObject::connect(ui->actionOpen, &QAction::triggered,
                     this, &PMIG::loadImage);
    QObject::connect(ui->actionSave, &QAction::triggered,
                     this, &PMIG::slot_saveImage);
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
            "",
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

void PMIG::slot_saveImage(){
    if(this->modified_image.isNull()){
        QTextStream(stdout) << "No save: No Image found\n";
        QMessageBox msg;
        msg.setWindowTitle("Save error");
        msg.setIcon(QMessageBox::Critical);
        msg.setText("Failed to Save image : No Image Found");
        msg.setInformativeText("Nothing to save");
        msg.exec();
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Image"),
                                                    "image.png",
                                                    tr("Image Files (*.png *.jpg *.bmp)"));

    if (fileName.isNull()){
        QTextStream(stdout) << "No save: File not found\n";
        QMessageBox msg;
        msg.setWindowTitle("Save error");
        msg.setIcon(QMessageBox::Information);
        msg.setText("Failed to Save image : Failed to load file");
        msg.setInformativeText("Failed to save");
        msg.exec();
        return;
    }

    bool res = modified_image.save(fileName, nullptr, -1);
    if (!res){
        QTextStream(stdout) << "No save: Failed to write\n";
        QMessageBox msg;
        msg.setWindowTitle("Save error");
        msg.setIcon(QMessageBox::Critical);
        msg.setText("Failed to Save image : Could not write to file");
        msg.setInformativeText("Failed to save");
        msg.exec();
        return;
    }
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
        QMessageBox msg;
        msg.setWindowTitle("Filter error");
        msg.setIcon(QMessageBox::Warning);
        msg.setText("Failed to apply filter : No Image Found");
        msg.setInformativeText("Please load an image before applying filters");
        msg.exec();
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
    this->deleteCustomConv();
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

void PMIG::initCustomConv(){
    ui->customConvTable->setRowCount(3);
    ui->customConvTable->setColumnCount(3);
    ui->customConvTable->verticalHeader()->setDefaultSectionSize(1);
    ui->customConvTable->horizontalHeader()->setDefaultSectionSize(1);

    QTextStream(stdout) << ui->customConvTable->styleSheet() << "\n";

    //ui->customConvTable->setStyleSheet("");

    QTableWidgetItem* tableItem;
    for (int i=0; i<ui->customConvTable->rowCount(); i++){
        for (int j=0; j<ui->customConvTable->columnCount(); j++){
            tableItem = new QTableWidgetItem(tr("%1").arg((i+1)*(j+1)));
            ui->customConvTable->setItem(i, j, tableItem);
        }
    }

    ui->customConvTable->setAnchor(1, 1);
    //ui->customConvTable->item(1,1)->setBackground(this->highlight);

    ui->customConvTable->update();
}

void PMIG::deleteCustomConv(){
    for (int i=0; i<ui->customConvTable->rowCount(); i++){
        for (int j=0; j<ui->customConvTable->columnCount(); j++){
            delete ui->customConvTable->item(i,j);
        }
    }
}
