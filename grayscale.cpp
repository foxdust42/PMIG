#include "grayscale.h"
#include <QTextStream>
namespace Task2 {

void LightnessGray::toGray(QImage *image){
    QRgb *row;
    QRgb r, g, b, val;
    for (int i=0; i<image->height(); i++){
        row = (QRgb*)image->scanLine(i);
        for (int j=0; j<image->width(); j++){
            r = qRed(row[j]);
            g = qGreen(row[j]);
            b = qBlue(row[j]);
            val = std::min(std::min(r, g), b);
            val += std::max(std::max(r, g), b);
            val /= 2;
            row[j] = qRgb(val, val, val);
        }
    }
    QTextStream(stdout) << "G? " << image->isGrayscale() << "\n";
}

void AverageGray::toGray(QImage *image){
    QRgb *row;
    QRgb r, g, b, val;
    for (int i=0; i<image->height(); i++){
        row = (QRgb*)image->scanLine(i);
        for (int j=0; j<image->width(); j++){
            r = qRed(row[j]);
            g = qGreen(row[j]);
            b = qBlue(row[j]);
            val = (r + b + g) / 3;
            row[j] = qRgb(val, val, val);
        }
    }
    QTextStream(stdout) << "G? " << image->isGrayscale() << "\n";
}

void LuminosityGray::toGray(QImage *image){
    QRgb *row;
    QRgb r, g, b, val;
    for (int i=0; i<image->height(); i++){
        row = (QRgb*)image->scanLine(i);
        for (int j=0; j<image->width(); j++){
            r = qRed(row[j]);
            g = qGreen(row[j]);
            b = qBlue(row[j]);
            val = (r * 0.3) + (g * 0.59) + (b * 0.11);
            row[j] = qRgb(val, val, val);
        }
    }
    QTextStream(stdout) << "G? " << image->isGrayscale() << "\n";
}


} // namespace Task2
