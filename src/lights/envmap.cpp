    #include <lightwave.hpp>

#include <vector>

namespace lightwave {

class EnvironmentMap final : public BackgroundLight {
    /// @brief The texture to use as background
    ref<Texture> m_texture;
    /// @brief An optional transform from local-to-world space
    ref<Transform> m_transform;

    /// @brief Height of the Environment Map, if it exists.
    int m_height = 0;
    /// @brief Width of the Environment Map, if it exists.
    int m_width = 0;
    /// @brief Defines whether an Environment Map was provided. Defaults to false
    bool providedEnvironmentMap = false;
    struct Distribution2D
    {
        std::vector<float> func;
        std::vector<float> cdf;
        int count;
    };
    Distribution2D m_rowDistribution;
    std::vector<Distribution2D> m_colDistributions;
    void computeCumulativeDistribution(Distribution2D &dist)
    {
        dist.cdf[0] = 0;
        for (size_t i = 1; i < dist.cdf.size(); ++i)
        {
            dist.cdf[i] = dist.cdf[i - 1] + dist.func[i - 1] / dist.func.size();
        }
    }
    float sampleContinuousDistribution(const Distribution2D &dist, float sample, float &pdf) const
    {
        auto it = std::lower_bound(dist.cdf.begin(), dist.cdf.end(), sample);
        int offset = std::max(0, static_cast<int>(it - dist.cdf.begin() - 1));
        float du = sample - dist.cdf[offset];
        if ((dist.cdf[offset + 1] - dist.cdf[offset]) > 0)
        {
            du /= (dist.cdf[offset + 1] - dist.cdf[offset]);
        }
        pdf = dist.func[offset] / (dist.func.size() * (dist.cdf.back() - dist.cdf.front()));
        return (offset + du) / dist.func.size();
    }
    /**
     * @brief Initializes the 2D sampling distribution from the environment map.
     *
     * This function creates a 2D sampling distribution for the environment map on luminance values, taking into
     * account the sine of the latitude to handle distortion at the poles. This distribution is then used for
     * importance sampling the environment map.
     */
    void initializeDistribution2D()
    {
        std::unique_ptr<float[]> imageLuminance(new float[m_width * m_height]);
        // Compute luminance for each pixel and row marginal CDF
        m_rowDistribution.func.resize(m_height);
        m_rowDistribution.cdf.resize(m_width + 1);
        for (int y = 0; y < m_height; ++y)
        {
            float rowSum = 0;
            // float sinTheta = sin(Pi * static_cast<float>(y + 0.5f) / (float)m_height);
            for (int x = 0; x < m_width; ++x)
            {
                Point2 uv((x + 0.5f) / m_width, (y + 0.5f) / m_height);
                Color c = m_texture->evaluate(uv);
                imageLuminance[y * m_width + x] = c.luminance();
                rowSum += c.luminance();
            }
            m_rowDistribution.func[y] = rowSum;
        }
        computeCumulativeDistribution(m_rowDistribution);
        // Compute column CDFs for each row
        m_colDistributions.resize(m_height);
        for (int y = 0; y < m_height; ++y)
        {
            m_colDistributions[y].func.resize(m_width);
            m_colDistributions[y].cdf.resize(m_width + 1);
            for (int x = 0; x < m_width; ++x)
            {
                m_colDistributions[y].func[x] = imageLuminance[y * m_width + x];
            }
            computeCumulativeDistribution(m_colDistributions[y]);
        }
    }
    /// @brief Convert (u, v) to an (x, y, z) direction vector. It is the opposite conversion done in "evaluate".
    inline std::pair<Vector, float> uvToDirection(const Point2 &uv, float mapPdf) const
    {
        // Convert uv back to spherical coordinates
        float theta = uv.y() * Pi;
        float phi = Pi * (1 - 2 * uv.x());
        float sinTheta = sin(theta);
        // Convert spherical coordinates back to 3D Cartesian coordinates
        Vector direction(sinTheta * cos(phi), cos(theta), sinTheta * sin(phi));
        // Compute Pdf for sampled infinite light direction and convert it to solid-angle measure
        float pdf = (sinTheta == 0) ? 0.0f : InvPi * Inv2Pi * mapPdf / sinTheta;
        // Conversion of the pdf from area measure (in uv space) to solid angle measure
        return {direction.normalized(), pdf};
    }

public:
    EnvironmentMap(const Properties &properties) : BackgroundLight(properties) {
        m_texture   = properties.getChild<Texture>();
        m_transform = properties.getOptionalChild<Transform>();
        // providedEnvironmentMap = false;
    }

    EmissionEval evaluate(const Vector &direction) const override {
        Point2 warped = Point2(0);
        // hints:
        // * if (m_transform) { transform direction vector from world to local
        // coordinates }    
        // * find the corresponding pixel coordinate for the given local
        // direction
        // * make use of std::atan2 instead of tangent function.
        // * check out the safe versions of sine and cosine, e.g. safe_acos
        // in math.hpp to avoid problematic edge cases
        Vector dir_temp;
        if (m_transform)
            dir_temp = m_transform->inverse(direction);

        dir_temp = dir_temp.normalized();
        float phi = std::atan2(dir_temp[2], dir_temp[0]);
        float theta = safe_acos(dir_temp[1]);

        float t_x = (Pi - phi) * Inv2Pi;
        float t_y = theta * InvPi; 
        warped = Point2(t_x, t_y);
        return {
            .value = m_texture->evaluate(warped),
        };
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        Point2 warped    = rng.next2D();
        Vector direction = squareToUniformSphere(warped);
        auto E           = evaluate(direction);
        // implement better importance sampling here, if you ever need it
        // (useful for environment maps with bright tiny light sources, like the
        // sun for example)
        return {
            .wi = direction,
            .weight = E.value * Inv4Pi,
            .distance = Infinity,
            .pdf = Infinity
        };
    }

    std::string toString() const override {
        return tfm::format(
            "EnvironmentMap[\n"
            "  texture = %s,\n"
            "  transform = %s\n"
            "]",
            indent(m_texture),
            indent(m_transform));
    }
};

} // namespace lightwave

REGISTER_LIGHT(EnvironmentMap, "envmap")
