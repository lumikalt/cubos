#include <cassert>
#include <cstdlib>
#include <list>
#include <string>
#include <utility>

#include <glad/gl.h>

#include <cubos/core/log.hpp>

#include "ogl_render_device.hpp"

using namespace cubos::core;
using namespace cubos::core::gl;

#define LOG_GL_ERROR(err) CUBOS_ERROR("OpenGL error: {}", err)

// OpenGL message callback (errors, warnings, etc)
static void GLAPIENTRY messageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                       const GLchar* message, const void* userParam)
{
    (void)length;
    (void)userParam;
    (void)id;

    if (severity == GL_DEBUG_SEVERITY_HIGH)
    {
        CUBOS_ERROR("OpenGL error (source = 0x{:x}, type = 0x{:x}): {}", source, type, message);
    }
    else if (severity == GL_DEBUG_SEVERITY_MEDIUM)
    {
        CUBOS_WARN("OpenGL warning (source = 0x{:x}, type = 0x{:x}): {}", source, type, message);
    }
}

// Converts a texture format into the necessary GL parameters
static bool textureFormatToGL(TextureFormat texFormat, GLenum& internalFormat, GLenum& format, GLenum& type)
{
    switch (texFormat)
    {
    case TextureFormat::R8UNorm:
        internalFormat = GL_R8;
        format = GL_RED;
        type = GL_UNSIGNED_BYTE;
        break;
    case TextureFormat::R8SNorm:
        internalFormat = GL_R8_SNORM;
        format = GL_RED;
        type = GL_BYTE;
        break;
    case TextureFormat::R8UInt:
        internalFormat = GL_R8UI;
        format = GL_RED_INTEGER;
        type = GL_UNSIGNED_BYTE;
        break;
    case TextureFormat::R8SInt:
        internalFormat = GL_R8I;
        format = GL_RED_INTEGER;
        type = GL_BYTE;
        break;
    case TextureFormat::RG8UNorm:
        internalFormat = GL_RG8;
        format = GL_RG;
        type = GL_UNSIGNED_BYTE;
        break;
    case TextureFormat::RG8SNorm:
        internalFormat = GL_RG8_SNORM;
        format = GL_RG;
        type = GL_BYTE;
        break;
    case TextureFormat::RG8UInt:
        internalFormat = GL_RG8UI;
        format = GL_RG_INTEGER;
        type = GL_UNSIGNED_BYTE;
        break;
    case TextureFormat::RG8SInt:
        internalFormat = GL_RG8I;
        format = GL_RG_INTEGER;
        type = GL_BYTE;
        break;
    case TextureFormat::RGBA8UNorm:
        internalFormat = GL_RGBA;
        format = GL_RGBA;
        type = GL_UNSIGNED_BYTE;
        break;
    case TextureFormat::RGBA8SNorm:
        internalFormat = GL_RGBA8_SNORM;
        format = GL_RGBA;
        type = GL_BYTE;
        break;
    case TextureFormat::RGBA8UInt:
        internalFormat = GL_RGBA8UI;
        format = GL_RGBA_INTEGER;
        type = GL_UNSIGNED_BYTE;
        break;
    case TextureFormat::RGBA8SInt:
        internalFormat = GL_RGBA8I;
        format = GL_RGBA_INTEGER;
        type = GL_BYTE;
        break;
    case TextureFormat::R16UNorm:
        internalFormat = GL_R16;
        format = GL_RED;
        type = GL_UNSIGNED_SHORT;
        break;
    case TextureFormat::R16SNorm:
        internalFormat = GL_R16_SNORM;
        format = GL_RED;
        type = GL_SHORT;
        break;
    case TextureFormat::R16UInt:
        internalFormat = GL_R16UI;
        format = GL_RED_INTEGER;
        type = GL_UNSIGNED_SHORT;
        break;
    case TextureFormat::R16SInt:
        internalFormat = GL_R16I;
        format = GL_RED_INTEGER;
        type = GL_SHORT;
        break;
    case TextureFormat::RG16UNorm:
        internalFormat = GL_RG16;
        format = GL_RG;
        type = GL_UNSIGNED_SHORT;
        break;
    case TextureFormat::RG16SNorm:
        internalFormat = GL_RG16_SNORM;
        format = GL_RG;
        type = GL_SHORT;
        break;
    case TextureFormat::RG16UInt:
        internalFormat = GL_RG16UI;
        format = GL_RG_INTEGER;
        type = GL_UNSIGNED_SHORT;
        break;
    case TextureFormat::RG16SInt:
        internalFormat = GL_RG16I;
        format = GL_RG_INTEGER;
        type = GL_SHORT;
        break;
    case TextureFormat::RGBA16UNorm:
        internalFormat = GL_RGBA16;
        format = GL_RGBA;
        type = GL_UNSIGNED_SHORT;
        break;
    case TextureFormat::RGBA16SNorm:
        internalFormat = GL_RGBA16_SNORM;
        format = GL_RGBA;
        type = GL_SHORT;
        break;
    case TextureFormat::RGBA16UInt:
        internalFormat = GL_RGBA16UI;
        format = GL_RGBA_INTEGER;
        type = GL_UNSIGNED_SHORT;
        break;
    case TextureFormat::RGBA16SInt:
        internalFormat = GL_RGBA16I;
        format = GL_RGBA_INTEGER;
        type = GL_SHORT;
        break;
    case TextureFormat::R16Float:
        internalFormat = GL_R16F;
        format = GL_RED;
        type = GL_FLOAT;
        break;
    case TextureFormat::R32Float:
        internalFormat = GL_R32F;
        format = GL_RED;
        type = GL_FLOAT;
        break;
    case TextureFormat::RG16Float:
        internalFormat = GL_RG16F;
        format = GL_RG;
        type = GL_FLOAT;
        break;
    case TextureFormat::RG32Float:
        internalFormat = GL_RG32F;
        format = GL_RG;
        type = GL_FLOAT;
        break;
    case TextureFormat::RGB16Float:
        internalFormat = GL_RGBA16F;
        format = GL_RGB;
        type = GL_FLOAT;
        break;
    case TextureFormat::RGB32Float:
        internalFormat = GL_RGB32F;
        format = GL_RGB;
        type = GL_FLOAT;
        break;
    case TextureFormat::RGBA16Float:
        internalFormat = GL_RGBA16F;
        format = GL_RGBA;
        type = GL_FLOAT;
        break;
    case TextureFormat::RGBA32Float:
        internalFormat = GL_RGBA32F;
        format = GL_RGBA;
        type = GL_FLOAT;
        break;
    case TextureFormat::Depth16:
        internalFormat = GL_DEPTH_COMPONENT16;
        format = GL_DEPTH_COMPONENT;
        type = GL_FLOAT;
        break;
    case TextureFormat::Depth32:
        internalFormat = GL_DEPTH_COMPONENT32F;
        format = GL_DEPTH_COMPONENT;
        type = GL_FLOAT;
        break;
    case TextureFormat::Depth24Stencil8:
        internalFormat = GL_DEPTH24_STENCIL8;
        format = GL_DEPTH_STENCIL;
        type = GL_UNSIGNED_INT_24_8;
        break;
    case TextureFormat::Depth32Stencil8:
        internalFormat = GL_DEPTH32F_STENCIL8;
        format = GL_DEPTH_STENCIL;
        type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
        break;

    default:
        return false;
    }

    return true;
}

// Converts an addressing mode into the necessary GL parameters
static void addressToGL(AddressMode mode, GLenum& address)
{
    switch (mode)
    {
    case AddressMode::Repeat:
        address = GL_REPEAT;
        break;
    case AddressMode::Mirror:
        address = GL_MIRRORED_REPEAT;
        break;
    case AddressMode::Clamp:
        address = GL_CLAMP_TO_EDGE;
        break;
    case AddressMode::Border:
        address = GL_CLAMP_TO_BORDER;
        break;
    default:
        abort(); // Invalid addressing mode
    }
}

// Converts a cube face to a GL face
static void cubeFaceToGL(CubeFace cubeFace, GLenum& face)
{
    switch (cubeFace)
    {
    case CubeFace::PositiveX:
        face = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        break;
    case CubeFace::NegativeX:
        face = GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
        break;
    case CubeFace::PositiveY:
        face = GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
        break;
    case CubeFace::NegativeY:
        face = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
        break;
    case CubeFace::PositiveZ:
        face = GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
        break;
    case CubeFace::NegativeZ:
        face = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
        break;
    default:
        abort(); // Invalid enum
    }
}

// Converts a face to a GL face
static void faceToGL(Face face, GLenum& glFace)
{
    switch (face)
    {
    case Face::Front:
        glFace = GL_FRONT;
        break;
    case Face::Back:
        glFace = GL_BACK;
        break;
    case Face::FrontAndBack:
        glFace = GL_FRONT_AND_BACK;
        break;
    default:
        abort(); // Invalid enum
    }
}

// Converts a winding to a GL winding
static void windingToGL(Winding winding, GLenum& glWinding)
{
    switch (winding)
    {
    case Winding::CW:
        glWinding = GL_CW;
        break;
    case Winding::CCW:
        glWinding = GL_CCW;
        break;
    default:
        abort(); // Invalid enum
    }
}

