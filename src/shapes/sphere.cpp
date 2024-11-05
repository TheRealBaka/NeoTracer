#include <lightwave.hpp>

namespace lightwave {

/// @brief Code structure for a sphere
class Sphere : public Shape {

public:
    Sphere(const Properties &properties) {
        NOT_IMPLEMENTED
    }

    bool intersect(const Ray &ray, Intersection &its,
                   Sampler &rng) const override {
        NOT_IMPLEMENTED
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
