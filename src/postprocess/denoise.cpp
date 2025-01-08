#include <lightwave.hpp>

// # ifdef LW_WITH_OIDN
#include <OpenImageDenoise/oidn.h>
#include <OpenImageDenoise/oidn.hpp>
#include <OpenImageDenoise/config.h>

namespace lightwave {

/**
 * @brief image.cpp taken as reference for image denoising.
 *
 * The implementation uses Intel's Open Image Denoise Library.
 * Refer: https://github.com/RenderKit/oidn for installation instructions
 * Current implementation only supports CPU based postprocessing
 */
class ImageDenoising : public Postprocess {
    /// @brief Normal image
    ref<Image> m_normals;
    /// @brief Albedo map
    ref<Image> m_albedo;
    /// @brief Device type used by OIDN
    oidn::DeviceRef m_device;
    /// @brief Float format for OIDN
    oidn::Format oidn_float;

public:
    ImageDenoising(const Properties &properties): Postprocess(properties) {
        m_normals = properties.get<Image>("normals", nullptr);
        m_albedo = properties.get<Image>("albedo", nullptr); 
        m_device = oidn::newDevice(oidn::DeviceType::CPU);
        m_device.commit(); // Commit all previous changes to device

        oidn_float = oidn::Format::Float3;
    }

    void execute() override {
        // TODO: If required, rewrite this function in a lucid way

        // ref<Image> image = std::make_shared<Image>();
        const Point2i res = m_input->resolution();
        m_output->initialize(res);

        const int width = res.x();
        const int height = res.y();
        // oidn::BufferRef colorBuf  = m_device.newBuffer(width * height * 3 * sizeof(float));
        // oidn::BufferRef albedoBuf = m_device.newBuffer(width * height * 3 * sizeof(float));

        // Initializing denoising filter
        oidn::FilterRef filter = m_device.newFilter("RT");

        filter.setImage("color",  m_input->data(),  oidn_float, width, height);
        filter.setImage("normal",  m_normals->data(),  oidn_float, width, height);
        filter.setImage("albedo",  m_albedo->data(),  oidn_float, width, height);
        filter.setImage("output",  m_output->data(),  oidn_float, width, height);
        filter.set("hdr", true);
        filter.commit();

        filter.execute();

        m_output->save();

        // Show image on tev
        Streaming stream { *m_output };
        stream.update();


    }

    std::string toString() const override { return "ImageDenoising[]"; }

};

}
REGISTER_POSTPROCESS(ImageDenoising, "denoising")

// #endif 