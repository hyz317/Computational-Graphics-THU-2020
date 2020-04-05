#ifndef CURVE_HPP
#define CURVE_HPP

#include "object3d.hpp"
#include "Bernstein.hpp"
#include <vecmath.h>
#include <vector>
#include <utility>

#include <algorithm>

// Already Done (PA3): Implement Bernstein class to compute spline basis function.
//       You may refer to the python-script for implementation.

// The CurvePoint object stores information about a point on a curve
// after it has been tesselated: the vertex (V) and the tangent (T)
// It is the responsiblility of functions that create these objects to fill in all the data.
struct CurvePoint {
    Vector3f V; // Vertex
    Vector3f T; // Tangent  (unit)
    CurvePoint(Vector3f v, Vector3f t) : V(v), T(t) {}
};

class Curve : public Object3D {
protected:
    std::vector<Vector3f> controls;
public:
    explicit Curve(std::vector<Vector3f> points) : controls(std::move(points)) {}

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        return false;
    }

    std::vector<Vector3f> &getControls() {
        return controls;
    }

    virtual void discretize(int resolution, std::vector<CurvePoint>& data) = 0;

    void drawGL() override {
        Object3D::drawGL();
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glDisable(GL_LIGHTING);
        glColor3f(1, 1, 0);
        glBegin(GL_LINE_STRIP);
        for (auto & control : controls) { glVertex3fv(control); }
        glEnd();
        glPointSize(4);
        glBegin(GL_POINTS);
        for (auto & control : controls) { glVertex3fv(control); }
        glEnd();
        std::vector<CurvePoint> sampledPoints;
        discretize(30, sampledPoints);
        glColor3f(1, 1, 1);
        glBegin(GL_LINE_STRIP);
        for (auto & cp : sampledPoints) { glVertex3fv(cp.V); }
        glEnd();
        glPopAttrib();
    }
};

class BezierCurve : public Curve {
public:
    explicit BezierCurve(const std::vector<Vector3f> &points) : Curve(points) {
        if (points.size() < 4 || points.size() % 3 != 1) {
            printf("Number of control points of BezierCurve must be 3n+1!\n");
            exit(0);
        }
    }

    void discretize(int resolution, std::vector<CurvePoint>& data) override {
        data.clear();
        // AlreadyDone (PA3): fill in data vector
        float* knot = Bernstein::bezier_knot(3);
        auto b = Bernstein(controls.size() - 1, 3, knot, 8);
        auto t_range = b.get_valid_range();
        auto t_num = b.get_range_point_num();

        for (int i = 0; i <= t_num * resolution; i++) {
            float t = t_range.first * (1.0f - i / ((float) t_num * resolution)) + t_range.second * 1.0f * i / ((float) t_num * resolution);
            // std::cout << "t: " << t << std::endl;
            auto basis = b.evaluate(t);
            Vector3f vertex = Vector3f::ZERO, tangent = Vector3f::ZERO;
            int j = 0;
            for (auto point : getControls()) {
                vertex += basis.first[j] * point;
                tangent += basis.second[j] * point;
                j++;
            }
            data.push_back(CurvePoint(vertex, tangent));
            // std::cout << "(" << vertex.x() << ", " << vertex.y() << ", " << vertex.z() << ")\n";
            delete[] basis.first;
            delete[] basis.second;
        }
    }

protected:

};

class BsplineCurve : public Curve {
public:
    BsplineCurve(const std::vector<Vector3f> &points) : Curve(points) {
        if (points.size() < 4) {
            printf("Number of control points of BspineCurve must be more than 4!\n");
            exit(0);
        }
    }

    void discretize(int resolution, std::vector<CurvePoint>& data) override {
        data.clear();
        // AlreadyDone (PA3): fill in data vector
        float* knot = Bernstein::bspline_knot(controls.size() - 1, 3);
        auto b = Bernstein(controls.size() - 1, 3, knot, controls.size() + 1 + 3);
        auto t_range = b.get_valid_range();
        auto t_num = b.get_range_point_num();

        for (int i = 0; i <= t_num * resolution; i++) {
            float t = (t_range.first + EPS) * (1.0f - i / ((float) t_num * resolution)) + t_range.second * 1.0f * i / ((float) t_num * resolution);
            // std::cout << "t: " << t << std::endl;
            auto basis = b.evaluate(t);
            Vector3f vertex = Vector3f::ZERO, tangent = Vector3f::ZERO;
            int j = 0;
            for (auto point : getControls()) {
                vertex += basis.first[j] * point;
                tangent += basis.second[j] * point;
                j++;
            }
            data.push_back(CurvePoint(vertex, tangent));
            // std::cout << "(" << vertex.x() << ", " << vertex.y() << ", " << vertex.z() << ")\n";
            delete[] basis.first;
            delete[] basis.second;
        }
    }

protected:

};

#endif // CURVE_HPP
