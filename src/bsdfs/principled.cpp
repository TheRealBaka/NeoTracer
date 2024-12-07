#include <lightwave.hpp>

#include "fresnel.hpp"
#include "microfacet.hpp"

namespace lightwave {

struct DiffuseLobe {
    Color color;

    BsdfEval evaluate(const Vector &wo, const Vector &wi) const {
        // NOT_IMPLEMENTED
        return {color * InvPi * Frame::absCosTheta(wi.normalized())};

        // hints:
        // * copy your diffuse bsdf evaluate here
        // * you do not need to query a texture, the albedo is given by `color`
    }

    BsdfSample sample(const Vector &wo, Sampler &rng) const {
        // NOT_IMPLEMENTED
        Vector wi = squareToCosineHemisphere(rng.next2D()).normalized();
        return {.wi = wi, .weight = color};

        // hints:
        // * copy your diffuse bsdf evaluate here
        // * you do not need to query a texture, the albedo is given by `color`
    }
};

struct MetallicLobe {
    float alpha;
    Color color;

    BsdfEval evaluate(const Vector &wo, const Vector &wi) const {
        // NOT_IMPLEMENTED

        // NOT_IMPLEMENTED
        Vector wm = (wi + wo).normalized(); // Microfacet normal

        float D = microfacet::evaluateGGX(alpha, wm); // Microfacet normal distribution
        float G1_i = microfacet::smithG1(alpha, wm, wi);
        float G1_o = microfacet::smithG1(alpha, wm, wo);

        return { .value = color * (D * G1_i * G1_o * 0.25) / Frame::absCosTheta(wo)};

        // hints:
        // * copy your roughconductor bsdf evaluate here
        // * you do not need to query textures
        //   * the reflectance is given by `color'
        //   * the variable `alpha' is already provided for you
    }

    BsdfSample sample(const Vector &wo, Sampler &rng) const {
        // NOT_IMPLEMENTED
        Vector wm = microfacet::sampleGGXVNDF(alpha, wo, rng.next2D());
        Vector wi = reflect(wo, wm);

        // Terms canceled after substituting Jacobian with p(wm)
        return { .wi = wi, .weight = color * microfacet::smithG1(alpha, wm , wi)};

        // hints:
        // * copy your roughconductor bsdf sample here
        // * you do not need to query textures
        //   * the reflectance is given by `color'
        //   * the variable `alpha' is already provided for you
    }
};

class Principled : public Bsdf {
    ref<Texture> m_baseColor;
    ref<Texture> m_roughness;
    ref<Texture> m_metallic;
    ref<Texture> m_specular;

    struct Combination {
        float diffuseSelectionProb;
        DiffuseLobe diffuse;
        MetallicLobe metallic;
    };

    Combination combine(const Point2 &uv, const Vector &wo) const {
        const auto baseColor = m_baseColor->evaluate(uv);
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));
        const auto specular = m_specular->scalar(uv);
        const auto metallic = m_metallic->scalar(uv);
        const auto F =
            specular * schlick((1 - metallic) * 0.08f, Frame::cosTheta(wo));

        const DiffuseLobe diffuseLobe = {
            .color = (1 - F) * (1 - metallic) * baseColor,
        };
        const MetallicLobe metallicLobe = {
            .alpha = alpha,
            .color = F * Color(1) + (1 - F) * metallic * baseColor,
        };

        const auto diffuseAlbedo = diffuseLobe.color.mean();
        const auto totalAlbedo =
            diffuseLobe.color.mean() + metallicLobe.color.mean();
        return {
            .diffuseSelectionProb =
                totalAlbedo > 0 ? diffuseAlbedo / totalAlbedo : 1.0f,
            .diffuse  = diffuseLobe,
            .metallic = metallicLobe,
        };
    }

public:
    Principled(const Properties &properties) {
        m_baseColor = properties.get<Texture>("baseColor");
        m_roughness = properties.get<Texture>("roughness");
        m_metallic  = properties.get<Texture>("metallic");
        m_specular  = properties.get<Texture>("specular");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        PROFILE("Principled")

        const auto combination = combine(uv, wo);
        // NOT_IMPLEMENTED
        BsdfEval diffuse_val = combination.diffuse.evaluate(wo, wi);
        BsdfEval metallic_val = combination.metallic.evaluate(wo, wi);
        return { .value = diffuse_val.value + metallic_val.value};

        // hint: evaluate `combination.diffuse` and `combination.metallic` and
        // combine their results
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        PROFILE("Principled")

        const auto combination = combine(uv, wo);
        // NOT_IMPLEMENTED
        BsdfSample val;
        float select_prob;

        if (rng.next() < combination.diffuseSelectionProb){
            val = combination.diffuse.sample(wo, rng);
            select_prob = combination.diffuseSelectionProb;
        } else {
            val = combination.metallic.sample(wo, rng);
            select_prob = 1.0f - combination.diffuseSelectionProb;         
        }
        return { .wi = val.wi, .weight = val.weight / select_prob};  


        // hint: sample either `combination.diffuse` (probability
        // `combination.diffuseSelectionProb`) or `combination.metallic`
    }

    std::string toString() const override {
        return tfm::format("Principled[\n"
                           "  baseColor = %s,\n"
                           "  roughness = %s,\n"
                           "  metallic  = %s,\n"
                           "  specular  = %s,\n"
                           "]",
                           indent(m_baseColor), indent(m_roughness),
                           indent(m_metallic), indent(m_specular));
    }
};

} // namespace lightwave

REGISTER_BSDF(Principled, "principled")
