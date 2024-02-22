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
    const char * FilterName;
    Filters::FilterClass Fclass;
    bool is_customisable;
    BaseFilter(FilterClass fc, const char *, bool is_customisable);
public:
    FilterClass getClass();
    const char * getName();
    virtual void applyFilter(QImage *image) = 0;
    virtual QDialog applyCustom(QImage * image) = 0;
    virtual ~BaseFilter() = 0;
};

class FunctionalFilter : public BaseFilter {
public:
    FunctionalFilter(const char *, bool is_customisable);
    virtual void applyFilter(QImage *image) = 0;
    virtual QDialog applyCustom(QImage *image) = 0;
    virtual ~FunctionalFilter() = 0;
};

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

} // namespace Filters

#endif // FILTERS_BASEFILTER_H
