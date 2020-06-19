#ifndef BOX_H
#define BOX_H

#include <vecmath.h>
#include "ray.hpp"
#include "hit.hpp"
#include "triangle.hpp"
#include <iostream>
#include <vector>

#define EPS 1e-3


class Box {
public:
    Box(Vector3f minm = Vector3f(1e10, 1e10, 1e10), Vector3f maxm = Vector3f(-1e10, -1e10, -1e10)) : 
            box_min(minm), box_max(maxm) {}
    ~Box() {}

    Vector3f box_min, box_max;

    bool contain(const Vector3f& p) {
        for (int coord = 0; coord < 3; coord++)
            if (p[coord] <= box_min[coord] - EPS || 
                p[coord] >= box_max[coord] + EPS) return false;
        return true;
    }

    float intersect(const Ray &r, bool print = false, float tmin = 1e-6) {
        float minDist = -1;
        for (int coord = 0; coord < 3; coord++) {
            float times = -1;
            if (r.getDirection()[coord] >= EPS)
                times = (box_min[coord] - r.getOrigin()[coord]) / r.getDirection()[coord];
            if (r.getDirection()[coord] <= -EPS)
                times = (box_max[coord] - r.getOrigin()[coord]) / r.getDirection()[coord];
            // std::cout << "\tray: " << r << "times: " << times << std::endl;
            if (times >= EPS) {
                Vector3f C = r.getOrigin() + r.getDirection() * times;
                if(print) std::cout << "\t\ttimes: " << times << " C " << C << " contain? " << contain(C) << std::endl; 
                if (contain(C)) {
                    float dist = (r.getOrigin() - C).length();
                    if (minDist <= -EPS || dist < minDist)
                        minDist = dist;
                }
            }
        }
        return minDist;
    }
    
};

class CurveBox : public Box {
public:
    CurveBox(Vector3f (*ff) (float, float), float un, float um, float vn, float vm) : 
             f(ff), umax(um), vmax(vm), umin(un), vmin(vn) { calcMargin(); }
    ~CurveBox() {}

    void calcMargin() {
        for (float i = umin; i < umax; i += 0.0005) {
            for (float j = vmin; j < vmax; j += 0.0005) {
                float x = f(i, j).x(), y = f(i, j).y(), z = f(i, j).z();
                box_min.x() = std::min(box_min.x(), x); box_min.y() = std::min(box_min.y(), y); box_min.z() = std::min(box_min.z(), z);
                box_max.x() = std::max(box_max.x(), x); box_max.y() = std::max(box_max.y(), y); box_max.z() = std::max(box_max.z(), z);
            }
        }
        //box_min = box_min + Vector3f(-0.01f, -0.01f, -0.01f);
        //box_max = box_max + Vector3f(0.01f, 0.01f, 0.01f);
    }

    float getumax() { return umax; } float getumin() { return umin; }
    float getvmax() { return vmax; } float getvmin() { return vmin; }

    CurveBox* next[4] = { 0 };

private:
    Vector3f (*f) (float, float);
    float umax, vmax;
    float umin, vmin;

};

class TriangleBox : public Box {
public:
    TriangleBox() {
        size = 0;
        plane = -1;
        split = 0;
        l = r = nullptr;
    }
    ~TriangleBox() {}

    void update(Triangle* tri) { 
        box_min.x() = std::min(box_min.x(), tri->getMin(0)); box_max.x() = std::max(box_max.x(), tri->getMax(0));
        box_min.y() = std::min(box_min.y(), tri->getMin(1)); box_max.y() = std::max(box_max.y(), tri->getMax(1));
        box_min.z() = std::min(box_min.z(), tri->getMin(2)); box_max.z() = std::max(box_max.z(), tri->getMax(2));
    }

    float area() {
        float a = box_max.x() - box_min.x();
        float b = box_max.y() - box_min.y();
        float c = box_max.z() - box_min.z();
        return 2 * (a * b + b * c + c * a);
    }

    std::vector<Triangle*> tris;
    int size;
    int plane;
    float split;

    TriangleBox* l;
    TriangleBox* r;
};

class CurveTetraTree {
public:
    CurveTetraTree(Vector3f (*ff) (float, float), float un, float um, float vn, float vm) : 
             f(ff), umax(um), vmax(vm), umin(un), vmin(vn) { startBuildTree(); }
    ~CurveTetraTree() { deleteTree(root); }

