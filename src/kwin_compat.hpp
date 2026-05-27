#pragma once

/**
 * Wrapper to include all compat headers
 * matching the current build env
 */


#include "kwin_version.hpp"

#if defined(BETTERBLUR_X11)
#   include "kwin_compat_x11.hpp"
#endif

#if KWIN_VERSION < KWIN_VERSION_CODE(6, 5, 80)
#   include "kwin_compat_6_5.hpp"
#endif

#if KWIN_VERSION < KWIN_VERSION_CODE(6, 6, 80)
#   include "kwin_compat_6_6.hpp"
#endif
