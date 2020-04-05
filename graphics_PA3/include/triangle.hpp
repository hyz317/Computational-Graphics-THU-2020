#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>
#include <iostream>

using namespace std;

// AlreadyDone (PA2): Copy from PA1
class Triangle: public Object3D
{

public:
	Triangle() = delete;
        ///@param a b c are three vertex positions of the triangle

	Triangle( const Vector3f& a, const Vector3f& b, const Vector3f& c, Material* m) : Object3D(m) {
	    vertices[0] = a;
		vertices[1] = b;
		vertices[2] = c;
		normal = Vector3f::cross(b - a, c - a).normalized();
		d = Vector3f::dot(normal, a);
	}

	bool intersect( const Ray& r,  Hit& h , float tmin) override {
        if (fabs(Vector3f::dot(r.getDirection(), normal)) < 1e-6) return false;
        float t = (d - Vector3f::dot(normal, r.getOrigin())) / Vector3f::dot(normal, r.getDirection());

        if (t < tmin) return false;
        if (t > h.getT()) return false;

		Vector3f p = r.getOrigin() + t * r.getDirection();
		Vector3f n1 = Vector3f::cross(vertices[1] - p, vertices[2] - p);
		Vector3f n2 = Vector3f::cross(vertices[0] - p, vertices[1] - p);
		Vector3f n3 = Vector3f::cross(vertices[2] - p, vertices[0] - p);

		if (Vector3f::dot(normal, n1) < 0) return false;
		if (Vector3f::dot(normal, n2) < 0) return false;
		if (Vector3f::dot(normal, n3) < 0) return false;

        h.set(t, material, normal);
        return true;
	}
	Vector3f normal;
	Vector3f vertices[3];

    void drawGL() override {
        Object3D::drawGL();
        glBegin(GL_TRIANGLES);
        glNormal3fv(normal);
        glVertex3fv(vertices[0]); glVertex3fv(vertices[1]); glVertex3fv(vertices[2]);
        glEnd();
    }

protected:
    float d;
};

#endif //TRIANGLE_H
