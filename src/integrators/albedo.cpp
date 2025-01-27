#include <lightwave.hpp>

namespace lightwave {

/**
 * @brief Codebase for albedo integrator
 */
class AlbedoIntegrator : public SamplingIntegrator {

public:
    AlbedoIntegrator(const Properties &properties)
        : SamplingIntegrator(properties) {}

    /**
     * @brief The job of an integrator is to return a color for a ray produced
     * by the camera model. This will be run for each pixel of the image,
     * potentially with multiple samples for each pixel.
     */
    Color Li(const Ray &ray, Sampler &rng) override {
        Ray current_ray = ray;

        while (true) {
            Intersection its = m_scene->intersect(current_ray, rng);

            // If no intersection, return black
            if (!its) {
                return Color(0.f);
            }

            // If the intersection is with a volume, skip and continue tracing
            if (its.instance->medium() != nullptr) {
                current_ray = Ray(its.position, current_ray.direction);
                continue;
            }

            // If there's no BSDF at the intersection, return black
            if (!its.instance->bsdf()) {
                return Color(0.f);
            }

            // Return the albedo color for the valid surface intersection
            return its.instance->bsdf()->albedo(its.uv);
        }
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
