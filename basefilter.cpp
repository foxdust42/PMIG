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

ConvolutionFilter::~ConvolutionFilter(){}
void ConvolutionFilter::applyFilter(QImage *image){
    //if filter attempts to query a pixel outside image bounds
    //use the closest pixel
    int x, y;
    QImage new_image = QImage(*image);
    QRgb* row;
    for (int i=0; i<new_image.height(); i++){
        row = (QRgb*)new_image.scanLine(i);
        for (int j=0; j<new_image.width(); j++){
            row[j] = apply_matrix(image, i, j);
        }
    }
    *image = new_image;
}

QRgb ConvolutionFilter::apply_matrix(QImage *image, int x, int y){
    int r=0, g=0, b=0;
    int sum=0;
    //int xt, yt;
    x -= anchor.x();
    y -= anchor.y();
    QColor c;
    for (int i=0; i<m_height; i++){
        for (int j=0; j<m_width; j++){
            c = image->pixelColor(std::max(std::min(y + i, image->width()),0),
                                  std::max(std::min(x + j, image->height()), 0));
            sum += matrix[i][j];
            r += c.red()*matrix[i][j];
            g += c.green()*matrix[i][j];
            b += c.blue()*matrix[i][j];
        }
    }
    if (sum == 0){
        sum = 1;
    }
    r /= sum;
    g /= sum;
    b /= sum;
    return qRgb(r, g, b);
}

int** ConvolutionFilter::generateMatrix(){
    int ** matrix = new int*[m_height];
    for (int i=0; i<m_width; i++){
        matrix[i] = new int[m_width];
    }
    return matrix;
}

void ConvolutionFilter::destroyMatrix(){
    for (int i=0; i<m_height; i++){
        delete[] matrix[i];
    }
    delete[] matrix;
}

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
