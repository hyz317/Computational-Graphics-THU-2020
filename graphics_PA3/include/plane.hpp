#ifndef PLANE_H
#define PLANE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// AlreadyDone (PA2): Copy from PA1

class Plane : public Object3D {
public:
    Plane() {

    }

    Plane(const Vector3f &norm, float d, Material *m) : Object3D(m) {
        this->norm = norm;
        this->d = d;
    }

    ~Plane() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        if (fabs(Vector3f::dot(r.getDirection(), norm)) < 1e-6) return false;
        float t = (d - Vector3f::dot(norm, r.getOrigin())) / Vector3f::dot(norm, r.getDirection());

        if (t < tmin) return false;
        if (t > h.getT()) return false;

        h.set(t, material, norm);
        return true;
    }

    void drawGL() override {
        Object3D::drawGL();
        Vector3f xAxis = Vector3f::RIGHT;
        Vector3f yAxis = Vector3f::cross(norm, xAxis);
        xAxis = Vector3f::cross(yAxis, norm);
        const float planeSize = 10.0;
        glBegin(GL_TRIANGLES);
        glNormal3fv(norm);
        glVertex3fv(d * norm + planeSize * xAxis + planeSize * yAxis);
        glVertex3fv(d * norm - planeSize * xAxis - planeSize * yAxis);
        glVertex3fv(d * norm + planeSize * xAxis - planeSize * yAxis);
        glNormal3fv(norm);
        glVertex3fv(d * norm + planeSize * xAxis + planeSize * yAxis);
        glVertex3fv(d * norm - planeSize * xAxis + planeSize * yAxis);
        glVertex3fv(d * norm - planeSize * xAxis - planeSize * yAxis);
        glEnd();
    }

protected:
    Vector3f norm;
    float d;

};

#endif //PLANE_H
		

