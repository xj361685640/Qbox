#ifndef GUARD_custom_line_source_h
#define GUARD_custom_line_source_h

#include <vector>
#include "source.h"

namespace qbox {
    class custom_line_source: public source {
    public:
        custom_line_source(std::vector<int> p1, std::vector<int> p2, double (*time_func)(double));
        void pulse();
    private:
        std::vector<int> p1, p2;
        double (*time_func)(double);
    };
}

#endif
