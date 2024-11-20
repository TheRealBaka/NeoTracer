#include <lightwave.hpp>

namespace lightwave {

class CheckerboardTexture : public Texture {
    Vector2 scale;
    Color color0;
    Color color1;

public:
    CheckerboardTexture(const Properties &properties) {
        scale = properties.get<Vector2>("scale");
        color0 = properties.get<Color>("color0", Color(0.0f));
        color1 = properties.get<Color>("color1", Color(1.0f));
    }

    Color evaluate(const Point2 &uv) const override { 
        Vector2i uv_scaled = Vector2i((int) floor(scale.x() * uv.x()), (int) floor(scale.y() * uv.y()));

        // Create checkerboard pattern
        int pattern = (uv_scaled.x() + uv_scaled.y()) % 2;

        return pattern == 0 ? color0 : color1;
    }

    std::string toString() const override {
        return tfm::format(
            "CheckerboardTexture[\n"
            "  scale = %s\n"
            "]",
            indent(scale));
    }
};

} // namespace lightwave

REGISTER_TEXTURE(CheckerboardTexture, "checkerboard")