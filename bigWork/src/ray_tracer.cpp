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
    return Vector3f::ZERO;
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
