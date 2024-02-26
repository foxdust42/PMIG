#include "basefilter.h"
#include <cmath>

namespace FunctionalFilters {

class BrightnessCorrectionFilter : public Filters::FunctionalFilter {
private:
    class BrightnessDialog : public QDialog {
        //Q_OBJECT
    public:
        explicit BrightnessDialog(QWidget *parent) {}
        static QStringList getStrings(QWidget *parent, bool *ok = nullptr);

    private:
        QList<QLineEdit*> fields;
    };

public:
    static const int base_offset = 10;
    BrightnessCorrectionFilter() :
        FunctionalFilter("Brightness Correction Filter", true){}
    ~BrightnessCorrectionFilter() {}

    void applyFilter(QImage *image, int offset){
        QRgb* row;
        for (int i=0; i<image->height(); i++) {
            row = (QRgb*)image->scanLine(i);
            for (int j=0; j<image->width(); j++){
                row[j] = qRgb(
                    std::min(255, offset + qRed(row[j])),
                    std::min(255, offset + qGreen(row[j])),
                    std::min(255, offset + qBlue(row[j]))
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

class ContrastEnchancementFilter : public Filters::FunctionalFilter {
public:
    ContrastEnchancementFilter() :
        FunctionalFilter("Contrast Enchancement Filter", false){}
    ~ContrastEnchancementFilter() {}

    constexpr static const double base_slope = 1.5;

    void applyFilter(QImage *image, double slope) {
        int down_shift = (int)std::floor(((base_slope*0.5) - 0.5)*255);
        QRgb* row;
        int red, green, blue;
        for (int i=0; i<image->height(); i++){
            row = (QRgb*)image->scanLine(i);
            for (int j = 0; j<image->width(); j++){
                red = (int)(qRed(row[j])*slope) - down_shift;
                green = (int)(qGreen(row[j])*slope) - down_shift;
                blue = (int)(qBlue(row[j])*slope) - down_shift;
                row[j] = qRgb(
                    std::min(std::max(red, 0), 255),
                    std::min(std::max(green, 0), 255),
                    std::min(std::max(blue, 0), 255)
                    );
            }
        }
    }

    void applyFilter(QImage *image) {
        this->applyFilter(image, base_slope);
    }
};

class GammaCorectionFilter : public Filters::FunctionalFilter {
public:
    GammaCorectionFilter() :
        FunctionalFilter("Gamma Correction Filter", false){}
    ~GammaCorectionFilter() {}

    constexpr static const double base_gamma = 0.45;

    void applyFilter(QImage *image, double gamma) {

        QRgb* row;
        int red, green, blue;
        for (int i=0; i<image->height(); i++){
            row = (QRgb*)image->scanLine(i);
            for (int j = 0; j<image->width(); j++){
                red = (int)((std::pow(qRed(row[j])/255.0, gamma))*255);
                green = (int)((std::pow(qGreen(row[j])/255.0, gamma))*255);
                blue = (int)((std::pow(qBlue(row[j])/255.0, gamma))*255);
                row[j] = qRgb(red, green, blue);
                /*
                row[j] = qRgb(
                    std::min(std::max(red, 0), 255),
                    std::min(std::max(green, 0), 255),
                    std::min(std::max(blue, 0), 255)
                    );
                */
            }
        }
    }

    void applyFilter(QImage *image) {
        this->applyFilter(image, base_gamma);
    }
};

}
