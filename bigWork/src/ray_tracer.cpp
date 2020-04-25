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
        light->getIllumination(p, dirToLight, lightColor);
        ans += material->Shade(ray, hit, dirToLight, lightColor) * material->diff_factor;
    }
    return ans;
}

Vector3f RayTracer::calcReflection(Ray ray, Hit& hit, int depth)
{
    // TODO: diffuse reflection
    Ray new_ray = Ray(ray.getOrigin() + ray.getDirection() * hit.getT(), 
                      ray.getDirection() - hit.getNormal() * 2 * Vector3f::dot(hit.getNormal(), ray.getDirection()) );

    /*if (ray.getDirection() * hit.getNormal() > 0) {
        std::cout << "oops! depth: " << depth << " direction: (" << ray.getDirection().x() << ", " <<ray.getDirection().y() << ", " << ray.getDirection().z() << ") normal: ("
                  << hit.getNormal().x() << ", " << hit.getNormal().y() << ", " << hit.getNormal().z() << ") hitPoint: ("
                  << new_ray.getOrigin().x() << ", " << new_ray.getOrigin().y() << ", " << new_ray.getOrigin().z() << ") refldir: ("
                  << new_ray.getDirection().x() << ", " << new_ray.getDirection().y() << ", " << new_ray.getDirection().z() << ")\n";
    }
    if ((ray.getDirection() - hit.getNormal() * 2 * Vector3f::dot(hit.getNormal(), ray.getDirection())).length() < 0.99 || (ray.getDirection() - hit.getNormal() * 2 * Vector3f::dot(hit.getNormal(), ray.getDirection())).length() > 1.01)
    std::cout << "you stupid!\n";*/
    
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
