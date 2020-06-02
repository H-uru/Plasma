#include "hsExceptions.h"

#include <cstring>
#include <iterator>

hsException::hsException(hsErrorEnum error, long param) noexcept
    : fError(error), fParam(param)
{
    static const char *kErrorNames[] = {
        "kNo_hsError",
        "kNilParam_hsError",
        "kBadParam_hsError",
        "kInternal_hsError",
        "kOS_hsError"
    };
    static_assert(std::size(kErrorNames) == hsErrorEnum_MAX,
        "kErrorNames not in sync with hsErrorEnum");

    if (fError >= 0 && fError < hsErrorEnum_MAX)
        snprintf(fWhat, std::size(fWhat), "%s (%ld)", kErrorNames[fError], fParam);
    else
        snprintf(fWhat, std::size(fWhat), "Unknown hsException error %d (%ld)", fError, fParam);
}
