#include "fresnel.hpp"
#include <lightwave.hpp>

namespace lightwave {

class Dielectric : public Bsdf {
    ref<Texture> m_ior;
    ref<Texture> m_reflectance;
    ref<Texture> m_transmittance;

public:
    Dielectric(const Properties &properties) {
        m_ior           = properties.get<Texture>("ior");
        m_reflectance   = properties.get<Texture>("reflectance");
        m_transmittance = properties.get<Texture>("transmittance");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        // the probability of a light sample picking exactly the direction `wi'
        // that results from reflecting or refracting `wo' is zero, hence we can
        // just ignore that case and always return black
        return BsdfEval::invalid();
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        // NOT_IMPLEMENTED
        float ior = m_ior->scalar(uv);
        bool entry_order = Frame::cosTheta(wo) > 0;
        float eta = entry_order ? ior : 1.0f / ior; // interior->exterior or exterior->interior
        Vector n = entry_order ? Vector(0.0f, 0.0f, 1.0f) : Vector(0.0f, 0.0f, -1.0f);

        float F = fresnelDielectric(Frame::cosTheta(wo), eta);

        if(rng.next() < F) return {.wi = reflect(wo, n), .weight = m_reflectance->evaluate(uv)};
        else return {.wi = refract(wo, n, eta), .weight = m_transmittance->evaluate(uv) / sqr(eta)};
    }

    Color albedo(const Point2 &uv) const override {
        float ior = m_ior->scalar(uv);
        float F = fresnelDielectric(1.0f, ior);
        return m_reflectance->evaluate(uv) * F + m_transmittance->evaluate(uv) * (1.0f - F);
    }

    std::string toString() const override {
        return tfm::format(
            "Dielectric[\n"
            "  ior           = %s,\n"
            "  reflectance   = %s,\n"
            "  transmittance = %s\n"
            "]",
            indent(m_ior),
            indent(m_reflectance),
            indent(m_transmittance));
    }
};

} // namespace lightwave

REGISTER_BSDF(Dielectric, "dielectric")
