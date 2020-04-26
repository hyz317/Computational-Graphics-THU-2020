#ifndef CAMERA_H
#define CAMERA_H

#include "ray.hpp"
#include <vecmath.h>
#include <float.h>
#include <cmath>


class Camera {
public:
    Camera(const Vector3f &center, const Vector3f &direction, const Vector3f &up, int imgW, int imgH) {
        this->center = center;
        this->direction = direction.normalized();
        this->horizontal = Vector3f::cross(this->direction, up);
        this->up = Vector3f::cross(this->horizontal, this->direction);
        this->width = imgW;
        this->height = imgH;
    }

    // Generate rays for each screen-space coordinate
    virtual Ray generateRay(const Vector2f &point) = 0;
    virtual ~Camera() = default;

    int getWidth() const { return width; }
    int getHeight() const { return height; }

protected:
    // Extrinsic parameters
    Vector3f center;
    Vector3f direction;
    Vector3f up;
    Vector3f horizontal;
    // Intrinsic parameters
    int width;
    int height;
};

// TODO: Implement Perspective camera
// You can add new functions or variables whenever needed.
class PerspectiveCamera : public Camera {

public:
    PerspectiveCamera(const Vector3f &center, const Vector3f &direction,
            const Vector3f &up, int imgW, int imgH, float w_angle, float h_angle) : Camera(center, direction, up, imgW, imgH) {
        // angle is in radian.
        this->w_angle = w_angle;
        this->h_angle = h_angle;
    }

    Ray generateRay(const Vector2f &point) override {
        // 
        float x = point.x();
        float y = point.y();
        Vector3f rayDir = direction + (tan(w_angle / 2.0f) * (x - width / 2.0f) / width * 2.0f * horizontal) + (tan(h_angle / 2.0f) * (y - height / 2.0f) / height * 2.0f * up);
        return Ray(center, rayDir.normalized());
    }

protected:
    float w_angle;
    float h_angle;

};

#endif //CAMERA_H
