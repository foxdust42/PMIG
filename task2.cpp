#include "task2.h"
#include <QMessageBox>
#include <QTextStream>
#include <QInputDialog>
#include <QApplication>
namespace Task2 {

typedef unsigned long long ull;

void AverageDithering::applyFilter(QImage *image){
    ull cum_avg = 0;
    ull cum_r = 0;
    ull cum_g = 0;
    ull cum_b = 0;
    ull n = 0;

    bool is_gray = image->isGrayscale();
    bool ok;
    unsigned int channels;
    while (true){

    channels = QInputDialog::getInt(
                            QApplication::activeWindow(),
                            "Set number of channels",
                            "Channels:",
                            2,
                            2,
                            256,
                            1,
                            &ok);

        if (!ok){
            QTextStream(stdout) << "operation cancelled\n";
            return;
        }

        if ((channels & (channels-1)) == 0 && channels != 0){
            //channels is a power of 2
            break;
        }
        QTextStream(stdout) << "Not a power of 2";
        QMessageBox msg;
        msg.setWindowTitle("Invalid channel no");
        msg.setIcon(QMessageBox::Warning);
        msg.setText("value must be a power of 2");
        msg.exec();
    }
    std::vector<int> thresholds(channels - 1, 0);
    std::vector<int> thresholds_r(channels - 1, 0);
    std::vector<int> thresholds_g(channels - 1, 0);
    std::vector<int> thresholds_b(channels - 1, 0);

    QRgb *row;

    int js = channels/4;

    if (is_gray){
        for (int i=0; i<image->height(); i++){
            row = (QRgb*)image->scanLine(i);
            ull loc_sum = 0;
            for (int j=0; j<image->width(); j++){
                loc_sum += qGray(row[j]);
                //QTextStream(stdout) << cum_avg << ", ";
            }
            cum_avg += loc_sum/image->width();
        }
        cum_avg/=image->height();

        thresholds[channels/2 -1] = cum_avg;
        while (js != 0){
            for (int i=js; i<=thresholds.size(); i+=2*js){
                //if (i % js*2 == 0) {continue;}
                thresholds[i-1] = ( (i == js ? 0 : thresholds[i-1-js]) +
                                     ( (i + js -1) >= thresholds.size() ? 255 : thresholds[i-1+js]) )/2;
                //QTextStream(stdout) << (i + js -1 ) << " : " << thresholds.size()
            }
            js /= 2;
        }

        std::vector<int> values(channels, 0);

        values[0] = 0;
        values[channels-1]=255;

        for(int i=1; i < values.size()-1; i++){
            values[i] = (thresholds[i-1] + thresholds[i])/2;
        }

        int cross=0;
        for (int i=0; i<image->height(); i++){
            row = (QRgb*)image->scanLine(i);
            for (int j=0; j<image->width(); j++){
                cross = 0;
                while(thresholds[cross] < qGray(row[j]) && cross < thresholds.size()){
                    cross++;
                }
                row[j] = qRgb(values[cross], values[cross], values[cross]);
            }
        }
    }
    else {
        for (int i=0; i<image->height(); i++){
            row = (QRgb*)image->scanLine(i);
            ull loc_sum_r = 0;
            ull loc_sum_g = 0;
            ull loc_sum_b = 0;
            for (int j=0; j<image->width(); j++){
                loc_sum_r += qRed(row[j]);
                loc_sum_g += qGreen(row[j]);
                loc_sum_b += qBlue(row[j]);
            }
            cum_r += loc_sum_r/image->width();
            cum_g += loc_sum_g/image->width();
            cum_b += loc_sum_b/image->width();
        }
        cum_r/=image->height();
        cum_g/=image->height();
        cum_b/=image->height();

        thresholds_r[channels/2 -1] = cum_r;
        thresholds_g[channels/2 -1] = cum_g;
        thresholds_b[channels/2 -1] = cum_b;
        while (js != 0){
            for (int i=js; i<=thresholds.size(); i+=2*js){
                thresholds_r[i-1] = ( (i == js ? 0 : thresholds_r[i-1-js]) +
                                     ( (i + js -1) >= thresholds_r.size() ? 255 : thresholds_r[i-1+js]) )/2;
                thresholds_g[i-1] = ( (i == js ? 0 : thresholds_g[i-1-js]) +
                                       ( (i + js -1) >= thresholds_g.size() ? 255 : thresholds_g[i-1+js]) )/2;
                thresholds_b[i-1] = ( (i == js ? 0 : thresholds_b[i-1-js]) +
                                       ( (i + js -1) >= thresholds_b.size() ? 255 : thresholds_b[i-1+js]) )/2;
            }
            js /= 2;
        }

        std::vector<int> values_r(channels, 0);
        std::vector<int> values_g(channels, 0);
        std::vector<int> values_b(channels, 0);

        values_r[0] = 0;
        values_r[channels-1]=255;
        values_g[0] = 0;
        values_g[channels-1]=255;
        values_b[0] = 0;
        values_b[channels-1]=255;

        for(int i=1; i < values_r.size()-1; i++){
            values_r[i] = (thresholds_r[i-1] + thresholds_r[i])/2;
            values_g[i] = (thresholds_g[i-1] + thresholds_g[i])/2;
            values_b[i] = (thresholds_b[i-1] + thresholds_b[i])/2;
        }

        int cross_r=0;
        int cross_g=0;
        int cross_b=0;
        for (int i=0; i<image->height(); i++){
            row = (QRgb*)image->scanLine(i);
            for (int j=0; j<image->width(); j++){
                cross_r = 0;
                cross_g = 0;
                cross_b = 0;
                while(thresholds_r[cross_r] < qRed(row[j]) && cross_r < thresholds_r.size()){
                    cross_r++;
                }
                while(thresholds_g[cross_g] < qGreen(row[j]) && cross_g < thresholds_g.size()){
                    cross_g++;
                }
                while(thresholds_b[cross_b] < qBlue(row[j]) && cross_b < thresholds_b.size()){
                    cross_b++;
                }
                row[j] = qRgb(values_r[cross_r], values_g[cross_g], values_b[cross_b]);
            }
        }
    }

}

} // namespace Task2
