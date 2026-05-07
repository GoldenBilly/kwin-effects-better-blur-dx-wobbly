#include "blurconfig.h"
#include "refraction_pass.hpp"
#include "utils.h"

#include <opengl/glshader.h>
#include <opengl/glshadermanager.h>

#include <QLoggingCategory>
#include <QRect>

#include <algorithm>

Q_LOGGING_CATEGORY(REFRACTION_PASS, "kwin_effect_better_blur_dx.refraction_pass", QtInfoMsg)

BBDX::RefractionPass::RefractionPass() {
    // The vertex shaders should always be the one of the
    // respective contrast pass.
    // The refraction uses a modified version of
    // the contrast fragment shader.

    m_rectangular.shader = KWin::ShaderManager::instance()->generateShaderFromFile(
            KWin::ShaderTrait::MapTexture,
            QStringLiteral(":/effects/better_blur_dx/shaders/vertex.vert"),
            QStringLiteral(":/effects/better_blur_dx/shaders/refraction.frag"));

    if (!m_rectangular.shader) {
        qCWarning(REFRACTION_PASS) << BBDX::LOG_PREFIX << "Failed to load refraction pass shader";
        return;
    } else {
        // contrast parameters
        m_rectangular.mvpMatrixLocation = m_rectangular.shader->uniformLocation("modelViewProjectionMatrix");
        m_rectangular.colorMatrixLocation = m_rectangular.shader->uniformLocation("colorMatrix");
        m_rectangular.offsetLocation = m_rectangular.shader->uniformLocation("offset");
        m_rectangular.halfpixelLocation = m_rectangular.shader->uniformLocation("halfpixel");
        // refraction parameters
        m_rectangular.refractionRectSizeLocation = m_rectangular.shader->uniformLocation("refractionRectSize");
        m_rectangular.refractionEdgeSizePixelsLocation = m_rectangular.shader->uniformLocation("refractionEdgeSizePixels");
        m_rectangular.refractionCornerRadiusPixelsLocation = m_rectangular.shader->uniformLocation("refractionCornerRadiusPixels");
        m_rectangular.refractionStrengthLocation = m_rectangular.shader->uniformLocation("refractionStrength");
        m_rectangular.refractionNormalPowLocation = m_rectangular.shader->uniformLocation("refractionNormalPow");
        m_rectangular.refractionRGBFringingLocation = m_rectangular.shader->uniformLocation("refractionRGBFringing");
        m_rectangular.refractionTextureRepeatModeLocation = m_rectangular.shader->uniformLocation("refractionTextureRepeatMode");
        m_rectangular.refractionModeLocation = m_rectangular.shader->uniformLocation("refractionMode");
    }
}

void BBDX::RefractionPass::reconfigure() {
    auto config = KWin::BlurConfig::self();

    if (!config) {
        qCWarning(REFRACTION_PASS) << BBDX::LOG_PREFIX
                                   << "RefractionPass::reconfigure() called before BlurConfig::read()";
        return;
    }

    // mark enabled if strength > 0
    m_enabled = config->refractionStrength() > 0;

    // scaled up by 10.0
    constexpr double scaleEdgeSizePixels{10.0};
    m_edgeSizePixels = static_cast<double>(config->refractionEdgeSize()) * scaleEdgeSizePixels;

    // snapped to nearest step
    constexpr double maxCornerRadiusPixels{200.0};
    constexpr double steps{30.0};
    constexpr double stepSize{maxCornerRadiusPixels / steps}; // ~6.6667, matching step size from blur_config.ui
    const double snapped{std::round(static_cast<double>(config->refractionCornerRadius()) / stepSize) * stepSize};
    m_cornerRadiusPixels = snapped;

    // expects range 0.0-1.0
    // max value from blur_config.ui
    constexpr double maxStrength{30.0};
    m_strength = static_cast<double>(config->refractionStrength()) / maxStrength;

    // XXX: why scaled down?
    constexpr double scaleNormalPow{0.5};
    m_normalPow = static_cast<double>(config->refractionNormalPow()) * scaleNormalPow;

    // expects range 0.0-1.0
    // max value from blur_config.ui
    constexpr double maxRGBFringing{30.0};
    m_RGBFringing = static_cast<double>(config->refractionRGBFringing()) / maxRGBFringing;

    // integer mode selectors
    m_textureRepeatMode = config->refractionTextureRepeatMode();
    m_mode = config->refractionMode();
}

bool BBDX::RefractionPass::pushShaderRectangular() const {
    if (!enabled())
        return false;

    KWin::ShaderManager::instance()->pushShader(m_rectangular.shader.get());

    return true;
}


bool BBDX::RefractionPass::setParametersRectangular(const QMatrix4x4 &projectionMatrix,
                                         const QMatrix4x4 &colorMatrix,
                                         const QVector2D &halfpixel,
                                         const float offset,
                                         const QRect &scaledBackgroundRect) const {

    if (!enabled())
        return false;

    // contrast parameters
    m_rectangular.shader->setUniform(m_rectangular.mvpMatrixLocation, projectionMatrix);
    m_rectangular.shader->setUniform(m_rectangular.colorMatrixLocation, colorMatrix);
    m_rectangular.shader->setUniform(m_rectangular.halfpixelLocation, halfpixel);
    m_rectangular.shader->setUniform(m_rectangular.offsetLocation, offset);
    // refraction parameters
    m_rectangular.shader->setUniform(m_rectangular.refractionRectSizeLocation,
                                     QVector2D(scaledBackgroundRect.width(), scaledBackgroundRect.height()));
    m_rectangular.shader->setUniform(m_rectangular.refractionEdgeSizePixelsLocation,
                                     std::min(static_cast<float>(m_edgeSizePixels),
                                              static_cast<float>(std::min(scaledBackgroundRect.width() / 2,
                                                                          scaledBackgroundRect.height() / 2))));
    m_rectangular.shader->setUniform(m_rectangular.refractionCornerRadiusPixelsLocation, static_cast<float>(m_cornerRadiusPixels));
    m_rectangular.shader->setUniform(m_rectangular.refractionStrengthLocation, static_cast<float>(m_strength));
    m_rectangular.shader->setUniform(m_rectangular.refractionNormalPowLocation, static_cast<float>(m_normalPow));
    m_rectangular.shader->setUniform(m_rectangular.refractionRGBFringingLocation, static_cast<float>(m_RGBFringing));
    m_rectangular.shader->setUniform(m_rectangular.refractionTextureRepeatModeLocation, m_textureRepeatMode);
    m_rectangular.shader->setUniform(m_rectangular.refractionModeLocation, m_mode);

    return true;
}
