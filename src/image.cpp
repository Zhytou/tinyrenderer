#include "image.hpp"

#include <iostream>

namespace tinyrenderer {

Image::Image(void* data, GLenum type, int width, int height, int channels) {
    m_data     = data;
    m_type     = type;
    m_width    = width;
    m_height   = height;
    m_channels = channels;
}

Image::Image(Image&& other) {
    if (other.m_data) {
        stbi_image_free(other.m_data);
    }
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
    if (other.m_data) {
        stbi_image_free(other.m_data);
    }
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
        return nullptr;
    }

    // Convert to desired channels when desired channels is not 0. Otherwise, use image original channels.
    return std::shared_ptr<Image>(new Image(data, type, width, height, desiredChannels == 0 ? channels : desiredChannels));
}

std::shared_ptr<Image> Image::merge(const std::vector<std::shared_ptr<Image>>& images, int channels) {
    if (images.empty() || images[0] == nullptr) {
        return nullptr;
    }

    int width = images[0]->getWidth(), height = images[0]->getHeight(), totChannels = 0;
    GLenum type = GL_UNSIGNED_BYTE;
    for (auto& image : images) {
        if (image == nullptr || width != image->getWidth() || height != image->getHeight() || channels < image->getChannels()) {
            return nullptr;
        }
        if (image->getDataType() == GL_FLOAT) {
            type = GL_FLOAT;
        } else if (image->getDataType() == GL_UNSIGNED_SHORT && type == GL_UNSIGNED_BYTE) {
            type = GL_UNSIGNED_SHORT;
        }
        totChannels += image->getChannels();
    }
    if (totChannels > channels) {
        return nullptr;
    }

    int bytes = type == GL_FLOAT ? 4 : (type == GL_UNSIGNED_SHORT ? 2 : 1);
    //! NOTE: Must use STBI_MALLOC, since desturctor use STBI_FREE.
    // #define STBI_MALLOC malloc
    void* data = malloc(width * height * channels * bytes);
    if (data == nullptr) {
        return nullptr;
    }

    int dstChannel  = 0;  // current channel index of destination image
    int srcChannels = 0;  // number of source image channels
    if (type == GL_FLOAT) {
        float* dst = static_cast<float*>(data);
        std::fill_n(dst, width * height * channels, 1.0f);

        for (const auto& image : images) {
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

        for (const auto& image : images) {
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

        for (const auto& image : images) {
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
    return std::shared_ptr<Image>(new Image(data, type, width, height, channels));
}

}  // namespace tinyrenderer