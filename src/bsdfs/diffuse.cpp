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
        // bsdf.value = m_albedo->evaluate(uv);
        return bsdf;
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        // NOT_IMPLEMENTED

        //naive implementation 

        // Vector wi = squareToUniformHemisphere(rng.next2D());
        // BsdfEval f = evaluate(uv, wo, wi);
        // return {.wi = wi.normalized(), .weight = f.value * (1 / uniformHemispherePdf())};

        // Cosine Weighted Hemisphere 

        Vector wi = squareToCosineHemisphere(rng.next2D());
        // BsdfEval f = evaluate(uv, wo, wi);
        // float wi_pdf = cosineHemispherePdf(wi);
        // if(wi_pdf <= 0 || !Frame::sameHemisphere(wi, wo)) // Linux machine returning nan on select pixels. Explicit handling seems to fix that
        if(!Frame::sameHemisphere(wi, wo))
            return {.wi = wi, .weight = Color(0.0f), .pdf = cosineHemispherePdf(wi.normalized())};
        else
        // return {.wi = wi, .weight = f.value * 1 / wi_pdf};
            return {.wi = wi, .weight = m_albedo->evaluate(uv), .pdf = cosineHemispherePdf(wi.normalized())};

    }

    Color albedo(const Point2 &uv) const override {
        return m_albedo->evaluate(uv);
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
