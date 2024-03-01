#ifndef CUSTOMFILTERWIDGET_CUSTOMCONVFILTERTABLE_H
#define CUSTOMFILTERWIDGET_CUSTOMCONVFILTERTABLE_H

#include <qspinbox.h>
#include <QObject>
#include <QTableWidget>
#include <QMouseEvent>
#include <QTextStream>

// CustomFilterWidget::CustomConvFilterTable
namespace CustomFilterWidget {

class CustomConvFilterTable : public QTableWidget
{   
    Q_OBJECT
private:
    QSpinBox *anchor = nullptr;
    int anchor_row;
    int anchor_col;
    const QBrush anchorBackground = QBrush(QColor(qRgb(200,0,0)));
    const QBrush anchorHighlight = QBrush(QColor(qRgb(200, 0, 200)));
    void init();

private slots:
    void slot_DClick(int row, int column);

public:
    CustomConvFilterTable(QWidget *parent = nullptr) : QTableWidget(parent) {
        QObject::connect(this, &QTableWidget::cellChanged,
                         this, &CustomConvFilterTable::slot_DClick);
    }
    CustomConvFilterTable(int rows, int columns, QWidget *parent = nullptr) :
        QTableWidget(rows, columns, parent) {
        QObject::connect(this, &QTableWidget::cellDoubleClicked,
                         this, &CustomConvFilterTable::slot_DClick);
    }
    ~CustomConvFilterTable() {
        this->del();
    }

    void mousePressEvent (QMouseEvent *event) override;

    void setAnchor(int row, int column, bool reset = false);

    void setSize(int rows, int columns);

    int calcDivisor();

    void init(int rows, int columns, std::vector<int>* mat = nullptr);

    void del();

    std::vector<int> collectMatrix();

    QPoint collectAnchor();
};

class CustomSpinBox : public QSpinBox {
private:
    int row;
    int column;
    CustomConvFilterTable* owner;
public:
    explicit CustomSpinBox(int row, int column, CustomConvFilterTable *parent = nullptr) : QSpinBox(parent){
        this->row = row;
        this->column = column;
        this->owner = (CustomConvFilterTable*)parent;
        this->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    }

    void mousePressEvent(QMouseEvent *event) override;

};

} // namespace CustomFilterWidget

#endif // CUSTOMFILTERWIDGET_CUSTOMCONVFILTERTABLE_H
