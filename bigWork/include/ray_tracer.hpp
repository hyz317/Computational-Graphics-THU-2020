#ifndef RAYTRACER_H
#define RAYTRACER_H

#include <cmath>
#include <vector>
#include "ray.hpp"
#include "group.hpp"
#include "light.hpp"
#include "Photonmap.hpp"
#include <vecmath.h>

#define EPS 1e-3

class RayTracer
{
public:
    RayTracer(int d, Group* g, std::vector<Light*>& l, std::string t = "PT", Vector3f c = Vector3f::ZERO, float tm = 1e-3) : 
              max_depth(d), group(g), lights(l), bkgcolor(c), tmin(tm), type(t) {}
    ~RayTracer() {}

    Vector3f trace(Ray ray, unsigned short Xi[], int depth = 1);
    Vector3f calcDiffusion(Ray ray, Hit& hit, int depth, unsigned short Xi[]); // Phong model
    Vector3f calcRandomDiffusion(Ray ray, Hit& hit, int depth, unsigned short Xi[]);
    Vector3f calcReflection(Ray ray, Hit& hit, int depth, unsigned short Xi[]);
    Vector3f calcRefraction(Ray ray, Hit& hit, int depth, unsigned short Xi[]);

    bool intersectLight(Ray ray, float& dis, Vector3f& color);
    void setPhotonMap(Photonmap* p) { photonmap = p; }

private:
    int max_depth;
    float tmin;
    Group* group;
    Vector3f bkgcolor;
    std::vector<Light*>& lights;
    std::string type;
    Photonmap* photonmap;
};

#endif //RAYTRACER_H
