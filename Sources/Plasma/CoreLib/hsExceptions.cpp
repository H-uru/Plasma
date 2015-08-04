#include "hsExceptions.h"

#include <cstring>

hsException::hsException(hsErrorEnum error, long param) HS_NOEXCEPT
    : fError(error), fParam(param)
{
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
        snprintf(fWhat, arrsize(fWhat), "%s (%ld)", kErrorNames[fError], fParam);
    else
        snprintf(fWhat, arrsize(fWhat), "Unknown hsException error %d (%ld)", fError, fParam);
}
