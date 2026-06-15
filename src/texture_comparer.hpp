#pragma once

#include "kwin_compat.hpp"

#include <effect/effectwindow.h>
#include <opengl/glshadermanager.h>
#include <opengl/glshader.h>
#include <opengl/gltexture.h>

#include <epoxy/gl.h>

#include <memory>
#include <unordered_map>

namespace BBDX {

class TextureComparer {
public:
    /**
     * Per Window (BlurCacheLRU) data
     */
    struct WindowData {
    private:
        /**
         * Use create()
         */
        WindowData() = default;

    public:
        // SSBO counting changed blocks
        GLuint counterBuffer{0};

        // GL query object used for conditional render
        GLuint query{0};

        /**
         * Create WindowData
         * nullptr on error
         */
        static std::unique_ptr<WindowData> create();

        /**
         * Cleanup GL resources
         */
        ~WindowData();
    };

private:
    // a compute shader instance
    // because we need one for each texture format
    struct ComputeShader {
        GLuint program{0};
        GLint dirtyRectLocation{};

        ~ComputeShader() {
            if (program > 0) {
                glDeleteProgram(program);
            }
        }
    };

    // compute shaders - we need to handle this ourselves :p
    // one for each format we encountered
    std::unordered_map<GLenum, std::unique_ptr<ComputeShader>> m_computeShaders{};

    // regular vert+frag so let KWin handle it
    std::unique_ptr<KWin::GLShader> m_glueShader{nullptr};

    TextureComparer() = default;

    /**
     * Build a compute shader for the given format
     * nullptr on error
     */
    static std::unique_ptr<ComputeShader> buildComputeShader(GLenum textureFormat);

public:
    /**
     * Create a new TextureComparer
     * nullptr on error
     */
    static std::unique_ptr<TextureComparer> create();

    /**
     * No copying
     */
    TextureComparer(TextureComparer &other) = delete;
    TextureComparer& operator=(TextureComparer &other) = delete;

    /**
     * Compare and update cachedBlit with freshBlit
     * within the localDirtyRegion (in GL coords)
     *
     * The EffectWindow is optional and only used
     * for extra logging in the debug build (BBDX_DEBUG)
     *
     * The result of the comparison can be found using the
     * query object returned by queryObject()
     */
    void compareAndUpdate(const WindowData *windowData, KWin::GLTexture *freshBlit, KWin::GLTexture *cachedBlit, const KWin::Region &localDirtyRegionGL, const KWin::EffectWindow *window = nullptr);
};

}
