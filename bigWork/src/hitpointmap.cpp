#include "hitpointmap.hpp"
#include <iostream>
#include <cmath>

Hitpoint::Hitpoint() {
	weight = Vector3f(1, 1, 1);
	rc = -1;
	plane = -1;
	R2 = maxR2 = 0;
	num = deltaNum = 0;
}

int Hitpoint::CalnIrradiance(Photon* photon) {
	deltaNum += 1;
	color += photon->power * hit.getMaterial()->BRDF(-photon->dir, N, -dir);
}

HitpointMap::HitpointMap(int size) {
	maxHitpoints = size;
	storedHitpoints = 0;
	hitpoints = new Hitpoint[size + 1];
	boxMin = Vector3f(INF, INF, INF);
	boxMax = Vector3f(-INF, -INF, -INF);
}

HitpointMap::~HitpointMap() {
	delete[] hitpoints;
}

void HitpointMap::BalanceSegment(Hitpoint* horg, int index, int st, int en) {
	if (st == en) {
		hitpoints[index] = horg[st];
		return;
	}
	
	int med = 1;
	while (4 * med <= en - st + 1) med <<= 1;
	if (3 * med <= en - st + 1)
		med = med * 2 + st - 1;
	else
		med = en + 1 - med;
	
	int axis = 2;
	if (boxMax.x() - boxMin.x() > boxMax.y() - boxMin.y() && boxMax.x() - boxMin.x() > boxMax.z() - boxMin.z()) axis = 0; else
	if (boxMax.y() - boxMin.y() > boxMax.z() - boxMin.z()) axis = 1;
	
	MedianSplit(horg, st, en, med, axis);
	hitpoints[index] = horg[med];
	hitpoints[index].plane = axis;
	
	if (st < med) {
		double split = boxMax.Axis(axis);
		boxMax.Axis(axis) = hitpoints[index].pos.Axis(axis);
		BalanceSegment(horg, index * 2, st, med - 1);
		boxMax.Axis(axis) = split;
	}
	
	if (en > med) {
		double split = boxMin.Axis(axis);
		boxMin.Axis(axis) = hitpoints[index].pos.Axis(axis);
		BalanceSegment(horg, index * 2 + 1, med + 1, en);
		boxMin.Axis(axis) = split;
	}
}

void HitpointMap::MedianSplit(Hitpoint* horg, int st, int en, int med, int axis) {
	int l = st, r = en;
	
	while (l < r) {
		double key = horg[r].pos.Axis(axis);
		int i = l - 1, j = r;
		for (; ; ) {
			while (horg[++i].pos.Axis(axis) < key);
			while (horg[--j].pos.Axis(axis) > key && j > l);
			if (i >= j) break;
			std::swap(horg[i], horg[j]);
		}
		
		std::swap(horg[i], horg[r]);
		if (i >= med) r = i - 1;
		if (i <= med) l = i + 1;
	}
}

void HitpointMap::LocatePhoton(Photon* photon, int index) {
	if (index > storedHitpoints) return;
	Hitpoint* hitpoint = &hitpoints[index];
	
	if (index * 2 <= storedHitpoints) {
		double dist = photon->pos.Axis(hitpoint->plane) - hitpoint->pos.Axis(hitpoint->plane);
		if (dist < 0) {
			LocatePhoton(photon, index * 2);
			if (index * 2 + 1 <= storedHitpoints && dist * dist < hitpoints[index * 2 + 1].maxR2) LocatePhoton(photon, index * 2 + 1);
		} else {
			LocatePhoton(photon, index * 2 + 1);
			if (dist * dist < hitpoints[index * 2].maxR2) LocatePhoton(photon, index * 2);
		}
	}
	
	double dist2 = (hitpoint->pos - photon->pos).squaredLength();
	if (dist2 <= hitpoints[index].R2)
		hitpoints[index].CalnIrradiance(photon);
}

void HitpointMap::Store(Hitpoint hitpoint) {
	if (storedHitpoints >= maxHitpoints) return;
	hitpoints[++storedHitpoints] = hitpoint;
	boxMin = Vector3f(std::min(boxMin.x(), hitpoint.pos.x()), std::min(boxMin.y(), hitpoint.pos.y()), std::min(boxMin.z(), hitpoint.pos.z()));
	boxMax = Vector3f(std::max(boxMax.x(), hitpoint.pos.x()), std::max(boxMax.y(), hitpoint.pos.y()), std::max(boxMax.z(), hitpoint.pos.z()));
}

void HitpointMap::MaintainHitpointsMaxR2() {
	for (int i = storedHitpoints; i >= 1; i--) {
		Hitpoint* hp = &hitpoints[i];
		hp->maxR2 = hp->R2;
		if ((i << 1) <= storedHitpoints && hp->maxR2 < hitpoints[i << 1].maxR2)
			hp->maxR2 = hitpoints[i << 1].maxR2;
		if ((i << 1) + 1 <= storedHitpoints && hp->maxR2 < hitpoints[(i << 1) + 1].maxR2)
			hp->maxR2 = hitpoints[(i << 1) + 1].maxR2;	
	}
}

void HitpointMap::MaintainHitpoints() {
	for (int i = storedHitpoints; i >= 1; i--) {
		Hitpoint* hp = &hitpoints[i];
		if (hp->deltaNum < EPS) continue;
		double k = (hp->num + reduction * hp->deltaNum) / (hp->num + hp->deltaNum);
		hp->R2 *= k;
		hp->color *= k;
		hp->num += reduction * hp->deltaNum;
		hp->deltaNum = 0;
	}
	
	MaintainHitpointsMaxR2();
}

void HitpointMap::Balance() {
	Hitpoint* horg = new Hitpoint[storedHitpoints + 1];
	
	for (int i = 0; i <= storedHitpoints; i++)
		horg[i] = hitpoints[i];
	
	BalanceSegment(horg, 1, 1, storedHitpoints);
	delete[] horg;
	
	MaintainHitpointsMaxR2();
}

void HitpointMap::InsertPhoton(Photon photon) {
	LocatePhoton(&photon, 1);
}
