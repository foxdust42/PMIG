#include "pmig.h"
#include "./ui_pmig.h"
#include "basefilter.h"
#include "functionalfilters.h"
#include "convolutionfilters.h"
#include "customconvfiltertable.h"
#include "grayscale.h"
#include "task2.h"

#include <QGraphicsPixmapItem>
#include <QDebug>
#include <QTextStream>
#include <QFileDialog>
#include <qscreen.h>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <fstream>

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
    filters.push_back(new Filters::MaxFilter());
    filters.push_back(new Filters::MinFilter());
    filters.push_back(new Task2::AverageDithering());
    filters.push_back(new Task2::MedianCut());
    filters.push_back(new Task2::HistogramStretchingWithThreshold());

    initFilterLists();

    //ui->customConvTable->set

    ui->customConvTable->init(3, 3);

    original_scene = new QGraphicsScene(this);
    new_scene = new CustomGraphicsScene(this);

    QObject::connect(ui->actionOpen, &QAction::triggered,
                     this, &PMIG::loadImage);
    QObject::connect(ui->actionSave, &QAction::triggered,
                     this, &PMIG::slot_saveImage);
    QObject::connect(ui->listWidget_Functional, &QListWidget::itemDoubleClicked,
                     this, &PMIG::slot_listDClick);
    QObject::connect(ui->listWidget_Convolutional, &QListWidget::itemDoubleClicked,
                     this, &PMIG::slot_listDClick);
    QObject::connect(ui->listWidget_other, &QListWidget::itemDoubleClicked,
                     this, &PMIG::slot_listDClick);
    QObject::connect(ui->listWidget_Convolutional, &QListWidget::itemClicked,
                     this, &PMIG::slot_loadCustomConv);
    QObject::connect(ui->actionReset_Image, &QAction::triggered,
                     this, &PMIG::slot_resetImage);

    //

    QObject::connect(ui->pushButton_ApplySize, &QAbstractButton::clicked,
                     this, &PMIG::slot_setCustomConv);
    QObject::connect(ui->pushButton_CalcDivisor, &QAbstractButton::clicked,
                     this, &PMIG::slot_calcDivisor);
    QObject::connect(ui->pushButton_Save, &QAbstractButton::clicked,
                     this, &PMIG::slot_saveCustom);

    //

    QObject::connect(ui->actionLightness, &QAction::triggered, this, &PMIG::slot_lightnessGray);
    QObject::connect(ui->actionAverage, &QAction::triggered, this, &PMIG::slot_averageGray);
    QObject::connect(ui->actionLuminosity, &QAction::triggered, this, &PMIG::slot_luminosityGray);

    //

    QObject::connect(ui->action_New, &QAction::triggered, this, &PMIG::slot_newImage);

    //

    QObject::connect(ui->spinBox_Red, QOverload<int>::of(&QSpinBox::valueChanged),
                     this, &PMIG::slot_updateColor);
    QObject::connect(ui->spinBox_Green, QOverload<int>::of(&QSpinBox::valueChanged),
                     this, &PMIG::slot_updateColor);
    QObject::connect(ui->spinBox_Blue, QOverload<int>::of(&QSpinBox::valueChanged),
                     this, &PMIG::slot_updateColor);

    QObject::connect(ui->spinBox_Thickness, QOverload<int>::of(&QSpinBox::valueChanged),
                     this, &PMIG::slot_updateThickness);

    sample_image = QImage(300, 100, QImage::Format_RGBA64);
    sample_image.fill(QColor(0,0,0));
    sample_scene = new QGraphicsScene(this);
    sample_item = new QGraphicsPixmapItem(QPixmap::fromImage(sample_image));
    sample_scene->addItem(sample_item);
    ui->graphicsView_ColorPreview->setScene(sample_scene);
    ui->graphicsView_ColorPreview->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView_ColorPreview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //ui->graphicsView_ColorPreview->fitInView(sample_image.rect());

    ui->radioButton_Line->click();

    QObject::connect(ui->radioButton_Line, &QRadioButton::clicked, this, &PMIG::slot_updateComponentType);
    QObject::connect(ui->radioButton_Polygon, &QRadioButton::clicked, this, &PMIG::slot_updateComponentType);
    QObject::connect(ui->radioButton_Circle, &QRadioButton::clicked, this, &PMIG::slot_updateComponentType);

    QObject::connect(ui->pushButton_VecDel, &QPushButton::clicked, new_scene, &CustomGraphicsScene::deleteSelected);
    QObject::connect(ui->pushButton_VecSetCol, &QPushButton::clicked, new_scene, &CustomGraphicsScene::setColorSelected);
    QObject::connect(ui->pushButton_VecSetThi, &QPushButton::clicked, new_scene, &CustomGraphicsScene::setThicknessSelected);
    QObject::connect(ui->action_Clear_Vector_Components, &QAction::triggered,this, &PMIG::slot_clearVector);
    QObject::connect(ui->checkBox_Antialias, &QCheckBox::stateChanged, this, &PMIG::slot_setAntialias);

    QObject::connect(ui->actionSave_Vector_Components, &QAction::triggered, this, &PMIG::slot_saveVectorComponents);
    QObject::connect(ui->action_Load_Vector_Components, &QAction::triggered, this, &PMIG::slot_loadVectorComponents);
    //QObject::connect(ui->spinBox_Red, &QSpinBox::valueChanged, this, nullptr);

    //

    QTextStream(stdout) << "Setup Done\n" ;

    //this->loadImage();
}

