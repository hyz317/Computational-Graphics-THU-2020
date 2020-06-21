#include "photon_tracer.hpp"
#include <iostream>

PhotonTracer::PhotonTracer(std::vector<Light*>& l, int e, int d, float tm) : lights(l) {
	iteration = 0;
	photonmap = nullptr;
	hitpointMap = nullptr;
    emit_photons = e;
    max_depth = d;
    tmin = tm;
}

bool PhotonTracer::PhotonDiffusion(Ray& ray, Hit& hit, Photon photon, int depth, bool refracted, float* prob, unsigned short Xi[]) {
	Material* material = hit.getMaterial();
	Vector3f color = material->getRealDiffuseColor(hit, ray);

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
	Vector3f color = material->getRealDiffuseColor(hit, ray);
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
			if (photonmap != nullptr)
				photonmap->Store(photon);
			if (hitpointMap != nullptr)
				hitpointMap->InsertPhoton(photon);	
		}
		
		float prob = 1;
		if ( PhotonDiffusion(ray, hit, photon, depth, refracted, &prob, Xi) == false )
		if ( PhotonReflection(ray, hit, photon, depth, refracted, &prob, Xi) == false )
		if ( PhotonRefraction(ray, hit, photon, depth, refracted, &prob, Xi) == false );
	}
}

void PhotonTracer::Emitting(int test) {
    double totalPower = 0;
	for (auto light : lights)
		totalPower += light->getColor().mean();
	double photonPower = totalPower / emit_photons;
	
	for (auto light : lights) {
		int lightPhotons = (int)(light->getColor().mean() / photonPower);

		#pragma omp parallel for
		for (int i = 0; i < lightPhotons; i++) {
			// if (photonmap == nullptr && test) printf("fuck you!!!\n");
			unsigned short Xi[3] = {i, i*i, i*i*i};
			/*if (scene->GetCamera()->getAlgorithm() == "PM" &&  (i & 65535) == 0)
				printf("Emitted Photons= %d, Stored Photons= %d\n", i, photonmap->getStoredPhotons());*/
			Photon photon = light->emitPhoton(Xi);
			photon.power *= totalPower;
			PhotonTracing(photon, 1, false, Xi);
		}
	}
}

Photonmap* PhotonTracer::CalcPhotonmap() {
	photonmap = new Photonmap(2000000); // TODO
	photonmap->setEmitPhotons(emit_photons);

	Emitting();

	photonmap->Balance();
	return photonmap;
}

Image* PhotonTracer::CalcVolumetricmap(Camera* camera)
{
	srand(time(nullptr));
	PerspectiveCamera* pcamera = (PerspectiveCamera*) camera;
	Image* image = new Image(camera->getWidth(), camera->getHeight());
	for (auto light : lights) {
		for (int i = 0; i < 720000000; i++) {
			unsigned short Xi[3] = {i, i*i, i*i*i*i};
			Photon photon = light->emitPhoton(Xi);
			Ray ray(photon.pos, photon.dir.normalized());
			Hit hit;
			if (group->intersect(ray, hit, 1e-3)) {
				unsigned short Xi[3] = {i, i*i, i*i*i};
				float t = hit.getT() * (1 - pow((static_cast<double>(rand()) / RAND_MAX + erand48(Xi)) / 2, 2));
				Vector3f pos = ray.pointAtParameter(t);
				int w, h;
				pcamera->calcPixel(pos, w, h);
				if (w >= camera->getWidth() || w < 0 || h >= camera->getHeight() || h < 0) continue;
				// std::cout << w << ' ' << h << ' ' << photon.power << std::endl;
				// std::cout << erand48(Xi) << std::endl;
				Vector3f power = image->GetPixel(w, h);
				image->SetPixel(w, h, photon.power / 100 + power);		
			}
			if (i % 1000000 == 0) {
				image->SaveImage("volumatrictest.bmp");
				std::cout << "volumetric light process " << i / 1000000 << '/' << 720 << std::endl;
			}
		}
	}
	/*Image* newimg = new Image(camera->getWidth(), camera->getHeight());
	for (int y = 0; y < camera->getHeight(); y++) {
		for (int x = 0; x < camera->getWidth(); x++) {
			int counter = 0;
			Vector3f color(0,0,0);
			for (int i = -2; i <= 2 && x-i >= 0 && x+i < camera->getWidth(); i++) {
				for (int j = -2; j <= 2 && y-j >= 0 && y+j < camera->getHeight(); j++) {
					counter++;
					color += image->GetPixel(x+i, y+j);
				}
			}
			// std::cout << x << ' ' << y << ' ' << counter << std::endl;
			newimg->SetPixel(x, y, color / counter);
		}
	}
	delete image;
	newimg->SaveImage("volumatrictest.bmp");
	return newimg;*/
	return image;
}
