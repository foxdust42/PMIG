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
    void slot_applyInv();
    void slot_listFunctional(QListWidgetItem *item);

private:
    Ui::PMIG *ui;

    std::vector<Filters::BaseFilter*> filters;

    //image display based on https://stackoverflow.com/questions/1357960/qt-jpg-image-display
    QGraphicsScene *original_scene;
    QGraphicsScene *new_scene;
    QGraphicsTextItem *text;
    QImage image;
    QImage modified_image;
    QGraphicsPixmapItem *item;
    QGraphicsPixmapItem *modified_item;

    void loadImage();
    void loadRightImage();

    void initFilterLists();



};
#endif // PMIG_H
