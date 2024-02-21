#include "imageview.h"

namespace Widgets {

//ImageView::ImageView() {}
void ImageView::mousePressEvent(QMouseEvent *event){
    QGraphicsView::mousePressEvent(event);
    this->scream();
}

} // namespace Widgets
