#include <lightwave.hpp>

namespace lightwave {

class PointLight final : public Light {

    ref<Light> point_light;

public:
    PointLight(const Properties &properties) : Light(properties) {
        point_light = properties.get<Light>("point");   
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        // NOT_IMPLEMENTED  
        point_light->
        
    }

    bool canBeIntersected() const override { return false; }

    std::string toString() const override {
        return tfm::format(
            "PointLight[\n"
            "]");
    }
};

} // namespace lightwave

REGISTER_LIGHT(PointLight, "point")
