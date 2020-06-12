#include "photon_tracer.hpp"
#include <iostream>

PhotonTracer::PhotonTracer(std::vector<Light*>& l, int e, int d, float tm) : lights(l) {
	iteration = 0;
	photonmap = nullptr;
    emit_photons = e;
    max_depth = d;
    tmin = tm;
}

bool PhotonTracer::PhotonDiffusion(Ray& ray, Hit& hit, Photon photon, int depth, bool refracted, float* prob, unsigned short Xi[]) {
	Material* material = hit.getMaterial();
	Vector3f color = material->getRealDiffuseColor(hit);

	float eta = material->diff_factor * (color.x() + color.y() + color.z()) / 3.0f;
	if (eta <= erand48(Xi) * ( *prob )) {
		*prob -= eta;
		return false;
	}

    // 获得随机漫反射方向
	float r1 = 2 * M_PI * erand48(Xi), r2 = erand48(Xi), r2s = sqrt(r2);  
    Vector3f w = hit.getNormal();
    Vector3f u = Vector3f::cross(fabs(w.x()) > 0.1 ? Vector3f(0, 1, 0) : Vector3f(1, 0, 0), w).normalized();
    Vector3f v = Vector3f::cross(w, u);
    photon.dir = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).normalized();

	photon.power = photon.power * color / (color.x() + color.y() + color.z()) * 3.0f;
	PhotonTracing(photon, depth + 1, refracted, Xi);
	return true;
}

bool PhotonTracer::PhotonReflection(Ray& ray, Hit& hit, Photon photon, int depth, bool refracted, float* prob, unsigned short Xi[]) {
	Material* material = hit.getMaterial();
	Vector3f color = material->getRealDiffuseColor(hit);
	double eta = material->spec_factor * (color.x() + color.y() + color.z()) / 3.0f;

	if (eta <= erand48(Xi) * ( *prob )) {
		*prob -= material->spec_factor;
		return false;
	}
	
    // 反射光线
    photon.dir = (photon.dir - hit.getNormal() * 2 * Vector3f::dot(hit.getNormal(), photon.dir)).normalized();

	// TODO: spec reflection

    /*if ( material->drefl > EPS ) {
		Vector3 Dx = photon.dir.GetAnVerticalVector();
		Vector3 Dy = photon.dir.Cross(Dx);
		Dx = Dx.GetUnitVector() * material->drefl;
		Dy = Dy.GetUnitVector() * material->drefl;
		double x , y;
		do {
			x = ran() * 2 - 1;
			y = ran() * 2 - 1;
		} while ( x * x + y * y > 1 );
		x *= material->drefl;
		y *= material->drefl;
		photon.dir += Dx * x + Dy * y;
	}*/

	photon.power = photon.power * color / (color.x() + color.y() + color.z()) * 3.0f;
	PhotonTracing(photon, depth + 1, refracted, Xi);
	return true;
}

bool PhotonTracer::PhotonRefraction(Ray& ray, Hit& hit, Photon photon, int depth, bool refracted, float* prob, unsigned short Xi[]) {
	Material* material = hit.getMaterial();
	double eta = material->refr_factor;
	if (refracted) {
		Vector3f trans = (material->absorbColor * -hit.getT()).Exp();
		eta *= (trans.x() + trans.y() + trans.z()) / 3.0f;
		photon.power = photon.power * trans / ( (trans.x() + trans.y() + trans.z()) / 3.0f );
	}

	if (eta <= erand48(Xi) * ( *prob )) {
		*prob -= material->refr_factor;
		return false;
	}
	
	double n = material->n;
	if (!refracted) n = 1 / n;
	bool nextRefracted = refracted;

    // 折射光线
    Vector3f V = photon.dir.normalized();
	float cosI = -Vector3f::dot(hit.getNormal(), V);
    float cosT2 = 1 - ( n * n ) * ( 1 - cosI * cosI );
	if ( cosT2 > EPS ) {
		refracted ^= true;
		photon.dir = V * n + hit.getNormal() * ( n * cosI - sqrt( cosT2 ) );
	} else {
        photon.dir = (V - hit.getNormal() * 2 * Vector3f::dot(hit.getNormal(), V)).normalized();
    }

	PhotonTracing(photon, depth + 1, nextRefracted, Xi);
	return true;
}

void PhotonTracer::PhotonTracing(Photon photon, int depth, bool refracted, unsigned short Xi[]) {
	if (depth > max_depth) return;
    Hit hit;
    Ray ray = Ray(photon.pos, photon.dir);
	// std::cout << "??????????????????\n";
	// std::cout << photon.pos << ' ' << photon.dir << std::endl;
	bool res = group->intersect(ray, hit, tmin);
	
	if (res) {	
		photon.pos = ray.pointAtParameter(hit.getT());
        Material* material = hit.getMaterial();
		if (material->diff_factor > EPS && depth > 1 ) {
			if (photonmap != NULL)
				photonmap->Store(photon);
			//if (hitpointMap != NULL)
			//	hitpointMap->InsertPhoton(photon);	
		}
		
		float prob = 1;
		if ( PhotonDiffusion(ray, hit, photon, depth, refracted, &prob, Xi) == false )
		if ( PhotonReflection(ray, hit, photon, depth, refracted, &prob, Xi) == false )
		if ( PhotonRefraction(ray, hit, photon, depth, refracted, &prob, Xi) == false );
	}
}

void PhotonTracer::Emitting() {
    double totalPower = 0;
	for (auto light : lights)
		totalPower += light->getColor().mean();
	double photonPower = totalPower / emit_photons;
	
	for (auto light : lights) {
		int lightPhotons = (int)(light->getColor().mean() / photonPower);

		#pragma omp parallel for
		for (int i = 0; i < lightPhotons ; i++) {
			unsigned short Xi[3] = {i, i*i, i*i*i};
			if (/*scene->GetCamera()->getAlgorithm() == "PM" && */ (i & 65535) == 0)
				printf("Emitted Photons= %d, Stored Photons= %d\n", i, photonmap->getStoredPhotons());
			Photon photon = light->emitPhoton(Xi);
			photon.power *= totalPower;
			PhotonTracing(photon, 1, false, Xi);
		}
	}
}

Photonmap* PhotonTracer::CalcPhotonmap() {
	photonmap = new Photonmap(1200000); // TODO
	photonmap->setEmitPhotons(emit_photons);

	Emitting();

	photonmap->Balance();
	return photonmap;
}
