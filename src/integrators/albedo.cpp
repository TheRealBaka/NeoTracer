#include <lightwave.hpp>

namespace lightwave {

/**
 * @brief Codebase for albedo integrator
 */
class AlbedoIntegrator : public SamplingIntegrator {

private:
    bool remap;

public:
    AlbedoIntegrator(const Properties &properties)
        : SamplingIntegrator(properties) {}

    /**
     * @brief The job of an integrator is to return a color for a ray produced
     * by the camera model. This will be run for each pixel of the image,
     * potentially with multiple samples for each pixel.
     */
    Color Li(const Ray &ray, Sampler &rng) override {
        Intersection its = m_scene->intersect(ray, rng);
        if(!its || !its.instance->bsdf()) return Color(0.f);
        return its.instance->bsdf()->albedo(its.uv);
    }

    /// @brief An optional textual representation of this class, which can be
    /// useful for debugging.
    std::string toString() const override {
        return tfm::format("AlbedoIntegrator[\n"
                           "  sampler = %s,\n"
                           "  image = %s,\n"
                           "]",
                           indent(m_sampler), indent(m_image));
    }
};

} // namespace lightwave

// this informs lightwave to use our class AlbedoIntegrator whenever a
// <integrator type="albedo" /> is found in a scene file
REGISTER_INTEGRATOR(AlbedoIntegrator, "albedo")
