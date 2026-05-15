#pragma once

#include "kwin_version.hpp"

#include <effect/effectwindow.h>
#include <opengl/glframebuffer.h>
#include <opengl/glshader.h>
#include <opengl/gltexture.h>
#include <opengl/glvertexbuffer.h>

#if KWIN_VERSION < KWIN_VERSION_CODE(6, 5, 80)
#  include "kwin_compat_6_5.hpp"
#else
#  include <core/rect.h>
#endif

#include <memory>
#include <optional>
#include <vector>

namespace KWin {
    struct BlurRenderData;
    class GLVertex2D;
}

namespace BBDX {

/**
 * A single valid entry
 */
struct BlurCacheEntry {
    // texture and framebuffer for the cache
    // with the size of scaledBackgroundRect from BlurEffect::blur()
    std::unique_ptr<KWin::GLTexture> cachedTexture{nullptr};
    std::unique_ptr<KWin::GLFramebuffer> cachedFramebuffer{nullptr};

    // texture of the previous "raw" pixels (blit grabbed from scene)
    // used to create this cache entry
    std::unique_ptr<KWin::GLTexture> blitTexture{nullptr};

    // priority index, lower meaning higher priority
    uint priority{0};

    // cache hits of this entry, incremented by BlurCacheLRU::select()
    uint hits{0};

    /**
     * Create a new BlurCacheEntry by allocating cachedTexture and cachedFramebuffer
     * blitTexture is cloned from the provided blitFramebuffer.
     */
    explicit BlurCacheEntry(const KWin::Rect &scaledBackgroundRect, GLenum textureFormat, KWin::GLFramebuffer *blitFramebuffer);
};

/**
 * Least Recently Used container
 * for BlurCacheEntry
 */
class BlurCacheLRU {
private:
    std::vector<std::unique_ptr<BlurCacheEntry>> m_entries{};
    size_t m_max{5};
    size_t m_next{0};
    BlurCacheEntry* m_valid{nullptr};

    KWin::EffectWindow* m_window{nullptr};
    QString m_windowClass{"unknown unknown"};
    pid_t m_windowPID{0};

public:
    explicit BlurCacheLRU(size_t max = 5)
        : m_max{max}
    {
        m_entries.reserve(max);
    }

    /**
     * Move current index back to start and unset m_valid
     */
    void reset();

    /**
     * Return a pointer to the next entry or nullptr
     * if none exists (empty or already past final) or one was already selected
     */
    const BlurCacheEntry* next();

    /**
     * Select does the following:
     *  - acknowledge the current cache entry was a hit (set pointer returned in valid())
     *  - move index back to the start
     *  - increase priority index
     */
    void select();

    /**
     * Add an entry to the cache, potentially removing the oldest entry.
     * The added entry is assumed to be valid and will always be selected.
     */
    void add(std::unique_ptr<BlurCacheEntry> entry);

    /**
     * If a valid cache entry was selected get a pointer to it, else nullptr
     */
    BlurCacheEntry* valid() { return m_valid; }

    /**
     * Explicitly clear all cache entries
     * and print sats to debug log
     */
    void invalidate(QStringView reason);

    /**
     * Set window using this cache for logging purposes
     * Locked once set
     */
    void setWindow(KWin::EffectWindow* w);
};

/**
 * Blur cache data unique to each EffectWindow and RenderView combination
 */
struct BlurCacheData {
    // cache entries
    BlurCacheLRU lru{};
};

class BlurCache {
private:
    struct {
        std::unique_ptr<KWin::GLShader> shader;
        int mvpMatrixLocation;
        int texUnitOldLocation;
        int texUnitNewLocation;
        int halfpixelLocation;
        int borderIgnore;
    } m_textureComparePass;

    struct {
        std::unique_ptr<KWin::GLShader> shader;
        int mvpMatrixLocation;
    } m_texturePass;

    bool m_glQueryAvailable{true};

public:
    /**
     * Loads and sets up shaders
     */
    explicit BlurCache();

    /**
     * Check if pass is ready i.e. all shaders loaded
     */
    bool ready() const { return !!m_texturePass.shader; }

    /**
     * Updates the BlurCacheData buffers of the given renderInfo
     */
    void updateBlurCacheDataBuffers(KWin::BlurRenderData &renderInfo, const KWin::Rect &scaledBackgroundRect, GLenum textureFormat) const;

    /**
     * Select a cache entry from renderInfo if a valid one exists
     */
    void selectCacheEntry(KWin::BlurRenderData &renderInfo, KWin::GLVertexBuffer *vbo);

    /**
     * Injects the geometry used for the cache, in logical pixels
     * but scaled to what would be drawn on the device.
     *
     * Always adds 6 vertices
     */
    void setupVBO(const KWin::Rect &scaledBackgroundRect, std::span<KWin::GLVertex2D> &map, size_t &vboIndex) const;

    /**
     * Draw the cached texture
     */
    void drawCached(const KWin::Rect &scaledBackgroundRect, const KWin::RenderViewport &viewport, KWin::BlurRenderData &renderInfo, KWin::GLVertexBuffer *vbo, const int vertexCount, const float modulation) const;

    /**
     * vbo->draw() wrapper to draw into BlurCacheData of the provided renderInfo
     */
    void drawToCache(KWin::BlurRenderData &renderInfo, KWin::GLVertexBuffer *vbo) const;
};

} // namespace BBDX
