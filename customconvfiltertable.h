#ifndef CUSTOMFILTERWIDGET_CUSTOMCONVFILTERTABLE_H
#define CUSTOMFILTERWIDGET_CUSTOMCONVFILTERTABLE_H

#include <QObject>
#include <QTableWidget>
#include <QMouseEvent>

// CustomFilterWidget::CustomConvFilterTable
namespace CustomFilterWidget {

class CustomConvFilterTable : public QTableWidget
{   
    Q_OBJECT
private:
    QTableWidgetItem *anchor = nullptr;
    const QBrush anchorBackground = QBrush(QColor(qRgb(200,0,0)));
    const QBrush anchorHighlight = QBrush(QColor(qRgb(200, 0, 200)));
    void init();
public:
    CustomConvFilterTable(QWidget *parent = nullptr) : QTableWidget(parent) {}
    CustomConvFilterTable(int rows, int columns, QWidget *parent = nullptr) :
        QTableWidget(rows, columns, parent) {}
    ~CustomConvFilterTable() {}

    void mousePressEvent (QMouseEvent *event) override;

    void setAnchor(int row, int column, bool reset = false);

    void setSize(int rows, int columns);

    int calcDivisor();
};

} // namespace CustomFilterWidget

#endif // CUSTOMFILTERWIDGET_CUSTOMCONVFILTERTABLE_H
