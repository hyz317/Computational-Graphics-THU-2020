#ifndef PHOTONTRACER_H
#define PHOTONTRACER_H

#include <cmath>
#include <vector>
#include <vecmath.h>
#include "ray.hpp"
#include "group.hpp"
#include "light.hpp"
#include "camera.hpp"
#include "Photonmap.hpp"
#include "hitpointmap.hpp"

#define EPS 1e-3

class PhotonTracer
{
public:
    PhotonTracer(std::vector<Light*>& l, int e, int d = 10, float tm = 1e-3);
    ~PhotonTracer() {}

    // void setLights(std::vector<Light*>& lights) { this->lights = lights; }
    void setGroup(Group* group) { this->group = group; }
    void setPhotonMap(Photonmap* photonmap) { this->photonmap = photonmap; }
    void SetHitpointMap(HitpointMap* _hitpointMap) { hitpointMap = _hitpointMap; }
    Photonmap* GetPhotonmap() { return photonmap; }
    Photonmap* CalcPhotonmap();
    Image* CalcVolumetricmap(Camera* camera);
    void Emitting(int test = 0);

private:
    int max_depth;
    float tmin;
    Group* group;
    Vector3f bkgcolor;
    std::vector<Light*>& lights;

    Photonmap* photonmap;
    HitpointMap* hitpointMap;
    int emit_photons;
	int iteration;

	void PhotonTracing(Photon photon, int depth, bool refracted, unsigned short Xi[]);
	bool PhotonDiffusion(Ray& ray, Hit& hit, Photon photon, int depth, bool refracted, float* prob, unsigned short Xi[]);
	bool PhotonReflection(Ray& ray, Hit& hit, Photon photon, int depth, bool refracted, float* prob, unsigned short Xi[]);
	bool PhotonRefraction(Ray& ray, Hit& hit, Photon photon, int depth, bool refracted, float* prob, unsigned short Xi[]);
};

#endif // PHOTONTRACER_H