// Converts a raster mode to the GL equivalent
static void rasterModeToGL(RasterMode rasterMode, GLenum& polygonMode)
{
    switch (rasterMode)
    {
    case RasterMode::Fill:
        polygonMode = GL_FILL;
        break;
    case RasterMode::Wireframe:
        polygonMode = GL_LINE;
        break;
    default:
        abort(); // Invalid enum
    }
}

// Converts a comparison function to the GL equivalent
static void compareToGL(Compare compare, GLenum& glCompare)
{
    switch (compare)
    {
    case Compare::Never:
        glCompare = GL_NEVER;
        break;
    case Compare::Less:
        glCompare = GL_LESS;
        break;
    case Compare::LEqual:
        glCompare = GL_LEQUAL;
        break;
    case Compare::Greater:
        glCompare = GL_GREATER;
        break;
    case Compare::GEqual:
        glCompare = GL_GEQUAL;
        break;
    case Compare::Equal:
        glCompare = GL_EQUAL;
        break;
    case Compare::NEqual:
        glCompare = GL_NOTEQUAL;
        break;
    case Compare::Always:
        glCompare = GL_ALWAYS;
        break;
    default:
        abort(); // Invalid enum
    }
}

// Converts a stencil action to the GL equivalent
static void stencilActionToGL(StencilAction action, GLenum& glAction)
{
    switch (action)
    {
    case StencilAction::Keep:
        glAction = GL_NEVER;
        break;
    case StencilAction::Zero:
        glAction = GL_ZERO;
        break;
    case StencilAction::Replace:
        glAction = GL_REPLACE;
        break;
    case StencilAction::Increment:
        glAction = GL_INCR;
        break;
    case StencilAction::IncrementWrap:
        glAction = GL_INCR_WRAP;
        break;
    case StencilAction::Decrement:
        glAction = GL_DECR;
        break;
    case StencilAction::DecrementWrap:
        glAction = GL_DECR_WRAP;
        break;
    case StencilAction::Invert:
        glAction = GL_INVERT;
        break;
    default:
        abort(); // Invalid enum
    }
}

// Converts a blend factor to the GL equivalent
static void blendFactorToGL(BlendFactor blendFactor, GLenum& glBlendFactor)
{
    switch (blendFactor)
    {
    case BlendFactor::Zero:
        glBlendFactor = GL_ZERO;
        break;
    case BlendFactor::One:
        glBlendFactor = GL_ONE;
        break;
    case BlendFactor::SrcColor:
        glBlendFactor = GL_SRC_COLOR;
        break;
    case BlendFactor::InvSrcColor:
        glBlendFactor = GL_ONE_MINUS_SRC_COLOR;
        break;
    case BlendFactor::DstColor:
        glBlendFactor = GL_DST_COLOR;
        break;
    case BlendFactor::InvDstColor:
        glBlendFactor = GL_ONE_MINUS_DST_COLOR;
        break;
    case BlendFactor::SrcAlpha:
        glBlendFactor = GL_SRC_ALPHA;
        break;
    case BlendFactor::InvSrcAlpha:
        glBlendFactor = GL_ONE_MINUS_SRC_ALPHA;
        break;
    case BlendFactor::DstAlpha:
        glBlendFactor = GL_DST_ALPHA;
        break;
    case BlendFactor::InvDstAlpha:
        glBlendFactor = GL_ONE_MINUS_DST_ALPHA;
        break;
    default:
        abort(); // Invalid enum
    }
}

// Converts a blend operation to the GL equivalent
static void blendOpToGL(BlendOp blendOp, GLenum& glBlendOp)
{
    switch (blendOp)
    {
    case BlendOp::Add:
        glBlendOp = GL_FUNC_ADD;
        break;
    case BlendOp::Substract:
        glBlendOp = GL_FUNC_SUBTRACT;
        break;
    case BlendOp::RevSubstract:
        glBlendOp = GL_FUNC_REVERSE_SUBTRACT;
        break;
    case BlendOp::Max:
        glBlendOp = GL_MAX;
        break;
    case BlendOp::Min:
        glBlendOp = GL_MIN;
        break;
    default:
        abort(); // Invalid enum
    }
}

class OGLFramebuffer : public impl::Framebuffer
{
public:
    OGLFramebuffer(GLuint id)
        : id(id)
    {
    }

    ~OGLFramebuffer() override
    {
        glDeleteFramebuffers(1, &this->id);
    }

    GLuint id;
};

class OGLRasterState : public impl::RasterState
{
public:
    OGLRasterState() = default;
    ~OGLRasterState() override = default;

    GLboolean cullEnabled;
    GLboolean scissorEnabled;
    GLenum frontFace;
    GLenum cullFace;
    GLenum polygonMode;
};

class OGLDepthStencilState : public impl::DepthStencilState
{
public:
    OGLDepthStencilState() = default;
    ~OGLDepthStencilState() override = default;

    GLboolean depthEnabled;
    GLboolean depthWriteEnabled;
    GLfloat depthNear;
    GLfloat depthFar;
    GLenum depthFunc;

    GLuint stencilRef;
    GLboolean stencilEnabled;
    GLuint stencilReadMask;
    GLuint stencilWriteMask;

    GLenum frontStencilFunc;
    GLenum frontFaceStencilFail;
    GLenum frontFaceStencilPass;
    GLenum frontFaceDepthFail;

    GLenum backStencilFunc;
    GLenum backFaceStencilFail;
    GLenum backFaceStencilPass;
    GLenum backFaceDepthFail;
};

class OGLBlendState : public impl::BlendState
{
public:
    OGLBlendState() = default;
    ~OGLBlendState() override = default;

    GLboolean blendEnabled;
    GLenum srcFactor;
    GLenum dstFactor;
    GLenum blendOp;
    GLenum srcAlphaFactor;
    GLenum dstAlphaFactor;
    GLenum alphaBlendOp;
};

class OGLSampler : public impl::Sampler
{
public:
    OGLSampler(GLuint id)
        : id(id)
    {
    }

    ~OGLSampler() override
    {
        glDeleteSamplers(1, &this->id);
    }

    GLuint id;
};

class OGLTexture1D : public impl::Texture1D
{
public:
    OGLTexture1D(GLuint id, GLenum internalFormat, GLenum format, GLenum type)
        : id(id)
        , internalFormat(internalFormat)
        , format(format)
        , type(type)
    {
    }

    ~OGLTexture1D() override
    {
        glDeleteTextures(1, &this->id);
    }

    void update(std::size_t x, std::size_t width, const void* data, std::size_t level) override
    {
        glBindTexture(GL_TEXTURE_1D, this->id);
        glTexSubImage1D(GL_TEXTURE_1D, static_cast<GLint>(level), static_cast<GLint>(x), static_cast<GLsizei>(width),
                        this->format, this->type, data);
    }

    void generateMipmaps() override
    {
        glBindTexture(GL_TEXTURE_1D, this->id);
        glGenerateMipmap(GL_TEXTURE_1D);
    }

    GLuint id;
    GLenum internalFormat;
    GLenum format;
    GLenum type;
};

class OGLTexture2D : public impl::Texture2D
{
public:
    OGLTexture2D(GLuint id, GLenum internalFormat, GLenum format, GLenum type)
        : id(id)
        , internalFormat(internalFormat)
        , format(format)
        , type(type)
    {
    }

    ~OGLTexture2D() override
    {
        glDeleteTextures(1, &this->id);
    }

    void update(std::size_t x, std::size_t y, std::size_t width, std::size_t height, const void* data,
                std::size_t level) override
    {
        glBindTexture(GL_TEXTURE_2D, this->id);
        glTexSubImage2D(GL_TEXTURE_2D, static_cast<GLint>(level), static_cast<GLint>(x), static_cast<GLint>(y),
                        static_cast<GLsizei>(width), static_cast<GLsizei>(height), this->format, this->type, data);
    }

    void generateMipmaps() override
    {
        glBindTexture(GL_TEXTURE_2D, this->id);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    GLuint id;
    GLenum internalFormat;
    GLenum format;
    GLenum type;
};

class OGLTexture2DArray : public impl::Texture2DArray
{
public:
    OGLTexture2DArray(GLuint id, GLenum internalFormat, GLenum format, GLenum type)
        : id(id)
        , internalFormat(internalFormat)
        , format(format)
        , type(type)
    {
    }

    ~OGLTexture2DArray() override
    {
        glDeleteTextures(1, &this->id);
    }

    void update(std::size_t x, std::size_t y, std::size_t i, std::size_t width, std::size_t height, const void* data,
                std::size_t level) override
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, this->id);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, static_cast<GLint>(level), static_cast<GLint>(x), static_cast<GLint>(y),
                        static_cast<GLint>(i), static_cast<GLsizei>(width), static_cast<GLsizei>(height), 1,
                        this->format, this->type, data);
    }

    void generateMipmaps() override
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, this->id);
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    }

    GLuint id;
    GLenum internalFormat;
    GLenum format;
    GLenum type;
};

