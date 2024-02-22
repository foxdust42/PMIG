#include "basefilter.h"

namespace FunctionalFilters {

class BrightnessCorrectionFilter : public Filters::FunctionalFilter {
public:
    static const int base_offset = 20;
    BrightnessCorrectionFilter() :
        FunctionalFilter("Brightness Correction Filter"){}
    ~BrightnessCorrectionFilter() {}

    void applyFilter(QImage *image, int offset){
        QRgb* row;
        for (int i=0; i<image->height(); i++) {
            row = (QRgb*)image->scanLine(i);
            for (int j=0; j<image->width(); j++){
                row[j] = qRgb(
                    std::max(255, offset + qRed(row[j])),
                    std::max(255, offset + qGreen(row[j])),
                    std::max(255, offset + qBlue(row[j]))
                    );
            }
        }
    }

    void applyFilter(QImage *image){
        this->applyFilter(image, base_offset);
    }
};

}
