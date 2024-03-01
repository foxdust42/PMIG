#include "customconvfiltertable.h"
#include "qheaderview.h"
#include "qspinbox.h"
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

void CustomConvFilterTable::setAnchor(int row, int column, bool reset){
    QWidget *new_anchor = (QWidget*)this->cellWidget(row, column);
    if (new_anchor == nullptr){
        QTextStream(stdout) << "setAnchor: No such cell\n";
        return;
    }

    if (this->anchor != nullptr && reset == false){
        //this->anchor->setBackground(new_anchor->background());
        this->anchor->setStyleSheet("");
    }
    //new_anchor->setBackground(this->anchorBackground);
    new_anchor->setStyleSheet("background-color:red");
    this->anchor = (CustomSpinBox*)new_anchor;
    this->anchor_row = row;
    this->anchor_col = column;
    QTextStream(stdout) << "New anchor: (" << this->anchor_row << ", " << this->anchor_col << ")\n";
}

int CustomConvFilterTable::calcDivisor(){
    int sum = 0;
    for (int i=0; i<this->rowCount(); i++){
        for (int j=0; j<this->columnCount(); j++){
            sum += ((QSpinBox*)this->cellWidget(i, j))->value();
        }
    }
    if (sum == 0){
        return 1;
    }
    return sum;
}

void CustomConvFilterTable::init(int rows, int columns, std::vector<int>* mat){
    setRowCount(rows);
    setColumnCount(columns);
    verticalHeader()->setDefaultSectionSize(1);
    horizontalHeader()->setDefaultSectionSize(1);

    CustomSpinBox* tableItem;
    for (int i=0; i<this->rowCount(); i++){
        for (int j=0; j<this->columnCount(); j++){
            //tableItem = new QTableWidgetItem(tr("%1").arg((i+1)*(j+1)));
            tableItem = new CustomSpinBox(i, j, this);
            tableItem->setMinimum(INT_MIN);
            tableItem->setMaximum(INT_MAX);
            //QObject::connect(tableItem, )
            this->setCellWidget(i, j, tableItem);
            if (mat != nullptr){
                ((QSpinBox*)this->cellWidget(i, j))->setValue(mat->at(j * this->columnCount() + i));
            }
            else{
                ((QSpinBox*)this->cellWidget(i, j))->setValue( (i+1)*(j+1) );
            }

            //ui->customConvTable->setItem(i, j, tableItem);
        }
    }

    this->setAnchor(rows/2, columns/2, true);
    //ui->customConvTable->item(1,1)->setBackground(this->highlight);

    this->update();
}

void CustomConvFilterTable::del(){
    for (int i=0; i<this->rowCount(); i++){
        for (int j=0; j<this->columnCount(); j++){
            delete this->item(i,j);
        }
    }
}

void CustomConvFilterTable::slot_DClick(int row, int column){
    QTextStream(stdout) << "slot\n";
}

void CustomSpinBox::mousePressEvent(QMouseEvent *event){
    QTextStream(stdout)<<"aaa\n";
    if(event->button() == Qt::MouseButton::RightButton){
        this->owner->setAnchor(this->row, this->column);
    }
    else{
        QSpinBox::mousePressEvent(event);
    }
}

std::vector<int> CustomConvFilterTable::collectMatrix(){
    std::vector<int> ans;
    int val;
    for (int i=0; i<this->rowCount(); i++){
        for (int j=0; j<this->columnCount(); j++){
            val = ((CustomSpinBox*)this->cellWidget(j, i))->value();
            ans.push_back(val);
        }
    }
    return ans;
}

QPoint CustomConvFilterTable::collectAnchor(){
    return QPoint(this->anchor_row, this->anchor_col);
}

} // namespace CustomFilterWidget
