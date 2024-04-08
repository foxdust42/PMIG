#ifndef TASK2_H
#define TASK2_H

#include "basefilter.h"

namespace Task2 {

class AverageDithering : public Filters::BaseFilter
{
public:
    AverageDithering(): Filters::BaseFilter(
                        Filters::FilterClass::FILTER_OTHER,
                        "Average Dithering",
                        false){}
    ~AverageDithering(){}

    void applyFilter(QImage *image);
};

class HistogramStretchingWithThreshold : public Filters::BaseFilter
{
public:
    HistogramStretchingWithThreshold(): Filters::BaseFilter(
            Filters::FilterClass::FILTER_OTHER,
            "Histogram Stretching With Threshold",
            false){}
    ~HistogramStretchingWithThreshold(){}

    void applyFilter(QImage *image);
};

class MedianCut : public Filters::BaseFilter{
private:
    typedef struct _bucket {
        //struct _bucket *l = nullptr;
        //struct _bucket *h = nullptr;
        QRgb color;
        int max_span = -1;
        char slpit_c = 0;
        std::vector<QRgb*> *pixels = nullptr;
    } bucket;

    //static void destroy_bucket(bucket *b);

    static void destroy_buckets(std::vector<bucket*> vec);

    static void calc_span(bucket* b);

    static void sort_pixels(bucket* b);

    static bucket* split_bucket(bucket * b);

    //static void parse_bucket(bucket *b, int level);

    static QRgb get_average(std::vector<QRgb*> *vec);
public:
    MedianCut() : Filters::BaseFilter(
                  Filters::FilterClass::FILTER_OTHER,
                  "Median Cut",
                  false){}
    ~MedianCut(){}

    void applyFilter(QImage *image);
};

} // namespace Task2

#endif // TASK2_H
