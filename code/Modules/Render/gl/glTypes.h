#pragma once
//------------------------------------------------------------------------------
/**
    @class Oryol::Render::glTypes
    @brief OpenGL type conversion helpers
*/
#include "Core/Types.h"
#include "Render/Types/PixelFormat.h"
#include "Render/Types/PrimitiveType.h"
#include "Render/Types/TextureType.h"
#include "Render/Types/Usage.h"
#include "Render/Types/VertexFormat.h"
#include "Render/Types/TextureWrapMode.h"
#include "Render/Types/TextureFilterMode.h"
#include "Render/Types/IndexType.h"
#include "Render/Types/ShaderType.h"
#include "Render/gl/gl_decl.h"

namespace Oryol {
namespace Render {
    
class glTypes {
public:
    /// convert Oryol primitive type to GL primitive type
    static GLenum AsGLPrimitiveType(PrimitiveType::Code c);
    /// convert Oryol index type to GL index type
    static GLenum AsGLIndexType(IndexType::Code c);
    /// convert Oryol pixel format to glTexImage format (or internalFormat)
    static GLenum AsGLTexImageFormat(PixelFormat::Code c);
    /// convert Oryol pixel format to glTexImage type
    static GLenum AsGLTexImageType(PixelFormat::Code c);
    /// convert Oryol resource usage to GL usage
    static GLenum AsGLUsage(Usage::Code);
    /// convert Oryol texture wrap mode to GL enum
    static GLenum AsGLTextureWrapMode(TextureWrapMode::Code c);
    /// convert Oryol texture filter mode to GL enum
    static GLenum AsGLTextureFilterMode(TextureFilterMode::Code c);
    /// convert Oryol texture type to GL enum
    static GLenum AsGLTextureTarget(TextureType::Code c);
    /// convert Oryol shader type to GL enum
    static GLenum AsGLShaderType(ShaderType::Code c);
};
    
} // namespace Render
} // namespace Oryol