/*
 * File containing functions only present in
 * Better Blur DX
 */


#include "blur.h"
#include "window.hpp"

#include <effect/effectwindow.h>
#include <scene/borderradius.h>

#include <KDecoration3/Decoration>

#include <QRegion>
#include <QEasingCurve>


namespace KWin
{

void BlurEffect::slotWindowWantsBlurRegionUpdate(EffectWindow *w) {
    updateBlurRegion(w);
}

void BlurEffect::slotWindowInvalidatedBlurCache(EffectWindow *w) {
    if (auto it = m_windows.find(w); it != m_windows.end()) {
        BlurEffectData &blurInfo = it->second;
        for (auto &[_, renderInfo] : blurInfo.render) {
            renderInfo.cache.valid = false;
        }
    }
}

}
