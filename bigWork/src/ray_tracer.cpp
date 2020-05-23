#include "ray_tracer.hpp"
#include <iostream>

Vector3f RayTracer::calcDiffusion(Ray ray, Hit& hit, int depth, unsigned short Xi[])
{
    Vector3f ans(0, 0, 0);
    for (auto light : lights) {
        Material* material = hit.getMaterial();
        Vector3f dirToLight;
        Vector3f lightColor;
        Vector3f p = ray.pointAtParameter(hit.getT());
        light->getIllumination(p, dirToLight, lightColor, group, Xi);
        ans += material->Shade(ray, hit, dirToLight, lightColor) * material->diff_factor;
    }
    return ans;
}

Vector3f RayTracer::calcRandomDiffusion(Ray ray, Hit& hit, int depth, unsigned short Xi[])
{
    float r1 = 2 * M_PI * erand48(Xi), r2 = erand48(Xi), r2s = sqrt(r2);  
    Vector3f w = hit.getNormal();
    Vector3f u = Vector3f::cross(fabs(w.x()) > 0.1 ? Vector3f(0, 1, 0) : Vector3f(1, 0, 0), w).normalized();
    Vector3f v = Vector3f::cross(w, u);
    Vector3f d = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).normalized();

    Vector3f tra = trace(Ray(ray.pointAtParameter(hit.getT()), d), Xi, depth + 1);
    Vector3f ans = hit.getMaterial()->getDiffuseColor() * tra;
    /*if (ans.length() > 1e-3)
    std::cout << "color: " << ans.x() << ' ' << ans.y() << ' ' << ans.z() << ' '
              << "hit pos: " << ray.pointAtParameter(hit.getT()).x() << ' ' << ray.pointAtParameter(hit.getT()).y() << ' ' << ray.pointAtParameter(hit.getT()).z() << ' ' 
              << "dir: " << d.x() << ' ' << d.y() << ' ' << d.z() << ' ' 
              << "depth: " << depth << std::endl;*/
    return ans;
}

Vector3f RayTracer::calcReflection(Ray ray, Hit& hit, int depth, unsigned short Xi[])
{
    // TODO: diffuse reflection
    Ray new_ray = Ray(ray.getOrigin() + ray.getDirection() * hit.getT(), 
                      (ray.getDirection() - hit.getNormal() * 2 * Vector3f::dot(hit.getNormal(), ray.getDirection())).normalized() );
  
    return trace(new_ray, Xi, depth + 1) * hit.getMaterial()->spec_factor;
}

Vector3f RayTracer::calcRefraction(Ray ray, Hit& hit, int depth, unsigned short Xi[])
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
        ans = calcReflection(ray, hit, depth, Xi);
    } else {
        Vector3f new_dir = ray.getDirection() * n + hit.getNormal() * (n * cosi - sqrt(cosr2));
        Ray new_ray = Ray(ray.getOrigin() + ray.getDirection() * hit.getT(), new_dir.normalized());
        Ray new_reflect_ray = Ray(ray.getOrigin() + ray.getDirection() * hit.getT(), 
                                 (ray.getDirection() - hit.getNormal() * 2 * Vector3f::dot(hit.getNormal(), ray.getDirection())).normalized() );
        float Er = cosi / n - sqrt(cosr2);
        float Ei = cosi / n + sqrt(cosr2);
        // ans += trace(new_reflect_ray, depth + 1) * hit.getMaterial()->refr_factor * Er / sqrt(Er * Er + Ei * Ei);
        ans += trace(new_ray, Xi, depth + 1) * hit.getMaterial()->refr_factor; // * Ei / sqrt(Er * Er + Ei * Ei);
    }

    if (!front) {
        Vector3f absorb = hit.getMaterial()->absorbColor * ray.pointAtParameter(hit.getT()).length();
        Vector3f transform(exp(absorb.x()), exp(absorb.y()), exp(absorb.z()));
        ans = ans * transform;
    }
    return ans;
}

Vector3f RayTracer::trace(Ray ray, unsigned short Xi[], int depth)
{
    // TODO: overall tracing
    if (depth > max_depth) return Vector3f::ZERO;
    Vector3f ans(0, 0, 0);
    Hit hit;
    float light_dis;
    Vector3f light_color;

    bool res1 = group->intersect(ray, hit, tmin);
    bool res2 = intersectLight(ray, light_dis, light_color);

    // if (res2 && (!res1 || light_dis < hit.getT())) std::cout << light_dis << ' ' << hit.getT() << "\n";

    if (res2 && (!res1 || light_dis < hit.getT())) {
        // if (depth > 2) std::cout << "ohhhh\n";
        return light_color;

    } else if (res1) {
        Material* material = hit.getMaterial();
        if (material->diff_factor > EPS) ans += calcDiffusion(ray, hit, depth, Xi);
        if (material->spec_factor > EPS) ans += calcReflection(ray, hit, depth, Xi);
        if (material->refr_factor > EPS) ans += calcRefraction(ray, hit, depth, Xi);

    } else {
        ans = bkgcolor;
    }

    return ans;
}

bool RayTracer::intersectLight(Ray ray, float& dis, Vector3f& color)
{
    for (auto light : lights) {
        if (light->intersect(ray, dis, color)) return true;
    }
    return false;
}
