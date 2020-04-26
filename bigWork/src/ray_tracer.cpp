#include "ray_tracer.hpp"
#include <iostream>

Vector3f RayTracer::calcDiffusion(Ray ray, Hit& hit, int depth)
{
    Vector3f ans(0, 0, 0);
    for (auto light : lights) {
        Material* material = hit.getMaterial();
        Vector3f dirToLight;
        Vector3f lightColor;
        Vector3f p = ray.pointAtParameter(hit.getT());
        light->getIllumination(p, dirToLight, lightColor, group);
        ans += material->Shade(ray, hit, dirToLight, lightColor) * material->diff_factor;
    }
    return ans;
}

Vector3f RayTracer::calcReflection(Ray ray, Hit& hit, int depth)
{
    // TODO: diffuse reflection
    Ray new_ray = Ray(ray.getOrigin() + ray.getDirection() * hit.getT(), 
                      (ray.getDirection() - hit.getNormal() * 2 * Vector3f::dot(hit.getNormal(), ray.getDirection())).normalized() );
  
    return trace(new_ray, depth + 1) * hit.getMaterial()->spec_factor;
}

Vector3f RayTracer::calcRefraction(Ray ray, Hit& hit, int depth)
{
    // TODO: Refraction
    bool front = (Vector3f::dot(ray.getDirection(), hit.getNormal()) < 0);
    float n = hit.getMaterial()->n;
    if (front) n = 1 / n;

    float cosi = Vector3f::dot(hit.getNormal(), ray.getDirection()) * -1.0f;
    float cosr2 = 1 - pow(n, 2.0f) * (1 - pow(cosi, 2.0f));
    // std::cout << 1 - pow(cosi, 2.0f) << ' ' << pow(n, 2.0f) << ' ' << cosr2 << std::endl;
    Vector3f ans(0, 0, 0);
    if (cosr2 < EPS) {
        ans = calcReflection(ray, hit, depth);
    } else {
        Vector3f new_dir = ray.getDirection() * n + hit.getNormal() * (n * cosi - sqrt(cosr2));
        Ray new_ray = Ray(ray.getOrigin() + ray.getDirection() * hit.getT(), new_dir.normalized());
        Ray new_reflect_ray = Ray(ray.getOrigin() + ray.getDirection() * hit.getT(), 
                                 (ray.getDirection() - hit.getNormal() * 2 * Vector3f::dot(hit.getNormal(), ray.getDirection())).normalized() );
        float Er = cosi / n - sqrt(cosr2);
        float Ei = cosi / n + sqrt(cosr2);
        // ans += trace(new_reflect_ray, depth + 1) * hit.getMaterial()->refr_factor * Er / sqrt(Er * Er + Ei * Ei);
        ans += trace(new_ray, depth + 1) * hit.getMaterial()->refr_factor; // * Ei / sqrt(Er * Er + Ei * Ei);
    }

    if (!front) {
        Vector3f absorb = hit.getMaterial()->absorbColor * ray.pointAtParameter(hit.getT()).length();
        Vector3f transform(exp(absorb.x()), exp(absorb.y()), exp(absorb.z()));
        ans = ans * transform;
    }
    return ans;
}

Vector3f RayTracer::trace(Ray ray, int depth)
{
    // TODO: overall tracing
    if (depth > max_depth) return Vector3f::ZERO;
    Vector3f ans(0, 0, 0);
    Hit hit;

    if (group->intersect(ray, hit, tmin)) {
        Material* material = hit.getMaterial();
        if (material->diff_factor > EPS) ans += calcDiffusion(ray, hit, depth);
        if (material->spec_factor > EPS) ans += calcReflection(ray, hit, depth);
        if (material->refr_factor > EPS) ans += calcRefraction(ray, hit, depth);

    } else {
        ans = bkgcolor;
    }

    return ans;
}
