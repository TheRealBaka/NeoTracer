#include <lightwave.hpp>

namespace lightwave {
class FogTracer : public SamplingIntegrator {

private:
    int m_depth;
    bool m_mis;
    Color isVisible(Ray &ray, Medium* &medium_type, DirectLightSample &dls, Sampler &rng) {
        // Medium-aware visibility check
        Color transmittance = Color(1.0f);
        Ray shadow_ray = ray;
        float distance = dls.distance;
        while(true){
            Intersection isect = m_scene->intersect(shadow_ray, rng);
            if(isect) {
                if (isect.instance->bsdf() != nullptr) {
                    return Color(0.f);
                }
                if(medium_type != nullptr) {
                    transmittance *= medium_type->evalTransmittance(shadow_ray, isect.t, rng);
                }
                distance -= (isect.position - shadow_ray.origin).length();
                shadow_ray = Ray(isect.position, shadow_ray.direction);
            }
            else {
                if(medium_type != nullptr){
                    transmittance *= medium_type->evalTransmittance(shadow_ray, distance, rng);
                }
                break;
            }
        }
        return transmittance;
    }

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
    FogTracer(const Properties &properties)
        : SamplingIntegrator(properties){
            m_depth = properties.get("depth", 2);
            m_mis = properties.get("mis", false);
    }

