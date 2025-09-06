#if defined(_MSC_VER) || ((defined(_WIN32) || defined(_WIN64)) && defined(__INTEL_COMPILER))
#  include <intrin.h>
#  define MSC_COMPATIBLE
#elif defined(__GNUC__)
#  include <cpuid.h>
#endif

int main(int argc, char *argv[])
{
#if defined(MSC_COMPATIBLE)
    // We need to test - Windows ARM64 will have the headers but not the definition
    unsigned int features[4] = {0, 0, 0, 0};
    __cpuid(features, 0);
#endif
    return 0;
}
