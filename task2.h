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

} // namespace Task2

#endif // TASK2_H