class OGLTexture3D : public impl::Texture3D
{
public:
    OGLTexture3D(GLuint id, GLenum internalFormat, GLenum format, GLenum type)
        : id(id)
        , internalFormat(internalFormat)
        , format(format)
        , type(type)
    {
    }

    ~OGLTexture3D() override
    {
        glDeleteTextures(1, &this->id);
    }

    void update(std::size_t x, std::size_t y, std::size_t z, std::size_t width, std::size_t height, std::size_t depth,
                const void* data, std::size_t level) override
    {
        glBindTexture(GL_TEXTURE_3D, this->id);
        glTexSubImage3D(GL_TEXTURE_3D, static_cast<GLint>(level), static_cast<GLint>(x), static_cast<GLint>(y),
                        static_cast<GLint>(z), static_cast<GLsizei>(width), static_cast<GLsizei>(height),
                        static_cast<GLsizei>(depth), this->format, this->type, data);
    }

    void generateMipmaps() override
    {
        glBindTexture(GL_TEXTURE_3D, this->id);
        glGenerateMipmap(GL_TEXTURE_3D);
    }

    GLuint id;
    GLenum internalFormat;
    GLenum format;
    GLenum type;
};

class OGLCubeMap : public impl::CubeMap
{
public:
    OGLCubeMap(GLuint id, GLenum internalFormat, GLenum format, GLenum type)
        : id(id)
        , internalFormat(internalFormat)
        , format(format)
        , type(type)
    {
    }

    ~OGLCubeMap() override
    {
        glDeleteTextures(1, &this->id);
    }

    void update(std::size_t x, std::size_t y, std::size_t width, std::size_t height, const void* data, CubeFace face,
                std::size_t level) override
    {
        GLenum glFace;
        cubeFaceToGL(face, glFace);

        glBindTexture(GL_TEXTURE_CUBE_MAP, this->id);
        glTexSubImage2D(glFace, static_cast<GLint>(level), static_cast<GLint>(x), static_cast<GLint>(y),
                        static_cast<GLsizei>(width), static_cast<GLsizei>(height), this->format, this->type, data);
    }

    void generateMipmaps() override
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, this->id);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }

    GLuint id;
    GLenum internalFormat;
    GLenum format;
    GLenum type;
};

class OGLCubeMapArray : public impl::CubeMapArray
{
public:
    OGLCubeMapArray(GLuint id, GLenum internalFormat, GLenum format, GLenum type)
        : id(id)
        , internalFormat(internalFormat)
        , format(format)
        , type(type)
    {
    }

    ~OGLCubeMapArray() override
    {
        glDeleteTextures(1, &this->id);
    }

    void update(std::size_t x, std::size_t y, std::size_t i, std::size_t width, std::size_t height, const void* data,
                CubeFace face, std::size_t level = 0) override
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, this->id);
        glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, static_cast<GLint>(level), static_cast<GLint>(x),
                        static_cast<GLint>(y), static_cast<GLint>(i) * 6 + static_cast<GLint>(face),
                        static_cast<GLsizei>(width), static_cast<GLsizei>(height), 1, this->format, this->type, data);
    }

    void generateMipmaps() override
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, this->id);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP_ARRAY);
    }

    GLuint id;
    GLenum internalFormat;
    GLenum format;
    GLenum type;
};

class OGLConstantBuffer : public impl::ConstantBuffer
{
public:
    OGLConstantBuffer(GLuint id)
        : id(id)
    {
    }

    ~OGLConstantBuffer() override
    {
        glDeleteBuffers(1, &this->id);
    }

    void* map() override
    {
        glBindBuffer(GL_UNIFORM_BUFFER, this->id);
        return glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
    }

    void unmap() override
    {
        glUnmapBuffer(GL_UNIFORM_BUFFER);
    }

    GLuint id;
};

class OGLIndexBuffer : public impl::IndexBuffer
{
public:
    OGLIndexBuffer(GLuint id, GLenum format, std::size_t indexSz)
        : id(id)
        , format(format)
        , indexSz(indexSz)
    {
    }

    ~OGLIndexBuffer() override
    {
        glDeleteBuffers(1, &this->id);
    }

    void* map() override
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
        return glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
    }

    void unmap() override
    {
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    }

    GLuint id;
    GLenum format;
    std::size_t indexSz;
};

class OGLVertexBuffer : public impl::VertexBuffer
{
public:
    OGLVertexBuffer(GLuint id)
        : id(id)
    {
    }

    ~OGLVertexBuffer() override
    {
        glDeleteBuffers(1, &this->id);
    }

    void* map() override
    {
        glBindBuffer(GL_ARRAY_BUFFER, this->id);
        return glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    }

    void unmap() override
    {
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }

    GLuint id;
};

class OGLVertexArray : public impl::VertexArray
{
public:
    OGLVertexArray(GLuint id, const VertexBuffer* buffers)
        : id(id)
    {
        for (std::size_t i = 0; i < CUBOS_CORE_GL_MAX_VERTEX_ARRAY_BUFFER_COUNT; ++i)
        {
            this->buffers[i] = buffers[i];
        }
    }

    ~OGLVertexArray() override
    {
        glDeleteVertexArrays(1, &this->id);
    }

    VertexBuffer buffers[CUBOS_CORE_GL_MAX_VERTEX_ARRAY_BUFFER_COUNT];
    GLuint id;
};

class OGLShaderStage : public impl::ShaderStage
{
public:
    OGLShaderStage(Stage type, GLuint shader)
        : type(type)
        , shader(shader)
    {
    }

    ~OGLShaderStage() override
    {
        glDeleteShader(this->shader);
    }

    Stage getType() override
    {
        return this->type;
    }

    Stage type;
    GLuint shader;
};

class OGLShaderBindingPoint : public impl::ShaderBindingPoint
{
public:
    OGLShaderBindingPoint(const char* name, int loc, int tex = 0)
        : name(name)
        , loc(loc)
        , tex(tex)
    {
    }

    void bind(Sampler sampler) override
    {
        if (sampler)
        {
            glBindSampler(static_cast<GLuint>(this->tex), std::static_pointer_cast<OGLSampler>(sampler)->id);
        }
        else
        {
            glBindSampler(static_cast<GLuint>(this->tex), 0);
        }
    }

    void bind(Texture1D tex) override
    {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(this->tex));
        if (tex)
        {
            glBindTexture(GL_TEXTURE_1D, std::static_pointer_cast<OGLTexture1D>(tex)->id);
        }
        else
        {
            glBindTexture(GL_TEXTURE_1D, 0);
        }
        glUniform1i(this->loc, this->tex);
    }

    void bind(Texture2D tex) override
    {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(this->tex));
        if (tex)
        {
            glBindTexture(GL_TEXTURE_2D, std::static_pointer_cast<OGLTexture2D>(tex)->id);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glUniform1i(this->loc, this->tex);
    }

    void bind(Texture2DArray tex) override
    {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(this->tex));
        if (tex)
        {
            glBindTexture(GL_TEXTURE_2D_ARRAY, std::static_pointer_cast<OGLTexture2DArray>(tex)->id);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        }
        glUniform1i(this->loc, this->tex);
    }

    void bind(Texture3D tex) override
    {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(this->tex));
        if (tex)
        {
            glBindTexture(GL_TEXTURE_3D, std::static_pointer_cast<OGLTexture3D>(tex)->id);
        }
        else
        {
            glBindTexture(GL_TEXTURE_3D, 0);
        }
        glUniform1i(this->loc, this->tex);
    }

    void bind(CubeMap cubeMap) override
    {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(this->tex));
        if (cubeMap)
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, std::static_pointer_cast<OGLCubeMap>(cubeMap)->id);
        }
        else
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        }
        glUniform1i(this->loc, this->tex);
    }

    void bind(CubeMapArray cubeMap) override
    {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(this->tex));
        if (cubeMap)
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, std::static_pointer_cast<OGLCubeMapArray>(cubeMap)->id);
        }
        else
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);
        }
        glUniform1i(this->loc, this->tex);
    }

    void bind(ConstantBuffer cb) override
    {
        if (cb)
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(this->loc),
                             std::static_pointer_cast<OGLConstantBuffer>(cb)->id);
        }
        else
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(this->loc), 0);
        }
    }

    void bind(gl::Texture2D tex, int level, Access access) override
    {
        auto texImpl = std::static_pointer_cast<OGLTexture2D>(tex);
        GLenum glAccess;
        switch (access)
        {
        case Access::Read:
            glAccess = GL_READ_ONLY;
            break;
        case Access::Write:
            glAccess = GL_WRITE_ONLY;
            break;
        case Access::ReadWrite:
            glAccess = GL_READ_WRITE;
            break;
        default:
            abort();
        }

        glUniform1i(this->loc, this->tex);
        glBindImageTexture(static_cast<GLuint>(this->tex), texImpl->id, level, GL_TRUE, 0, glAccess,
                           texImpl->internalFormat);
    }

    void setConstant(glm::vec2 val) override
    {
        glUniform2fv(loc, 1, &val[0]);
    }

    void setConstant(glm::vec3 val) override
    {
        glUniform3fv(loc, 1, &val[0]);
    }

    void setConstant(glm::vec4 val) override
    {
        glUniform4fv(loc, 1, &val[0]);
    }

    void setConstant(glm::ivec2 val) override
    {
        glUniform2iv(loc, 1, &val[0]);
    }

    void setConstant(glm::ivec3 val) override
    {
        glUniform3iv(loc, 1, &val[0]);
    }

    void setConstant(glm::ivec4 val) override
    {
        glUniform4iv(loc, 1, &val[0]);
    }

    void setConstant(glm::uvec2 val) override
    {
        glUniform2uiv(loc, 1, &val[0]);
    }

    void setConstant(glm::uvec3 val) override
    {
        glUniform3uiv(loc, 1, &val[0]);
    }

    void setConstant(glm::uvec4 val) override
    {
        glUniform4uiv(loc, 1, &val[0]);
    }

    void setConstant(glm::mat4 val) override
    {
        glUniformMatrix4fv(loc, 1, GL_FALSE, &val[0][0]);
    }

    void setConstant(float val) override
    {
        glUniform1f(loc, val);
    }

    void setConstant(int val) override
    {
        glUniform1i(loc, val);
    }

    void setConstant(unsigned int val) override
    {
        glUniform1ui(loc, val);
    }

    bool queryConstantBufferStructure(ConstantBufferStructure* /*structure*/) override
    {
        return false; // TODO
    }

    std::string name;
    int loc, tex;
};

