#include "basefilter.h"

namespace Filters {

BaseFilter::BaseFilter(FilterClass fc) {
    Fclass = fc;
}
BaseFilter::~BaseFilter(){}
FilterClass BaseFilter::getClass(){
    return Fclass;
}

FunctionalFilter::FunctionalFilter() : BaseFilter(FILTER_FUNCTIONAL){}
FunctionalFilter::~FunctionalFilter() {}

InversionFilter::InversionFilter() : FunctionalFilter() {}
InversionFilter::~InversionFilter() {}

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
