#ifndef RAYTRACER_H
#define RAYTRACER_H

#include <cmath>
#include <vector>
#include "ray.hpp"
#include "group.hpp"
#include "light.hpp"
#include <vecmath.h>


class RayTracer
{
public:
    RayTracer(int d, Group* g, std::vector<Light*>& l, Vector3f c = Vector3f::ZERO, float tm = 1e-8) : depth(d), group(g), lights(l), bkgcolor(c), tmin(tm) {}
    ~RayTracer() {}

    Vector3f trace(Ray ray);
    Vector3f calcDiffusion(Ray ray);
    Vector3f calcReflection(Ray ray);
    Vector3f calcRefraction(Ray ray);

private:
    int depth;
    float tmin;
    Group* group;
    Vector3f bkgcolor;
    std::vector<Light*>& lights;
};

#endif //RAYTRACER_H
