#ifndef CUSTOMFILTERWIDGET_CUSTOMCONVFILTERTABLE_H
#define CUSTOMFILTERWIDGET_CUSTOMCONVFILTERTABLE_H

#include <QObject>
#include <QTableWidget>

// CustomFilterWidget::CustomConvFilterTable
namespace CustomFilterWidget {

class CustomConvFilterTable : public QTableWidget
{
    Q_OBJECT
public:
    CustomConvFilterTable(QWidget *parent = nullptr) : QTableWidget(parent) {}
    CustomConvFilterTable(int rows, int columns, QWidget *parent = nullptr) :
        QTableWidget(rows, columns, parent) {}
    ~CustomConvFilterTable() {}
};

} // namespace CustomFilterWidget

#endif // CUSTOMFILTERWIDGET_CUSTOMCONVFILTERTABLE_H
