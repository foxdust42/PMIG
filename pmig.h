#ifndef PMIG_H
#define PMIG_H

#include <qgraphicsitem.h>
#include <QObject>
#include <QMainWindow>
#include <qgraphicsscene.h>
#include <qlistwidget.h>

#include "basefilter.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PMIG; }
QT_END_NAMESPACE

class PMIG : public QMainWindow
{
    Q_OBJECT

public:
    PMIG(QWidget *parent = nullptr);
    ~PMIG();

public slots:
    void slot_loadImage();
    void slot_listDClick(QListWidgetItem *item);
    void slot_resetImage();
    void slot_saveImage();
    void slot_setCustomConv();
    void slot_calcDivisor();
    void slot_saveCustom();
    void slot_loadCustomConv(QListWidgetItem*);

private:
    Ui::PMIG *ui;

    //QBrush highlight = QBrush(QColor(qRgb(200,0,0)));

    std::vector<Filters::BaseFilter*> filters;
    std::vector<QListWidgetItem*> listEntries;

    //image display based on https://stackoverflow.com/questions/1357960/qt-jpg-image-display
    QGraphicsScene *original_scene;
    QGraphicsScene *new_scene;
    //QGraphicsTextItem *text;
    QImage image;
    QImage modified_image;
    QGraphicsPixmapItem *item;
    QGraphicsPixmapItem *modified_item;

    //void initCustomConv(int rows, int columns);
    //void deleteCustomConv();

    void loadImage();
    void loadRightImage();

    void initFilterLists();
    void pushFilter(Filters::BaseFilter *filter, int ind);


};
#endif // PMIG_H
