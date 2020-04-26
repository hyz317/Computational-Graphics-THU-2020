#ifndef SPHERE_H
#define SPHERE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// TODO: Implement functions and add more fields as necessary

class Sphere : public Object3D {
public:
    Sphere() {
        // unit ball at the center
    }

    Sphere(const Vector3f &center, float radius, Material *material) : Object3D(material) {
        this->center = center;
        this->radius = radius;
    }

    ~Sphere() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {

        Vector3f p = r.getOrigin() - center;
        float b = Vector3f::dot(p, r.getDirection()) * -1.0;
        float det = b * b - p.length() * p.length() + radius * radius;
        float t;
        bool front;

        if (det > 5e-4) {
            det = sqrt(det);
            float x1 = b - det, x2 = b + det;
            if (x2 < 5e-4) return false;
            if (x1 > 5e-4) {
                t = x1;
                front = true;
            }
            else {
                t = x2;
                front = false;
            }
        }
        else {
            return false;
        }

        if (t < tmin) return false;
        if (t > h.getT()) return false;

        Vector3f normal_P = (r.getOrigin() + r.getDirection() * t - center).normalized();
        if (!front) normal_P = -normal_P;
        h.set(t, material, normal_P);
        return true;
    }

protected:

    Vector3f center;
    float radius;

};


#endif
