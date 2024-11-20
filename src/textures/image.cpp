#include <lightwave.hpp>

namespace lightwave {

class ImageTexture : public Texture {
    enum class BorderMode {
        Clamp,
        Repeat,
    };

    enum class FilterMode {
        Nearest,
        Bilinear,
    };

    ref<Image> m_image;
    float m_exposure;
    BorderMode m_border;
    FilterMode m_filter;

public:
    ImageTexture(const Properties &properties) {
        if (properties.has("filename")) {
            m_image = std::make_shared<Image>(properties);
        } else {
            m_image = properties.getChild<Image>();
        }
        m_exposure = properties.get<float>("exposure", 1);

        // clang-format off
        m_border = properties.getEnum<BorderMode>("border", BorderMode::Repeat, {
            { "clamp", BorderMode::Clamp },
            { "repeat", BorderMode::Repeat },
        });

        m_filter = properties.getEnum<FilterMode>("filter", FilterMode::Bilinear, {
            { "nearest", FilterMode::Nearest },
            { "bilinear", FilterMode::Bilinear },
        });
        // clang-format on
    }

    Point2 BorderCorrection(const Point2 &uv) const {
        Point2 uv_corrected = uv;
        if(m_border == BorderMode::Repeat){
            uv_corrected.x() = uv.x() - floor(uv.x()); // Value goes back to [0, 1] range
            uv_corrected.y() = uv.y() - floor(uv.y());
        }
        else{
            uv_corrected.x() = clamp(uv.x(), 0.0f, 1.0f);
            uv_corrected.y() = clamp(uv.y(), 0.0f, 1.0f);
        }
        uv_corrected.y() = 1.0f - uv_corrected.y();

        return uv_corrected;
    }

    Point2i BorderCorrectionImg(const Point2i &uv, const Point2i &res) const {
        if(m_border == BorderMode::Repeat) return Point2i((uv.x() % res.x() + res.x()) % res.x(), ((uv.y() % res.y()) + res.y()) % res.y());
        else return Point2i(clamp(uv.x(), 0, res.x() - 1), clamp(uv.y(), 0, res.y() - 1));
    }

    Color evaluate(const Point2 &uv) const override {
        // NOT_IMPLEMENTED
        Point2 uv_corrected = BorderCorrection(uv);
        Point2i res = m_image->resolution();
        
        if(m_filter == FilterMode::Nearest){
            Point2i uv_nearest = Point2i(floor(uv_corrected.x() * res.x()), floor(uv_corrected.y() * res.y()));
            return m_image->operator()(BorderCorrectionImg(uv_nearest, res))*m_exposure;
        }
        else{
            Point2 uv_centered = Point2(uv_corrected.x() * res.x() - 0.5f, uv_corrected.y() * res.y() - 0.5f);
            Point2i uv_centered_int = Point2i(floor(uv_centered.x()), floor(uv_centered.y())); // [x]
            Point2 uv_fractional = Point2(uv_centered.x() - uv_centered_int.x(), uv_centered.y() - uv_centered_int.y()); // {x} := x - [x]

            // Area made by rectangles within the point inside the pixel (with side lengths obtained accordingly)
            // Equivalent trilinear interpolation visualization for voxels (used as reference):
            // https://upload.wikimedia.org/wikipedia/commons/thumb/6/62/Trilinear_interpolation_visualisation.svg/800px-Trilinear_interpolation_visualisation.svg.png
            return (m_image->operator()(BorderCorrectionImg(uv_centered_int, res)) * (1 - uv_fractional.x()) * (1 - uv_fractional.y())
            + m_image->operator()(BorderCorrectionImg(Point2i(uv_centered_int.x() + 1, uv_centered_int.y()), res)) * uv_fractional.x() * (1 - uv_fractional.y())
            + m_image->operator()(BorderCorrectionImg(Point2i(uv_centered_int.x(), uv_centered_int.y() + 1), res)) * (1 - uv_fractional.x()) * uv_fractional.y()
            + m_image->operator()(BorderCorrectionImg(Point2i(uv_centered_int.x() + 1, uv_centered_int.y() + 1), res)) * uv_fractional.x() * uv_fractional.y())*m_exposure;
        }
    }

    std::string toString() const override {
        return tfm::format(
            "ImageTexture[\n"
            "  image = %s,\n"
            "  exposure = %f,\n"
            "]",
            indent(m_image),
            m_exposure);
    }
};

} // namespace lightwave

REGISTER_TEXTURE(ImageTexture, "image")