class OGLShaderPipeline : public impl::ShaderPipeline
{
public:
    OGLShaderPipeline(ShaderStage vs, ShaderStage ps, GLuint program)
        : vs(std::move(vs))
        , ps(std::move(ps))
        , program(program)
    {
        mTexCount = 0;
        mUboCount = 0;
        mSsboCount = 0;
    }

    OGLShaderPipeline(ShaderStage vs, ShaderStage gs, ShaderStage ps, GLuint program)
        : OGLShaderPipeline(std::move(vs), std::move(ps), program)
    {
        this->gs = std::move(gs);
    }

    OGLShaderPipeline(ShaderStage cs, GLuint program)
        : cs(std::move(cs))
        , program(program)
    {
        mTexCount = 0;
        mUboCount = 0;
        mSsboCount = 0;
    }

    ~OGLShaderPipeline() override
    {
        glDeleteProgram(this->program);
    }

    ShaderBindingPoint getBindingPoint(const char* name) override
    {
        // Search for already existing binding point
        for (auto& bp : this->bps)
        {
            if (bp.name == name)
            {
                return &bp;
            }
        }

        // No existing binding point was found, it must be created first

        auto loc = glGetUniformLocation(this->program, name);
        if (loc != -1)
        {
            bps.emplace_back(name, loc, mTexCount++);
            return &bps.back();
        }

        // Search for uniform block binding
        auto index = glGetUniformBlockIndex(this->program, name);
        if (index != GL_INVALID_INDEX)
        {
            auto loc = mUboCount;
            glUniformBlockBinding(this->program, index, static_cast<GLuint>(loc));

            GLenum glErr = glGetError();
            if (glErr != 0)
            {
                LOG_GL_ERROR(glErr);
                return nullptr;
            }

            mUboCount += 1;
            bps.emplace_back(name, loc);
            return &bps.back();
        }

        // Search for shader storage block binding
        index = glGetProgramResourceIndex(this->program, GL_SHADER_STORAGE_BLOCK, name);
        if (index != GL_INVALID_INDEX)
        {
            auto loc = mSsboCount;
            glShaderStorageBlockBinding(this->program, index, static_cast<GLuint>(loc));

            GLenum glErr = glGetError();
            if (glErr != 0)
            {
                LOG_GL_ERROR(glErr);
                return nullptr;
            }

            mSsboCount += 1;
            bps.emplace_back(name, loc);
            return &bps.back();
        }

        return nullptr;
    }

    ShaderStage vs, gs, ps, cs;
    GLuint program;
    std::list<OGLShaderBindingPoint> bps;

private:
    int mTexCount;
    int mUboCount;
    int mSsboCount;
};

OGLRenderDevice::OGLRenderDevice()
{
    // Set the debug message callback
    // TODO: disable this on release for performance reasons (?)
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(messageCallback, nullptr);

    // Create default states
    mDefaultRS = OGLRenderDevice::createRasterState({});
    mDefaultDSS = OGLRenderDevice::createDepthStencilState({});
    mDefaultBS = OGLRenderDevice::createBlendState({});
}

Framebuffer OGLRenderDevice::createFramebuffer(const FramebufferDesc& desc)
{
    // Validate arguments
    if (desc.targetCount == 0)
    {
        CUBOS_ERROR("Framebuffer must have at least one render target");
        return nullptr;
    }
    if (desc.targetCount > CUBOS_CORE_GL_MAX_FRAMEBUFFER_RENDER_TARGET_COUNT)
    {
        CUBOS_ERROR("Framebuffer can only have at most {} render targets",
                    CUBOS_CORE_GL_MAX_FRAMEBUFFER_RENDER_TARGET_COUNT);
        return nullptr;
    }

    for (uint32_t i = 0; i < desc.targetCount; ++i)
    {
        switch (desc.targets[i].getTargetType())
        {
        case FramebufferDesc::TargetType::CubeMap:
            if (desc.targets[i].getCubeMapTarget().handle == nullptr)
            {
                CUBOS_ERROR("Target {} is nullptr", i);
                return nullptr;
            }
            break;
        case FramebufferDesc::TargetType::Texture2D:
            if (desc.targets[i].getTexture2DTarget().handle == nullptr)
            {
                CUBOS_ERROR("Target {} is nullptr", i);
                return nullptr;
            }
            break;
        case FramebufferDesc::TargetType::CubeMapArray:
            if (desc.targets[i].getCubeMapArrayTarget().handle == nullptr)
            {
                CUBOS_ERROR("Target {} is nullptr", i);
                return nullptr;
            }
            break;
        case FramebufferDesc::TargetType::Texture2DArray:
            if (desc.targets[i].getTexture2DArrayTarget().handle == nullptr)
            {
                CUBOS_ERROR("Target {} is nullptr", i);
                return nullptr;
            }
            break;
        }
    }

    // Initialize framebuffer
    GLuint id;
    glGenFramebuffers(1, &id);
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    // Attach targets
    std::vector<GLenum> drawBuffers;
    for (uint32_t i = 0; i < desc.targetCount; ++i)
    {
        drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);

        switch (desc.targets[i].getTargetType())
        {
        case FramebufferDesc::TargetType::CubeMap:
            GLenum face;
            cubeFaceToGL(desc.targets[i].getCubeMapTarget().face, face);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, face,
                                   std::static_pointer_cast<OGLCubeMap>(desc.targets[i].getCubeMapTarget().handle)->id,
                                   static_cast<GLint>(desc.targets[i].mipLevel));
            break;
        case FramebufferDesc::TargetType::Texture2D:
            glFramebufferTexture2D(
                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D,
                std::static_pointer_cast<OGLTexture2D>(desc.targets[i].getTexture2DTarget().handle)->id,
                static_cast<GLint>(desc.targets[i].mipLevel));
            break;
        case FramebufferDesc::TargetType::CubeMapArray:
            glFramebufferTexture(
                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                std::static_pointer_cast<OGLCubeMapArray>(desc.targets[i].getCubeMapArrayTarget().handle)->id,
                static_cast<GLint>(desc.targets[i].mipLevel));
            break;
        case FramebufferDesc::TargetType::Texture2DArray:
            glFramebufferTexture(
                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                std::static_pointer_cast<OGLTexture2DArray>(desc.targets[i].getTexture2DArrayTarget().handle)->id,
                static_cast<GLint>(desc.targets[i].mipLevel));
            break;
        }
    }

    // Attach depth stencil texture
    if (desc.depthStencil.isSet())
    {
        bool formatError = false;
        switch (desc.depthStencil.getTargetType())
        {
        case FramebufferDesc::TargetType::CubeMap: {
            const auto& target = desc.depthStencil.getCubeMapTarget();
            GLenum face;
            cubeFaceToGL(target.face, face);
            auto ds = std::static_pointer_cast<OGLCubeMap>(target.handle);
            if (ds->format == GL_DEPTH_COMPONENT)
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, face, ds->id,
                                       static_cast<GLint>(desc.depthStencil.mipLevel));
            }
            else if (ds->format == GL_DEPTH_STENCIL)
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, ds->id,
                                       static_cast<GLint>(desc.depthStencil.mipLevel));
            }
            else
            {
                formatError = true;
            }
            break;
        }
        case FramebufferDesc::TargetType::Texture2D: {
            auto ds = std::static_pointer_cast<OGLTexture2D>(desc.depthStencil.getTexture2DTarget().handle);
            if (ds->format == GL_DEPTH_COMPONENT)
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ds->id,
                                       static_cast<GLint>(desc.depthStencil.mipLevel));
            }
            else if (ds->format == GL_DEPTH_STENCIL)
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, ds->id,
                                       static_cast<GLint>(desc.depthStencil.mipLevel));
            }
            else
            {
                formatError = true;
            }
            break;
        }
        case FramebufferDesc::TargetType::CubeMapArray: {
            auto ds = std::static_pointer_cast<OGLCubeMapArray>(desc.depthStencil.getCubeMapArrayTarget().handle);
            if (ds->format == GL_DEPTH_COMPONENT)
            {
                glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, ds->id,
                                     static_cast<GLint>(desc.depthStencil.mipLevel));
            }
            else if (ds->format == GL_DEPTH_STENCIL)
            {
                glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, ds->id,
                                     static_cast<GLint>(desc.depthStencil.mipLevel));
            }
            else
            {
                formatError = true;
            }
            break;
        }
        case FramebufferDesc::TargetType::Texture2DArray: {
            auto ds = std::static_pointer_cast<OGLTexture2DArray>(desc.depthStencil.getTexture2DArrayTarget().handle);
            if (ds->format == GL_DEPTH_COMPONENT)
            {
                glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, ds->id,
                                     static_cast<GLint>(desc.depthStencil.mipLevel));
            }
            else if (ds->format == GL_DEPTH_STENCIL)
            {
                glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, ds->id,
                                     static_cast<GLint>(desc.depthStencil.mipLevel));
            }
            else
            {
                formatError = true;
            }
            break;
        }
        }

        if (formatError)
        {
            glDeleteFramebuffers(1, &id);
            CUBOS_ERROR("Invalid depth stencil target format");
            return nullptr;
        }
    }

    // Define draw buffers
    glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());

    // Check errors
    GLenum glErr = glGetError();
    if (glErr != 0)
    {
        glDeleteFramebuffers(1, &id);
        LOG_GL_ERROR(glErr);
        return nullptr;
    }

    // Check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        glDeleteFramebuffers(1, &id);
        CUBOS_ERROR("glCheckFramebufferStatus didn't return GL_FRAMEBUFFER_COMPLETE");
        return nullptr;
    }

    return std::make_shared<OGLFramebuffer>(id);
}

