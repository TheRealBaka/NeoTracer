#include <lightwave.hpp>

namespace lightwave {

class PointLight final : public Light {

    Color m_power;
    Point m_position;

public:
    PointLight(const Properties &properties) : Light(properties) {
        // blender_exporter/lightwave_blender/light.py has two properties for point source: position and power
        m_position = properties.get<Point>("position");
        m_power = properties.get<Color>("power");
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        // NOT_IMPLEMENTED  
        // return format: direction, sample weight, distance from queried point to sampled point
        Vector dir = m_position - origin;
        float dist = dir.length();

        // Convert power to intensity (Ref. Pg. 28 lecture slides)
        Color intensity = (m_power * Inv4Pi) / sqr(dist);

        return { .wi =  dir.normalized(), .weight = intensity, .distance = dist};
        
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
