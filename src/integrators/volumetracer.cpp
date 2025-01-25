#include <lightwave.hpp>

namespace lightwave {
class VolumeTracer : public SamplingIntegrator {

private:
    int m_depth;

    Color DirectLightContribution(Intersection &its, Color &throughput, bool is_medium, Sampler &rng){
        Color final_color = Color(0.0f);
        if(m_scene->hasLights()){
            // SampleLight function
            LightSample light_sample = m_scene->sampleLight(rng);
            if(!light_sample.isInvalid()){ // Removes segmentation fault
                DirectLightSample dls = light_sample.light->sampleDirect(its.position, rng, its); 
                // Tracing secondary ray
                Ray shadow_ray = Ray(its.position, dls.wi);
                Color bsdf_eval = its.evaluateBsdf(dls.wi).value;

                if(is_medium){
                    Color ray_march = Color(1.0f);
                    float distance = dls.distance;
                    while(true){
                        Intersection hit_shadow = m_scene->intersect(shadow_ray, rng);

                        if(!hit_shadow || distance < hit_shadow.t || hit_shadow.instance->bsdf() != nullptr) break;
                        
                        ray_march *= hit_shadow.instance->medium()->sampleAttenuation(hit_shadow.t);
                        shadow_ray = Ray(hit_shadow.position, shadow_ray.direction);
                    }
                    final_color += ray_march * (1 / light_sample.probability) * dls.weight * bsdf_eval;
                } else {
                    final_color += throughput * (1 / light_sample.probability) * dls.weight * bsdf_eval;
                }
            }
        }
        return final_color;
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

        bool inside_medium = false;
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

            // Medium sampling strategy taken from Mitsuba
            if(medium_type != nullptr) sampled_dist = medium_type->sampleDistance(primary_ray, rng);
            else sampled_dist = Infinity;

            // Add emission term
            final_color += throughput * its.evaluateEmission().value;

            // Better fog observed when depth condition is checked after emission addition
            // if(depth >= m_depth - 1) break;

            if (sampled_dist < its.t) {
                inside_medium = true;
                float transmittance = medium_type->evalTransmittance(primary_ray, sampled_dist, rng);

                Intersection its_medium = Intersection(its.wo, sampled_dist);
                its_medium.position = primary_ray(sampled_dist);

                Color medium_contribution = DirectLightContribution(its_medium, throughput, inside_medium, rng);

                if(depth >= m_depth - 1) medium_contribution = Color(0.0f);

                final_color += throughput * medium_type->getColor() * transmittance * medium_type->getDensity() * medium_type->getSigmaS() * medium_type->HGPhase(Frame::cosTheta(its.shadingFrame().toLocal(its.wo)));
                // throughput *= medium_type->getColor() * transmittance * medium_type->getDensity() * medium_type->getSigmaS() * medium_type->HGPhase(Frame::cosTheta(its.shadingFrame().toLocal(its.wo)));

                final_color += throughput * medium_contribution;

                primary_ray = Ray(its_medium.position, its_medium.sampleBsdf(rng).wi.normalized());
            }
            else {
                inside_medium = false;
                Color surface_contribution = Color(0.0f);
                if(its.instance->bsdf() != nullptr){
                    surface_contribution = DirectLightContribution(its, throughput, inside_medium, rng);
                }

                if(depth >= m_depth - 1) surface_contribution = Color(0.0f);

                final_color += throughput * surface_contribution;

                BsdfSample sample_;
                // Adding below if condition allows volume visibility. Can resolve in a more elegant way?
                if(!its.instance->bsdf()) {
                    sample_.wi = -its.wo;
                    sample_.weight = Color(1.0f);
                }
                else sample_ = its.sampleBsdf(rng);

                bool inside_volume = Frame::cosTheta(its.shadingFrame().toLocal(sample_.wi)) < 0;
                bool enter_volume = Frame::cosTheta(its.shadingFrame().toLocal(its.wo)) > 0;

                if(inside_volume && enter_volume) medium_type = its.instance->medium();
                else if (!inside_medium && !enter_volume) medium_type = nullptr;

                primary_ray = Ray(its.position, sample_.wi.normalized());
                throughput *= sample_.weight;
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