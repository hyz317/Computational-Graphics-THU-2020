#ifndef SAMPLER_H
#define SAMPLER_H

#include <cmath>
#include <vector>
#include "ray.hpp"
#include "group.hpp"
#include "light.hpp"
#include "Photonmap.hpp"
#include "hitpointmap.hpp"
#include "ray_tracer.hpp"
#include "photon_tracer.hpp"
#include "camera.hpp"
#include "image.hpp"
#include <vecmath.h>

#define EPS 1e-3

class Sampler
{
public:
    Sampler(std::vector<Light*>& l, Camera* c, Image* i, RayTracer* r, Group* g, int ww, int hh) : 
            lights(l), img(i), group(g), tracer(r), camera(c), w(ww), h(hh), dof_sample(16), iterations(64), aperture(0) {}
    ~Sampler() {}

    void start();
    void sampling();
    void randomsampling();
    void resampling();
    void ProgressivePhotonMapping(int SPPMIter);

private:
    Camera* camera;
    Image* img;
    Group* group;
    RayTracer* tracer;
    HitpointMap* hitpointMap;
    std::vector<Light*>& lights;

    int w, h;
    int dof_sample;
    int iterations;
    float aperture;
    
};

#endif //SAMPLER_H
