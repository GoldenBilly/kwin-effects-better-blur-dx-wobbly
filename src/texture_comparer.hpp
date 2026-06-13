#pragma once

#include <opengl/glshadermanager.h>
#include <opengl/glshader.h>

#include <epoxy/gl.h>

#include <memory>

namespace BBDX {

class TextureComparer {
    // compute shader - we need to handle this ourselves :p
    GLuint m_computeShader{0};

    // regular vert+frag so let KWin handle it
    std::unique_ptr<KWin::GLShader> m_glueShader{nullptr};

    TextureComparer() = default;

public:
    /**
     * Create a new TextureComparer
     * nullptr on error
     */
    static std::unique_ptr<TextureComparer> create();

    /**
     * Clean up GL resources
     */
    ~TextureComparer();

    /**
     * No copying
     */
    TextureComparer(TextureComparer &other) = delete;
    TextureComparer& operator=(TextureComparer &other) = delete;
};

}
