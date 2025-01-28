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

    float sampleDistance(Sampler &rng) const override {
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
        return m_sigmaS * m_density;
    }

    float getSigmaT() const override {
        return m_sigmaT;
    }

    float getDensity() const override {
        return m_density;
    }

    float HGPhase(const Vector &wo, const Vector &wi) const override {

        float cos_theta = -wo.dot(wi);

        float denom = 1 + sqr(m_hg) - 2 * m_hg * cos_theta;
        return Inv4Pi * (1 - sqr(m_hg)) / (denom * sqrtf(denom));

    }

    // inline Vector localToWorld(const Vector& v, const Vector& lx, const Vector& ly,
    //                       const Vector& lz)
    //     {
    //       Vector ret;
    //       for (int i = 0; i < 3; ++i) {
    //         ret[i] = v[0] * lx[i] + v[1] * ly[i] + v[2] * lz[i];
    //       }
    //       return ret;
    //     }

    void sampleDirection(const Vector &wo, Sampler &sampler, Vector &wi) const override {
        const Point2 u = sampler.next2D();
        float cos_theta;
        if(std::abs(m_hg) < 1e-3) {
            cos_theta = 1 - 2 * u.x();
        } else {
            const float sqr_term = (1 - sqr(m_hg)) / (1 - m_hg + 2 * m_hg * u.x());
            cos_theta = (1 + sqr(m_hg) - sqr(sqr_term)) / (2 * m_hg);
        }
        const float sin_theta = sqrt(max(1.0f - sqr(cos_theta), 0.0f));
        const float phi = 2 * Pi * u.y();
        const Vector wi_local = Vector(cos(phi) * sin_theta, cos_theta, sin(phi) * sin_theta);

        Vector t, b;
        buildOrthonormalBasis(-wo, t, b);

        // localToWorld
          for (int i = 0; i < 3; ++i) {
            wi[i] = wi_local[0] * t[i] + wi_local[1] * -wo[i] + wi_local[2] * b[i];
          }
        
        // wi = localToWorld(wi_local, t, -wo, b);
        // return HGPhase(wo, wi);
        return;
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