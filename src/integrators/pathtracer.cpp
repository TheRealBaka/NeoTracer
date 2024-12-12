#include <lightwave.hpp>

namespace lightwave {

/**
 * @brief Codebase for direct lighting integrator
 */

class PathTracer : public SamplingIntegrator {

private:
    int m_depth;

public:
    PathTracer(const Properties &properties)
        : SamplingIntegrator(properties){
            m_depth = properties.get("depth", 2);
    }

    Color Li(const Ray &ray, Sampler &rng) override {
        // Color weighted = Color(1.0f);
        Ray primary_ray = Ray(ray);
        Color final_color = Color(0.0f);
        Color throughput = Color(1.0f);
        for (int depth = 0; ; ++depth){
            // Primary ray intersection
            Intersection its = m_scene->intersect(primary_ray, rng);

            // Check for no intersection
            if(!its){  
                EmissionEval x = its.evaluateEmission();
                final_color += throughput * x.value;
                return final_color;
            }

            if(depth >= m_depth) break;

            // Add emission term
            final_color += throughput * its.evaluateEmission().value;

            // Direct Lighting
            if(m_scene->hasLights()){
                // SampleLight function
                LightSample light_sample = m_scene->sampleLight(rng);
                if(!light_sample.isInvalid()){ // Removes segmentation fault
                    DirectLightSample dls = light_sample.light->sampleDirect(its.position, rng); 
                    // Tracing secondary ray
                    Ray shadow_ray = Ray(its.position, dls.wi);
                    Color bsdf_eval = its.evaluateBsdf(dls.wi).value;

                    bool occluded = m_scene->intersect(shadow_ray, dls.distance, rng);
                    if(!occluded) final_color += throughput * (1 / light_sample.probability) * dls.weight * bsdf_eval;
                }
            }

            // Sample BSDF and get the new direction 
            BsdfSample sample_ = its.sampleBsdf(rng);
            if (sample_.weight == Color(0.0f))
                break;
            primary_ray = Ray(its.position, sample_.wi.normalized());
            throughput *= sample_.weight;
        }
        return final_color;
    }

    /// @brief An optional textual representation of this class, which can be
    /// useful for debugging.
    std::string toString() const override {
        return tfm::format("PathTracer[\n"
                           "  sampler = %s,\n"
                           "  image = %s,\n"
                           "]",
                           indent(m_sampler), indent(m_image));
    }


};

}

REGISTER_INTEGRATOR(PathTracer, "pathtracer")