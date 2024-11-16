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
        if(!its && !remap) return Color(0.f); // return 0 for no intersection and no remap
        Vector normal = its ? its.shadingNormal : Vector(0.0f);
        Color color_nrml = remap ? (Color(normal) + Color(1)) / 2 : Color(0.5f);
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
