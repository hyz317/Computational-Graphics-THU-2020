#include "ray_tracer.hpp"
#include <iostream>

Vector3f RayTracer::calcDiffusion(Ray ray)
{
    Hit hit;
    Vector3f ans(0, 0, 0);
    if (group->intersect(ray, hit, tmin)) {
        Material* material = hit.getMaterial();
        for (auto light : lights) {
            Vector3f dirToLight;
            Vector3f lightColor;
            Vector3f p = ray.pointAtParameter(hit.getT());
            light->getIllumination(p, dirToLight, lightColor);
            ans += material->Shade(ray, hit, dirToLight, lightColor);
        }
    }
    else {
        ans = bkgcolor;
    }
    return ans;
}

Vector3f RayTracer::calcReflection(Ray ray)
{
    // TODO: Reflection
    return Vector3f::ZERO;
}

Vector3f RayTracer::calcRefraction(Ray ray)
{
    // TODO: Refraction
    return Vector3f::ZERO;
}

Vector3f RayTracer::trace(Ray ray)
{
    // TODO: overall tracing
    return calcDiffusion(ray);
}
