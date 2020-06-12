#ifndef Photonmap_H
#define Photonmap_H

#include <cmath>
#include <vector>
#include "ray.hpp"
#include "group.hpp"
#include <vecmath.h>

#define EPS 1e-3
#define INF 1e10

class Photon
{
public:
    Vector3f pos; // 碰撞位置
    Vector3f dir; // 入射方向
    Vector3f power; // 光子能量
    int plane; // KDTree 划分平面
};

class Nearestphotons {
public:
	Vector3f pos; // 光线射入位置
	int max_photons; // k 值
    int found; // 已经找到的光子数

	bool got_heap; // 是否已建堆
	float* dist2; // 最近光子与 pos 的欧氏距离
	Photon** photons; // 最近光子的一个大顶堆

	Nearestphotons();
	~Nearestphotons();
};

class Photonmap
{
public:
    Photonmap(int photon_num);
    ~Photonmap();

    void BalanceSegment(Photon* porg, int index, int st, int ed);
    void MedianSplit(Photon* porg, int st, int ed, int mi, int axis);
    void LocatePhotons(Nearestphotons* np, int index);

    Vector3f GetIrradiance(Ray& ray, Hit& hit, float max_dist, int n);
    float GetRadius2(Ray& ray, Hit& hit, double max_dist, int n);
    int getStoredPhotons() { return stored_photons; }
    void setEmitPhotons(int e) { emit_photons = e; }

    void Store(Photon photon);
    void Balance();

private:
    int max_photons;
    int stored_photons;
    int emit_photons;
    Photon* photons;
    Vector3f box_min, box_max;
};

#endif //Photonmap_H
