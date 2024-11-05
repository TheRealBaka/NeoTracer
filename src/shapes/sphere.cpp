#include <lightwave.hpp>

namespace lightwave {

/// @brief Code structure for a sphere
// Reference: https://pbr-book.org/4ed/Shapes/Spheres

class Sphere : public Shape {

public:
    inline void populate(SurfaceEvent &surf, const Point &position) const {

        Frame shading_frame = surf.shadingFrame();
        surf.position = shading_frame.normal;

        buildOrthonormalBasis(shading_frame.normal, shading_frame.tangent, shading_frame.bitangent);

        // surf.uv.x() = acos(position.y()) * Inv2Pi;
        // surf.uv.y() = atan2(position.x(), position.z());

        // cross product between tangent and bi-tangent provides normal direction (CCW)
        surf.tangent = shading_frame.tangent;
        // and accordingly, the normal always points in the positive z direction
        surf.shadingNormal  = shading_frame.normal;
        surf.geometryNormal = shading_frame.normal;

        // surf.uv.x() = atan2(shading_frame.normal.x(), shading_frame.normal.z()) * Inv2Pi;
        // surf.uv.y() = acos(shading_frame.normal.y()) * InvPi;

        surf.uv.x() = 0.5f - (float) atan2(shading_frame.normal.z(), shading_frame.normal.x()) * Inv2Pi;
        surf.uv.y() = 0.5f - (float) asin(shading_frame.normal.y()) * InvPi;

        surf.pdf = 0.0f;
        }

    Sphere(const Properties &properties) {}

    bool intersect(const Ray &ray, Intersection &its,
                   Sampler &rng) const override {
        PROFILE("Sphere")

        const float radius = 1.0f;
        Point center = getCentroid();

        Vector orig_centered = ray.origin - center;

        // Calculate sphere quadratic coefficients
        float a = ray.direction.lengthSquared(); // ||d||^2
        float b = 2.0f * (ray.direction.dot(orig_centered)); // 2 * (d . o)
        float c = orig_centered.lengthSquared() - sqr(radius); // ||o||^2 - r^2

        float D = sqr(b) - (4 * a * c);

        if(D < Epsilon)
            return false;

        // TODO: Use the more accurate quadratic approach from pbrt. Using classical definition for now.

        D = sqrt(D);

        float q;
        if(b < 0)
            q = -0.5f * (b - D);
        else
            q = -0.5f * (b + D);
        
        float t0 = q / a;
        float t1 = c / q;

        if (t0 > t1)
            std::swap(t0, t1); // Ensuring t0 is smaller
        
        // Check for nearest intersection

        if (t0 > its.t || t1 < Epsilon)
            return false; // Values are out of shape bounds
        
        float t_hit = t0;

        if (t_hit < Epsilon){
            t_hit = t1;
            if (t_hit > its.t)
                return false;
        }

        Point position = ray(t_hit);
        its.t = t_hit;
        populate(its,
                 position);

        return true;
    }

    Bounds getBoundingBox() const override {
        return Bounds(Point(-1.0f), Point(1.0f));
    }

    Point getCentroid() const override { 
        return Point(0.0f);
    }

    AreaSample sampleArea(Sampler &rng) const override {
        NOT_IMPLEMENTED
    }

    std::string toString() const override { 
        return "Sphere[]"; 
    }
};

} // namespace lightwave

// this informs lightwave to use our class Sphere whenever a <shape
// type="sphere" /> is found in a scene file
REGISTER_SHAPE(Sphere, "sphere")