void OGLRenderDevice::setFramebuffer(Framebuffer fb)
{
    if (fb)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, std::static_pointer_cast<OGLFramebuffer>(fb)->id);
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

RasterState OGLRenderDevice::createRasterState(const RasterStateDesc& desc)
{
    auto rs = std::make_shared<OGLRasterState>();
    rs->cullEnabled = static_cast<GLboolean>(desc.cullEnabled);
    rs->scissorEnabled = static_cast<GLboolean>(desc.scissorEnabled);
    faceToGL(desc.cullFace, rs->cullFace);
    windingToGL(desc.frontFace, rs->frontFace);
    rasterModeToGL(desc.rasterMode, rs->polygonMode);
    return rs;
}

void OGLRenderDevice::setRasterState(RasterState rs)
{
    auto rsImpl = std::static_pointer_cast<OGLRasterState>(rs ? rs : mDefaultRS);

    if (rsImpl->cullEnabled == 0U)
    {
        glDisable(GL_CULL_FACE);
    }
    else
    {
        glEnable(GL_CULL_FACE);
        glCullFace(rsImpl->cullFace);
    }

    if (rsImpl->scissorEnabled == 0U)
    {
        glDisable(GL_SCISSOR_TEST);
    }
    else
    {
        glEnable(GL_SCISSOR_TEST);
    }

    glFrontFace(rsImpl->frontFace);
    glPolygonMode(GL_FRONT_AND_BACK, rsImpl->polygonMode);
}

DepthStencilState OGLRenderDevice::createDepthStencilState(const DepthStencilStateDesc& desc)
{
    auto dss = std::make_shared<OGLDepthStencilState>();

    dss->depthEnabled = static_cast<GLboolean>(desc.depth.enabled);
    dss->depthWriteEnabled = static_cast<GLboolean>(desc.depth.writeEnabled);
    dss->depthNear = desc.depth.near;
    dss->depthFar = desc.depth.far;
    compareToGL(desc.depth.compare, dss->depthFunc);

    dss->stencilEnabled = static_cast<GLboolean>(desc.stencil.enabled);
    dss->stencilRef = desc.stencil.ref;
    dss->stencilReadMask = desc.stencil.readMask;
    dss->stencilWriteMask = desc.stencil.writeMask;

    compareToGL(desc.stencil.frontFace.compare, dss->frontStencilFunc);
    stencilActionToGL(desc.stencil.frontFace.fail, dss->frontFaceStencilFail);
    stencilActionToGL(desc.stencil.frontFace.pass, dss->frontFaceStencilPass);
    stencilActionToGL(desc.stencil.frontFace.depthFail, dss->frontFaceDepthFail);

    compareToGL(desc.stencil.backFace.compare, dss->backStencilFunc);
    stencilActionToGL(desc.stencil.backFace.fail, dss->backFaceStencilFail);
    stencilActionToGL(desc.stencil.backFace.pass, dss->backFaceStencilPass);
    stencilActionToGL(desc.stencil.backFace.depthFail, dss->backFaceDepthFail);

    return dss;
}

void OGLRenderDevice::setDepthStencilState(DepthStencilState dss)
{
    auto dssImpl = std::static_pointer_cast<OGLDepthStencilState>(dss ? dss : mDefaultDSS);

    if (dssImpl->depthEnabled == 0U)
    {
        glDisable(GL_DEPTH_TEST);
    }
    else
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(dssImpl->depthFunc);
        glDepthMask(dssImpl->depthWriteEnabled);
        glDepthRange(dssImpl->depthNear, dssImpl->depthFar);
    }

    if (dssImpl->stencilEnabled == 0U)
    {
        glDisable(GL_STENCIL_TEST);
    }
    else
    {
        glEnable(GL_STENCIL_TEST);

        glStencilFuncSeparate(GL_FRONT, dssImpl->frontStencilFunc, static_cast<GLint>(dssImpl->stencilRef),
                              dssImpl->stencilReadMask);
        glStencilMaskSeparate(GL_FRONT, dssImpl->stencilWriteMask);
        glStencilOpSeparate(GL_FRONT, dssImpl->frontFaceStencilFail, dssImpl->frontFaceDepthFail,
                            dssImpl->frontFaceStencilPass);
        glStencilFuncSeparate(GL_BACK, dssImpl->backStencilFunc, static_cast<GLint>(dssImpl->stencilRef),
                              dssImpl->stencilReadMask);
        glStencilMaskSeparate(GL_BACK, dssImpl->stencilWriteMask);
        glStencilOpSeparate(GL_BACK, dssImpl->backFaceStencilFail, dssImpl->backFaceDepthFail,
                            dssImpl->backFaceStencilPass);
    }
}

BlendState OGLRenderDevice::createBlendState(const BlendStateDesc& desc)
{
    auto bs = std::make_shared<OGLBlendState>();

    bs->blendEnabled = static_cast<GLboolean>(desc.blendEnabled);
    blendFactorToGL(desc.alpha.src, bs->srcAlphaFactor);
    blendFactorToGL(desc.alpha.dst, bs->dstAlphaFactor);
    blendOpToGL(desc.alpha.op, bs->alphaBlendOp);
    blendFactorToGL(desc.color.src, bs->srcFactor);
    blendFactorToGL(desc.color.dst, bs->dstFactor);
    blendOpToGL(desc.color.op, bs->blendOp);

    return bs;
}

void OGLRenderDevice::setBlendState(BlendState bs)
{
    auto bsImpl = std::static_pointer_cast<OGLBlendState>(bs ? bs : mDefaultBS);

    if (bsImpl->blendEnabled == 0U)
    {
        glDisable(GL_BLEND);
    }
    else
    {
        glEnable(GL_BLEND);
        glBlendFuncSeparate(bsImpl->srcFactor, bsImpl->dstFactor, bsImpl->srcAlphaFactor, bsImpl->dstAlphaFactor);
        glBlendEquationSeparate(bsImpl->blendOp, bsImpl->alphaBlendOp);
    }
}

