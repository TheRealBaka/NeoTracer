/**
 * @file medium.hpp
 * @brief Contains the Medium interface and related structures.
 */

#pragma once

#include <lightwave/color.hpp>
#include <lightwave/core.hpp>
#include <lightwave/math.hpp>

namespace lightwave {

/// @brief A Medium, represents matter surrounding the volume (terminology taken from Mitsuba)
/// surfaces.
class Medium : public Object {
public:
    Medium(const Properties &properties) {}

    virtual Emission *emission() const = 0;

    /**
     * @brief Samples distance along ray segment
     *
     * @param ray Ray along which sampling takes place.
     */
    virtual float sampleDistance(Sampler &rng) const = 0;

    /**
     * @brief Returns sample attenuation for given scatting coefficients
     */
    virtual float sampleAttenuation(const float &its_t) const = 0;

    /**
     * @brief Computes transmittance along ray segment
     */
    virtual float evalTransmittance(const Ray &ray, const float &its_t, Sampler &rng) const = 0;

    /// @brief Implements Henyey-Greenstein phase function
    virtual float HGPhase(const Vector &wo, const Vector &wi) const = 0;

    virtual void sampleDirection(const Vector &wo, Sampler &sampler, Vector &wi) const = 0;

    /// @brief Flag to check homogeneous medium. Code for heterogeneous not implemented at the moment
    // virtual bool isHomogeneous() const = 0;

    /// @brief For homogeneous medium, return density coefficient
    virtual float getDensity() const = 0;

    /// @brief For homogeneous medium, return scattering coefficient
    virtual float getSigmaS() const = 0;

    /// @brief For homogeneous medium, return transmission coefficient
    virtual float getSigmaT() const = 0;

    /// @brief Return light color
    virtual Color getColor() const = 0;
};

} // namespace lightwave
