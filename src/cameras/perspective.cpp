#include <lightwave.hpp>

namespace lightwave {

/**
 * @brief A perspective camera with a given field of view angle and transform.
 *
 * In local coordinates (before applying m_transform), the camera looks in
 * positive z direction [0,0,1]. Pixels on the left side of the image ( @code
 * normalized.x < 0 @endcode ) are directed in negative x direction ( @code
 * ray.direction.x < 0 ), and pixels at the bottom of the image ( @code
 * normalized.y < 0 @endcode ) are directed in negative y direction ( @code
 * ray.direction.y < 0 ).
 */

// Warping technique considered from:
// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-generating-camera-rays/generating-camera-rays.html

class Perspective : public Camera {

private:
    float cam_fov, ar;
    std::string fovAxis;
    Vector fov_multiplier;

public:
    Perspective(const Properties &properties) : Camera(properties) {
        // NOT_IMPLEMENTED

        // hints:
        // * precompute any expensive operations here (most importantly
        // trigonometric functions)
        // * use m_resolution to find the aspect ratio of the image
        cam_fov = tan(properties.get<float>("fov") * Deg2Rad * 0.5f);
        ar = m_resolution.x() / (float) m_resolution.y();
        fovAxis = properties.get<std::string>("fovAxis");
        fov_multiplier = Vector(
            cam_fov * (fovAxis == "x" ? 1.0f: ar),
            cam_fov * (fovAxis == "x" ? 1.0f / ar : 1.0f),
            1.0f
        );

    }

    CameraSample sample(const Point2 &normalized, Sampler &rng) const override {
        // NOT_IMPLEMENTED

        // hints:
        // * use m_transform to transform the local camera coordinate system
        // into the world coordinate system

        // Assignment 0
        //     return CameraSample{
        //         .ray = Ray(Vector(normalized.x(), normalized.x(), 0.f),
        //         Vector(0.f, 0.f, 1.f)),
        //         .weight = Color(1.0f)};
        // }

        Vector warped_points = Vector(
            normalized.x(),
            normalized.y(),
            1.0f
        ) * fov_multiplier;
        
        return CameraSample{
            .ray = m_transform->apply(Ray(Vector(0.0f), warped_points.normalized())).normalized(),
            .weight = Color(1.0f)};
    }

    std::string toString() const override {
        return tfm::format("Perspective[\n"
                           "  width = %d,\n"
                           "  height = %d,\n"
                           "  transform = %s,\n"
                           "]",
                           m_resolution.x(), m_resolution.y(),
                           indent(m_transform));
    }
};

} // namespace lightwave

REGISTER_CAMERA(Perspective, "perspective")