    Color Li(const Ray &ray, Sampler &rng) override {
        Ray primary_ray = Ray(ray);
        Color final_color = Color(0.0f);
        Color throughput = Color(1.0f);

        float sampled_dist = Infinity;
        Medium *medium_type = nullptr;
        int depth = 0;
        int volume_counter = 0;
        float medium_throughput;
        float p_bsdf = Infinity, p_light = 0.0f;
        float lightSelectionProb = m_scene->lightSelectionProbability(nullptr);

        // for (int depth = 0; ; ++depth){
        while(depth < m_depth){

            
            Intersection its = m_scene->intersect(primary_ray, rng);

            bool is_scattered = false;

            if(!its){  
                EmissionEval x = its.evaluateEmission();
                final_color += throughput * x.value;
                return final_color;
            } else {
                if (depth > 2) {
                    float russian_roulette_prob = std::min((throughput.r() + throughput.g() + throughput.b()) / 3.0f, 1.0f);
                    if (rng.next() >= russian_roulette_prob) break;
                    throughput /= russian_roulette_prob;
                }

            }

            if(its.instance->medium() != nullptr) {
                volume_counter++;
                // Two intersection check added
                if(volume_counter == 2) {
                    // update primary ray to advance through the volume;
                    primary_ray = Ray(its.position, primary_ray.direction);
                    Intersection new_its = m_scene->intersect(primary_ray, rng);
                    EmissionEval x = new_its.evaluateEmission();
                    final_color += throughput * x.value;
                    return final_color;
                }
                medium_type = its.instance->medium();
                primary_ray = Ray(its.position, primary_ray.direction);
                its = m_scene->intersect(primary_ray, rng);
            }

            if (depth == 0 || its.instance->light() == nullptr)
                final_color += throughput * its.evaluateEmission().value;
            else if(m_mis){
                p_light = GetSolidAngle(its.pdf, its.t, its.shadingFrame().normal, its.wo) * lightSelectionProb;
                float mis_weight = BalancedHeuristic(p_bsdf, p_light);
                final_color += mis_weight * throughput * its.evaluateEmission().value;
            }

            // Distance sampling
            if(medium_type != nullptr) sampled_dist = medium_type->sampleDistance(rng);
            else sampled_dist = Infinity;

            if(its.t < sampled_dist){
                medium_throughput = 1.0f; 
            }
            else{
                is_scattered = true;
                Vector new_dir;
                medium_type->sampleDirection(-primary_ray.direction, rng, new_dir);
                float pdf_dist = medium_type->getSigmaT() * medium_type->sampleAttenuation(sampled_dist);
                medium_throughput = medium_type->sampleAttenuation(sampled_dist) * medium_type->getSigmaS() / pdf_dist;
                // float phase = medium_type->HGPhase(its.shadingFrame().toLocal(its.wo), squareToUniformSphere(rng.next2D()));
                float phase = medium_type->HGPhase(-primary_ray.direction, new_dir);

                if(m_scene->hasLights()){
                    // SampleLight function
                    LightSample light_sample = m_scene->sampleLight(rng);
                    if(!light_sample.isInvalid()){ // Removes segmentation fault
                        DirectLightSample dls = light_sample.light->sampleDirect(primary_ray(sampled_dist), rng, its); 
                        // Tracing secondary ray
                        Ray shadow_ray = Ray(its.position, dls.wi);
                        Color medium_transmittance = isVisible(shadow_ray, medium_type, dls, rng);
                        float mis_weight = 1.0f;
                        if(medium_transmittance != Color(0.f)) {
                            // ask about medium color
                            if(m_mis){
                                p_light = dls.pdf * lightSelectionProb;
                                mis_weight = BalancedHeuristic(p_light, phase);
                            }
                            final_color += mis_weight * medium_transmittance * (1 / lightSelectionProb) * medium_throughput * phase * dls.weight * medium_type->getColor();
                            // final_color += medium_throughput * medium_color * medium_transmittance * phase;

                        }
                    }
                }
                // }
                // advance ray via in-scattering
                // ray.origin = ray(sampled_dist);
                // Vector new_dir;
                // medium_type->sampleDirection(-primary_ray.direction, rng, new_dir);
                // ray.direction = new_dir;
                primary_ray = Ray(Point(primary_ray(sampled_dist)), new_dir);

                // update throughput
                throughput *= medium_throughput;
            }

            bool is_reflected = false;
            if(!is_scattered){
                if(m_scene->hasLights()){
                    // SampleLight function
                    LightSample light_sample = m_scene->sampleLight(rng);
                    if(!light_sample.isInvalid()){ // Removes segmentation fault
                        DirectLightSample dls = light_sample.light->sampleDirect(its.position, rng, its); 
                        // Tracing secondary ray
                        Ray shadow_ray = Ray(its.position, dls.wi);
                        BsdfEval bsdf_eval = its.evaluateBsdf(dls.wi);
                        // p_bsdf = its.evaluateBsdf(dls.wi).pdf;

                        // bool occluded = m_scene->intersect(shadow_ray, dls.distance, rng);
                        // if(!occluded) final_color += throughput * (1 / light_sample.probability) * dls.weight * bsdf_eval;
                        Color medium_transmittance = isVisible(shadow_ray, medium_type, dls, rng);
                        float mis_weight = 1.0f; 
                        if(medium_transmittance != Color(0.f)) {
                            if (m_mis){
                                p_light = dls.pdf * lightSelectionProb;
                                mis_weight = BalancedHeuristic(p_light, bsdf_eval.pdf);
                            }
                            final_color += mis_weight * throughput * medium_transmittance * (1 / lightSelectionProb) * dls.weight * bsdf_eval.value;
                            is_reflected = true;
                        // }
                    }
                }
            }
            // Sample BSDF and get the new direction 
            BsdfSample sample_ = its.sampleBsdf(rng);
            // if(!its.instance->bsdf()) {
            //     sample_.wi = -its.wo;
            //     sample_.weight = Color(1.0f);
            // }

            if (sample_.weight == Color(0.0f))
                break;
            primary_ray = Ray(its.position, sample_.wi.normalized());
            throughput *= sample_.weight;
            p_bsdf = sample_.pdf;
        }
        if (is_scattered || is_reflected) { depth++; }
        }
        return final_color;
    }

    /// @brief An optional textual representation of this class, which can be
    /// useful for debugging.
    std::string toString() const override {
        return tfm::format("FogTracer[\n"
                           "  sampler = %s,\n"
                           "  image = %s,\n"
                           "]",
                           indent(m_sampler), indent(m_image));
    }


};

}

REGISTER_INTEGRATOR(FogTracer, "fogtracer")