#ifndef LIGHT_H
#define LIGHT_H

#include <Vector3f.h>
#include "object3d.hpp"
#include "group.hpp"
#include "Photonmap.hpp"

class Light {
public:
    Light() = default;

    virtual ~Light() = default;

    virtual void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col, Group* group, unsigned short Xi[], int sampling_factor = 3) const = 0;
    virtual bool intersect(Ray ray, float& dis, Vector3f& c) { return false; }
    virtual Vector3f getColor() { return Vector3f::ZERO; }
    virtual Photon emitPhoton(unsigned short Xi[]) = 0;
};


class DirectionalLight : public Light {
public:
    DirectionalLight() = delete;

    DirectionalLight(const Vector3f &d, const Vector3f &c) {
        direction = d.normalized();
        color = c;
    }

    ~DirectionalLight() override = default;

    ///@param p unsed in this function
    ///@param distanceToLight not well defined because it's not a point light
    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col, Group* group, unsigned short Xi[], int sampling_factor = 3) const override {
        // the direction to the light is the opposite of the
        // direction of the directional light source
        dir = -direction;
        col = color;
    }

    bool intersect(Ray ray, float& dis, Vector3f& c) { return false; }

    Photon emitPhoton(unsigned short Xi[]) {
        return Photon();
    }

private:

    Vector3f direction;
    Vector3f color;

};

class PointLight : public Light {
public:
    PointLight() = delete;

    PointLight(const Vector3f &p, const Vector3f &c) {
        position = p;
        color = c;
    }

    ~PointLight() override = default;

    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col, Group* group, unsigned short Xi[], int sampling_factor = 3) const override {
        // the direction to the light is the opposite of the
        // direction of the directional light source

        // 2020.4.26: judge for shadows.
        dir = (position - p);
        dir = dir / dir.length();

        Hit hit;
        if (group->intersect(Ray(p, dir), hit, 1e-3) && hit.getT() < (position - p).length()) {
            col = Vector3f::ZERO;
        } else {
            col = color;
        }
    }

    bool intersect(Ray ray, float& dis, Vector3f& c) { return false; }

    Photon emitPhoton(unsigned short Xi[]) {
        Photon ret;
        ret.power = color / color.mean();
        ret.pos = position;
        ret.dir = Vector3f(2 * erand48(Xi) - 1, 2 * erand48(Xi) - 1, 2 * erand48(Xi) - 1).normalized();
        return ret;
    }

private:

    Vector3f position;
    Vector3f color;

};

class AreaLight : public Light {
public:
    AreaLight() = delete;
    ~AreaLight() override = default;

    AreaLight(const Vector3f& p, const Vector3f& x, const Vector3f& y, const Vector3f& c, float e) : 
        position(p), x_axis(x), y_axis(y), color(c), emission(e) { ndir = Vector3f::cross(x, y).normalized(); }

    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col, Group* group, unsigned short Xi[], int sampling_factor = 3) const override {
        int shade = 0;
        dir = (position - p);
        dir = dir / dir.length();

        for (int i = -2; i < 2; i++) {
            for (int j = -2; j < 2; j++) {
                for (int k = 0; k < sampling_factor; k++) {
                    Vector3f v = position - p + x_axis * ((erand48(Xi) + i) / 2) + y_axis * ((erand48(Xi) + j) / 2);
                    float dis = v.length();
                    // std::cout << dis << ' ' << (position - p).length() << std::endl;

                    Hit hit;
                    if (group->intersect(Ray(p, v.normalized()), hit, 1e-3) && hit.getT() < dis) {
                        shade++;
                    }
                }
            }
        }
        // std::cout << x_axis.x() << ' ' << x_axis.y() << ' ' << x_axis.z() << std::endl;
        // if (shade != 48 && shade != 0) std::cout << shade << "\n";
        col = color * (1 - (float) shade / (16.0f * sampling_factor));
        // if ((1 - (float) shade / (16.0f * sampling_factor)) > 0.5f) std::cout << "factor" << (1 - (float) shade / (16.0f * sampling_factor)) << "\n";
    }

    bool intersect(Ray ray, float& dis, Vector3f& c) { 
        Vector3f n = Vector3f::cross(x_axis, y_axis).normalized();
        float d = Vector3f::dot(n, ray.getDirection());
        if (fabs(d) < 1e-3) return false;
        float l = Vector3f::dot(n * Vector3f::dot(position, n) - ray.getOrigin(), n) / d;
        if (l < 1e-3) return false;

        Vector3f v = ray.getOrigin() + ray.getDirection() * l - position;
        if (fabs(Vector3f::dot(x_axis, v)) > Vector3f::dot(x_axis, x_axis)) return false;
        if (fabs(Vector3f::dot(y_axis, v)) > Vector3f::dot(y_axis, y_axis)) return false;

        dis = l;
        c = color * emission;
        // std::cout << "l: " << l << std::endl;
        // std::cout << "c: " << (v + position).x() << ' ' << (v + position).y() << ' ' << (v + position).z() << "\n";
        // std::cout << "v: " << (v).x() << ' ' << (v).y() << ' ' << (v).z() << "\n";
        return true;
    }

    Vector3f getColor() { return color; }

    Photon emitPhoton(unsigned short Xi[]) {
        Photon ret;
        ret.power = color / color.mean();
        ret.pos = position + x_axis * ( erand48(Xi) + (static_cast<double>(rand()) / RAND_MAX) - 1 ) + y_axis * ( erand48(Xi) + (static_cast<double>(rand()) / RAND_MAX) - 1 );
        ret.dir = Vector3f((static_cast<double>(rand()) / RAND_MAX) + erand48(Xi) - 1, (static_cast<double>(rand()) / RAND_MAX) + erand48(Xi) - 1, (static_cast<double>(rand()) / RAND_MAX) + erand48(Xi) - 1).normalized();
        float co = fabs(Vector3f::dot(ret.dir, ndir));
        while ((static_cast<double>(rand()) / RAND_MAX) > co) {
            ret.dir = Vector3f((static_cast<double>(rand()) / RAND_MAX) + erand48(Xi) - 1, (static_cast<double>(rand()) / RAND_MAX) + erand48(Xi) - 1, (static_cast<double>(rand()) / RAND_MAX) + erand48(Xi) - 1).normalized();
            co = fabs(Vector3f::dot(ret.dir, ndir));
        }
        // std::cout << fabs(Vector3f::dot(ret.dir, ndir)) << std::endl;
        // std::cout << "origin " << ret.pos << " dir " << ret.dir << std::endl;
        return ret;
    }


private:
    Vector3f position;
    Vector3f x_axis;
    Vector3f y_axis;
    Vector3f color;
    Vector3f ndir;
    float emission;
};

#endif // LIGHT_H
