#ifndef SPHERE_H
#define SPHERE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>
#include <glut.h>

// AlreadyDone (PA2): Copy from PA1

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
                Vector3f normal = Vector3f::cross(r.getDirection(), center - r.getOrigin());
        Vector3f vertical = Vector3f::cross(normal, r.getDirection()).normalized();
        
        float len1 = ( ( r.getOrigin().x() * r.getDirection().y() / r.getDirection().x() - r.getOrigin().y()) - 
                       ( center.x()        * r.getDirection().y() / r.getDirection().x() - center.y()) )
                     / ( vertical.x()      * r.getDirection().y() / r.getDirection().x() - vertical.y());
        float len2 = ( center.x() - r.getOrigin().x() + len1 * vertical.x() ) / r.getDirection().x();

        if (r.getOrigin().x() == r.getOrigin().y()) {
            if (fabs(r.getDirection().y()) > 1e-6) {
                len1 = ( ( r.getOrigin().y() * r.getDirection().z() / r.getDirection().y() - r.getOrigin().z()) - 
                       ( center.y()        * r.getDirection().z() / r.getDirection().y() - center.z()) )
                     / ( vertical.y()      * r.getDirection().z() / r.getDirection().y() - vertical.z());
                len2 = ( center.y() - r.getOrigin().y() + len1 * vertical.y() ) / r.getDirection().y();
            }
            else if (fabs(r.getDirection().x()) > 1e-6) {
                len1 = ( ( r.getOrigin().x() * r.getDirection().z() / r.getDirection().x() - r.getOrigin().z()) - 
                       ( center.x()        * r.getDirection().z() / r.getDirection().x() - center.z()) )
                     / ( vertical.x()      * r.getDirection().z() / r.getDirection().x() - vertical.z());
                len2 = ( center.x() - r.getOrigin().x() + len1 * vertical.x() ) / r.getDirection().x();
            }
            else {
                if (fabs(vertical.x()) > 1e-6) {
                    len1 = -center.x() / vertical.x();
                    len2 = ( center.z() - r.getOrigin().z() + len1 * vertical.z() ) / r.getDirection().z();
                }
                else {
                    len1 = 0.0f;
                    len2 = ( center.z() - r.getOrigin().z() ) / r.getDirection().z();
                    vertical = Vector3f::ZERO;
                }
            }
        }

        float len_CH = (len1 * vertical).length();
        if (len_CH > radius) return false;

        float len_OH = (len2 * r.getDirection()).length();
        float t = len_OH - sqrt(pow(radius, 2) - pow(len_CH, 2));

        if (t < tmin) return false;
        if (t > h.getT()) return false;

        Vector3f normal_P = (r.getOrigin() + t * r.getDirection().normalized() - center).normalized();
        h.set(t, material, normal_P);
        return true;
    }

    void drawGL() override {
        Object3D::drawGL();
        glMatrixMode(GL_MODELVIEW); glPushMatrix();
        glTranslatef(center.x(), center.y(), center.z());
        glutSolidSphere(radius, 80, 80);
        glPopMatrix();
    }

protected:
    Vector3f center;
    float radius;

};


#endif
