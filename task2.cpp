#include "task2.h"
#include <QMessageBox>
#include <QTextStream>
#include <QInputDialog>
#include <QApplication>
#include <random>
#include <algorithm>
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

    std::vector<int> thresholds(channels - 1, 0);
    std::vector<int> thresholds_r(channels - 1, 0);
    std::vector<int> thresholds_g(channels - 1, 0);
    std::vector<int> thresholds_b(channels - 1, 0);

    QRgb *row;
    //int js = channels/4;

    ull count = image->width()*image->height();
    std::vector<ull> gray_pdf(256, 0);
    std::vector<ull> red_pdf(256, 0);
    std::vector<ull> green_pdf(256, 0);
    std::vector<ull> blue_pdf(256, 0);

    if (is_gray){
        QTextStream(stdout) << "adhaisd\n";
        for (int i=0; i < image->height(); i++){
            row = (QRgb*)image->scanLine(i);
            ull loc_sum = 0;
            for (int j=0; j < image->width(); j++){
                gray_pdf[qGray(row[j])] += 1;
            }
        }

        ull r_count = 0;
        int tmp = 1;
        for (int i = 0; i < 255; i++){
            r_count += gray_pdf[i];
            if ( (((double)r_count)/((double)(count))) > (((double)(tmp))/((double)(channels))) ) {
                thresholds[tmp-1] = i+1;
                tmp ++;
            }
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
        for (int i=0; i < image->height(); i++){
            row = (QRgb*)image->scanLine(i);
            ull loc_sum = 0;
            for (int j=0; j < image->width(); j++){
                red_pdf[qRed(row[j])] += 1;
                green_pdf[qGreen(row[j])] += 1;
                blue_pdf[qBlue(row[j])] += 1;
            }
        }

        ull red_count = 0;
        ull green_count = 0;
        ull blue_count = 0;
        int r_tmp = 1;
        int g_tmp = 1;
        int b_tmp = 1;
        for (int i = 0; i < 255; i++){
            red_count += red_pdf[i];
            green_count += green_pdf[i];
            blue_count += blue_pdf[i];
            if ( (((double)red_count)/((double)(count))) > (((double)(r_tmp))/((double)(channels))) ) {
                thresholds_r[r_tmp-1] = i+1;
                r_tmp ++;
            }
            if ( (((double)green_count)/((double)(count))) > (((double)(g_tmp))/((double)(channels))) ) {
                thresholds_g[g_tmp-1] = i+1;
                g_tmp ++;
            }
            if ( (((double)blue_count)/((double)(count))) > (((double)(b_tmp))/((double)(channels))) ) {
                thresholds_b[b_tmp-1] = i+1;
                b_tmp ++;
            }
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

void MedianCut::applyFilter(QImage *image){
    bool ok;
    unsigned int colors;
    while (true){
        colors = QInputDialog::getInt(
            QApplication::activeWindow(),
            "Set number of colors",
            "Colors:",
            2,
            2,
            INT_MAX,
            1,
            &ok);

        if (!ok){
            QTextStream(stdout) << "operation cancelled\n";
            return;
        }
        break;
        /*
        if ((colors & (colors-1)) == 0 && colors != 0){
            //channels is a power of 2
            break;
        }
        QTextStream(stdout) << "Not a power of 2";
        QMessageBox msg;
        msg.setWindowTitle("Invalid color no");
        msg.setIcon(QMessageBox::Warning);
        msg.setText("value must be a power of 2");
        msg.exec();*/
    }

    int levels = 0;
    bucket *root = new bucket;

    std::vector<bucket*> buckets;

    root->pixels = new std::vector<QRgb*>();
    QRgb *row;
    for (int i=0; i<image->height(); i++){
        row = (QRgb*)image->scanLine(i);
        for(int j=0; j<image->width(); j++){
            root->pixels->push_back(&row[j]);
        }
    }



    std::shuffle(root->pixels->begin(), root->pixels->end(),
                 std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count()));

    buckets.push_back(root);

    int ind = -1;
    int max_span = -1;

    bucket *tmp;
    colors--;
    while(colors != 0){
        ind = -1;
        max_span = -1;
        for (int i = 0; i < buckets.size(); i++){
            if(buckets[i]->max_span == -1){
                calc_span(buckets[i]);
            }
            if (buckets[i]->max_span > max_span){
                max_span = buckets[i]->max_span;
                ind = i;
            }
        }
        MedianCut::sort_pixels(buckets[ind]);
        tmp = MedianCut::split_bucket(buckets[ind]);
        buckets.insert(buckets.begin() + ind, tmp);

        colors--;
    }

    for (bucket *b : buckets) {
        b->color = MedianCut::get_average(b->pixels);
        //QTextStream(stdout) << b->pixels->size() << ", ";
        for (QRgb* p : *b->pixels){
            *p = b->color;
        }
        //break;
    }
    QTextStream(stdout) << "\n";

    //MedianCut::parse_bucket(root, levels);

    MedianCut::destroy_buckets(buckets);

    //MedianCut::destroy_bucket(root);
}

MedianCut::bucket* MedianCut::split_bucket(bucket* b){
    bucket *n = new bucket;
    //n->pixels;// = new std::vector<QRgb*>;
    //n->pixels->reserve(b->pixels->size()/2);
    std::vector<QRgb*> *tmp;

    if (b->pixels->size() <= 1){
        n->pixels = new std::vector<QRgb*>;
        if (b->pixels->size() == 1){
            tmp = new std::vector<QRgb*>(b->pixels->begin(), b->pixels->end());
        }
        else{
            tmp = new std::vector<QRgb*>;
        }
    }
    else{
        n->pixels = new std::vector<QRgb*>(b->pixels->begin(), b->pixels->begin() + b->pixels->size()/2 );
        tmp = new std::vector<QRgb*>(b->pixels->begin() + b->pixels->size()/2+1, b->pixels->end());
    }

    b->max_span = -1;
    b->color = 0;

    delete b->pixels;
    b->pixels = tmp;
    return n;
}

void MedianCut::sort_pixels(bucket* b){
    int (*qColor)(QRgb val);

    switch (b->slpit_c) {
    case 'r':
        qColor = qRed;
        break;
    case 'g':
        qColor = qGreen;
        break;
    case 'b':
        qColor = qBlue;
        break;
    default:
        throw std::invalid_argument("bucket must have calculated spans before it can be sorted");
        break;
    }

    std::sort(b->pixels->begin(), b->pixels->end(), [qColor](QRgb* l, QRgb* r){ return qColor(*l) < qColor(*r);});
}

void MedianCut::calc_span(bucket* b){
    int max_r = 0;
    int max_g = 0;
    int max_b = 0;
    int min_r = 255;
    int min_g = 255;
    int min_b = 255;

    for (QRgb* p : *(b->pixels)){
        if (qRed(*p)    > max_r ) { max_r = qRed(*p);}
        if (qGreen(*p)  > max_g ) { max_g = qGreen(*p);}
        if (qBlue(*p)   > max_b ) { max_b = qBlue(*p);}
        if (qRed(*p)    < min_r ) { min_r = qRed(*p);}
        if (qGreen(*p)  < min_g ) { min_g = qGreen(*p);}
        if (qBlue(*p)   < min_b ) { min_b = qBlue(*p);}
    }

    if ( (max_r - min_r) > (max_g - min_g) && (max_r - min_r) > (max_b - min_b)){
        b->slpit_c = 'r';
        b->max_span = max_r - min_r;
    } else if ((max_g - min_g) > (max_b - min_b)) {
        b->slpit_c = 'g';
        b->max_span = max_g - min_g;
    } else {
        b->slpit_c = 'b';
        b->max_span = max_b - min_b;
    }

}


QRgb MedianCut::get_average(std::vector<QRgb*> *vec){
    ull red = 0;
    ull green = 0;
    ull blue = 0;

    if(vec->size() == 0){
        return qRgb(0,0,0);
    }

    for (QRgb* p : *vec){
        red += qRed(*p);
        green += qGreen(*p);
        blue += qBlue(*p);
    }
    red /= (*vec).size();
    green /= (*vec).size();
    blue /= (*vec).size();

    return qRgb((int)red, (int)green, (int)blue);
}

void MedianCut::destroy_buckets(std::vector<bucket*> vec){
    for (bucket *b : vec){
        delete b->pixels;
        delete b;
        b = nullptr;
    }
}
/*
void MedianCut::destroy_bucket(bucket *b){
    if(b->l != nullptr){
        MedianCut::destroy_bucket(b->l);
    }
    if (b->h != nullptr){
        MedianCut::destroy_bucket(b->h);
    }
    if (b->pixels != nullptr){
        delete b->pixels;
    }
    delete b;
}*/

void HistogramStretchingWithThreshold::applyFilter(QImage *image){
    int threshold = 1000;

    int r_min = -1;
    int g_min = -1;
    int b_min = -1;

    int r_max = -1;
    int g_max = -1;
    int b_max = -1;

    //std::vector<ull> gray_pdf(256, 0);
    std::vector<ull> r_pdf(256, 0);
    std::vector<ull> g_pdf(256, 0);
    std::vector<ull> b_pdf(256, 0);

    for (int i=0; i < image->height(); i++){
        QRgb* row = (QRgb*)image->scanLine(i);
        //ull loc_sum = 0;
        for (int j=0; j < image->width(); j++){
            r_pdf[qRed(row[j])] += 1;
            g_pdf[qGreen(row[j])] += 1;
            b_pdf[qBlue(row[j])] += 1;
        }
    }

    int tmp = std::min(std::min(*std::min_element(r_pdf.begin(), r_pdf.end()),
                                *std::min_element(g_pdf.begin(), g_pdf.end())),
                                 *std::min_element(b_pdf.begin(), b_pdf.end()));

    threshold = std::max(threshold, tmp + 2);

    int i=0;

    while (r_min == -1 || g_min == -1 || b_min == -1) {
        if (r_min == -1 && r_pdf[i] >= threshold) {
            r_min = i;
        }
        if (g_min == -1 && g_pdf[i] >= threshold) {
            g_min = i;
        }
        if (b_min == -1 && b_pdf[i] >= threshold) {
            b_min = i;
        }
        i++;
    }
    i = 255;
    while (r_max == -1 || g_max == -1 || b_max == -1) {
        if (r_max == -1 && r_pdf[i] >= threshold) {
            r_max = i;
        }
        if (g_max == -1 && g_pdf[i] >= threshold) {
            g_max = i;
        }
        if (b_max == -1 && b_pdf[i] >= threshold) {
            b_max = i;
        }
        i--;
    }

    for (int i=0; i < image->height(); i++){
        QRgb* row = (QRgb*)image->scanLine(i);
        //ull loc_sum = 0;
        for (int j=0; j < image->width(); j++){
            int tmp_r = qRed(row[j]);
            int tmp_g = qGreen(row[j]);
            int tmp_b = qBlue(row[j]);

            tmp_r -= r_min;
            tmp_g -= g_min;
            tmp_b -= b_min;

            tmp_r *= 255;
            tmp_g *= 255;
            tmp_b *= 255;

            tmp_r /= (r_max - r_min);
            tmp_g /= (g_max - g_min);
            tmp_b /= (b_max - b_min);

            tmp_r = std::max(0, std::min(255, tmp_r));
            tmp_g = std::max(0, std::min(255, tmp_g));
            tmp_b = std::max(0, std::min(255, tmp_b));

            row[j] = qRgb(tmp_r, tmp_g, tmp_b);
        }
    }

}

} // namespace Task2
