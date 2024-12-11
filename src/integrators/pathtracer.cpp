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
        Ray primary_ray = Ray(ray.origin, ray.direction, ray.depth);
        Color final_color = Color(0.0f);
        for (int depth = 0; depth < m_depth; depth++){
            // (a) First scene intersection
            Intersection its = m_scene->intersect(primary_ray, rng);

            // (b) check for no intersection
            if(!its){  
                EmissionEval x = its.evaluateEmission();
                return x.value;
            }

            final_color += its.evaluateEmission().value;

            if(depth == m_depth - 1) break;

        // int depth_counter = ray.depth;


        
        
        
        
        
            if(m_scene->hasLights()){
                // (c) SampleLight function
                LightSample light_sample = m_scene->sampleLight(rng);
                if(!light_sample.isInvalid()){ // Removes segmentation fault
                    DirectLightSample dls = light_sample.light->sampleDirect(its.position, rng); 
                    // (d) Tracing secondary ray
                    Ray shadow_ray = Ray(its.position, dls.wi);
                    Color bsdf_eval = its.evaluateBsdf(dls.wi).value;

                    bool occluded = m_scene->intersect(shadow_ray, dls.distance, rng);
                    if(!occluded) final_color += (1 / light_sample.probability) * dls.weight * bsdf_eval;
                }
            }

            if(its.evaluateEmission().value == Color(0.0f)){

                BsdfSample sample_ = its.sampleBsdf(rng);
                primary_ray = Ray(its.position, sample_.wi.normalized());
                // weighted *= sample_.weight;
                // Intersection next_its = m_scene->intersect(primary_ray, rng);
                // final_color += sample_.weight * next_its.evaluateEmission().value;
            }   
            else
                final_color += its.evaluateEmission().value;  
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