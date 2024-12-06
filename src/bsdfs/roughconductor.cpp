#include "fresnel.hpp"
#include "microfacet.hpp"
#include <lightwave.hpp>

namespace lightwave {

class RoughConductor : public Bsdf {
    ref<Texture> m_reflectance;
    ref<Texture> m_roughness;

public:
    RoughConductor(const Properties &properties) {
        m_reflectance = properties.get<Texture>("reflectance");
        m_roughness   = properties.get<Texture>("roughness");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        // Using the squared roughness parameter results in a more gradual
        // transition from specular to rough. For numerical stability, we avoid
        // extremely specular distributions (alpha values below 10^-3)
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));

        // NOT_IMPLEMENTED
        Vector wm = (wi + wo).normalized(); // Microfacet normal

        float D = microfacet::evaluateGGX(alpha, wm); // Microfacet normal distribution
        float G1_i = microfacet::smithG1(alpha, wm, wi);
        float G1_o = microfacet::smithG1(alpha, wm, wo);

        // cos(wi) cancels from rendering equation term
        return { .value = m_reflectance->evaluate(uv) * (D * G1_i * G1_o * 0.25) / Frame::absCosTheta(wo)};
        // hints:
        // * the microfacet normal can be computed from `wi' and `wo'
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));

        // NOT_IMPLEMENTED
        Vector wm = microfacet::sampleGGXVNDF(alpha, wo, rng.next2D());
        Vector wi = reflect(wo, wm);

        // 4, |wi.wo| canceled from Jacobian, rest of the terms canceled from p(wm), with cos(wi) included in rendering equation
        return { .wi = wi, .weight = m_reflectance->evaluate(uv) * microfacet::smithG1(alpha, wm , wi)};

        // hints:
        // * do not forget to cancel out as many terms from your equations as
        // possible!
        //   (the resulting sample weight is only a product of two factors)
    }

    std::string toString() const override {
        return tfm::format(
            "RoughConductor[\n"
            "  reflectance = %s,\n"
            "  roughness = %s\n"
            "]",
            indent(m_reflectance),
            indent(m_roughness));
    }
};

} // namespace lightwave

REGISTER_BSDF(RoughConductor, "roughconductor")
