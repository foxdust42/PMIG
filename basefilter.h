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
    const char * FilterName;
    Filters::FilterClass Fclass;
    BaseFilter(FilterClass fc, const char *);
public:
    FilterClass getClass();
    const char * getName();
    virtual void applyFilter(QImage *image) = 0;
    virtual ~BaseFilter() = 0;
};

class FunctionalFilter : public BaseFilter {
public:
    FunctionalFilter(const char *);
    virtual void applyFilter(QImage *image) = 0;
    virtual ~FunctionalFilter() = 0;
};

class InversionFilter : public FunctionalFilter {
public:
    InversionFilter();
    ~InversionFilter();
    void applyFilter(QImage *image);
    void applyFilter(QImage *image, int offset);
};

} // namespace Filters

#endif // FILTERS_BASEFILTER_H
