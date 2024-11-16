#include <lightwave/core.hpp>
#include <lightwave/instance.hpp>
#include <lightwave/registry.hpp>
#include <lightwave/sampler.hpp>

namespace lightwave {

void Instance::transformFrame(SurfaceEvent &surf, const Vector &wo) const {

    // Create shading frame in object space

    Frame shading_frame = surf.shadingFrame();

    if (shading_frame.normal.dot(wo) < 0){
        shading_frame.bitangent *= -1;
        shading_frame.tangent *= -1;
        shading_frame.normal *= -1;
    }
    
    // Transform to world coordinate space
    surf.position = m_transform->apply(surf.position);
    shading_frame.normal = m_transform->applyNormal(shading_frame.normal).normalized();
    shading_frame.tangent = m_transform->apply(shading_frame.tangent).normalized();
    shading_frame.bitangent = m_transform->apply(shading_frame.bitangent).normalized();

    // Create orthonormal basis (updates bitangent)
    buildOrthonormalBasis(shading_frame.normal, shading_frame.tangent, shading_frame.bitangent);

    // Vectors already normalized. Need not normalize again
    Vector normal = shading_frame.tangent.cross(shading_frame.bitangent);
    
    surf.shadingNormal = normal;
    surf.geometryNormal = normal;
    surf.tangent = shading_frame.tangent;

    surf.pdf = 0.0f;
}

inline void validateIntersection(const Intersection &its) {
    // use the following macros to make debugginer easier:
    // * assert_condition(condition, { ... });
    // * assert_normalized(vector, { ... });
    // * assert_ortoghonal(vec1, vec2, { ... });
    // * assert_finite(value or vector or color, { ... });

    // each assert statement takes a block of code to execute when it fails
    // (useful for printing out variables to narrow done what failed)

    assert_finite(its.t, {
        logger(
            EError,
            "  your intersection produced a non-finite intersection distance");
        logger(EError, "  offending shape: %s", its.instance->shape());
    });
    assert_condition(its.t >= Epsilon, {
        logger(EError,
               "  your intersection is susceptible to self-intersections");
        logger(EError, "  offending shape: %s", its.instance->shape());
        logger(EError,
               "  returned t: %.3g (smaller than Epsilon = %.3g)",
               its.t,
               Epsilon);
    });
}

bool Instance::intersect(const Ray &worldRay, Intersection &its,
                         Sampler &rng) const {
    if (!m_transform) {
        // fast path, if no transform is needed
        const Ray localRay        = worldRay;
        const bool wasIntersected = m_shape->intersect(localRay, its, rng);
        if (wasIntersected) {
            its.instance = this;
            validateIntersection(its);
        }
        return wasIntersected;
    }

    const float previousT = its.t;
    Ray localRay;
    // NOT_IMPLEMENTED
    localRay = m_transform->inverse(worldRay);
    const float ray_length = localRay.direction.length();
    localRay = localRay.normalized();
    its.t = previousT * ray_length;

    // hints:
    // * transform the ray (do not forget to normalize!)
    // * how does its.t need to change?

    const bool wasIntersected = m_shape->intersect(localRay, its, rng);
    if (wasIntersected) {
        its.instance = this;
        validateIntersection(its);
        // hint: how does its.t need to change?
        its.t = its.t / ray_length;

        transformFrame(its, -localRay.direction);
    } else {
        its.t = previousT;
    }

    return wasIntersected;
}

Bounds Instance::getBoundingBox() const {
    if (!m_transform) {
        // fast path
        return m_shape->getBoundingBox();
    }

    const Bounds untransformedAABB = m_shape->getBoundingBox();
    if (untransformedAABB.isUnbounded()) {
        return Bounds::full();
    }

    Bounds result;
    for (int point = 0; point < 8; point++) {
        Point p = untransformedAABB.min();
        for (int dim = 0; dim < p.Dimension; dim++) {
            if ((point >> dim) & 1) {
                p[dim] = untransformedAABB.max()[dim];
            }
        }
        p = m_transform->apply(p);
        result.extend(p);
    }
    return result;
}

Point Instance::getCentroid() const {
    if (!m_transform) {
        // fast path
        return m_shape->getCentroid();
    }

    return m_transform->apply(m_shape->getCentroid());
}

AreaSample Instance::sampleArea(Sampler &rng) const {
    AreaSample sample = m_shape->sampleArea(rng);
    transformFrame(sample, Vector());
    return sample;
}

} // namespace lightwave

REGISTER_CLASS(Instance, "instance", "default")
