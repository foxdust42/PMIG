#ifndef CONVOLUTIONFILTERS_H
#define CONVOLUTIONFILTERS_H

#include "basefilter.h"

namespace ConvolutionFilters {

class BlurFilter : public Filters::ConvolutionFilter {
    BlurFilter() : Filters::ConvolutionFilter("Blur Filter", false) {
        //m_height = 3;
        //m_width = 3;
        int tmp[3][3] =
            {{1,1,1},
             {1,1,1},
             {1,1,1,}};
        //matrix = tmp;
    }
};

}

#endif // CONVOLUTIONFILTERS_H
