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

class MedianCut : public Filters::BaseFilter{
private:
    typedef struct _bucket {
        struct _bucket *l = nullptr;
        struct _bucket *h = nullptr;
        QRgb color;
        int split_val;
        char slpit_c;
        std::vector<QRgb*> *pixels = nullptr;
    } bucket;

    static void destroy_bucket(bucket *b);

    static void parse_bucket(bucket *b, int level);

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