Sampler OGLRenderDevice::createSampler(const SamplerDesc& desc)
{
    GLenum addressU;
    GLenum addressV;
    GLenum addressW;

    addressToGL(desc.addressU, addressU);
    addressToGL(desc.addressV, addressV);
    addressToGL(desc.addressW, addressW);

    GLenum minFilter;
    GLenum magFilter;

    switch (desc.minFilter)
    {
    case TextureFilter::Nearest:
        if (desc.mipmapFilter == TextureFilter::None)
        {
            minFilter = GL_NEAREST;
        }
        else if (desc.mipmapFilter == TextureFilter::Nearest)
        {
            minFilter = GL_NEAREST_MIPMAP_NEAREST;
        }
        else if (desc.mipmapFilter == TextureFilter::Linear)
        {
            minFilter = GL_NEAREST_MIPMAP_LINEAR;
        }
        else
        {
            abort(); // Invalid enum value
        }
        break;
    case TextureFilter::Linear:
        if (desc.mipmapFilter == TextureFilter::None)
        {
            minFilter = GL_LINEAR;
        }
        else if (desc.mipmapFilter == TextureFilter::Nearest)
        {
            minFilter = GL_LINEAR_MIPMAP_NEAREST;
        }
        else if (desc.mipmapFilter == TextureFilter::Linear)
        {
            minFilter = GL_LINEAR_MIPMAP_LINEAR;
        }
        else
        {
            abort(); // Invalid enum value
        }
        break;
    default:
        abort(); // Invalid enum value
    }

    switch (desc.magFilter)
    {
    case TextureFilter::Nearest:
        magFilter = GL_NEAREST;
        break;
    case TextureFilter::Linear:
        magFilter = GL_LINEAR;
        break;
    default:
        abort(); // Invalid enum value
    }

    // Initialize sampler
    GLuint id;
    glGenSamplers(1, &id);
    glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(minFilter));
    glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(magFilter));
    if (GL_ARB_texture_filter_anisotropic)
    {
        glSamplerParameteri(id, GL_TEXTURE_MAX_ANISOTROPY, static_cast<GLint>(desc.maxAnisotropy));
    }
    glSamplerParameteri(id, GL_TEXTURE_WRAP_S, static_cast<GLint>(addressU));
    glSamplerParameteri(id, GL_TEXTURE_WRAP_T, static_cast<GLint>(addressV));
    glSamplerParameteri(id, GL_TEXTURE_WRAP_R, static_cast<GLint>(addressW));
    glSamplerParameterfv(id, GL_TEXTURE_BORDER_COLOR, desc.borderColor);

    // Check errors
    GLenum glErr = glGetError();
    if (glErr != 0)
    {
        glDeleteSamplers(1, &id);
        LOG_GL_ERROR(glErr);
        return nullptr;
    }

    return std::make_shared<OGLSampler>(id);
}

Texture1D OGLRenderDevice::createTexture1D(const Texture1DDesc& desc)
{
    if (desc.format == TextureFormat::Depth16 || desc.format == TextureFormat::Depth32 ||
        desc.format == TextureFormat::Depth24Stencil8 || desc.format == TextureFormat::Depth32Stencil8)
    {
        CUBOS_ERROR("Depth/stencil formats are not supported on 1D textures");
        return nullptr;
    }

    GLenum internalFormat;
    GLenum format;
    GLenum type;

    if (!textureFormatToGL(desc.format, internalFormat, format, type))
    {
        CUBOS_ERROR("Unsupported texture format {}", desc.format);
        return nullptr;
    }

    // Initialize texture
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_1D, id);
    for (std::size_t i = 0, div = 1; i < desc.mipLevelCount; ++i, div *= 2)
    {
        glTexImage1D(GL_TEXTURE_1D, static_cast<GLint>(i), static_cast<GLint>(internalFormat),
                     static_cast<GLsizei>(desc.width / div), 0, format, type, desc.data[i]);
    }
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    if (GL_ARB_texture_filter_anisotropic)
    {
        glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0F);
    }

    // Check errors
    GLenum glErr = glGetError();
    if (glErr != 0)
    {
        glDeleteTextures(1, &id);
        LOG_GL_ERROR(glErr);
        return nullptr;
    }

    return std::make_shared<OGLTexture1D>(id, internalFormat, format, type);
}

Texture2D OGLRenderDevice::createTexture2D(const Texture2DDesc& desc)
{
    GLenum internalFormat;
    GLenum format;
    GLenum type;

    if (!textureFormatToGL(desc.format, internalFormat, format, type))
    {
        CUBOS_ERROR("Unsupported texture format {}", desc.format);
        return nullptr;
    }

    // Initialize texture
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    for (std::size_t i = 0, div = 1; i < desc.mipLevelCount; ++i, div *= 2)
    {
        glTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(i), static_cast<GLint>(internalFormat),
                     static_cast<GLsizei>(desc.width / div), static_cast<GLsizei>(desc.height / div), 0, format, type,
                     desc.data[i]);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (GL_ARB_texture_filter_anisotropic)
    {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0F);
    }

    // Check errors
    GLenum glErr = glGetError();
    if (glErr != 0)
    {
        glDeleteTextures(1, &id);
        LOG_GL_ERROR(glErr);
        return nullptr;
    }

    return std::make_shared<OGLTexture2D>(id, internalFormat, format, type);
}

Texture2DArray OGLRenderDevice::createTexture2DArray(const Texture2DArrayDesc& desc)
{
    GLenum internalFormat;
    GLenum format;
    GLenum type;

    if (!textureFormatToGL(desc.format, internalFormat, format, type))
    {
        CUBOS_ERROR("Unsupported texture format {}", desc.format);
        return nullptr;
    }

    // Initialize texture
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, id);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, static_cast<GLsizei>(desc.mipLevelCount), internalFormat,
                   static_cast<GLsizei>(desc.width), static_cast<GLsizei>(desc.height),
                   static_cast<GLsizei>(desc.size));
    for (std::size_t i = 0; i < desc.size; ++i)
    {
        for (std::size_t j = 0, div = 1; i < desc.mipLevelCount; ++j, div *= 2)
        {
            if (desc.data[i][j] != nullptr)
            {
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, static_cast<GLint>(j), 0, 0, static_cast<GLint>(i),
                                static_cast<GLsizei>(desc.width / div), static_cast<GLsizei>(desc.height / div), 1,
                                format, type, desc.data[i][j]);
            }
        }
    }
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (GL_ARB_texture_filter_anisotropic)
    {
        glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0F);
    }

    // Check errors
    GLenum glErr = glGetError();
    if (glErr != 0)
    {
        glDeleteTextures(1, &id);
        LOG_GL_ERROR(glErr);
        return nullptr;
    }

    return std::make_shared<OGLTexture2DArray>(id, internalFormat, format, type);
}

Texture3D OGLRenderDevice::createTexture3D(const Texture3DDesc& desc)
{
    if (desc.format == TextureFormat::Depth16 || desc.format == TextureFormat::Depth32 ||
        desc.format == TextureFormat::Depth24Stencil8 || desc.format == TextureFormat::Depth32Stencil8)
    {
        CUBOS_ERROR("Depth/stencil formats are not supported on 3D textures");
        return nullptr;
    }

    GLenum internalFormat;
    GLenum format;
    GLenum type;

    if (!textureFormatToGL(desc.format, internalFormat, format, type))
    {
        CUBOS_ERROR("Unsupported texture format {}", desc.format);
        return nullptr;
    }

    // Initialize texture
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_3D, id);
    for (std::size_t i = 0, div = 1; i < desc.mipLevelCount; ++i, div *= 2)
    {
        glTexImage3D(GL_TEXTURE_3D, static_cast<GLint>(i), static_cast<GLint>(internalFormat),
                     static_cast<GLsizei>(desc.width / div), static_cast<GLsizei>(desc.height / div),
                     static_cast<GLsizei>(desc.depth / div), 0, format, type, desc.data[i]);
    }
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    if (GL_ARB_texture_filter_anisotropic)
    {
        glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0F);
    }

    // Check errors
    GLenum glErr = glGetError();
    if (glErr != 0)
    {
        glDeleteTextures(1, &id);
        LOG_GL_ERROR(glErr);
        return nullptr;
    }

    return std::make_shared<OGLTexture3D>(id, internalFormat, format, type);
}

CubeMap OGLRenderDevice::createCubeMap(const CubeMapDesc& desc)
{
    GLenum internalFormat;
    GLenum format;
    GLenum type;

    if (!textureFormatToGL(desc.format, internalFormat, format, type))
    {
        CUBOS_ERROR("Unsupported texture format {}", desc.format);
        return nullptr;
    }

    // Initialize texture
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    for (std::size_t i = 0, div = 1; i < desc.mipLevelCount; ++i, div *= 2)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, static_cast<GLint>(i), static_cast<GLint>(internalFormat),
                     static_cast<GLsizei>(desc.width / div), static_cast<GLsizei>(desc.height / div), 0, format, type,
                     desc.data[static_cast<int>(CubeFace::PositiveX)][i]);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, static_cast<GLint>(i), static_cast<GLint>(internalFormat),
                     static_cast<GLsizei>(desc.width / div), static_cast<GLsizei>(desc.height / div), 0, format, type,
                     desc.data[static_cast<int>(CubeFace::NegativeX)][i]);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, static_cast<GLint>(i), static_cast<GLint>(internalFormat),
                     static_cast<GLsizei>(desc.width / div), static_cast<GLsizei>(desc.height / div), 0, format, type,
                     desc.data[static_cast<int>(CubeFace::PositiveY)][i]);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, static_cast<GLint>(i), static_cast<GLint>(internalFormat),
                     static_cast<GLsizei>(desc.width / div), static_cast<GLsizei>(desc.height / div), 0, format, type,
                     desc.data[static_cast<int>(CubeFace::NegativeY)][i]);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, static_cast<GLint>(i), static_cast<GLint>(internalFormat),
                     static_cast<GLsizei>(desc.width / div), static_cast<GLsizei>(desc.height / div), 0, format, type,
                     desc.data[static_cast<int>(CubeFace::PositiveZ)][i]);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, static_cast<GLint>(i), static_cast<GLint>(internalFormat),
                     static_cast<GLsizei>(desc.width / div), static_cast<GLsizei>(desc.height / div), 0, format, type,
                     desc.data[static_cast<int>(CubeFace::NegativeZ)][i]);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (GL_ARB_texture_filter_anisotropic)
    {
        glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0F);
    }

    // Check errors
    GLenum glErr = glGetError();
    if (glErr != 0)
    {
        glDeleteTextures(1, &id);
        LOG_GL_ERROR(glErr);
        return nullptr;
    }

    return std::make_shared<OGLCubeMap>(id, internalFormat, format, type);
}

