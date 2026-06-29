#include "image.hpp"

#include <iostream>

namespace tinyglrenderer {

Image::Image(const std::string& filename, void* data, GLenum type, int width, int height, int channels) {
    m_filename = filename;
    m_data     = data;
    m_type     = type;
    m_width    = width;
    m_height   = height;
    m_channels = channels;
}

Image::Image(Image&& other) {
    if (m_data) {
        stbi_image_free(m_data);
    }
    m_filename       = std::move(other.m_filename);
    m_data           = other.m_data;
    m_type           = other.m_type;
    m_width          = other.m_width;
    m_height         = other.m_height;
    m_channels       = other.m_channels;
    other.m_data     = nullptr;
    other.m_type     = GL_UNSIGNED_BYTE;
    other.m_width    = 0;
    other.m_height   = 0;
    other.m_channels = 0;
}

Image& Image::operator=(Image&& other) {
    if (m_data) {
        stbi_image_free(m_data);
    }
    m_filename       = std::move(other.m_filename);
    m_data           = other.m_data;
    m_type           = other.m_type;
    m_width          = other.m_width;
    m_height         = other.m_height;
    m_channels       = other.m_channels;
    other.m_data     = nullptr;
    other.m_type     = GL_UNSIGNED_BYTE;
    other.m_width    = 0;
    other.m_height   = 0;
    other.m_channels = 0;
    return *this;
}

Image::~Image() {
    if (m_data) {
        stbi_image_free(m_data);
    }
}

std::shared_ptr<Image> Image::create(const std::filesystem::path& path, int desiredChannels, bool flip) {
    stbi_set_flip_vertically_on_load(flip);
    void* data  = nullptr;
    GLenum type = GL_UNSIGNED_BYTE;
    int width = 0, height = 0, channels = 0;

    if (!stbi_info(path.c_str(), &width, &height, &channels)) {
        const char* reason = stbi_failure_reason();
        // std::cerr << "[STBI ERROR] Invalid image or format not supported: " << path.string()
        //           << "\n  Reason: " << (reason ? reason : "Unknown") << std::endl;
        throw std::runtime_error("Image::create: stbi load failed " + path.string() + " (" + (reason ? reason : "unknown") + ")");
    }

    if (stbi_is_hdr(path.c_str())) {
        data = stbi_loadf(path.c_str(), &width, &height, &channels, desiredChannels);
        type = GL_FLOAT;
    } else if (stbi_is_16_bit(path.c_str())) {
        data = stbi_load_16(path.c_str(), &width, &height, &channels, desiredChannels);
        type = GL_UNSIGNED_SHORT;
    } else {
        data = stbi_load(path.c_str(), &width, &height, &channels, desiredChannels);
        type = GL_UNSIGNED_BYTE;
    }

    if (data == nullptr) {
        const char* reason = stbi_failure_reason();
        // std::cerr << "[STBI ERROR] Failed to load image: " << path.string()
        //           << "\n  Reason: " << (reason ? reason : "Unknown stbi error") << std::endl;
        throw std::runtime_error("Image::create: stbi load failed " + path.string() + " (" + (reason ? reason : "unknown") + ")");
    }

    // Convert to desired channels when desired channels is not 0. Otherwise, use image original channels.
    return std::shared_ptr<Image>(new Image(path.string(), data, type, width, height, desiredChannels == 0 ? channels : desiredChannels));
}

std::shared_ptr<Image> Image::merge(const std::vector<std::shared_ptr<Image>>& images, int channels) {
    if (images.empty() || images[0] == nullptr) {
        throw std::runtime_error("Image::merge: Empty image list or null image");
    }

    std::stringstream filename;
    std::vector<std::shared_ptr<Image>> resizedImages;
    int width = images[0]->getWidth(), height = images[0]->getHeight(), totChannels = 0;
    GLenum type = GL_UNSIGNED_BYTE;
    for (auto& image : images) {
        if (image == nullptr) {
            throw std::runtime_error("Image::merge: Null image");
        }
        filename << image->getFilename() << ", ";
        if (width != image->getWidth() || height != image->getHeight()) {
            resizedImages.push_back(resize(image, width, height));
        } else {
            resizedImages.push_back(image);
        }

        if (image->getDataType() == GL_FLOAT) {
            type = GL_FLOAT;
        } else if (image->getDataType() == GL_UNSIGNED_SHORT && type == GL_UNSIGNED_BYTE) {
            type = GL_UNSIGNED_SHORT;
        }
        totChannels += image->getChannels();
    }
    if (totChannels > channels) {
        throw std::runtime_error("Image::merge: Total channels must be less than or equal to desired channels");
    }

    int bytes = type == GL_FLOAT ? 4 : (type == GL_UNSIGNED_SHORT ? 2 : 1);
    //! WARNING: Must use STBI_MALLOC, since desturctor use STBI_FREE.
    // #define STBI_MALLOC malloc
    void* data = malloc(width * height * channels * bytes);
    if (data == nullptr) {
        throw std::runtime_error("Image::merge: Failed to allocate memory for merged image");
    }

    int dstChannel  = 0;  // current channel index of destination image
    int srcChannels = 0;  // number of source image channels
    if (type == GL_FLOAT) {
        float* dst = static_cast<float*>(data);
        std::fill_n(dst, width * height * channels, 1.0f);

        for (const auto& image : resizedImages) {
            float* src  = static_cast<float*>(image->getData());
            srcChannels = image->getChannels();

            for (int i = 0; i < width * height; ++i) {
                for (int c = 0; c < srcChannels; ++c) {
                    dst[i * channels + (dstChannel + c)] = src[i * srcChannels + c];
                }
            }
            dstChannel += srcChannels;
        }
    } else if (type == GL_UNSIGNED_SHORT) {
        uint16_t* dst = static_cast<uint16_t*>(data);
        std::fill_n(dst, width * height * channels, 65535);

        for (const auto& image : resizedImages) {
            uint16_t* src = static_cast<uint16_t*>(image->getData());
            srcChannels   = image->getChannels();

            for (int i = 0; i < width * height; ++i) {
                for (int c = 0; c < srcChannels; ++c) {
                    dst[i * channels + (dstChannel + c)] = src[i * srcChannels + c];
                }
            }
            dstChannel += srcChannels;
        }
    } else {  // GL_UNSIGNED_BYTE
        uint8_t* dst = static_cast<uint8_t*>(data);
        std::fill_n(dst, width * height * channels, 255);

        for (const auto& image : resizedImages) {
            uint8_t* src = static_cast<uint8_t*>(image->getData());
            srcChannels  = image->getChannels();

            for (int i = 0; i < width * height; ++i) {
                for (int c = 0; c < srcChannels; ++c) {
                    dst[i * channels + (dstChannel + c)] = src[i * srcChannels + c];
                }
            }
            dstChannel += srcChannels;
        }
    }
    return std::shared_ptr<Image>(new Image(filename.str(), data, type, width, height, channels));
}

std::shared_ptr<Image> Image::resize(const std::shared_ptr<Image>& image, int width, int height) {
    if (image == nullptr) {
        return nullptr;
    }

    auto filename= image->getFilename();
    int channels  = image->getChannels();
    GLenum type   = image->getDataType();
    GLenum format = image->getFormat();
    int bytes     = type == GL_FLOAT ? 4 : (type == GL_UNSIGNED_SHORT ? 2 : 1);
    void* data    = malloc(width * height * channels * bytes);  // destination image data
    if (data == nullptr) {
        return nullptr;
    }

    stbir_pixel_layout layout = STBIR_RGBA;
    if (format == GL_RGBA) {
        layout = STBIR_RGBA;
    } else if (format == GL_RGB) {
        layout = STBIR_RGB;
    } else if (format == GL_RG) {
        layout = STBIR_2CHANNEL;
    } else if (format == GL_R) {
        layout = STBIR_1CHANNEL;
    }

    void *src = image->getData(), *dst = data;
    int srcW = image->getWidth(), srcH = image->getHeight();
    int dstW = width, dstH = height;

    stbir_resize;
    if (type == GL_FLOAT) {
        stbir_resize_float_linear(static_cast<float*>(src), srcW, srcH, 0, static_cast<float*>(dst), dstW, dstH, 0, layout);
    } else if (type == GL_UNSIGNED_SHORT) {
        // stbir_resize_uint16_linear(static_cast<uint16_t*>(src), srcW, srcH, 0, static_cast<uint16_t*>(dst), dstW, dstH, 0, layout);
    } else {  // GL_UNSIGNED_BYTE
        stbir_resize_uint8_linear(static_cast<uint8_t*>(src), srcW, srcH, 0, static_cast<uint8_t*>(dst), dstW, dstH, 0, layout);
    }

    return std::shared_ptr<Image>(new Image(filename, data, type, width, height, channels));
}

}  // namespace tinyglrenderer