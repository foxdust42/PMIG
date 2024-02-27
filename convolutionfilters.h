#ifndef CONVOLUTIONFILTERS_H
#define CONVOLUTIONFILTERS_H

#include "basefilter.h"
#include <cmath>
#include <QTextStream>

namespace ConvolutionFilters {

class BlurFilter : public Filters::ConvolutionFilter {
public:
    BlurFilter() : Filters::ConvolutionFilter("Blur Filter") {
        m_height = 3;
        m_width = 3;
        anchor = QPoint(1,1);
        matrix = this->generateMatrix();
        for (int i=0; i<m_height; i++){
            for (int j=0; j<m_width; j++){
                matrix[i][j] = 1;
            }
        }
    }
    ~BlurFilter() {
        this->destroyMatrix();
    }
};

class GaussBlurFilter : public Filters::ConvolutionFilter {
private:
    int makeGaussian(int x, int y, double sigma){
        QTextStream(stdout) << "\n(" << x << " " << y << "): ";
        double ans = - (std::pow(x, 2) + std::pow(y,2))/(2*std::pow(sigma,2));
        QTextStream(stdout) << ans <<" : ";
        ans = std::exp(ans);
        QTextStream(stdout) << ans <<" : ";
        ans *= (1/(2*M_PI*std::pow(sigma, 2)));
        QTextStream(stdout) << ans <<" : ";
        return (int) std::round(ans);
    }
public:
    GaussBlurFilter() : Filters::ConvolutionFilter("Gaussian Blur") {
        m_height = 3;
        m_width = 3;
        anchor = QPoint(1,1);
        double sigma = 0.5;
        matrix = this->generateMatrix();
        for (int i=0; i<m_height; i++){
            for (int j=0; j<m_width; j++){
                matrix[i][j] = makeGaussian(j - anchor.x(),
                                            i - anchor.y(),
                                            sigma);
                QTextStream(stdout) << matrix[i][j] << " ";
            }
            QTextStream(stdout) << "\n";
        }
        QTextStream(stdout) << "\n";

    }
};

}

#endif // CONVOLUTIONFILTERS_H
