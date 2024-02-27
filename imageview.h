#ifndef WIDGETS_IMAGEVIEW_H
#define WIDGETS_IMAGEVIEW_H

#include <QObject>
#include <QGraphicsView>
#include <QWidget>
#include <QTextStream>

namespace Widgets {

class ImageView : public QGraphicsView
{
    Q_OBJECT
    /*
    *
    */
public:
    ImageView(QWidget *parent = nullptr) : QGraphicsView(parent) {}
    ImageView(QGraphicsScene *scene, QWidget *parent = nullptr) : QGraphicsView(scene, parent) {}
    ImageView(QGraphicsViewPrivate &a, QWidget *parent = nullptr) : QGraphicsView(a, parent) {}
    ~ImageView() {}

protected:
    void mousePressEvent(QMouseEvent *event) override;
    //void mouseMoveEvent(QMouseEvent *event) override;
    //void mouseReleaseEvent(QMouseEvent *event) override;
    //void paintEvent(QPaintEvent *event) override;
    //void resizeEvent(QResizeEvent *event) override;

    void wheelEvent (QWheelEvent *event) override;


private:

private slots:
};

} // namespace Widgets

#endif // WIDGETS_IMAGEVIEW_H
