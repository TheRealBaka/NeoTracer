#include <lightwave.hpp>

namespace lightwave {

/// @brief Code structure for a sphere
// Reference: https://pbr-book.org/4ed/Shapes/Spheres
// UV map reference: https://en.wikipedia.org/wiki/UV_mapping

class Sphere : public Shape {
private:
    bool m_improved; // Implements Fred Akalin's cone sampling strategy
    // Ref: https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-lighting/introduction-to-lighting-spherical-light-cone-sampling.html

    inline float ConePdf(SurfaceEvent &surf, const Point &refpos) const {
        Vector refpos_orig = getCentroid() - refpos;
        float pos_len = refpos_orig.length();
        float sin_theta_max = 1.0f / pos_len;
        float cos_theta_max = safe_sqrt(1.0f - sqr(sin_theta_max));
        float sur_pdf = Inv2Pi / (1.0f - cos_theta_max);

        auto [distance, wi] = (surf.position - refpos).lengthAndNormalized();
        return sur_pdf * abs(surf.shadingFrame().normal.dot(-wi)) / sqr(distance);
    }

public:
    inline void populate(SurfaceEvent &surf, const Point &position) const {

        Vector normal = (position - getCentroid()).normalized();

        float phi = atan2(normal.z(), normal.x());
        float theta = asin(normal.y()); // radius is one, else n_y would be divided with it

        surf.uv.x() = phi * Inv2Pi + 0.5;
        surf.uv.y() = theta * InvPi + 0.5; 

        // Ref: https://computergraphics.stackexchange.com/questions/5498/compute-sphere-tangent-for-normal-mapping
        // Vector tangent = Vector(-sin(phi), 0.0f, cos(phi)); // Using normalized representation

        surf.position = position;

        Frame shading_frame = Frame(normal.normalized());
        surf.geometryNormal = shading_frame.normal;
        surf.shadingNormal = shading_frame.normal;
        surf.tangent = shading_frame.tangent;

        // Choose an arbitrary vector to create the tangent
        // Vector V = (surf.shadingNormal.dot(Vector(1.0f, 0.0f, 0.0f)) < Epsilon) ? Vector(1.0f, 0.0f, 0.0f) : Vector(0.0f, 1.0f, 0.0f);
        
        // Compute tangent and normalize
        // its.tangent = its.shadingNormal.cross(V).normalized();

        surf.pdf = Inv4Pi;
        }

    Sphere(const Properties &properties) {
        m_improved = properties.get<bool>("improved", true); // Defaults to true
    }

    bool intersect(const Ray &ray, Intersection &its,
                   Sampler &rng) const override {
        PROFILE("Sphere")

        const float radius = 1.0f;

        Vector orig_centered = ray.origin - getCentroid();

        // Calculate sphere quadratic coefficients
        float a = ray.direction.lengthSquared(); // ||d||^2
        float b = 2.0f * (ray.direction.dot(orig_centered)); // 2 * (d . o)
        float c = orig_centered.lengthSquared() - sqr(radius); // ||o||^2 - r^2

        float D = sqr(b) - (4 * a * c);

        if(D < Epsilon)
            return false;

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
        
        if(m_improved) its.pdf = ConePdf(its, ray.origin);

        return true;
    }

    Bounds getBoundingBox() const override {
        return Bounds(Point(-1.0f), Point(1.0f));
    }

    Point getCentroid() const override { 
        return Point(0.0f);
    }

    AreaSample sampleArea(Sampler &rng) const override {
        // NOT_IMPLEMENTED
        AreaSample sample;
        Point position = squareToUniformSphere(rng.next2D()).normalized();
        populate(sample, position);
        return sample;
    }

    AreaSample sampleArea(Sampler &rng, const SurfaceEvent &ref) const override {
        if(!m_improved) return sampleArea(rng);

        AreaSample sample;
        Point position = squareToUniformSphereCone(rng.next2D(), ref.position).normalized();
        populate(sample, position);

        sample.pdf = ConePdf(sample, ref.position);

        return sample;
    }

    std::string toString() const override { 
        return "Sphere[]"; 
    }
};

} // namespace lightwave

// this informs lightwave to use our class Sphere whenever a <shape
// type="sphere" /> is found in a scene file
REGISTER_SHAPE(Sphere, "sphere")