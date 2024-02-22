#include "basefilter.h"

namespace FunctionalFilters {

class BrightnessCorrectionFilter : public Filters::FunctionalFilter {
private:
    class BrightnessDialog : public QDialog {
        Q_OBJECT
    public:
        explicit BrightnessDialog(QWidget *parent) {}
        static QStringList getStrings(QWidget *parent, bool *ok = nullptr);

    private:
        QList<QLineEdit*> fields;
    };

public:
    static const int base_offset = 20;
    BrightnessCorrectionFilter() :
        FunctionalFilter("Brightness Correction Filter", true){}
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

    QDialog applyCustom(QImage *image){
        throw new std::runtime_error("Not Implemented");
    }
};



}
