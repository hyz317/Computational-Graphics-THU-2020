#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>
#include <iostream>

class Rectangle : public Object3D {

public:
	Rectangle() = delete;

    // a b c are three vertex positions of the triangle
	Rectangle(const Vector3f& p, const Vector3f& x, const Vector3f& y, Material* m) : 
              Object3D(m), position(p), x_axis(x), y_axis(y) {

	}

	bool intersect(const Ray& r, Hit& h, float tmin) override {
        Vector3f n = Vector3f::cross(x_axis, y_axis).normalized();
        float d = Vector3f::dot(n, r.getDirection());
        if (fabs(d) < 1e-3) return false;
        float t = Vector3f::dot(n * Vector3f::dot(position, n) - r.getOrigin(), n) / d;
        if (t < tmin) return false;
        if (t > h.getT()) return false;

        Vector3f v = r.getOrigin() + r.getDirection() * t - position;
        if (fabs(Vector3f::dot(x_axis, v)) > Vector3f::dot(x_axis, x_axis)) return false;
        if (fabs(Vector3f::dot(y_axis, v)) > Vector3f::dot(y_axis, y_axis)) return false;

        if (d > 0) n = -n;

		h.set(t, material, n, 'r', true);
        h.setRectangleParameters(position, x_axis, y_axis);
        
        return true;
	}
	
// protected:
    Vector3f position;
    Vector3f x_axis;
    Vector3f y_axis;
};

#endif // RECTANGLE_H
