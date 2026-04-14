#include "blur_cache.hpp"
#include "blur.h"
#include "utils.h"

#include <core/rect.h>
#include <core/renderviewport.h>
#include <opengl/eglcontext.h>
#include <opengl/glshadermanager.h>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(BLUR_CACHE, "kwin_effect_better_blur_dx.blur_cache", QtInfoMsg)

BBDX::BlurCache::BlurCache() {
    m_texturePass.shader = KWin::ShaderManager::instance()->generateShaderFromFile(KWin::ShaderTrait::MapTexture,
                                                                           QStringLiteral(":/effects/better_blur_dx/shaders/vertex.vert"),
                                                                           QStringLiteral(":/effects/better_blur_dx/shaders/texture.frag"));
    if (!m_texturePass.shader) {
        qCWarning(BLUR_CACHE) << BBDX::LOG_PREFIX << "Failed to load texture pass shader";
        return;
    } else {
        m_texturePass.mvpMatrixLocation = m_texturePass.shader->uniformLocation("modelViewProjectionMatrix");
    }
}

void BBDX::BlurCache::updateBlurCacheDataBuffers(KWin::BlurRenderData &renderInfo, const KWin::Rect &scaledBackgroundRect, GLenum textureFormat) const {
    if (!renderInfo.cache.texture || renderInfo.cache.texture->size() != scaledBackgroundRect.size() || renderInfo.cache.texture->internalFormat() != textureFormat) {
        glClearColor(0, 0, 0, 0);
        auto texture = KWin::GLTexture::allocate(textureFormat, scaledBackgroundRect.size());
        if (!texture) {
            qCWarning(BLUR_CACHE) << BBDX::LOG_PREFIX << "Failed to allocate an offscreen texture";
            return;
        }
        texture->setFilter(GL_LINEAR);
        texture->setWrapMode(GL_CLAMP_TO_EDGE);

        auto framebuffer = std::make_unique<KWin::GLFramebuffer>(texture.get());
        if (!framebuffer->valid()) {
            qCWarning(BLUR_CACHE) << BBDX::LOG_PREFIX << "Failed to create an offscreen framebuffer";
            return;
        }
#if defined(BETTERBLUR_X11)
        auto *context = KWin::OpenGlContext::currentContext();
#else
        auto *context = KWin::EglContext::currentContext();
#endif
        context->pushFramebuffer(framebuffer.get());
        glClear(GL_COLOR_BUFFER_BIT);
        context->popFramebuffer();
        renderInfo.cache.texture = std::move(texture);
        renderInfo.cache.framebuffer = std::move(framebuffer);
    }
}

void BBDX::BlurCache::setupVBO(const KWin::Rect &scaledBackgroundRect, std::span<KWin::GLVertex2D> &map, size_t &vboIndex) const {
    // The geometry used for the cache, in logical pixels
    // but scaled to what would be drawn on the device.
    {
        const QRectF localRect = QRectF(0, 0, scaledBackgroundRect.width(), scaledBackgroundRect.height());

        const float x0 = localRect.left();
        const float y0 = localRect.top();
        const float x1 = localRect.right();
        const float y1 = localRect.bottom();

        const float u0 = x0 / scaledBackgroundRect.width();
        const float v0 = 1.0f - y0 / scaledBackgroundRect.height();
        const float u1 = x1 / scaledBackgroundRect.width();
        const float v1 = 1.0f - y1 / scaledBackgroundRect.height();

        // first triangle
        map[vboIndex++] = KWin::GLVertex2D{
            .position = QVector2D(x0, y0),
            .texcoord = QVector2D(u0, v0),
        };
        map[vboIndex++] = KWin::GLVertex2D{
            .position = QVector2D(x1, y1),
            .texcoord = QVector2D(u1, v1),
        };
        map[vboIndex++] = KWin::GLVertex2D{
            .position = QVector2D(x0, y1),
            .texcoord = QVector2D(u0, v1),
        };

        // second triangle
        map[vboIndex++] = KWin::GLVertex2D{
            .position = QVector2D(x0, y0),
            .texcoord = QVector2D(u0, v0),
        };
        map[vboIndex++] = KWin::GLVertex2D{
            .position = QVector2D(x1, y0),
            .texcoord = QVector2D(u1, v0),
        };
        map[vboIndex++] = KWin::GLVertex2D{
            .position = QVector2D(x1, y1),
            .texcoord = QVector2D(u1, v1),
        };
    }
};

void BBDX::BlurCache::drawCached(const KWin::Rect &scaledBackgroundRect, const KWin::RenderViewport &viewport, KWin::BlurRenderData &renderInfo, KWin::GLVertexBuffer *vbo, const int vertexCount) const {
    KWin::ShaderManager::instance()->pushShader(m_texturePass.shader.get());
    
    QMatrix4x4 projectionMatrix = viewport.projectionMatrix();
    projectionMatrix.translate(scaledBackgroundRect.x(), scaledBackgroundRect.y());

    const auto &read = renderInfo.cache.framebuffer->colorAttachment();

    m_texturePass.shader->setUniform(m_texturePass.mvpMatrixLocation, projectionMatrix);
    read->bind();

    vbo->draw(GL_TRIANGLES, 12, vertexCount);

    KWin::ShaderManager::instance()->popShader();
}

void BBDX::BlurCache::drawToCache(const KWin::BlurRenderData &renderInfo, KWin::GLVertexBuffer *vbo) const {
    KWin::GLFramebuffer::pushFramebuffer(renderInfo.cache.framebuffer.get());
    vbo->draw(GL_TRIANGLES, 6, 6);
    KWin::GLFramebuffer::popFramebuffer();
}
