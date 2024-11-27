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
        Color weighted = Color(1.0f);
        Color final_color = Color(0.0f);

        // (a) First scene intersection
        Intersection its = m_scene->intersect(ray, rng);

        // (b) check for no intersection
        if(!its){  
            EmissionEval x = its.evaluateEmission();
            return weighted*x.value;
        }

        // (c) SampleLight function
        LightSample light_sample = m_scene->sampleLight(rng);
        DirectLightSample dls = light_sample.light->sampleDirect(its.position, rng); 

        // (d) Tracing secondary ray
        Ray shadow_ray = Ray(its.position, dls.wi);
        Color bsdf_eval = its.evaluateBsdf(dls.wi).value;

        bool next = m_scene->intersect(shadow_ray, dls.distance, rng);
        if(!next) final_color += weighted*dls.weight*bsdf_eval * (1 / light_sample.probability);

        return final_color;


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