CubeMapArray OGLRenderDevice::createCubeMapArray(const CubeMapArrayDesc& desc)
{
    GLenum internalFormat;
    GLenum format;
    GLenum type;

    if (!textureFormatToGL(desc.format, internalFormat, format, type))
    {
        CUBOS_ERROR("Unsupported texture format {}", desc.format);
        return nullptr;
    }

    // Initialize texture
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, id);
    glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, static_cast<GLsizei>(desc.mipLevelCount), internalFormat,
                   static_cast<GLsizei>(desc.width), static_cast<GLsizei>(desc.height),
                   static_cast<GLsizei>(desc.size * 6));
    for (std::size_t i = 0; i < desc.size; ++i)
    {
        for (GLint face = 0; face < 6; ++face)
        {
            for (std::size_t j = 0, div = 1; i < desc.mipLevelCount; ++j, div *= 2)
            {
                if (desc.data[i][face][j] != nullptr)
                {
                    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, static_cast<GLint>(j), 0, 0, static_cast<GLint>(i) * 6 + face,
                                    static_cast<GLsizei>(desc.width / div), static_cast<GLsizei>(desc.height / div), 1,
                                    format, type, desc.data[i][face][j]);
                }
            }
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (GL_ARB_texture_filter_anisotropic)
    {
        glTexParameterf(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0F);
    }

    // Check errors
    GLenum glErr = glGetError();
    if (glErr != 0)
    {
        glDeleteTextures(1, &id);
        LOG_GL_ERROR(glErr);
        return nullptr;
    }

    return std::make_shared<OGLCubeMapArray>(id, internalFormat, format, type);
}

ConstantBuffer OGLRenderDevice::createConstantBuffer(std::size_t size, const void* data, Usage usage)
{
    // Validate arguments
    if (usage == Usage::Static && data == nullptr)
    {
        abort();
    }

    GLenum glUsage;
    if (usage == Usage::Default || usage == Usage::Static)
    {
        glUsage = GL_STATIC_DRAW;
    }
    else if (usage == Usage::Dynamic)
    {
        glUsage = GL_DYNAMIC_DRAW;
    }
    else
    {
        abort(); // Invalid enum value
    }

    // Initialize buffer
    GLuint id;
    glGenBuffers(1, &id);
    glBindBuffer(GL_UNIFORM_BUFFER, id);
    glBufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(size), data, glUsage);

    // Check errors
    GLenum glErr = glGetError();
    if (glErr != 0)
    {
        glDeleteBuffers(1, &id);
        LOG_GL_ERROR(glErr);
        return nullptr;
    }

    return std::make_shared<OGLConstantBuffer>(id);
}

IndexBuffer OGLRenderDevice::createIndexBuffer(std::size_t size, const void* data, IndexFormat format, Usage usage)
{
    // Validate arguments
    if (usage == Usage::Static && data == nullptr)
    {
        abort();
    }

    GLenum glUsage;
    if (usage == Usage::Default || usage == Usage::Static)
    {
        glUsage = GL_STATIC_DRAW;
    }
    else if (usage == Usage::Dynamic)
    {
        glUsage = GL_DYNAMIC_DRAW;
    }
    else
    {
        abort(); // Invalid enum value
    }

    GLenum glFormat;
    std::size_t indexSz;
    if (format == IndexFormat::UShort)
    {
        glFormat = GL_UNSIGNED_SHORT;
        indexSz = 2;
    }
    else if (format == IndexFormat::UInt)
    {
        glFormat = GL_UNSIGNED_INT;
        indexSz = 4;
    }
    else
    {
        abort(); // Invalid enum value
    }

    // Initialize buffer
    GLuint id;
    glGenBuffers(1, &id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), data, glUsage);

    // Check errors
    GLenum glErr = glGetError();
    if (glErr != 0)
    {
        glDeleteBuffers(1, &id);
        LOG_GL_ERROR(glErr);
        return nullptr;
    }

    return std::make_shared<OGLIndexBuffer>(id, glFormat, indexSz);
}

void OGLRenderDevice::setIndexBuffer(IndexBuffer ib)
{
    auto ibImpl = std::static_pointer_cast<OGLIndexBuffer>(ib);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibImpl->id);
    mCurrentIndexFormat = static_cast<int>(ibImpl->format);
    mCurrentIndexSz = ibImpl->indexSz;
}

VertexBuffer OGLRenderDevice::createVertexBuffer(std::size_t size, const void* data, Usage usage)
{
    // Validate arguments
    if (usage == Usage::Static && data == nullptr)
    {
        abort();
    }

    GLenum glUsage;
    if (usage == Usage::Default || usage == Usage::Static)
    {
        glUsage = GL_STATIC_DRAW;
    }
    else if (usage == Usage::Dynamic)
    {
        glUsage = GL_DYNAMIC_DRAW;
    }
    else
    {
        abort(); // Invalid enum value
    }

    // Initialize buffer
    GLuint id;
    glGenBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), data, glUsage);

    // Check errors
    GLenum glErr = glGetError();
    if (glErr != 0)
    {
        glDeleteBuffers(1, &id);
        LOG_GL_ERROR(glErr);
        return nullptr;
    }

    return std::make_shared<OGLVertexBuffer>(id);
}

VertexArray OGLRenderDevice::createVertexArray(const VertexArrayDesc& desc)
{
    auto pp = std::static_pointer_cast<OGLShaderPipeline>(desc.shaderPipeline);
    assert(pp != nullptr);

    // Initialize vertex array
    GLuint id;
    glGenVertexArrays(1, &id);
    glBindVertexArray(id);

    // Link elements
    assert(desc.elementCount <= CUBOS_CORE_GL_MAX_VERTEX_ARRAY_ELEMENT_COUNT);
    for (std::size_t i = 0; i < desc.elementCount; ++i)
    {
        // Get buffer
        auto vb = std::static_pointer_cast<OGLVertexBuffer>(desc.buffers[desc.elements[i].buffer.index]);
        assert(vb != nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, vb->id);

        // Get attribute location
        GLint loc = glGetAttribLocation(pp->program, desc.elements[i].name);
        if (loc == -1)
        {
            glDeleteVertexArrays(1, &id);
            CUBOS_ERROR("Could not find vertex element with name '{}'", desc.elements[i].name);
            return nullptr;
        }

        // Get type
        GLenum type;
        bool normalized;
        bool integer;

        switch (desc.elements[i].type)
        {
        case Type::Byte:
            type = GL_BYTE;
            normalized = GL_FALSE;
            integer = true;
            break;
        case Type::Short:
            type = GL_SHORT;
            normalized = GL_FALSE;
            integer = true;
            break;
        case Type::Int:
            type = GL_INT;
            normalized = GL_FALSE;
            integer = true;
            break;
        case Type::UByte:
            type = GL_UNSIGNED_BYTE;
            normalized = GL_FALSE;
            integer = true;
            break;
        case Type::UShort:
            type = GL_UNSIGNED_SHORT;
            normalized = GL_FALSE;
            integer = true;
            break;
        case Type::UInt:
            type = GL_UNSIGNED_INT;
            normalized = GL_FALSE;
            integer = true;
            break;
        case Type::NByte:
            type = GL_BYTE;
            normalized = GL_TRUE;
            integer = false;
            break;
        case Type::NShort:
            type = GL_SHORT;
            normalized = GL_TRUE;
            integer = false;
            break;
        case Type::NUByte:
            type = GL_UNSIGNED_BYTE;
            normalized = GL_TRUE;
            integer = false;
            break;
        case Type::NUShort:
            type = GL_UNSIGNED_SHORT;
            normalized = GL_TRUE;
            integer = false;
            break;
        case Type::Float:
            type = GL_FLOAT;
            normalized = GL_FALSE;
            integer = false;
            break;
        default:
            abort(); // Invalid vertex element type
        }

        assert(desc.elements[i].size >= 1 && desc.elements[i].size <= 4);

        glEnableVertexAttribArray(static_cast<GLuint>(loc));
        if (!integer)
        {
            glVertexAttribPointer(static_cast<GLuint>(loc), (GLint)desc.elements[i].size, type,
                                  static_cast<GLboolean>(normalized), (GLsizei)desc.elements[i].buffer.stride,
                                  (const void*)desc.elements[i].buffer.offset);
        }
        else
        {
            glVertexAttribIPointer(static_cast<GLuint>(loc), (GLint)desc.elements[i].size, type,
                                   (GLsizei)desc.elements[i].buffer.stride,
                                   (const void*)desc.elements[i].buffer.offset);
        }
    }

    // Check errors
    GLenum glErr = glGetError();
    if (glErr != 0)
    {
        glDeleteVertexArrays(1, &id);
        LOG_GL_ERROR(glErr);
        return nullptr;
    }

    return std::make_shared<OGLVertexArray>(id, desc.buffers);
}

