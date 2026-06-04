#pragma once

#include <glad/glad.h>

#include <stb_image/stb_image.h>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace tinyrenderer {

class Image {
   private:
    Image() = default;
    Image(void* data, GLenum type, int width, int height, int channels);

   public:
    Image(const Image& other)            = delete;
    Image& operator=(const Image& other) = delete;
    Image(Image&& other);
    Image& operator=(Image&& other);
    ~Image();

    void* getData() const { return m_data; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getChannels() const { return m_channels; }
    // Get CPU memory data type (how pixel data is stored in RAM)
    GLenum getDataType() const { return m_type; }
    // Get CPU memory channel order (how pixel data is arranged in RAM)
    GLenum getFormat() const {
        switch (m_channels) {
            case 1: return GL_R;
            case 2: return GL_RG;
            case 3: return GL_RGB;
            case 4: return GL_RGBA;
            default: return GL_RGBA;
        }
    }

    //  Create an image object from file, automatically detecting HDR/16-bit/8-bit formats
    static std::shared_ptr<Image> create(const std::filesystem::path& path, int desiredChannels = 0, bool flip = true);
    // Merge multiple image objects into a single image object by channel packing
    static std::shared_ptr<Image> merge(const std::vector<std::shared_ptr<Image>>& images, int channels);

   private:
    void* m_data   = nullptr;
    GLenum m_type  = GL_UNSIGNED_BYTE;
    int m_width    = 0;
    int m_height   = 0;
    int m_channels = 0;
};

// class ImageLoader {
//    public:
//     static ImageLoader& getInstance() {
//         static ImageLoader m_instance;
//         return m_instance;
//     }
//     std::shared_ptr<Image> load(const std::string& path) {
//         std::shared_ptr<Image> image = nullptr;
//         if (m_cache.count(path)) {
//             if (image = m_cache[path].lock()) {
//                 return image;
//             } else {
//                 m_cache.erase(path);
//             }
//         }
//         if (image == nullptr) {
//             image         = Image::create(path);
//             m_cache[path] = image;
//         }
//         return image;
//     }

//    private:
//     ImageLoader()  = default;
//     ~ImageLoader() = default;

//     std::unordered_map<std::string, std::weak_ptr<Image>> m_cache;
// };

}  // namespace tinyrenderer