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
    if (m_id == 0) return;

    // filtering
    glSamplerParameteri(m_id, GL_TEXTURE_MIN_FILTER, sampler.minFilter);
    glSamplerParameteri(m_id, GL_TEXTURE_MAG_FILTER, sampler.magFilter);

    // wrapping
    glSamplerParameteri(m_id, GL_TEXTURE_WRAP_S, sampler.wrapS);
    glSamplerParameteri(m_id, GL_TEXTURE_WRAP_T, sampler.wrapT);
    glSamplerParameteri(m_id, GL_TEXTURE_WRAP_R, sampler.wrapR);

    // border color
    glSamplerParameterfv(m_id, GL_TEXTURE_BORDER_COLOR, sampler.borderColor);

    // level of detail (LOD)
    glSamplerParameterf(m_id, GL_TEXTURE_LOD_BIAS, sampler.lodBias);
    glSamplerParameterf(m_id, GL_TEXTURE_MIN_LOD, sampler.minLod);
    glSamplerParameterf(m_id, GL_TEXTURE_MAX_LOD, sampler.maxLod);

    if (sampler.compareMode != GL_NONE) {
        glSamplerParameteri(m_id, GL_TEXTURE_COMPARE_MODE, sampler.compareMode);
        glSamplerParameteri(m_id, GL_TEXTURE_COMPARE_FUNC, sampler.compareFunc);
    }
}

}  // namespace tinyrenderer