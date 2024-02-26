#include "basefilter.h"

namespace Filters {

BaseFilter::BaseFilter(FilterClass fc, const char * name, bool is_customisable) {
    Fclass = fc;
    FilterName = name;
}
BaseFilter::~BaseFilter(){}
FilterClass BaseFilter::getClass(){
    return Fclass;
}
const char * BaseFilter::getName(){
    return FilterName;
}

FunctionalFilter::FunctionalFilter(const char * name, bool is_customisable) : BaseFilter(FILTER_FUNCTIONAL, name, is_customisable){}
FunctionalFilter::~FunctionalFilter() {}



////
InversionFilter::InversionFilter() : FunctionalFilter("Inversion Filter", false) {}
InversionFilter::~InversionFilter() {}

QDialog InversionFilter::applyCustom(QImage *image){
    throw new std::logic_error("This filter is not customisable");
}

void InversionFilter::applyFilter(QImage *image){
    //Actual types assigned to pixels may vary between machines
    QRgb *row;
    for (int i=0; i<image->height(); i++){
        row = (QRgb*)image->scanLine(i);
        for (int j=0; j<image->width(); j++){
            row[j] = qRgb(255-qRed(row[j]), 255-qGreen(row[j]),
                          255-qBlue(row[j]));
        }
    }
}

} // namespace Filters
