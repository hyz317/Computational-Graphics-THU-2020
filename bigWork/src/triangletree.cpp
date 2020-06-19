#include "box.hpp"

TriangleTree::TriangleTree() {
	root = new TriangleBox;
}

TriangleTree::~TriangleTree() {
	deleteTree(root);
}

void TriangleTree::deleteTree(TriangleBox* node) {
	if (node->l != nullptr)
		deleteTree(node->l);
	if (node->r != nullptr)
		deleteTree(node->r);
	delete node;
}

void TriangleTree::sortTriangle(Triangle** tris, int l, int r, int coord, bool minCoord) {
	float (Triangle::*GetCoord)(int) = minCoord ? &Triangle::getMin : &Triangle::getMax;
	if (l >= r) return;
	int i = l, j = r;
	Triangle* key = tris[(l + r) >> 1];
	while (i <= j) {
		while (j >= l && (key->*GetCoord)(coord) < (tris[j]->*GetCoord)(coord)) j--;
		while (i <= r && (tris[i]->*GetCoord)(coord) < (key->*GetCoord)(coord)) i++;
		if (i <= j) {
			std::swap(tris[i], tris[j]);
			i++;
			j--;
		}	
	}
	sortTriangle(tris, i, r, coord, minCoord);
	sortTriangle(tris, l, j, coord, minCoord);
}

void TriangleTree::divideNode(TriangleBox* node) {
	//iff area0 * size0 + area1 * size1 + totalArea <= totalArea * totalSize then divide
	Triangle** minNode = new Triangle*[node->size];
	Triangle** maxNode = new Triangle*[node->size];
	for (int i = 0; i < node->size; i++) {
		minNode[i] = node->tris[i];
		maxNode[i] = node->tris[i];
	}
	
	double thisCost = node->area() * (node->size - 1);
	double minCost = thisCost;
	int bestCoord = -1, leftSize = 0, rightSize = 0;
	double bestSplit = 0;
	for (int coord = 0; coord < 3; coord++) {
		sortTriangle(minNode, 0, node->size - 1, coord, true);
		sortTriangle(maxNode, 0, node->size - 1, coord, false);
		TriangleBox leftBox;
		TriangleBox rightBox;
		leftBox.box_min = node->box_min; leftBox.box_max = node->box_max;
		rightBox.box_min = node->box_min; rightBox.box_max = node->box_max;

		int j = 0;
		for (int i = 0; i < node->size; i++) {
			double split = minNode[i]->getMin(coord);
			leftBox.box_max.Axis(coord) = split;
			rightBox.box_min.Axis(coord) = split;
			for ( ; j < node->size && maxNode[j]->getMax(coord) <= split + EPS; j++);
			double cost = leftBox.area() * i + rightBox.area() * (node->size - j);
			if (cost < minCost) {
				minCost = cost;
				bestCoord = coord;
				bestSplit = split;
				leftSize = i;
				rightSize = node->size - j;
			}
		}

		j = 0;
		for (int i = 0; i < node->size; i++) {
			double split = maxNode[i]->getMax(coord);
			leftBox.box_max.Axis(coord) = split;
			rightBox.box_min.Axis(coord) = split;
			for ( ; j < node->size && minNode[j]->getMin(coord) <= split - EPS; j++);
			double cost = leftBox.area() * j + rightBox.area() * (node->size - i);
			if (cost < minCost) {
				minCost = cost;
				bestCoord = coord;
				bestSplit = split;
				leftSize = j;
				rightSize = node->size - i;
			}
		}
	}

	delete minNode;
	delete maxNode;

	if (bestCoord != -1) {
		leftSize = rightSize = 0;
		for (int i = 0; i < node->size; i++) {
			if (node->tris[i]->getMin(bestCoord) <= bestSplit - EPS || node->tris[i]->getMax(bestCoord) <= bestSplit + EPS)
				leftSize++;
			if (node->tris[i]->getMax(bestCoord) >= bestSplit + EPS || node->tris[i]->getMin(bestCoord) >= bestSplit - EPS)
				rightSize++;
		}
		TriangleBox leftBox;
		TriangleBox rightBox;
		leftBox.box_min = node->box_min; leftBox.box_max = node->box_max;
		rightBox.box_min = node->box_min; rightBox.box_max = node->box_max;

		leftBox.box_max.Axis(bestCoord) = bestSplit;
		rightBox.box_min.Axis(bestCoord) = bestSplit;
		double cost = leftBox.area() * leftSize + rightBox.area() * rightSize;

		if (cost < thisCost) {
			node->plane = bestCoord;
			node->split = bestSplit;

			node->l = new TriangleBox;
			node->l->box_min = node->box_min; node->l->box_max = node->box_max;
			node->l->box_max.Axis(node->plane) = node->split;
			
			node->r = new TriangleBox;
			node->r->box_min = node->box_min; node->r->box_max = node->box_max;
			node->r->box_min.Axis(node->plane) = node->split;
			
			for (int i = 0; i < node->size; i++) {
				if (node->tris[i]->getMin(node->plane) <= node->split - EPS || node->tris[i]->getMax(node->plane) <= node->split + EPS)
					node->l->tris.emplace_back(node->tris[i]);
				if (node->tris[i]->getMax(node->plane) >= node->split + EPS || node->tris[i]->getMin(node->plane) >= node->split - EPS)
					node->r->tris.emplace_back(node->tris[i]);
			}
			node->l->size = node->l->tris.size();
			node->r->size = node->r->tris.size();

			divideNode(node->l);
			divideNode(node->r);
		}
	}
}

bool TriangleTree::intersect(TriangleBox* node, const Ray& ray, Hit& hit) {
	
	if (!node->contain(ray.getOrigin()) && node->intersect(ray) <= -EPS)
		return false;

	if (node->l == nullptr && node->r == nullptr) {
		bool res = false;
		for (int i = 0; i < node->size; i++) {
			// std::cout << node->size << std::endl;
			res |= node->tris[i]->intersect(ray, hit, 1e-3);
		}
		return res;
	}
	
	if (node->l->contain(ray.getOrigin())) {
		bool res = intersect(node->l, ray, hit);
		if (res) return true;
		return intersect(node->r, ray, hit);
	}
	if (node->r->contain(ray.getOrigin())) {
		bool res = intersect(node->r, ray, hit);
		if (res) return true;
		return intersect(node->l, ray, hit);
	}

	float leftDist = node->l->intersect(ray);
	float rightDist = node->r->intersect(ray);
	if (rightDist <= -EPS)
		return intersect(node->l, ray, hit);
	if (leftDist <= -EPS)
		return intersect(node->r, ray, hit);
	
	if (leftDist < rightDist) {
		bool res = intersect(node->l, ray, hit);
		if (res) return true;
		return intersect(node->r, ray, hit);
	}
	bool res = intersect(node->r, ray, hit);
	if (res) return true;
	return intersect(node->l, ray, hit);
}
