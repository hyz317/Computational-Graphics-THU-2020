#ifndef CAMERA_H
#define CAMERA_H

#include "ray.hpp"
#include <vecmath.h>
#include <float.h>
#include <cmath>


class Camera {
public:
    Camera(const Vector3f &center, const Vector3f &direction, const Vector3f &up, int imgW, int imgH, int photons) {
        this->center = center;
        this->direction = direction.normalized();
        this->horizontal = Vector3f::cross(this->direction, up);
        this->up = Vector3f::cross(this->horizontal, this->direction);
        this->width = imgW;
        this->height = imgH;
        this->photons = photons;
        this->type = (photons == 0) ? "PT" : "PM";
        this->focal_len = 30;
        this->aperture = 0;
    }

    // Generate rays for each screen-space coordinate
    virtual Ray generateRay(const Vector2f &point) = 0;
    virtual Ray generateRandomRay(const Vector2f &point, unsigned short Xi[]) = 0;
    virtual ~Camera() = default;

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getPhotons() const { return photons; }
    std::string getType() const { return type; }

protected:
    // Extrinsic parameters
    Vector3f center;
    Vector3f direction;
    Vector3f up;
    Vector3f horizontal;
    // Intrinsic parameters
    int width;
    int height;
    int photons;
    float aperture;
    int focal_len;
    std::string type;
};

// TODO: Implement Perspective camera
// You can add new functions or variables whenever needed.
class PerspectiveCamera : public Camera {

public:
    PerspectiveCamera(const Vector3f &center, const Vector3f &direction,
            const Vector3f &up, int imgW, int imgH, float w_angle, float h_angle, int photons) : Camera(center, direction, up, imgW, imgH, photons) {
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

    Ray generateRandomRay(const Vector2f &point, unsigned short Xi[]) override {
        Vector3f focalPoint = center + generateRay(point).getDirection() * focal_len;
        double x, y;
        do {
            x = erand48(Xi) * 2 - 1;
            y = erand48(Xi) * 2 - 1;
        } while (x * x + y * y > 1);
        Vector3f newOrigin = center + horizontal * aperture * x + up * aperture * y;
        Vector3f newDirection = (focalPoint - newOrigin).normalized();
        // std::cout << "newOrigin " << newOrigin << " newDirection " << newDirection << std::endl;
        return Ray(newOrigin, newDirection);
    }

    

protected:
    float w_angle;
    float h_angle;

};

#endif //CAMERA_H
