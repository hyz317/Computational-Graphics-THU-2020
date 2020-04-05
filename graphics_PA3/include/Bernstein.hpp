#ifndef BERNSTEIN_HPP
#define BERNSTEIN_HPP

#define EPS 1e-5

#include "object3d.hpp"
#include <vecmath.h>
#include <vector>
#include <utility>

#include <algorithm>

class Bernstein
{
private:
    int n;
    int k;
    float* t;
    int t_size;
    float* tpad;
    int t_pad_size;

public:
    Bernstein(int n, int k, float* t, int t_size);
    ~Bernstein();

    static float* bezier_knot(int k);
    static float* bspline_knot(int n, int k);
    int get_bpos(float mu);
    int get_range_point_num();
    std::pair<float, float> get_valid_range();
    std::pair<float*, float*> evaluate(float mu);

    bool equal(float a, float b);

};


#endif // BERNSTEIN_HPP