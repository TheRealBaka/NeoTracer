#include <lightwave.hpp>

namespace lightwave {  

/**
 * @brief Code implementation for image blooming
 *
 * Adapted from https://github.dev/mmp/pbrt-v4 `pbrt/cmd/imgtool.cpp`
 */
class ImageBloom : public Postprocess {

private:
    int m_width;
    float m_iterations;
    float m_threshold;
    float m_scale;
    float m_sigma;

    std::vector<float> GaussianKernel(int width, int sigma) {
        const int kernel_size = width * 2 + 1; // odd kernel size
        // using Vector3i = TVector<int, 3>;

        std::vector<float> kernel(sqr(kernel_size));

        // Create Gaussian values
        float sum = 0.0f;
        for (int i = -width; i <= width; i++){
            for(int j = -width; j <= width; j++) {
                float value = exp(-(sqr(i) + sqr(j)) / (2 * sqr(sigma)));
                kernel[(i + width) * kernel_size + (j + width)] = value;
                sum += value;
            }
        }

        // Normalize kernel
        for (int i = 0; i < sqr(kernel_size); i++) {
            kernel[i] /= sum;
        }

        return kernel;
    }

    Image GaussianBlur(const Point2i &res, const Image &image, const int width, float sigma) {
        const int kernel_size = width * 2 + 1;
        std::vector<float> kernel = GaussianKernel(width, sigma);

        Image result(res);

        for (int x = 0; x < res.x(); x++) {
            for (int y = 0; y < res.y(); y++) {
                Color total;
                for (int i = -width; i <= width; i++) {
                    for (int j = -width; j <= width; j++) {
                        // Clamp x and y values
                        int xc = max(0,min(x+i,res.x() - 1));
                        int yc = max(0,min(y+j,res.y() - 1));
                        float weight = kernel[(i + width) * kernel_size + (j + width)];
                        total += image.get(Point2i(xc, yc)) * weight; // Kernel "convolution"
                    }
                }
                result.get(Point2i(x, y)) = total;
            }
        }

        return result;
    }

public:
    ImageBloom(const Properties &properties) : Postprocess(properties) {
        m_width = properties.get<int>("width", 5);
        m_iterations = properties.get<int>("iterations", 3);
        m_threshold = properties.get<float>("threshold", 1.f);
        m_scale = properties.get<float>("scale", 1.0f);
        m_sigma = m_width / 2.0f;
    }

    void execute() override {

        logger(EDebug, "Adding bloom effects...");
        
        const Point2i res = m_input->resolution();
        m_output->copy(*m_input); // Postprocess input image. Don't init canvas

        const int width = res.x();
        const int height = res.y();
        
        Image result;
        result.copy(*m_input);

        // First apply thresholding to source/input image
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                const Color rgb = result.get(Point2i(x, y));
                if(m_threshold < rgb.luminance()) result.get(Point2i(x, y)) *= 1.0f;
                else result.get(Point2i(x, y)) *= 0.0f;
            }
        }

        // Apply Gaussian blur to image
        for (int i = 0; i < m_iterations; i++) {
            result = GaussianBlur(res, result, m_width, m_sigma);
        }

        // Blend with image weighted with scale (convention from PBRT)
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                m_output->get(Point2i(x, y)) += result.get(Point2i(x, y)) * m_scale / m_iterations;
            }
        }
        
        m_output->save();

        logger(EDebug, "Bloomed successfully applied");

        Streaming stream { *m_output };
        stream.update();
    }

    std::string toString() const override { return "ImageBloom[]"; }
};

}
REGISTER_POSTPROCESS(ImageBloom, "bloom")