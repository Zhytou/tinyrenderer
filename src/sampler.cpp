#include "sampler.hpp"

namespace tinyrenderer {
Sampler::Sampler() {
    glGenSamplers(1, &m_id);
}

Sampler::Sampler(const SamplerDesc& sampler) {
    glGenSamplers(1, &m_id);
    set(sampler);
}

Sampler::Sampler(Sampler&& other) {
    if (m_id) {
        glDeleteSamplers(1, &m_id);
    }
    m_id       = other.m_id;
    other.m_id = 0;
}

Sampler& Sampler::operator=(Sampler&& other) {
    if (m_id) {
        glDeleteSamplers(1, &m_id);
    }
    m_id       = other.m_id;
    other.m_id = 0;
    return *this;
}

Sampler::~Sampler() {
    if (m_id) {
        glDeleteSamplers(1, &m_id);
    }
}

void Sampler::bind(uint32_t slot) const {
    glBindSampler(slot, m_id);
}

void Sampler::set(const SamplerDesc& sampler) {
    // glSamplerParameterfv(m_id, GL_TEXTURE_BORDER_COLOR, sampler.borderColor);
    // glSamplerParameterf(m_id, GL_TEXTURE_MAX_ANISOTROPY, sampler.maxAnisotropy);
    // glSamplerParameteri(m_id, GL_TEXTURE_COMPARE_MODE, sampler.compareMode);
    // glSamplerParameteri(m_id, GL_TEXTURE_COMPARE_FUNC, sampler.compareFunc);
}

}  // namespace tinyrenderer