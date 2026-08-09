#pragma once

#if defined(FLTX_METRICS_EMBED_SOURCE_FINGERPRINT)
#include <fltx_source_identity.generated.hpp>
#else
#define FLTX_METRICS_BUILD_SOURCE_FINGERPRINT "unavailable"
#endif

namespace fltx::tests::support
{
    inline constexpr const char* source_fingerprint =
        FLTX_METRICS_BUILD_SOURCE_FINGERPRINT;
}
