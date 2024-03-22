#include "task2.h"
#include <QMessageBox>
#include <QTextStream>
#include <QInputDialog>
#include <QApplication>
#include <random>
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

/*
void MedianCut::parse_bucket(bucket *b, int level){
    if (level == 0){
        b->color = get_average(b->pixels);
        for (int i=0; i< b->pixels->size(); i++){
            *(*b->pixels)[i] = b->color;
        }
        return;
    }

    int red_l = 255;
    int red_h = 0;
    int green_l = 255;
    int green_h = 0;
    int blue_l = 255;
    int blue_h = 0;

    for (QRgb* p : *b->pixels){
        red_l   = std::min(red_l, qRed(*p));
        red_h   = std::max(red_h, qRed(*p));
        green_l = std::min(green_l, qGreen(*p));
        green_h = std::max(green_h, qGreen(*p));
        blue_l  = std::min(blue_l, qBlue(*p));
        blue_h  = std::max(blue_h, qBlue(*p));
    }

    if ((red_h - red_l) >= (blue_h - blue_l) && (red_h - red_l) >= (green_h - green_l)){
        b->color = 'r';
    }
    else {
        if((blue_h - blue_l) >= (green_h - green_l)){
            b->color = 'b';
        } else{
            b->color = 'g';
        }
    }

    int (*col)(QRgb);

    switch (b->color) {
    case 'r':
        b->split_val = (red_h+red_l)/2;
        col = qRed;
        break;
    case 'g':
        b->split_val = (green_h+green_l)/2;
        col = qGreen;
        break;
    case 'b':
        b->split_val = (blue_h+blue_l)/2;
        col = qBlue;
        break;
    default:
        throw new std::domain_error("????");
        break;
    }

    b->h = new bucket;
    b->h->pixels = new std::vector<QRgb*>();

    b->l = new bucket;
    b->l->pixels = new std::vector<QRgb*>();

    for(QRgb* p : *b->pixels){
        if (col(*p) >= b->split_val){
            b->h->pixels->push_back(p);
        }
        else{
            b->l->pixels->push_back(p);
        }
    }

    delete b->pixels;

    b->pixels = nullptr;

    parse_bucket(b->h, level-1);
    parse_bucket(b->l, level-1);

}
*/
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

} // namespace Task2
