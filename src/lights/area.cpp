#include <lightwave.hpp>

namespace lightwave {

class AreaLight final : public Light {

    ref<Instance> m_shape;

public:
    AreaLight(const Properties &properties) : Light(properties) {
        m_shape = properties.getChild<Instance>();
        m_shape->setLight(this);
    }

    DirectLightSample sampleDirectMain(const Point &origin,
                                   const AreaSample sample) const {
        Vector dir = sample.position - origin;
        float dist = dir.length();
        dir = dir.normalized();

        Vector wo = sample.shadingFrame().toLocal(-dir).normalized();
        Color E = m_shape->emission()->evaluate(sample.uv, wo).value;

        return { .wi =  dir, .weight = (E * Frame::absCosTheta(wo)) / (sqr(dist) * sample.pdf), .distance = dist, .pdf = GetSolidAngle(sample.pdf, dist, sample.shadingFrame().normal, dir)};
    }
    
    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        // NOT_IMPLEMENTED  
        AreaSample sample = m_shape->sampleArea(rng);
        return sampleDirectMain(origin, sample);
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng, const SurfaceEvent &ref) const override {
        AreaSample sample = m_shape->sampleArea(rng, ref);
        return sampleDirectMain(origin, sample);
    }

    bool canBeIntersected() const override { return false; }

    std::string toString() const override {
        return tfm::format(
            "AreaLight[\n"
            "]");
    }
};

} // namespace lightwave

REGISTER_LIGHT(AreaLight, "area")