void OGLRenderDevice::setVertexArray(VertexArray va)
{
    glBindVertexArray(std::static_pointer_cast<OGLVertexArray>(va)->id);
}

ShaderStage OGLRenderDevice::createShaderStage(Stage stage, const char* src)
{
    GLenum shaderType;
    switch (stage)
    {
    case Stage::Vertex:
        shaderType = GL_VERTEX_SHADER;
        break;
    case Stage::Geometry:
        shaderType = GL_GEOMETRY_SHADER;
        break;
    case Stage::Pixel:
        shaderType = GL_FRAGMENT_SHADER;
        break;
    case Stage::Compute:
        shaderType = GL_COMPUTE_SHADER;
        break;
    default:
        CUBOS_CRITICAL("Unsupported shader stage type");
        return nullptr;
    }

    // Initialize shader
    GLuint id = glCreateShader(shaderType);
    glShaderSource(id, 1, (const GLchar* const*)&src, nullptr);
    glCompileShader(id);

    // Check for errors
    GLint success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (success == 0)
    {
        GLchar infoLog[512];
        glGetShaderInfoLog(id, sizeof(infoLog), nullptr, infoLog);
        glDeleteShader(id);
        CUBOS_ERROR("Could not compile shader: {}", infoLog);
        return nullptr;
    }

    // Check for OpenGL errors
    GLenum glErr = glGetError();
    if (glErr != 0)
    {
        glDeleteShader(id);
        LOG_GL_ERROR(glErr);
        return nullptr;
    }

    return std::make_shared<OGLShaderStage>(stage, id);
}

ShaderPipeline OGLRenderDevice::createShaderPipeline(ShaderStage vs, ShaderStage ps)
{
    auto vsImpl = std::static_pointer_cast<OGLShaderStage>(vs);
    auto psImpl = std::static_pointer_cast<OGLShaderStage>(ps);

    // Initialize program
    auto id = glCreateProgram();
    glAttachShader(id, vsImpl->shader);
    glAttachShader(id, psImpl->shader);
    glLinkProgram(id);

    // Check for linking errors
    GLint success;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (success == 0)
    {
        char infoLog[512];
        glGetProgramInfoLog(id, sizeof(infoLog), nullptr, infoLog);
        glDeleteProgram(id);
        CUBOS_ERROR("Could not link program (shader pipeline): {}", infoLog);
        return nullptr;
    }

    // Check for OpenGL errors
    GLenum glErr = glGetError();
    if (glErr != 0)
    {
        glDeleteProgram(id);
        LOG_GL_ERROR(glErr);
        return nullptr;
    }

    return std::make_shared<OGLShaderPipeline>(vsImpl, psImpl, id);
}

ShaderPipeline OGLRenderDevice::createShaderPipeline(ShaderStage vs, ShaderStage gs, ShaderStage ps)
{
    auto vsImpl = std::static_pointer_cast<OGLShaderStage>(vs);
    auto gsImpl = std::static_pointer_cast<OGLShaderStage>(gs);
    auto psImpl = std::static_pointer_cast<OGLShaderStage>(ps);

    // Initialize program
    auto id = glCreateProgram();
    glAttachShader(id, vsImpl->shader);
    glAttachShader(id, gsImpl->shader);
    glAttachShader(id, psImpl->shader);
    glLinkProgram(id);

    // Check for linking errors
    GLint success;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (success == 0)
    {
        char infoLog[512];
        glGetProgramInfoLog(id, sizeof(infoLog), nullptr, infoLog);
        glDeleteProgram(id);
        CUBOS_ERROR("Could not link program (shader pipeline): {}", infoLog);
        return nullptr;
    }

    // Check for OpenGL errors
    GLenum glErr = glGetError();
    if (glErr != 0)
    {
        glDeleteProgram(id);
        LOG_GL_ERROR(glErr);
        return nullptr;
    }

    return std::make_shared<OGLShaderPipeline>(vsImpl, gsImpl, psImpl, id);
}

ShaderPipeline OGLRenderDevice::createShaderPipeline(ShaderStage cs)
{
    auto csImpl = std::static_pointer_cast<OGLShaderStage>(cs);

    // Initialize program
    auto id = glCreateProgram();
    glAttachShader(id, csImpl->shader);
    glLinkProgram(id);

    // Check for linking errors
    GLint success;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (success == 0)
    {
        char infoLog[512];
        glGetProgramInfoLog(id, sizeof(infoLog), nullptr, infoLog);
        glDeleteProgram(id);
        CUBOS_ERROR("Could not link program (shader pipeline): {}", infoLog);
        return nullptr;
    }

    // Check for OpenGL errors
    GLenum glErr = glGetError();
    if (glErr != 0)
    {
        glDeleteProgram(id);
        LOG_GL_ERROR(glErr);
        return nullptr;
    }

    return std::make_shared<OGLShaderPipeline>(csImpl, id);
}

void OGLRenderDevice::setShaderPipeline(ShaderPipeline pipeline)
{
    glUseProgram(std::static_pointer_cast<OGLShaderPipeline>(pipeline)->program);
}

void OGLRenderDevice::clearColor(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void OGLRenderDevice::clearTargetColor(std::size_t target, float r, float g, float b, float a)
{
    float color[] = {r, g, b, a};
    glClearBufferfv(GL_COLOR, static_cast<GLint>(target), color);
}

void OGLRenderDevice::clearDepth(float depth)
{
    glClearDepth(depth);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void OGLRenderDevice::clearStencil(int stencil)
{
    glClearStencil(stencil);
    glClear(GL_STENCIL_BUFFER_BIT);
}

void OGLRenderDevice::drawTriangles(std::size_t offset, std::size_t count)
{
    glDrawArrays(GL_TRIANGLES, static_cast<GLint>(offset), static_cast<GLsizei>(count));
}

void OGLRenderDevice::drawTrianglesIndexed(std::size_t offset, std::size_t count)
{
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(count), static_cast<GLenum>(mCurrentIndexFormat),
                   reinterpret_cast<const void*>(offset * mCurrentIndexSz));
}

void OGLRenderDevice::drawTrianglesInstanced(std::size_t offset, std::size_t count, std::size_t instanceCount)
{
    glDrawArraysInstanced(GL_TRIANGLES, static_cast<GLint>(offset), static_cast<GLsizei>(count),
                          static_cast<GLsizei>(instanceCount));
}

void OGLRenderDevice::drawTrianglesIndexedInstanced(std::size_t offset, std::size_t count, std::size_t instanceCount)
{
    glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(count), static_cast<GLenum>(mCurrentIndexFormat),
                            reinterpret_cast<const void*>(offset * mCurrentIndexSz),
                            static_cast<GLsizei>(instanceCount));
}

void OGLRenderDevice::dispatchCompute(std::size_t x, std::size_t y, std::size_t z)
{
    glDispatchCompute(static_cast<GLuint>(x), static_cast<GLuint>(y), static_cast<GLuint>(z));
}

void OGLRenderDevice::memoryBarrier(MemoryBarriers barriers)
{
    GLbitfield barrier = 0;
    if ((barriers & MemoryBarriers::VertexBuffer) != MemoryBarriers::None)
    {
        barrier |= GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT;
    }
    if ((barriers & MemoryBarriers::IndexBuffer) != MemoryBarriers::None)
    {
        barrier |= GL_ELEMENT_ARRAY_BARRIER_BIT;
    }
    if ((barriers & MemoryBarriers::ConstantBuffer) != MemoryBarriers::None)
    {
        barrier |= GL_UNIFORM_BARRIER_BIT;
    }
    if ((barriers & MemoryBarriers::ImageAccess) != MemoryBarriers::None)
    {
        barrier |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
    }
    if ((barriers & MemoryBarriers::TextureAccess) != MemoryBarriers::None)
    {
        barrier |= GL_TEXTURE_FETCH_BARRIER_BIT;
    }
    if ((barriers & MemoryBarriers::Framebuffer) != MemoryBarriers::None)
    {
        barrier |= GL_FRAMEBUFFER_BARRIER_BIT;
    }
    glMemoryBarrier(barrier);
}

void OGLRenderDevice::setViewport(int x, int y, int w, int h)
{
    glViewport(x, y, w, h);
}

void OGLRenderDevice::setScissor(int x, int y, int w, int h)
{
    glScissor(x, y, w, h);
}

int OGLRenderDevice::getProperty(Property prop)
{
    GLint major;
    GLint minor;

    switch (prop)
    {
    case Property::MaxAnisotropy:
        if (!GL_ARB_texture_filter_anisotropic)
        {
            return 1;
        }
        else
        {
            GLfloat val;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &val);
            return static_cast<int>(val);
        }

    case Property::ComputeSupported:
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        return (major >= 4 && minor >= 3) ? 1 : 0;

    default:
        return -1;
    }
}
