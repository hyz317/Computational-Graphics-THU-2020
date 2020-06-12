#include "Photonmap.hpp"

Photonmap::Photonmap(int size) {
	max_photons = size;
	stored_photons = 0;
	photons = new Photon[size + 1];
	box_min = Vector3f(INF, INF, INF);
	box_max = Vector3f(-INF, -INF, -INF);
}

Photonmap::~Photonmap() {
    delete[] photons;
}

void Photonmap::BalanceSegment(Photon* porg, int index, int st, int ed) {
	if (st == ed) {
		photons[index] = porg[st];
		return;
	}

	int mi = 1;
	while ( 4 * mi <= ed - st + 1 ) mi <<= 1;

	if ( 3 * mi <= ed - st + 1 ) mi = mi * 2 + st - 1;
	else mi = ed + 1 - mi;
	
	// 选取划分的坐标轴
	int axis = 2;
	if (box_max.x() - box_min.x() > box_max.y() - box_min.y() && 
		box_max.x() - box_min.x() > box_max.z() - box_min.z() ) axis = 0;
	else if (box_max.y() - box_min.y() > box_max.z() - box_min.z() ) axis = 1;

	MedianSplit(porg, st, ed, mi, axis);
	photons[index] = porg[mi];
	photons[index].plane = axis;

	if (st < mi) { // 构建左子树
		float tmp = box_max.Axis(axis);
		box_max.Axis(axis) = photons[index].pos.Axis(axis);
		BalanceSegment(porg, index * 2, st, mi - 1);
		box_max.Axis(axis) = tmp;
	}

	if (mi < ed) { // 构建右子树
		float tmp = box_min.Axis(axis);
		box_min.Axis(axis) = photons[index].pos.Axis(axis);
		BalanceSegment(porg, index * 2 + 1, mi + 1, ed);
		box_min.Axis(axis) = tmp;
	}
}

void Photonmap::MedianSplit(Photon* porg, int st, int ed, int mi, int axis) {
	int l = st , r = ed;

	while (l < r) {
		float key = porg[r].pos.Axis(axis);
		int i = l - 1 , j = r;
		while (true) {
			while (porg[++i].pos.Axis(axis) < key);
			while (porg[--j].pos.Axis(axis) > key && j > l);
			if (i >= j) break;
			std::swap(porg[i], porg[j]);
		}

		std::swap(porg[i], porg[r]); // 此时 i 左侧都是小于 i 的，i 右侧都是大于 i 的
		if (i >= mi) r = i - 1;
		if (i <= mi) l = i + 1;
	}
}

void Photonmap::LocatePhotons(Nearestphotons* np, int index) {
	if (index > stored_photons) return;
	Photon *photon = &photons[index];

	if (index * 2 <= stored_photons) {
		float dist = np->pos.Axis(photon->plane) - photon->pos.Axis(photon->plane);
		if (dist < 0) {
			LocatePhotons(np, index * 2); // 优先查找左子树
			if (dist * dist < np->dist2[0]) LocatePhotons(np , index * 2 + 1); // 如果距离右子树比堆顶距离小，再查找右子树
		} else {
			LocatePhotons(np , index * 2 + 1); // 优先查找右子树
			if (dist * dist < np->dist2[0]) LocatePhotons(np , index * 2); // 如果距离左子树比堆顶距离小，再查找左子树
		}
	}

	float dist2 = (photon->pos - np->pos).squaredLength();
	if (dist2 > np->dist2[0]) return; // 如果距离比大根堆顶大，那么返回

	if (np->found < np->max_photons) { // 如果 k 近邻没有存满，先存起来
		np->found++;
		np->dist2[np->found] = dist2;
		np->photons[np->found] = photon;
	} else {
		if (!np->got_heap) { // 没建堆的话，开始建大根堆
			for (int i = np->found >> 1; i >= 1 ; i--) { // 准备下滤
				int par = i;
				Photon* tmp_photon = np->photons[i];
				float tmp_dist2 = np->dist2[i];
				while ((par << 1) <= np->found) {
					int j = par << 1;
					if ( j + 1 <= np->found && np->dist2[j] < np->dist2[j + 1] ) j++;
					if ( tmp_dist2 >= np->dist2[j] ) break; // 如果比俩儿子都大，不管
					
					np->photons[par] = np->photons[j]; // 下滤
					np->dist2[par] = np->dist2[j];
					par = j;
				}
				np->photons[par] = tmp_photon;
				np->dist2[par] = tmp_dist2;
			}
			np->got_heap = true;
		}

		int par = 1;
		while ((par << 1) <= np->found) {
			int j = par << 1;
			if (j + 1 <= np->found && np->dist2[j] < np->dist2[j + 1]) j++;
			if ( dist2 > np->dist2[j] ) break;

			np->photons[par] = np->photons[j]; // 上滤，直接把堆顶 pop 掉
			np->dist2[par] = np->dist2[j];
			par = j;
		}
		np->photons[par] = photon;
		np->dist2[par] = dist2;

		np->dist2[0] = np->dist2[1]; // 新堆顶
	}
}

Vector3f Photonmap::GetIrradiance(Ray& ray, Hit& hit, float max_dist, int n) {
	Vector3f ret;

	Nearestphotons np;
	np.pos = ray.pointAtParameter(hit.getT());
	np.max_photons = n;
	np.dist2 = new float[n + 1];
	np.photons = new Photon*[n + 1];
	np.dist2[0] = max_dist * max_dist;

	LocatePhotons(&np , 1);
	if ( np.found <= 8 ) return ret;

	for ( int i = 1 ; i <= np.found ; i++ ) {
		Vector3f dir = np.photons[i]->dir;
		if (Vector3f::dot(hit.getNormal(), dir) < 0)
			ret += np.photons[i]->power * hit.getMaterial()->BRDF(-dir, hit.getNormal(), -ray.getDirection());
	}
	
	ret = ret * (4 / (emit_photons * np.dist2[0]));
	return ret;
}

float Photonmap::GetRadius2(Ray& ray, Hit& hit, double max_dist, int n) {
	Nearestphotons np;
	np.pos = ray.pointAtParameter(hit.getT());
	np.max_photons = n;
	np.dist2 = new float[n + 1];
	np.photons = new Photon*[n + 1];
	np.dist2[0] = max_dist * max_dist;

	LocatePhotons( &np , 1 );
	return np.dist2[0];
}

void Photonmap::Store(Photon photon) {
	if ( stored_photons >= max_photons ) return;
	photons[++stored_photons] = photon;
	box_min = Vector3f(std::min(box_min.x(), photon.pos.x()), std::min(box_min.y(), photon.pos.y()), std::min(box_min.z(), photon.pos.z()));
	box_max = Vector3f(std::max(box_max.x(), photon.pos.x()), std::max(box_max.y(), photon.pos.y()), std::max(box_max.z(), photon.pos.z()));
}

void Photonmap::Balance() {
	Photon* porg = new Photon[stored_photons + 1];

	for (int i = 0; i <= stored_photons; i++)
		porg[i] = photons[i];
	
	BalanceSegment(porg, 1, 1, stored_photons);
	delete[] porg;
}

Nearestphotons::Nearestphotons() {
	max_photons = 0;
	found = 0;
	got_heap = false;
	dist2 = nullptr;
	photons = nullptr;
}

Nearestphotons::~Nearestphotons() {
	delete[] dist2;
	delete[] photons;
}
