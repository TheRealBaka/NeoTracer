#include <lightwave.hpp>

namespace lightwave {

class DirectionalLight final : public Light {

    Color m_intensity;
    Vector m_direction;

public:
    DirectionalLight(const Properties &properties) : Light(properties) {
        // blender_exporter/lightwave_blender/light.py has two properties for directional
        // source: direction and intensity
        m_direction = properties.get<Vector>("direction");
        m_intensity = properties.get<Color>("intensity");
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        // NOT_IMPLEMENTED  
        // return format: direction, sample weight, distance from queried point to sampled point
        return { .wi =  m_direction.normalized(), .weight = m_intensity, .distance = Infinity, .pdf = 1.f};
        
    }

    bool canBeIntersected() const override { return false; }

    std::string toString() const override {
        return tfm::format(
            "DirectionalLight[\n"
            "]");
    }
};

} // namespace lightwave

REGISTER_LIGHT(DirectionalLight, "directional")
