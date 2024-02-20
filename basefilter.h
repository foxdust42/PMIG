#ifndef FILTERS_BASEFILTER_H
#define FILTERS_BASEFILTER_H

#include <QImage>

namespace Filters {

enum FilterClass{
    UNKNOWN,
    FILTER_FUNCTIONAL,
    FILTER_CONVOLUTION
};

class BaseFilter //Abstract filter class
{
protected:
    Filters::FilterClass Fclass;
    BaseFilter(FilterClass fc);
public:
    FilterClass getClass();
    virtual void applyFilter(QImage *image) = 0;
    virtual ~BaseFilter() = 0;
};

class FunctionalFilter : public BaseFilter {
public:
    FunctionalFilter();
    virtual void applyFilter(QImage *image) = 0;
    virtual ~FunctionalFilter() = 0;
};

class InversionFilter : public FunctionalFilter {
public:
    InversionFilter();
    ~InversionFilter();
    void applyFilter(QImage *image);
};

} // namespace Filters

#endif // FILTERS_BASEFILTER_H
