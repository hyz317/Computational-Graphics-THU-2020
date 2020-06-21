#ifndef PARAMETRIC_H
#define PARAMETRIC_H

#include "object3d.hpp"
#include "box.hpp"
#include <vecmath.h>
#include <cmath>

#define _EPS 1e-4

class Parametric : public Object3D {
public:
    Parametric(Material* material, float un, float um, float vn, float vm, std::string func) :
               Object3D(material), umax(um), vmax(vm), umin(un), vmin(vn) {
        if (func == "f1") f = f1;
        if (func == "f2") f = f2;
        if (func == "f3") f = f3;
        if (func == "f4") f = f4;
        tree = new CurveTetraTree(f, un, um, vn, vm);
    }

    ~Parametric() { delete tree; };

    bool intersect(const Ray &r, Hit &h, float tmin = 1e-3) override {
        // float u = 0, v = 0, t = 250;
        float u, v, t;
        if (!tree->intersect(r, u, v, t)) return false;
        // tree->intersect(r, u, v, t);
        
        //u = 0; v = 0; t = 250;
        //std::cout << "start newton. original point: " << f(u, v) << std::endl;
        for (int i = 0; i < 15; i++) {
            Vector3f ff = r.pointAtParameter(t) - f(u, v);
            Vector3f du = (f(u+0.0001, v) - f(u-0.0001, v)) / 0.0002;
            Vector3f dv = (f(u, v+0.0001) - f(u, v-0.0001)) / 0.0002;

            //std::cout << "\tt: " << t << " u: " << u << " v: " << v << " length: " << ff.length() << " point: " << f(u, v) << std::endl;
            //std::cout << "du: " << du << " dv: " << dv << std::endl;

            Vector3f n = Vector3f::cross(du, dv).normalized();
            // if (Vector3f::dot(n, r.getDirection() > 0)) n = -n;

            if (ff.length() < _EPS) {
                // std::cout << "intersect!\n";
                h.set(t, material, n, 'c');
                h.setParametricParameters((u - umin) / (umax - umin), (v - vmin) / (vmax - vmin));
                return true;
            }
            float D = Vector3f::dot(r.getDirection(), Vector3f::cross(dv, du));
            // TODO: might have bugs here.
            t -= Vector3f::dot(dv, Vector3f::cross(du, ff)) / D; // 注意 dt 的方向和 ray 的方向相反！
            u += Vector3f::dot(r.getDirection(), Vector3f::cross(dv, ff)) / D;
            v -= Vector3f::dot(r.getDirection(), Vector3f::cross(du, ff)) / D;
            if (u < 0.0)
            {
                u = -u + M_PI;
            }
            if (u >= 2 * M_PI)
            {
                u = fmod(u, 2 * M_PI);
            }
            if (v < 0.0)
            {
                v = -v + M_PI;
            }
            if (v >= 2 * M_PI)
            {
                v = fmod(v, 2 * M_PI);
            }
        }
        return false;
    }

protected:
    Vector3f (*f) (float, float);
    float umax, vmax;
    float umin, vmin;
    CurveTetraTree* tree;

private:
    static Vector3f f1(float u, float v) {
        Vector3f offset(50, 50, 50);
        float a = 20, b = 10;
        float x = (a + b * cos(v)) * cos(u); // dx/du = -(a+b*cos(v))*sin(u)
        float y = (a + b * cos(v)) * sin(u); // dy/du = (a+b*cos(v))*cos(u)
        float z = b * sin(v); // dz/du = 0
        return Vector3f(x, y, z) + offset;
    }
    static Vector3f f2(float u, float v) {
        Vector3f offset(50, 50, 50);
        float a = 20;
        float x = cos(u) * sin(v) * a;
        float y = sin(u) * sin(v) * a;
        float z = cos(z) * a;
        return Vector3f(x, y, z) + offset;
    }
    static Vector3f f3(float u, float v) {
        Vector3f offset(50, 50, 100);
        float r = 10;
        float x = r * sin(3 * u) / (2 + cos(v));
        float y = r * (sin(u) + 2 * sin(2 * u)) / (2 + cos(v + M_PI * 2 / 3));
        float z = r / 2 * (cos(u) - 2 * cos(2 * u)) * (2 + cos(v)) * (2 + cos(v + M_PI * 2 / 3)) / 4;
        return Vector3f(x, y, z) + offset;
    }
    static Vector3f f4(float u, float v) {
        // Vector3f offset(50, 75, 30);
        Vector3f offset(2.5, 11.8, 12.5);
        // float scale = 10;
        float scale = 0.7;
        float r = 2 + sin(7 * u + 5 * v);
        float x = r * cos(u) * sin(v);
        float y = r * cos(v);
        float z = r * sin(u) * sin(v);
        return Vector3f(x, y, z) * scale + offset;
    }
};


#endif
