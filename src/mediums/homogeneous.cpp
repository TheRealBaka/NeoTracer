#include <lightwave.hpp>

namespace lightwave {

class HomogeneousMedium : public Medium {
    float m_sigmaT;
    float m_sigmaS;
    float m_sigmaA;
    float m_density;
    float m_hg;
    Color m_color;
    ref<Emission> m_emission;

public:
    HomogeneousMedium(const Properties &properties) : Medium(properties) {
        m_sigmaA = properties.get<float>("sigmaA", 0.5);
        m_sigmaS = properties.get<float>("sigmaS", 0.5);
        m_density = properties.get<float>("density", 1);
        m_hg = properties.get<float>("hg", 0);
        m_sigmaT = (m_sigmaA + m_sigmaS) * m_density;
        m_color = properties.get<Color>("color", Color(0));
        m_emission = properties.getOptionalChild<Emission>();
    }

    Emission *emission() const override { return m_emission.get(); }

    float evalTransmittance(const Ray &ray, const float &its_t, Sampler &rng) const override {
        float negLength = (ray.origin - ray(its_t)).length();

        return exp(-negLength * m_sigmaT);
    }

    float sampleDistance(const Ray &ray, Sampler &rng) const override {
        // sample a distance at which we scatter in the volume
        return -std::log(1 - rng.next())/m_sigmaT;
    }

    float sampleAttenuation(const float &its_t) const override {
        return exp(-m_sigmaT * its_t);
    }

    Color getColor() const override {
        return m_color;
    }

    float getSigmaS() const override {
        return m_sigmaS;
    }

    float getDensity() const override {
        return m_density;
    }

    float HGPhase(const float &cos_theta) const override {
        float denom = 1 + sqr(m_hg) - 2 * m_hg * cos_theta;
        return Inv4Pi * (1 - sqr(m_hg)) / (denom * sqrtf(denom));
    }

    std::string toString() const override {
        return tfm::format("Homogeneous medium[\n"
                           "  density = %s\n"
                           "]",
                           indent(m_sigmaT));
    }
};

} // namespace lightwave

REGISTER_CLASS(HomogeneousMedium, "medium", "homogeneous")
