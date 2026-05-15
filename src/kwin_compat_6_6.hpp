#pragma once

#include "kwin_version.hpp"
#if KWIN_VERSION < KWIN_VERSION_CODE(6, 5, 80)
#  include "kwin_compat_6_5.hpp"
#else
#  include <core/region.h>
#endif


// Compatibility bits for "upgrading"
// KWin 6.6 API to 6.7

namespace KWin {
    /**
     * 6.7 introduced a floating point version of Region
     * older version will have to deal with rounding errors
     */
    using RegionF = Region;
}
