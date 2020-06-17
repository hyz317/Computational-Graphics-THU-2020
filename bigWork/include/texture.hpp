#ifndef TEXTURE_H
#define TEXTURE_H

#include "image.hpp"
#include <vecmath.h>
#include <cstring>
#include <cmath>

#define EPS 1e-3

class Texture {
public:
    Texture(const char* filename) {
        if (!strlen(filename)) {
            _haveTexture = false;
        } else {
            image = Image::LoadBMP(filename);
            _haveTexture = true;
        }
    }
    ~Texture() {
        if (_haveTexture) delete image;
    }

    bool haveTexture() { return _haveTexture; }

    Vector3f getTextureColor(float u, float v) {
        float x = (u - floor(u)) * image->Width();
        float y = (v - floor(v)) * image->Height();

        int x1 = (int) floor(x + EPS);
        int y1 = (int) floor(y + EPS);
        int x2 = x1 + 1;
        int y2 = y1 + 1;

        float factor_x1 = x2 - (float) x;
        float factor_y1 = y2 - (float) y;

        if (x1 < 0) x1 = image->Width() - 1;
        if (y1 < 0) y1 = image->Height() - 1;
        if (x2 >= image->Width()) x2 = 0;
        if (y2 >= image->Height()) y2 = 0;

        Vector3f color = Vector3f::ZERO;
        color += image->GetPixel(x1, y1) * factor_x1 * factor_y1;
        color += image->GetPixel(x1, y2) * factor_x1 * (1 - factor_y1);
        color += image->GetPixel(x2, y1) * (1 - factor_x1) * factor_y1;
        color += image->GetPixel(x2, y2) * (1 - factor_x1) * (1 - factor_y1);

        return color;
    }

    Vector3f calcSphereTexture(const Vector3f& normal) {
        float u = 0.5f + atan2(normal.x(), normal.z()) / 2.0f / M_PI;
        float v = 0.5f - asin(-normal.y()) / M_PI;
        return getTextureColor(u, v);
    }

    Vector3f calcRectangleTexture(const Vector3f& hitpoint, Vector3f x_axis, Vector3f y_axis, Vector3f pos) {
        Vector3f p = hitpoint - pos;
        float u = (Vector3f::dot(p, x_axis) / x_axis.squaredLength() + 1) / 2;
        float v = (Vector3f::dot(p, y_axis) / y_axis.squaredLength() + 1) / 2;

        // std::cout << u << ' ' << v << ' ' << getTextureColor(u, v) << std::endl;
        return getTextureColor(u, v);
    }

    Vector3f calcParametricTexture(const float& u, const float& v) {
        return getTextureColor(u, v);
    }

    Vector3f calcSimpleMeshTexture(const float& u, const float& v) {
        return getTextureColor(u, v);
    }


protected:
    Image* image;
    bool _haveTexture;

};


#endif // TEXTURE_H
