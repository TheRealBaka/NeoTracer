#include <lightwave.hpp>

namespace lightwave {

/**
 * @brief Codebase for path tracer
 */

class PathTracer : public SamplingIntegrator {

private:
    int m_depth;
    bool m_mis;

    float BalancedHeuristic(float pdf_a, float pdf_b)
        {
            pdf_a = clamp(pdf_a, Epsilon, Infinity);
            pdf_b = clamp(pdf_b, Epsilon, Infinity);

            if (pdf_a == Infinity)
                return 1.0f;
            else if (pdf_b == Infinity)
                return 0.0f;
            else
            {
                float weight_a = pdf_a;
                float weight_b = pdf_b;
                return weight_a / (weight_a + weight_b);
            }
        }

public:
    PathTracer(const Properties &properties)
        : SamplingIntegrator(properties){
            m_depth = properties.get("depth", 2);
            m_mis = properties.get("mis", false);
    }

    Color Li(const Ray &ray, Sampler &rng) override {
        Ray primary_ray = Ray(ray);
        Color final_color = Color(0.0f);
        Color throughput = Color(1.0f);
        float p_bsdf = Infinity, p_light = 0.0f;
        float lightSelectionProb = m_scene->lightSelectionProbability(nullptr);

        for (int depth = 0; ; ++depth){
            // Primary ray intersection
            Intersection its = m_scene->intersect(primary_ray, rng);

            // Check for no intersection
            if(!its){  
                EmissionEval x = its.evaluateEmission();
                // if (depth == 0 && its.background == nullptr)
                //     return final_color;
                final_color += throughput * x.value;
                return final_color;
            }

            // Add emission term
            // final_color += throughput * its.evaluateEmission().value;

            if (depth == 0 || its.instance->light() == nullptr)
                final_color += throughput * its.evaluateEmission().value;
            else if (m_mis){
                p_light = GetSolidAngle(its.pdf, its.t, its.shadingFrame().normal, its.wo) * lightSelectionProb;
                float mis_weight = BalancedHeuristic(p_bsdf, p_light);
                final_color += mis_weight * throughput * its.evaluateEmission().value;
            }

            if(depth >= m_depth - 1) break;

            // Direct Lighting
            if(m_scene->hasLights()){
                // SampleLight function
                LightSample light_sample = m_scene->sampleLight(rng);
                if(!light_sample.isInvalid()){ // Removes segmentation fault
                    DirectLightSample dls = light_sample.light->sampleDirect(its.position, rng, its); 
                    // Tracing secondary ray
                    Ray shadow_ray = Ray(its.position, dls.wi);
                    Color bsdf_eval = its.evaluateBsdf(dls.wi).value;
                    

                    bool occluded = m_scene->intersect(shadow_ray, dls.distance, rng);
                    if(!occluded){
                        float mis_weight = 1.0f;
                        if (m_mis){
                            p_light = dls.pdf * lightSelectionProb;
                            mis_weight = BalancedHeuristic(p_light, its.evaluateBsdf(dls.wi).pdf);
                            final_color += mis_weight * throughput * (1 / lightSelectionProb) * dls.weight * bsdf_eval;
                        }
                        else {
                            final_color += mis_weight * throughput * (1 / light_sample.probability) * dls.weight * bsdf_eval;
                        }
                    }
                }
            }

            // Sample BSDF and get the new direction 
            BsdfSample sample_ = its.sampleBsdf(rng);
            if (sample_.weight == Color(0.0f))
                break;
            primary_ray = Ray(its.position, sample_.wi.normalized());
            throughput *= sample_.weight;
            p_bsdf = sample_.pdf;
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