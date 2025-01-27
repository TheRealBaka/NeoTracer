#include <lightwave.hpp>

namespace lightwave {
class VolumeTracer : public SamplingIntegrator {

private:
    int m_depth;
    Color isVisible(Intersection &its, Ray &ray, Medium* &medium_type, DirectLightSample &dls, Sampler &rng) {
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

public:
    VolumeTracer(const Properties &properties)
        : SamplingIntegrator(properties){
            m_depth = properties.get("depth", 2);
    }

    Color Li(const Ray &ray, Sampler &rng) override {
        Ray primary_ray = Ray(ray);
        Color final_color = Color(0.0f);
        Color throughput = Color(1.0f);

        float sampled_dist = Infinity;
        Medium *medium_type = nullptr;

        for (int depth = 0; ; ++depth){
            // Primary ray intersection
            Intersection its = m_scene->intersect(primary_ray, rng);

            // Check for no intersection
            if(!its){  
                EmissionEval x = its.evaluateEmission();
                final_color += throughput * x.value;
                return final_color;
            }
            else {
                if (depth > 2) {
                    float russian_roulette_prob = std::min((throughput.r() + throughput.g() + throughput.b()) / 3.0f, 1.0f);
                    if (rng.next() >= russian_roulette_prob) break;
                    throughput /= russian_roulette_prob;
                }
            }

            Color medium_color = medium_type != nullptr ? medium_type->getColor() : Color(1.f);
            float medium_density = medium_type != nullptr ? medium_type->getDensity() : 1.f;
            final_color += throughput * its.evaluateEmission().value / medium_density;
            if(depth >= m_depth - 1) break;

            bool is_scattered = false;
            if(medium_type != nullptr) {
                float medium_throughput;
                sampled_dist = medium_type->sampleDistance(primary_ray, rng);
                if(its.t < sampled_dist){
                    medium_throughput = 1.0f; // medium_type->sampleAttenuation(its.t)
                }
                else {
                    is_scattered = true;
                    float pdf_dist = medium_type->getSigmaT() * medium_type->sampleAttenuation(sampled_dist);
                    medium_throughput = medium_type->sampleAttenuation(sampled_dist) * medium_type->getSigmaS() / pdf_dist;
                    float phase = medium_type->HGPhase(its.shadingFrame().toLocal(its.wo), squareToUniformSphere(rng.next2D()));
                    Intersection its_medium = Intersection(its.wo, sampled_dist);
                    its_medium.uv = Point2(0,0);
                    its_medium.position = primary_ray(sampled_dist);
                    if(m_scene->hasLights()){
                        // SampleLight function
                        LightSample light_sample = m_scene->sampleLight(rng);
                        if(!light_sample.isInvalid()){ // Removes segmentation fault
                            DirectLightSample dls = light_sample.light->sampleDirect(its_medium.position, rng, its); 
                            // Tracing secondary ray
                            Ray shadow_ray = Ray(its.position, dls.wi);

                            Color medium_transmittance = isVisible(its_medium, shadow_ray, medium_type, dls, rng);
                            if(medium_transmittance != Color(0.f)) {
                                // final_color += medium_transmittance * medium_throughput * phase * dls.weight * medium_type->getDensity();
                                final_color += medium_throughput * medium_color * medium_transmittance * phase;

                            }
                        }
                    }
                }
                throughput *= medium_throughput;
            }
            if(!is_scattered){
                if(m_scene->hasLights()){
                    // SampleLight function
                    LightSample light_sample = m_scene->sampleLight(rng);
                    if(!light_sample.isInvalid()){ // Removes segmentation fault
                        DirectLightSample dls = light_sample.light->sampleDirect(its.position, rng, its); 
                        // Tracing secondary ray
                        Ray shadow_ray = Ray(its.position, dls.wi);
                        Color bsdf_eval = its.evaluateBsdf(dls.wi).value;

                        bool occluded = m_scene->intersect(shadow_ray, dls.distance, rng);
                        if(!occluded) final_color += throughput * (1 / light_sample.probability) * dls.weight * bsdf_eval;
                        // Color medium_transmittance = isVisible(its, shadow_ray, medium_type, dls, rng);
                        // if(medium_transmittance != Color(0.f)) {
                        //     final_color += throughput * (1 / light_sample.probability) * dls.weight * bsdf_eval;
                        // }
                    }
                }
                // BsdfSample sample_ = its.sampleBsdf(rng);
                BsdfSample sample_ = its.sampleBsdf(rng);
                // Adding below if condition allows volume visibility. Can resolve in a more elegant way?
                if(!its.instance->bsdf()) {
                    sample_.wi = -its.wo;
                    sample_.weight = Color(1.0f);
                }

                bool inside_volume = Frame::cosTheta(its.shadingFrame().toLocal(sample_.wi)) < 0;
                bool enter_volume = Frame::cosTheta(its.shadingFrame().toLocal(its.wo)) > 0;

                if(inside_volume && enter_volume) medium_type = its.instance->medium();
                else if (!inside_volume && !enter_volume) medium_type = nullptr;

                if (sample_.weight == Color(0.0f))
                    break;
                primary_ray = Ray(its.position, sample_.wi.normalized());
                throughput *= sample_.weight;

                // bool is_entered = its.shadingFrame().toLocal(its.wo).dot(its.shadingFrame().normal) < 0;

                // if(is_entered) medium_type = its.instance->medium();
                // else medium_type = nullptr;
            }
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

REGISTER_INTEGRATOR(VolumeTracer, "volumetracer")