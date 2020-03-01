#ifndef PLANE_H
#define PLANE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// TODO: Implement Plane representing an infinite plane
// function: ax+by+cz=d
// choose your representation , add more fields and fill in the functions

class Plane : public Object3D {
public:
    Plane() {

    }

    Plane(const Vector3f &normal, float d, Material *m) : Object3D(m) {
        this->normal = normal;
        this->d = d;
    }

    ~Plane() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        if (fabs(Vector3f::dot(r.getDirection(), normal)) < 1e-6) return false;
        float t = (d - Vector3f::dot(normal, r.getOrigin())) / Vector3f::dot(normal, r.getDirection());

        if (t < tmin) return false;
        if (t > h.getT()) return false;

        h.set(t, material, normal);
        return true;
    }

protected:
    Vector3f normal;
    float d;

};

#endif //PLANE_H
		

