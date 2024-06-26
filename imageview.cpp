#include "imageview.h"

#include <QWheelEvent>

namespace Widgets {

//ImageView::ImageView() {}
void ImageView::mousePressEvent(QMouseEvent *event){
    QGraphicsView::mousePressEvent(event);


    //QTextStream(stdout) << event->pos().x() << " " << event->pos().y() << "\n";
    //QTextStream(stdout) << event->localPos().x() << " " << event->localPos().y() << "\n";
    //this->scream();
}

void ImageView::wheelEvent(QWheelEvent *event) {
    //https://doc.qt.io/qt-6/qwheelevent.html#angleDelta
    QPoint point = event->angleDelta() / 8;
    qreal factor = qreal(point.y()) / 300.0;
    factor += 1;
    QTextStream(stdout) << factor << "\n";
    this->scale(factor, factor);

    event->accept();
}

} // namespace Widgets
