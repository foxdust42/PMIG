#ifndef FILTERS_BASEFILTER_H
#define FILTERS_BASEFILTER_H

#include <qabstractspinbox.h>
#include <QImage>
#include <QDialog>

namespace Filters {

enum FilterClass{
    UNKNOWN,
    FILTER_FUNCTIONAL,
    FILTER_CONVOLUTION
};

class BaseFilter //Abstract filter class
{
protected:
    QString FilterName;
    Filters::FilterClass Fclass;
    bool is_customisable;
    BaseFilter(FilterClass fc, QString, bool is_customisable);
public:
    FilterClass getClass();
    QString getName();
    virtual void applyFilter(QImage *image) = 0;
    //virtual QDialog applyCustom(QImage * image) = 0;
    virtual ~BaseFilter() = 0;
};

class FunctionalFilter : public BaseFilter {
public:
    FunctionalFilter(QString, bool is_customisable);
    virtual void applyFilter(QImage *image) = 0;
    //virtual QDialog applyCustom(QImage *image) = 0;
    virtual ~FunctionalFilter() = 0;
};


////
class InversionFilter : public FunctionalFilter {
private:
    //https://stackoverflow.com/questions/17512542/getting-multiple-inputs-from-qinputdialog-in-qt

public:
    InversionFilter();
    ~InversionFilter();
    void applyFilter(QImage *image);
    void applyFilter(QImage *image, int offset);
    QDialog applyCustom(QImage *image);
};
////

class ConvolutionFilter : public BaseFilter {
protected:
    int m_height;
    int m_width;
    int **matrix;
    int divisor;
    int offset = 0;
    QPoint anchor;

    QRgb apply_matrix(QImage *image, int divisor, int x, int y);
    int** generateMatrix();
    void destroyMatrix();

    int clamp(int val);

public:
    ConvolutionFilter(QString name) :
        BaseFilter(FILTER_CONVOLUTION, name, true) {}
    virtual ~ConvolutionFilter() = 0;
    void applyFilter(QImage *image);


};

} // namespace Filters

#endif // FILTERS_BASEFILTER_H