void PMIG::slot_setAntialias() {
    new_scene->setAntialiasing(ui->checkBox_Antialias->isChecked());
}

void PMIG::scream(QString title, QMessageBox::Icon icon, QString text, QString info_text, QString out){
    QTextStream(stdout) << out;
    QMessageBox msg;
    msg.setWindowTitle(title);
    msg.setIcon(icon);
    msg.setText(text);
    msg.setInformativeText(info_text);
    msg.exec();
}

void PMIG::slot_clearVector(){
    new_scene->ClearVectorComponents();
}

void PMIG::slot_loadVectorComponents(){
    if(this->modified_image.isNull()){
        scream("Load error", QMessageBox::Critical, "Failed to Load image : No Image Found", "Nothing to load to", "No load: nowhere to load\n");
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Load Vector Components"),
                                                    "components.pmig.vec",
                                                    tr("PMIG vector Files (*.pmig.vec *.json)"));

    if (fileName.isNull()){
        QTextStream(stdout) << "No load: File not found\n";
        QMessageBox msg;
        msg.setWindowTitle("Load error");
        msg.setIcon(QMessageBox::Information);
        msg.setText("Failed to LOad Vector Components : Failed to open file for reading");
        msg.setInformativeText("Failed to load");
        msg.exec();
        return;
    }

    QFile infile(fileName);

    if (!infile.open(QIODevice::ReadOnly | QIODevice::Text)){
        scream("Failed to open file", QMessageBox::Critical, "Coudn't open file : system error", "failed to open file", "deserialize: noopen\n");
        return;
    }

    new_scene->deserializeComponents(&infile);
    infile.close();
}

void PMIG::slot_saveVectorComponents(){

    if (new_scene->componentsCount() <= 0) {
        scream("Save error", QMessageBox::Warning, "Failed to save Vector Components : nothing to save", "Nothing to save\n");
        return;
    }

    if(this->new_scene == nullptr){
        scream("Save error", QMessageBox::Critical, "Failed to Save image : No Image Found", "Nothing to save", "No save: nothing to save\n");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Vector Components"),
                                                    "components.pmig.vec",
                                                    tr("PMIG vector Files (*.pmig.vec *.json)"));

    if (fileName.isNull()){
        QTextStream(stdout) << "No save: File not found\n";
        QMessageBox msg;
        msg.setWindowTitle("Save error");
        msg.setIcon(QMessageBox::Information);
        msg.setText("Failed to Save Vector Components : Failed to open file for writing");
        msg.setInformativeText("Failed to save");
        msg.exec();
        return;
    }

    std::ofstream savefile;
    savefile.open(fileName.toStdString(), std::ios_base::out);
    savefile << new_scene->serializeComponents();
    savefile.close();
}

void PMIG::slot_updateComponentType(){
    if (ui->radioButton_Line->isChecked()) {
        new_scene->setVectorComponentType(VECTOR_COMPONENT_LINE);
    }
    else if (ui->radioButton_Polygon->isChecked()) {
        new_scene->setVectorComponentType(VECTOR_COMPONENT_POLYGON);
    }
    else if (ui->radioButton_Circle->isChecked()) {
        new_scene->setVectorComponentType(VECTOR_COMPONENT_CIRCLE);
    }
    else {
        throw std::logic_error("Failed to get checked button");
    }
}

