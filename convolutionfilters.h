#ifndef CONVOLUTIONFILTERS_H
#define CONVOLUTIONFILTERS_H

#include "basefilter.h"
#include <cmath>
#include <QTextStream>

namespace ConvolutionFilters {

class BlurFilter : public Filters::ConvolutionFilter {
public:
    BlurFilter() : Filters::ConvolutionFilter("Blur Filter") {
        m_height = 5;
        m_width = 5;
        anchor = QPoint(1,1);
        matrix = this->generateMatrix();
        for (int i=0; i<m_height; i++){
            for (int j=0; j<m_width; j++){
                matrix[i][j] = 1;
            }
        }
        this->divisor = m_height * m_width;
    }
    ~BlurFilter() {
        this->destroyMatrix();
    }
};

class GaussBlurFilter : public Filters::ConvolutionFilter {
private:
    double makeGaussian(int x, int y, double sigma){
        //QTextStream(stdout) << "\n(" << x << " " << y << "): ";
        double ans = - (std::pow(x, 2) + std::pow(y,2))/(2*std::pow(sigma,2));
        //QTextStream(stdout) << ans <<" : ";
        ans = std::exp(ans);
        //QTextStream(stdout) << ans <<" : ";
        ans *= (1/(2*M_PI*std::pow(sigma, 2)));
        //QTextStream(stdout) << ans <<" : ";
        return ans;
    }
public:
    GaussBlurFilter() : Filters::ConvolutionFilter("Gaussian Blur") {
        m_height = 5;
        m_width = 5;
        //Indexing from 0
        anchor = QPoint(2,2);
        double sigma = 0.85;
        /*
         *  For 3x3:
         *  sigma = 0.5
         *
         *  For 5x5:
         *  sigma = 0.85
        */
        int div = 0;
        double correction_factor = 0.0;
        double ** tmp_matrix = new double*[m_height];
        for (int i=0; i<m_height; i++){
            tmp_matrix[i] = new double[m_width];
        }

        matrix = this->generateMatrix();

        for (int i=0; i<m_height; i++){
            for (int j=0; j<m_width; j++){
                tmp_matrix[i][j] = makeGaussian(j - (int)std::floor(m_width/2),
                                            i - (int)std::floor(m_height/2),
                                            sigma);
            }
            //QTextStream(stdout) << "\n";
        }
        //QTextStream(stdout) << "\n";

        correction_factor = tmp_matrix[(int)std::floor(m_height/2)][(int)std::floor(m_width/2)];

        correction_factor = std::pow(2, m_height-1) / correction_factor;

        for (int i=0; i<m_height; i++){
            for (int j=0; j<m_width; j++){
                matrix[i][j] = (int)std::round(tmp_matrix[i][j] * correction_factor);
                div += matrix[i][j];
                //QTextStream(stdout) << matrix[i][j] << "    ";
                //QTextStream(stdout) << "(" << j << " " << i << "): " << tmp_matrix[i][j] << " -> " << tmp_matrix[i][j]*correction_factor << " -> " << matrix[i][j] << "\n";
            }
            //QTextStream(stdout) << "\n";
        }

        for (int i=0; i<m_height; i++){
            delete[] tmp_matrix[i];
        }
        delete[] tmp_matrix;
        if (div == 0){
            this->divisor = 1;
        }
        else {
            this->divisor = div;
        }
    }

    ~GaussBlurFilter(){
        this->destroyMatrix();
    }
};

class SharpenFilter : public Filters::ConvolutionFilter {
public:
    SharpenFilter() : Filters::ConvolutionFilter("Sharpen Filter") {
        m_height = 3;
        m_width = 3;
        anchor = QPoint(1,1);
        int a = 1;
        int b = 5;
        int sum = 0;


        matrix = this->generateMatrix();
        int s = b - 4*a;

        for(int i=0; i<m_height; i++){
            for (int j=0; j<m_width; j++){
                if (i == 1 && j == 1){
                    matrix[i][j] = b/s;
                }
                else if (j == 1 || i == 1){
                    matrix[i][j] = -a/s;
                }
                else {
                    matrix[i][j] = 0;
                }
                sum += matrix[i][j];
            }
        }
        if (sum == 0){
            this->divisor = 1;
        }
        else{
            this->divisor = sum;
        }
    }
    ~SharpenFilter(){
        this->destroyMatrix();
    }
};

class EdgeDetectionDiag : public Filters::ConvolutionFilter{
public:
    EdgeDetectionDiag() : Filters::ConvolutionFilter("Diagonal Edge Detection"){
        m_height = 3;
        m_width = 3;
        anchor = QPoint(1,1);
        matrix = this->generateMatrix();

        this->divisor = 1;

        for (int i=0; i<m_height; i++){
            for (int j=0; j<m_width; j++){
                matrix[i][j] = 0;
            }
        }
        matrix[1][1] = 1;
        matrix[0][0] = -1;

    }
    ~EdgeDetectionDiag(){
        this->destroyMatrix();
    }
};

class SouthEmoss : public Filters::ConvolutionFilter{
public:
    SouthEmoss() : Filters::ConvolutionFilter("South Emboss"){
        m_height = 3;
        m_width = 3;
        anchor = QPoint(1,1);        
        matrix = this->generateMatrix();

        this->divisor = 1;
        for (int i=0; i<m_height; i++){
            for (int j=0; j<m_width; j++){
                matrix[i][j] = i-1;
            }
        }
        matrix[1][1] = 1;
    }
    ~SouthEmoss(){
        this->destroyMatrix();
    }
};

}

#endif // CONVOLUTIONFILTERS_H
