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
        if (!Frame::sameHemisphere(wi, wo))
            return BsdfEval::invalid();
        bsdf.value = m_albedo->evaluate(uv) * InvPi * Frame::absCosTheta(wi.normalized());
        // bsdf.value = m_albedo->evaluate(uv) * InvPi;
        return bsdf;
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        // NOT_IMPLEMENTED

        //naive implementation 

        Vector wi = squareToUniformHemisphere(rng.next2D());
        BsdfEval f = evaluate(uv, wo, wi);
        return {.wi = wi, .weight = f.value * (1 / uniformHemispherePdf())};

        //TODO: fix this
        // Vector wi = squareToCosineHemisphere(rng.next2D());
        // BsdfEval f = evaluate(uv, wi, wo);
        // return {.wi = wi, .weight = f.value * (1 / cosineHemispherePdf(wi))};

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
