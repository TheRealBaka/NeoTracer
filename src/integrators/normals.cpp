#include <lightwave.hpp>

namespace lightwave {

/**
 * @brief Codebase for normals integrator
 */
class NormalsIntegrator : public SamplingIntegrator {

private:
    bool remap;

public:
    NormalsIntegrator(const Properties &properties)
        : SamplingIntegrator(properties) {

        remap = properties.get<bool>("remap", true);
    }

    /**
     * @brief The job of an integrator is to return a color for a ray produced
     * by the camera model. This will be run for each pixel of the image,
     * potentially with multiple samples for each pixel.
     */
    Color Li(const Ray &ray, Sampler &rng) override {
        Intersection its = m_scene->intersect(ray, rng);
        Vector normal = its ? its.geometryNormal : Vector(0.0f); // Check with shadingNormal too?
        Color color_nrml = remap ? Color((normal + Vector(1.0f)) * 0.5f) : Color(normal);
        return color_nrml;
    }

    /// @brief An optional textual representation of this class, which can be
    /// useful for debugging.
    std::string toString() const override {
        return tfm::format("NormalsIntegrator[\n"
                           "  sampler = %s,\n"
                           "  image = %s,\n"
                           "]",
                           indent(m_sampler), indent(m_image));
    }
};

} // namespace lightwave

// this informs lightwave to use our class NormalsIntegrator whenever a
// <integrator type="normals" /> is found in a scene file
REGISTER_INTEGRATOR(NormalsIntegrator, "normals")