void PMIG::slot_updateColor(int){

    sample_color = QColor(ui->spinBox_Red->value(), ui->spinBox_Green->value(), ui->spinBox_Blue->value());
    QTextStream(stdout) << sample_color.red() << " " << sample_color.green() << " " << sample_color.blue() << "\n";
    sample_image.fill(sample_color);
    sample_scene->clear();

    sample_item = new QGraphicsPixmapItem(QPixmap::fromImage(sample_image));
    sample_scene->addItem(sample_item);

    ui->graphicsView_ColorPreview->setScene(sample_scene);

    new_scene->setColor(sample_color.rgb());

}

void PMIG::slot_updateThickness(int t){
    new_scene->setThickness(t);
}

void PMIG::slot_lightnessGray(){
    if (modified_image.isNull()){
        scream("Gray error", QMessageBox::Critical, "Failed to apply grayscale : No Image Found", "Please load an image before applying filters", "No Image found\n");
        return;
    }
    Task2::LightnessGray::toGray(&modified_image);
    loadRightImage();
}
void PMIG::slot_averageGray(){
    if (modified_image.isNull()){
        scream("Gray error", QMessageBox::Critical, "Failed to apply grayscale : No Image Found", "Please load an image before applying filters", "No Image found\n");
        return;
    }
    Task2::AverageGray::toGray(&modified_image);
    loadRightImage();
}
void PMIG::slot_luminosityGray(){
    if (modified_image.isNull()){
        scream("Gray error", QMessageBox::Critical, "Failed to apply grayscale : No Image Found", "Please load an image before applying filters", "No Image found\n");
        return;
    }
    Task2::LuminosityGray::toGray(&modified_image);
    loadRightImage();
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

    //QImage::Format f = QImage::Format::Format_A2BGR30_Premultiplied;

    //QTextStream(stdout) << image.format() << '\n';

    if (image.isNull()){
        QTextStream(stderr) << "Failed to load image";
        return;
    }

    original_scene->clear();
    //original_scene = new QGraphicsScene(this);
    ui->graphicsViewLeft->setScene(original_scene);

    modified_image = QImage(image);
    this->item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
    //this->item->setFlag(QGraphicsItem::ItemIsMovable);
    original_scene->addItem(this->item);

    loadRightImage();

}

void PMIG::newImage() {

    bool ok;

    QList<int> size = NewImageDialog::getVals(this, &ok);

    if (!ok){
        QTextStream(stdout) << "Cancelled\n";
        return;
    }

    image = QImage(size[0], size[1], QImage::Format::Format_RGB32);

    image.fill(QColor::fromRgb(255,255,255));

    original_scene->clear();
    //original_scene = new QGraphicsScene(this);
    ui->graphicsViewLeft->setScene(original_scene);

    modified_image = QImage(image);
    this->item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
    //this->item->setFlag(QGraphicsItem::ItemIsMovable);
    original_scene->addItem(this->item);

    loadRightImage();

}

void PMIG::slot_newImage(){
    PMIG::newImage();
}

