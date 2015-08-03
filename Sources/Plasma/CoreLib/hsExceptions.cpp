#include "hsExceptions.h"

#include <cstring>

const char *hsException::what() const HS_NOEXCEPT
{
    char buffer[64];

    static const char *kErrorNames[] = {
        "kNo_hsError",
        "kNilParam_hsError",
        "kBadParam_hsError",
        "kInternal_hsError",
        "kOS_hsError"
    };
    static_assert(arrsize(kErrorNames) == hsErrorEnum_MAX,
        "kErrorNames not in sync with hsErrorEnum");

    if (fError >= 0 && fError < hsErrorEnum_MAX)
        snprintf(buffer, arrsize(buffer), "%s (%ld)", kErrorNames[fError], fParam);
    else
        snprintf(buffer, arrsize(buffer), "Unknown hsException error %d (%ld)", fError, fParam);

    return buffer;
}
