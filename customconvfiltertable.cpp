#include "customconvfiltertable.h"
#include <QTextStream>

namespace CustomFilterWidget {

//CustomConvFilterTable::CustomConvFilterTable() {}

void CustomConvFilterTable::mousePressEvent(QMouseEvent *event){
    QTableWidget::mousePressEvent(event);
    if (event->button() == Qt::MouseButton::RightButton){
        QTextStream(stdout) << "Hello\n";
        this->setAnchor(
            this->currentItem()->row(),
            this->currentItem()->column()
            );
    }

}

void CustomConvFilterTable::setAnchor(int row, int column){
    QTableWidgetItem *new_anchor = this->item(row, column);
    if (new_anchor == nullptr){
        QTextStream(stdout) << "setAnchor: No such cell\n";
        return;
    }

    if (this->anchor != nullptr){
        this->anchor->setBackground(new_anchor->background());
        //this->anchor->setForeground(new_anchor->foreground());
    }
    new_anchor->setBackground(this->anchorBackground);
    //new_anchor->setForeground(this->anchorHighlight);
    this->anchor = new_anchor;
    QTextStream(stdout) << "New anchor: (" << this->anchor->row() << ", " << this->anchor->column() << ")\n";
}

} // namespace CustomFilterWidget
