#include "utils.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace tinyglrenderer {

std::ostream& operator<<(std::ostream& stream, const glm::vec3& vec) {
    stream << vec.x << ' ' << vec.y << ' ' << vec.z;
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const glm::vec4& vec) {
    stream << vec.x << ' ' << vec.y << ' ' << vec.z << ' ' << vec.w;
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const glm::mat4& mat) {
    stream << mat[0] << '\n'
           << mat[1] << '\n'
           << mat[2] << '\n'
           << mat[3] << '\n';

    return stream;
}

void glDebug(const char* msg) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cout << msg << " openGL error" << ": 0x" << std::hex << err << std::dec << std::endl;
    }
}

std::string glMacro2Str(GLenum value) {
    switch (value) {
        // data type
        case GL_UNSIGNED_BYTE:          return "GL_UNSIGNED_BYTE";
        case GL_BYTE:                   return "GL_BYTE";
        case GL_UNSIGNED_SHORT:         return "GL_UNSIGNED_SHORT";
        case GL_SHORT:                  return "GL_SHORT";
        case GL_UNSIGNED_INT:           return "GL_UNSIGNED_INT";
        case GL_INT:                    return "GL_INT";
        case GL_FLOAT:                  return "GL_FLOAT";
        case GL_HALF_FLOAT:             return "GL_HALF_FLOAT";
        case GL_DOUBLE:                 return "GL_DOUBLE";
        case GL_UNSIGNED_SHORT_5_6_5:   return "GL_UNSIGNED_SHORT_5_6_5";
        case GL_UNSIGNED_SHORT_4_4_4_4: return "GL_UNSIGNED_SHORT_4_4_4_4";
        case GL_UNSIGNED_SHORT_5_5_5_1: return "GL_UNSIGNED_SHORT_5_5_5_1";
        // format 
        case GL_RED:                    return "GL_RED";
        case GL_RG:                     return "GL_RG";
        case GL_RGB:                    return "GL_RGB";
        case GL_BGR:                    return "GL_BGR";
        case GL_RGBA:                   return "GL_RGBA";
        case GL_BGRA:                   return "GL_BGRA";
        case GL_DEPTH_COMPONENT:        return "GL_DEPTH_COMPONENT";
        case GL_STENCIL_INDEX:          return "GL_STENCIL_INDEX";
        case GL_DEPTH_STENCIL:          return "GL_DEPTH_STENCIL";
        // internel format
        case GL_R8:                     return "GL_R8";
        case GL_R16F:                   return "GL_R16F";
        case GL_R32F:                   return "GL_R32F";
        case GL_RG8:                    return "GL_RG8";
        case GL_RG16F:                  return "GL_RG16F";
        case GL_RG32F:                  return "GL_RG32F";
        case GL_RGB8:                   return "GL_RGB8";
        case GL_RGB16F:                 return "GL_RGB16F";
        case GL_RGB32F:                 return "GL_RGB32F";
        case GL_RGBA8:                  return "GL_RGBA8";
        case GL_RGBA16F:                return "GL_RGBA16F";
        case GL_RGBA32F:                return "GL_RGBA32F";
        case GL_DEPTH_COMPONENT16:      return "GL_DEPTH_COMPONENT16";
        case GL_DEPTH_COMPONENT24:      return "GL_DEPTH_COMPONENT24";
        case GL_DEPTH_COMPONENT32F:     return "GL_DEPTH_COMPONENT32F";
        case GL_DEPTH24_STENCIL8:       return "GL_DEPTH24_STENCIL8";
        // texture type
        case GL_TEXTURE_1D:                     return "GL_TEXTURE_1D";
        case GL_TEXTURE_2D:                     return "GL_TEXTURE_2D";
        case GL_TEXTURE_3D:                     return "GL_TEXTURE_3D";
        case GL_TEXTURE_CUBE_MAP:               return "GL_TEXTURE_CUBE_MAP";
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:    return "GL_TEXTURE_CUBE_MAP_POSITIVE_X";
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:    return "GL_TEXTURE_CUBE_MAP_NEGATIVE_X";
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:    return "GL_TEXTURE_CUBE_MAP_POSITIVE_Y";
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:    return "GL_TEXTURE_CUBE_MAP_NEGATIVE_Y";
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:    return "GL_TEXTURE_CUBE_MAP_POSITIVE_Z";
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:    return "GL_TEXTURE_CUBE_MAP_NEGATIVE_Z";
        case GL_TEXTURE_2D_ARRAY:               return "GL_TEXTURE_2D_ARRAY";
        default:                return "GL_UNKNOWN_ENUM_VALUE";
    }
}

}  // namespace tinyglrenderer
