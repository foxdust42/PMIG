#include "basefilter.h"

namespace Filters {

class BrightnessCorrectionFilter : public FunctionalFilter {
public:
    static const int base_offset = 20;
    BrightnessCorrectionFilter() : FunctionalFilter(){}
    ~BrightnessCorrectionFilter();

    void applyFilter(QImage *image){
        QRgb* row;
        for (int i=0; i<image->height(); i++) {
            row = (QRgb*)image->scanLine(i);
            for (int j=0; j<image->width(); j++){
                row[j] = qRgb(
                    std::max(255, base_offset + qRed(row[j])),
                    std::max(255, base_offset + qGreen(row[j])),
                    std::max(255, base_offset + qBlue(row[j]))
                    );
            }
        }
    }
};

}