void PMIG::slot_saveImage(){

    //if (this->new_scene->)

    if(this->modified_image.isNull()){
        scream("Save error", QMessageBox::Critical, "Failed to Save image : No Image Found", "Nothing to save", "No save: No Image found\n");
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
    //new_scene->clear();
    //delete new_scene;
    //modified_item = new QGraphicsPixmapItem(QPixmap::fromImage(modified_image));
    //new_scene = new CustomGraphicsScene(this);
    //new_scene->addItem(modified_item);
    //this->modified_item->setFlag(QGraphicsItem::ItemIsMovable);
    new_scene->setBackgroundImage(&modified_image);
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

    delete sample_item;
    delete sample_scene;

}

void PMIG::initFilterLists(){
    //QListWidget *list;
    for (int i = 0; i < filters.size(); i++){
        pushFilter(filters[i], i);
        //QTextStream(stdout) << ui->listWidget_Functional->count() << "\n";
    }
}

void PMIG::pushFilter(Filters::BaseFilter *filter, int ind){
    QListWidgetItem *item;
    switch (filter->getClass()){
    case Filters::FilterClass::FILTER_FUNCTIONAL:
        item = new QListWidgetItem(QString(filter->getName()),
                                   ui->listWidget_Functional,
                                   QListWidgetItem::ItemType::UserType);
        //ui->listWidget_Functional->addItem(QString(filters[i]->getName()));
        item->setData(Qt::UserRole + 0x1, QVariant(ind));
        this->ui->listWidget_Functional->addItem(item);
        break;
    case Filters::FilterClass::FILTER_CONVOLUTION:
        item = new QListWidgetItem(QString(filter->getName()),
                                   ui->listWidget_Convolutional,
                                   QListWidgetItem::ItemType::UserType);
        item->setData(Qt::UserRole + 0x1, QVariant(ind));
        this->ui->listWidget_Convolutional->addItem(item);
        break;
    case Filters::FilterClass::FILTER_OTHER:
        item = new QListWidgetItem(QString(filter->getName()),
                                   ui->listWidget_other,
                                   QListWidgetItem::ItemType::UserType);
        item->setData(Qt::UserRole + 0x1, QVariant(ind));
        this->ui->listWidget_other->addItem(item);
        break;
    default:
        throw new std::invalid_argument("Unknown filter type");
        break;
    }
    listEntries.push_back(item);
}

void PMIG::slot_setCustomConv(){
    int rows = this->ui->spinBox_Height->value();
    int columns = this->ui->spinBox_Width->value();

    ui->customConvTable->del();
    ui->customConvTable->init(rows, columns);
}


void PMIG::slot_calcDivisor(){
    int divisor = ui->customConvTable->calcDivisor();

    ui->spinBox_Divisor->setValue(divisor);
}

void PMIG::slot_saveCustom(){
    int divisor = ui->spinBox_Divisor->value();
    if (divisor == 0){
        QTextStream(stdout) << "Division By 0\n";
        QMessageBox msg;
        msg.setWindowTitle("Filter error");
        msg.setIcon(QMessageBox::Critical);
        msg.setText("Failed to make filter : division by 0");
        msg.setInformativeText("Please refrain from dividing by 0");
        msg.exec();
        return;
    }
    int offset = ui->spinBox_Offset->value();
    QString name = "C | " + ui->lineEdit_FilterName->text();
    std::vector<int> matrix = ui->customConvTable->collectMatrix();
    QPoint anchor = ui->customConvTable->collectAnchor();
    int height = ui->customConvTable->rowCount();
    int width = ui->customConvTable->columnCount();

    filters.push_back(new ConvolutionFilters::CustomConvolution(name,
                        height, width, anchor, divisor, offset, matrix));

    this->pushFilter(filters[filters.size()-1], filters.size()-1);

}

void PMIG::slot_loadCustomConv(QListWidgetItem* item){
    Filters::ConvolutionFilter* filter = (Filters::ConvolutionFilter*)filters[item->data(Qt::UserRole + 0x1).toInt()];
    ui->spinBox_Width->setValue(filter->getWidth());
    ui->spinBox_Height->setValue(filter->getHeight());
    ui->spinBox_Divisor->setValue(filter->getDivisor());
    ui->spinBox_Offset->setValue(filter->getOffset());

    ui->customConvTable->del();
    std::vector<int> mat = filter->getMatrix();
    ui->customConvTable->init(filter->getHeight(), filter->getWidth(), &mat);

}


NewImageDialog::NewImageDialog(QWidget *parent) : QDialog(parent) {
    QFormLayout *mainLayout = new QFormLayout(this);

    QLabel *l1 = new QLabel(QString("Width"), this);
    QSpinBox *s1 = new QSpinBox(this);
    s1->setRange(1, 20000);

    mainLayout->addRow(l1, s1);

    fields << s1;

    QLabel *l2 = new QLabel(QString("Height"), this);
    QSpinBox *s2 = new QSpinBox(this);
    s2->setRange(1, 20000);

    fields << s2;

    mainLayout->addRow(l2, s2);

    QDialogButtonBox *db = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal, this);

    mainLayout->addRow(db);

    bool connect;

    connect = this->connect(db, &QDialogButtonBox::accepted,
                            this, &NewImageDialog::accept);

    Q_ASSERT(connect);

    connect = this->connect(db, &QDialogButtonBox::rejected,
                            this, &NewImageDialog::reject);

    Q_ASSERT(connect);

    this->setLayout(mainLayout);

    //free mainLayout;
}

QList<int> NewImageDialog::getVals(QWidget *parent, bool *ok){
    NewImageDialog *d = new NewImageDialog(parent);

    QList<int> list;

    const int ret = d->exec();

    if(ok){
        *ok = !!ret;
    }

    if (ret) {
        foreach (QSpinBox* field, d->fields) {
            list << field->value();
        }
    }

    d->deleteLater();

    return list;
}
