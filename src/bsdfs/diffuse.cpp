#include <lightwave.hpp>

namespace lightwave {

class Diffuse : public Bsdf {
    ref<Texture> m_albedo;

public:
    Diffuse(const Properties &properties) {
        m_albedo = properties.get<Texture>("albedo");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        // NOT_IMPLEMENTED
        
        BsdfEval bsdf;
        bsdf.value = m_albedo->evaluate(uv) * InvPi * Frame::absCosTheta(wi.normalized());
        // bsdf.value = m_albedo->evaluate(uv) * InvPi;
        return bsdf;
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        // NOT_IMPLEMENTED

        //caution: directions probably reversed in this case

        //naive implementation 

        // Vector wi = squareToUniformHemisphere(uv);
        // BsdfEval f = evaluate(uv, wo, wi);
        // return {.wi = wi, .weight = f.value * uniformHemispherePdf()};

        Vector wi = squareToCosineHemisphere(uv);
        BsdfEval f = evaluate(uv, wi, wo);
        return {.wi = wi, .weight = f.value * cosineHemispherePdf(wi)};

    }

    std::string toString() const override {
        return tfm::format(
            "Diffuse[\n"
            "  albedo = %s\n"
            "]",
            indent(m_albedo));
    }
};

} // namespace lightwave

REGISTER_BSDF(Diffuse, "diffuse")
