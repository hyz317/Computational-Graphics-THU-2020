#ifndef HITPOINTMAP_H
#define HITPOINTMAP_H

#include <cmath>
#include <vector>
#include <vecmath.h>
#include "ray.hpp"
#include "hit.hpp"
#include "Photonmap.hpp"

extern const double PI;
#define INF 1e10

class Hitpoint {
public:
	Vector3f pos, dir, N;
	int rc;
	Vector3f weight;
	int plane;
	float R2, maxR2;
	float num, deltaNum;
	Vector3f color;
    Hit hit;
	
	Hitpoint();
	int CalnIrradiance(Photon*);
};

class HitpointMap {
	int maxHitpoints, storedHitpoints;
	Hitpoint* hitpoints;
	Vector3f boxMin, boxMax;
	double reduction;

	void BalanceSegment(Hitpoint* horg, int index, int st, int en);
	void MedianSplit(Hitpoint* horg, int st, int en, int med, int axis);
	void MaintainHitpointsMaxR2();
	void LocatePhoton(Photon* photon, int index); //called by index = 1

public:
	HitpointMap(int size);
	~HitpointMap();
	
	void SetReduction(double _reduction) { reduction = _reduction; }
	int GetStoredHitpoints() { return storedHitpoints; }
	Hitpoint* GetHitpoints() { return hitpoints; }
	void Store(Hitpoint);
	void MaintainHitpoints();
	void Balance();
	void InsertPhoton(Photon);
};

#endif
