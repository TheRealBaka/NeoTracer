#include <lightwave.hpp>

namespace lightwave {

/**
 * @brief Codebase for direct lighting integrator
 */

class DirectIntegrator : public SamplingIntegrator {

public:
    DirectIntegrator(const Properties &properties)
        : SamplingIntegrator(properties){
    }

    Color Li(const Ray &ray, Sampler &rng) override {
        Color throughput = Color(1.0f);
        Intersection its = m_scene->intersect(ray, rng);

        if(!its){
            EmissionEval x = its.evaluateEmission();
            throughput = x.value;
            return throughput;
        }

        LightSample y = m_scene->sampleLight(rng);
        const Light* z = y.light;
        DirectLightSample w = z->sampleDirect(its.position, rng); 

        Ray shadow_ray = Ray(its.position, w.wi);
        throughput *= its.evaluateBsdf(ray.direction).value; 

        bool next = m_scene->intersect(shadow_ray, w.distance, rng);
        if(!next){
            // throughput = its.evaluateBsdf(ray.direction).value * w.weight;
            throughput *= w.weight;
        }

        return throughput;


    }

    /// @brief An optional textual representation of this class, which can be
    /// useful for debugging.
    std::string toString() const override {
        return tfm::format("DirectIntegrator[\n"
                           "  sampler = %s,\n"
                           "  image = %s,\n"
                           "]",
                           indent(m_sampler), indent(m_image));
    }


};

}

REGISTER_INTEGRATOR(DirectIntegrator, "direct")