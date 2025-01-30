#include "fresnel.hpp"
#include "microfacet.hpp"
#include <lightwave.hpp>

// Reference for implementation
// https://www.graphics.cornell.edu/~bjw/microfacetbsdf.pdf

namespace lightwave {

class RoughDielectric : public Bsdf {
    ref<Texture> m_reflectance;
    ref<Texture> m_transmittance;
    ref<Texture> m_ior;
    ref<Texture> m_roughness;

public:
    RoughDielectric(const Properties &properties) {
        m_reflectance = properties.get<Texture>("reflectance");
        m_roughness   = properties.get<Texture>("roughness");
        m_ior   = properties.get<Texture>("ior");
        m_transmittance   = properties.get<Texture>("transmittance");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));

        float eta = m_ior->scalar(uv);
        bool same_hemisphere = Frame::sameHemisphere(wo, wi);

        Vector wm;
        // Half-vector creation
        if(same_hemisphere) wm = (wi + wo).normalized(); // Reflection
        else wm = -(wi + (wo * eta)).normalized(); // minus as per paper


        float D = microfacet::evaluateGGX(alpha, wm); // Microfacet normal distribution
        float G1_i = microfacet::smithG1(alpha, wm, wi);
        float G1_o = microfacet::smithG1(alpha, wm, wo);
        float F = fresnelDielectric(wi.dot(wm), eta);

        if(same_hemisphere){
            return { .value = m_reflectance->evaluate(uv) * (F * G1_i * G1_o * D * 0.25) / Frame::absCosTheta(wo), .pdf = 0.0f};
            // .pdf = clamp(microfacet::pdfGGXVNDF(alpha, wm, wo.normalized()) * microfacet::detReflection(wm, wo.normalized()), Epsilon, Infinity)}; // Eqn. 20 of paper
        }
        else {
            float numerator = abs(wi.dot(wm)) * abs(wo.dot(wm)) * sqr(eta) * (1 - F) * G1_i * G1_o * D;
            float denonimator = Frame::absCosTheta(wo) * sqr(wi.dot(wm) + (eta * wo.dot(wm)));
            return { .value = m_transmittance->evaluate(uv) * (numerator/denonimator)
             , .pdf = 0.0f}; // Eqn. 21 of paper
        }
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));

        Vector wm = microfacet::sampleGGXVNDF(alpha, wo, rng.next2D());
        
        float ior = m_ior->scalar(uv);
        bool entry_order = Frame::cosTheta(wo) > 0;
        float eta = entry_order ? ior : 1.0f / ior; 

        float F = fresnelDielectric(wo.dot(wm), eta);

        Vector wi;
        if(rng.next() < F) {
            wi = reflect(wo, wm);
            float G1_i = microfacet::smithG1(alpha, wm, wi);
            return {.wi = wi, .weight = m_reflectance->evaluate(uv) * G1_i, .pdf = F};
        }
        else {
            wi = refract(wo, wm, eta);
            float G1_i = microfacet::smithG1(alpha, wm, wi);
            return { .wi = wi, .weight = (m_transmittance->evaluate(uv) / sqr(eta)) * G1_i, .pdf = 1.0f - F};
        }
        
    }

    Color albedo(const Point2 &uv) const override {
        float ior = m_ior->scalar(uv);
        float F = fresnelDielectric(1.0f, ior);
        return m_reflectance->evaluate(uv) * F + m_transmittance->evaluate(uv) * (1.0f - F);
    }

    std::string toString() const override {
        return tfm::format(
            "RoughDielectric[\n"
            "  reflectance = %s,\n"
            "  roughness = %s\n"
            "  transmittance = %s,\n"
            "  ior = %s\n"
            "]",
            indent(m_reflectance),
            indent(m_roughness),
            indent(m_transmittance),
            indent(m_ior));
    }
};

}

REGISTER_BSDF(RoughDielectric, "roughdielectric")