    void startBuildTree() {
        std::cout << "start build tree!\n";
        root = new CurveBox(f, umin, umax, vmin, vmax);
        buildTree(root, 1);
        std::cout << "build tree finished!\n";
    }

    void buildTree(CurveBox* box, int depth) {
        if (depth > 12) return;
        // std::cout << "box depth: " << depth << " bounding: " << box->box_min << " -> " << box->box_max << std::endl;
        float umid = (box->getumax() + box->getumin()) / 2.0f;
        float vmid = (box->getvmax() + box->getvmin()) / 2.0f;
        box->next[0] = new CurveBox(f, box->getumin(), umid+EPS, box->getvmin(), vmid+EPS);
        box->next[1] = new CurveBox(f, umid-EPS, box->getumax(), box->getvmin(), vmid+EPS);
        box->next[2] = new CurveBox(f, box->getumin(), umid+EPS, vmid-EPS, box->getvmax());
        box->next[3] = new CurveBox(f, umid-EPS, box->getumax(), vmid-EPS, box->getvmax());
        for (int i = 0; i < 4; i++)
            buildTree(box->next[i], depth + 1);
    }

    void deleteTree(CurveBox* box) {
        if (box == 0) return;
        for (int i = 0; i < 4; i++)
            deleteTree(box->next[i]);
        delete box;
        box = 0;
    }

    bool intersect(const Ray& r, float& res_u, float& res_v, float& res_t0, float tmin = 1e-6) {
        CurveBox* box = root;
        //std::cout << "ray: " << r << " bounding: " << box->box_min << " -> " << box->box_max << std::endl;
        /*if (res_t0 = box->intersect(r) < EPS) return false;
        //std::cout << "start find box. ray: " << r << std::endl;
        for (int i = 1; i <= 4; i++) {
            float min_dist = 1e10;
            int index = -1;
            for (int j = 0; j < 4; j++) {
                //cout << "u: " << box->next[j]->getumin() << " -> " << box->next[j]->getumax() << " v: " <<
                //    box->next[j]->getvmin() << " -> " << box->next[j]->getvmax() << std::endl;
                float dist = box->next[j]->intersect(r);
                if ((min_dist <= -EPS || dist < min_dist) && dist > EPS) {
                    min_dist = dist;
                    index = j;
                    // std::cout << "huh? " << min_dist << std::endl;
                }
            }
            if (min_dist < EPS) return false;
            box = box->next[index];
            res_t0 = min_dist;
            //std::cout << "\tround " << i << " -> u " << box->getumin() << "," << box->getumax() << " boxmin " << box->box_min << " boxmax " << box->box_max << " dist " << min_dist << std::endl;
        }
        res_u = (box->getumax() + box->getumin()) / 2;
        res_v = (box->getvmax() + box->getvmin()) / 2;*/

        float nowlowest = 1e10;
        CurveBox* tempBox;
        find(r, root, 1, nowlowest, tempBox);

        if (nowlowest < 1e9) {
            res_u = (tempBox->getumax() + tempBox->getumin()) / 2;
            res_v = (tempBox->getvmax() + tempBox->getvmin()) / 2;
            res_t0 = nowlowest;
            return true;
        }
        return false;
    }

    void find(const Ray &r, CurveBox* box, int depth, float& nowlowest, CurveBox*& tempBox) {
        float dist = box->intersect(r);
        if (dist > EPS) {
            //std::cout << "depth: " << depth << " boxmin " << box->box_min << " boxmax " << box->box_max << " dist " << dist << std::endl;
            //std::cout << "\tu: " << box->getumin() << " -> " << box->getumax() << " v: " <<
            //        box->getvmin() << " -> " << box->getvmax() << std::endl;
            if (depth == 12) {
                if (dist < nowlowest) { // 特判>0?
                    tempBox = box;
                    nowlowest = dist;
                }
                return;
            }
            for (int i = 0; i < 4; i++) {
                find(r, box->next[i], depth + 1, nowlowest, tempBox);
            }
        }
    }

private:
    Vector3f (*f) (float, float);
    float umax, vmax;
    float umin, vmin;
    CurveBox* root;
    // CurveBox* tempBox;
    // float nowlowest; // initialization!!!!
};

class TriangleTree {
public:
    TriangleTree();
	~TriangleTree();

	void sortTriangle(Triangle** tris, int l, int r, int axis, bool minAxis);
	void divideNode(TriangleBox* node);
    bool intersect(TriangleBox* node, const Ray& ray, Hit& hit);
    void deleteTree(TriangleBox* node);

    TriangleBox* root;
};

#endif // BOX_H
