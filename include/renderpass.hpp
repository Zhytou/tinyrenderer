#pragma once
#include <string>
#include <vector>

#include "framebuffer.hpp"

namespace tinyrenderer {

enum class LoadOp {
    LOAD_OP_LOAD,
    LOAD_OP_CLEAR,
    LOAD_OP_DONT_CARE
};

enum class StoreOp {
    STORE_OP_STORE,
    STORE_OP_DONT_CARE
};

union ClearValue {
};

struct AttachmentDesc {
    std::string name;
    GLenum slot     = GL_COLOR_ATTACHMENT0;  // attachment slot
    GLenum type     = GL_TEXTURE_2D;         // texture type
    GLenum format   = GL_RGBA8;              // internal format
    LoadOp loadOp   = LoadOp::LOAD_OP_CLEAR;
    StoreOp storeOp = StoreOp::STORE_OP_STORE;
    // ClearValue clearValue = ;
};

/**
 * @brief Execution blueprint defining how attachments are managed across a rendering phase.
 * @note Decouples layout intent (Load/Store operations) from actual hardware memory.
 * Provides structural layout compatibility matching modern API pass architectures.
 */
struct RenderPass {
    std::vector<AttachmentDesc> attachments;

    void begin(const std::shared_ptr<FrameBuffer>& framebuffer) {
        framebuffer->bind();
        for (auto attachment : attachments) {
            if (attachment.loadOp == LoadOp::LOAD_OP_CLEAR) {
                framebuffer->clear(attachment.slot);
            }
        }
    }
    void end() {
    }
};
};  // namespace tinyrenderer