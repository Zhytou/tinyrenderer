#include <glad/glad.h>

#include <iostream>

namespace tinyrenderer {

/**
 * @brief Shared GPU data interface block mapped to std140 layout structures.
 * @note Standardizes global context streaming (e.g., ViewMatrices, LightConfigs)
 * across distinct shaders without individual layout tracking overhead.
 */
class UniformBuffer {
   public:
    UniformBuffer(size_t size);
    ~UniformBuffer();
    UniformBuffer(const UniformBuffer&)            = delete;
    UniformBuffer& operator=(const UniformBuffer&) = delete;

    // Bind uniform buffer to shader binding point.
    void bind(uint32_t unit) const;
    // Upload uniform buffer data.
    void upload(const void* data);

   private:
    GLuint m_id   = 0;
    size_t m_size = 0;
};

}  // namespace tinyrenderer