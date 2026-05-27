#include "rounded_corners_pass.hpp"

#include "blur.h"
#include "kwin_compat.hpp"
#include "utils.h"

#if KWIN_VERSION >= KWIN_VERSION_CODE(6, 5, 80)
#  include <core/rect.h>
#  include <core/region.h>
#endif

#include <core/pixelgrid.h>
#include <core/renderviewport.h>
#include <effect/effect.h>
#include <effect/effectwindow.h>
#include <epoxy/gl_generated.h>
#include <opengl/glshader.h>
#include <opengl/glshadermanager.h>
#include <scene/borderradius.h>

#include <QLoggingCategory>
#include <QRect>
#include <QImage>


Q_LOGGING_CATEGORY(ROUNDED_CORNERS_PASS, "kwin_effect_better_blur_dx.rounded_corners_pass", QtInfoMsg)

BBDX::RoundedCornersPass::RoundedCornersPass() {
    m_shader = KWin::ShaderManager::instance()->generateShaderFromFile(
            KWin::ShaderTrait::MapTexture,
            QStringLiteral(":/effects/better_blur_dx/shaders/rounded_corners.vert"),
            QStringLiteral(":/effects/better_blur_dx/shaders/rounded_corners.frag"));

    if (!m_shader) {
        qCWarning(ROUNDED_CORNERS_PASS) << BBDX::LOG_PREFIX << "Failed to load rounded corners pass shader";
        return;
    } else {
        m_mvpMatrixLocation = m_shader->uniformLocation("modelViewProjectionMatrix");
        m_boxLocation = m_shader->uniformLocation("box");
        m_cornerRadiusLocation = m_shader->uniformLocation("cornerRadius");
    }
}

void BBDX::RoundedCornersPass::apply(const KWin::BorderRadius &cornerRadius,
                                     const KWin::RenderViewport &viewport,
                                     const QRect &scaledBackgroundRect,
                                     BBDX::BlurRenderData &renderInfo,
                                     const KWin::EffectWindow *w,
                                     const KWin::WindowPaintData &data,
                                     KWin::GLVertexBuffer *vbo,
                                     const BBDX::BlurCache *blurCache) const {
        if (!ready()) [[unlikely]] {
            return;
        }

        KWin::ShaderManager::instance()->pushShader(m_shader.get());

        QMatrix4x4 projectionMatrix;
        projectionMatrix.ortho(QRectF(0.0, 0.0, scaledBackgroundRect.width(), scaledBackgroundRect.height()));

        // should contain the raw un-blurred pixels
        const auto &read = renderInfo.cache.get()->blitFramebuffer;

        const KWin::RectF transformedRect = KWin::RectF{
            w->frameGeometry().x() + data.xTranslation(),
            w->frameGeometry().y() + data.yTranslation(),
            w->frameGeometry().width() * data.xScale(),
            w->frameGeometry().height() * data.yScale(),
        };
# if KWIN_VERSION < KWIN_VERSION_CODE(6, 6, 90)
        const KWin::RectF nativeBox = KWin::snapToPixelGridF(KWin::scaledRect(transformedRect, viewport.scale()))
                                .translated(-scaledBackgroundRect.topLeft());
#else
        const KWin::RectF nativeBox = transformedRect
                                    .scaled(viewport.scale())
                                    .rounded()
                                    .translated(-scaledBackgroundRect.topLeft());
#endif
        const KWin::BorderRadius nativeCornerRadius = cornerRadius.scaled(viewport.scale()).rounded();

        m_shader->setUniform(m_mvpMatrixLocation, projectionMatrix);
#if KWIN_VERSION < KWIN_VERSION_CODE(6, 6, 90)
        m_shader->setUniform(m_boxLocation, QVector4D(nativeBox.x() + nativeBox.width() * 0.5, nativeBox.y() + nativeBox.height() * 0.5, nativeBox.width() * 0.5, nativeBox.height() * 0.5));
#else
        m_shader->setUniform(m_boxLocation, QVector4D(nativeBox.horizontalCenter(), nativeBox.verticalCenter(), nativeBox.width() * 0.5, nativeBox.height() * 0.5));
#endif
        m_shader->setUniform(m_cornerRadiusLocation, nativeCornerRadius.toVector());

        BBDX::setTextureSwizzle(read->colorAttachment());
        read->colorAttachment()->bind();

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        blurCache->drawToCache(renderInfo, vbo);

        glDisable(GL_BLEND);

        KWin::ShaderManager::instance()->popShader();
}
