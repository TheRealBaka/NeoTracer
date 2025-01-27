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
        Ray current_ray = ray;

        while (true) {
            Intersection its = m_scene->intersect(current_ray, rng);

            // If no intersection, return black or default
            if (!its) {
                return remap ? Color(0.5f) : Color(0.f);
            }

            // If the intersection is with a volume, ignore and continue tracing
            if (its.instance->medium() != nullptr) {
                // Update the ray to continue past the volume
                current_ray = Ray(its.position, current_ray.direction);
                continue;
            }

            // Handle normal mapping for valid surface intersection
            Vector normal = its.shadingNormal;
            Color color_nrml = remap ? (Color(normal) + Color(1)) / 2 : Color(0.5f);
            return color_nrml;
        }
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