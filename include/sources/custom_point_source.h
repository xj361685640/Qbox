#ifndef GUARD_custom_point_source_h
#define GUARD_custom_point_source_h

#include "source.h"

namespace qbox {
    class custom_point_source: public source {
    public:
        custom_point_source(ivec p, double (*time_func)(double));
        void pulse();
    private:
        ivec p;
        double (*time_func)(double);
    };
}

#endif
