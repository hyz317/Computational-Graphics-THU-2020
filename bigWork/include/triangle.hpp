#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>
#include <iostream>
using namespace std;

// TODO: implement this class and add more fields as necessary,
class Triangle: public Object3D {

public:
	Triangle() = delete;

    // a b c are three vertex positions of the triangle
	Triangle( const Vector3f& a, const Vector3f& b, const Vector3f& c, Material* m) : Object3D(m) {
		vertices[0] = a;
		vertices[1] = b;
		vertices[2] = c;
		normal = Vector3f::cross(b - a, c - a).normalized();
		d = Vector3f::dot(normal, a);
	}

	bool intersect(const Ray& r, Hit& h, float tmin) override {
        if (fabs(Vector3f::dot(r.getDirection(), normal)) < 1e-3) return false;
        float t = (d - Vector3f::dot(normal, r.getOrigin())) / Vector3f::dot(normal, r.getDirection());

        if (t < tmin) return false;
        if (t > h.getT()) return false;

		Vector3f p = r.getOrigin() + t * r.getDirection();
		Vector3f n1 = Vector3f::cross(vertices[1] - p, vertices[2] - p);
		Vector3f n2 = Vector3f::cross(vertices[0] - p, vertices[1] - p);
		Vector3f n3 = Vector3f::cross(vertices[2] - p, vertices[0] - p);

		/*
		cout << "normal? " << normal.x() << ' ' << normal.y() << ' ' << normal.z() << endl;
		cout << "n2? " << n2.x() << ' ' << n2.y() << ' ' << n2.z() << endl;
		cout << "a? " << vertices[0].x() << ' ' << vertices[0].y() << ' ' << vertices[0].z() << endl;
		cout << "b? " << vertices[1].x() << ' ' << vertices[1].y() << ' ' << vertices[1].z() << endl;
		cout << "c? " << vertices[2].x() << ' ' << vertices[2].y() << ' ' << vertices[2].z() << endl;
		cout << "p? " << p.x() << ' ' << p.y() << ' ' << p.z() << endl << endl;
		*/

		if (Vector3f::dot(normal, n1) < 0) return false;
		if (Vector3f::dot(normal, n2) < 0) return false;
		if (Vector3f::dot(normal, n3) < 0) return false;

		bool front = (Vector3f::dot(normal, r.getDirection()) < 0);

		if (!front) h.set(t, material, -normal, 't', front);
		else h.set(t, material, normal, 't', front);

		Vector3f po = r.pointAtParameter(h.getT());
		h.setParametricParameters(fmod((po.x() + po.y()) / 100, 1),
								  fmod((po.y() + po.z()) / 100, 1));
        
        return true;
	}

	float getMin(int axis) {
        int x0 = vertices[0][axis], x1 = vertices[1][axis], x2 = vertices[2][axis];
        return std::min(x0, std::min(x1, x2));
    }

    float getMax(int axis) {
        int x0 = vertices[0][axis], x1 = vertices[1][axis], x2 = vertices[2][axis];
        return std::max(x0, std::max(x1, x2));
    }

	Vector3f normal;
	Vector3f vertices[3];
protected:
	float d;
};

#endif //TRIANGLE_H
