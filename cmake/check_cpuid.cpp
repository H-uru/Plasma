#if defined(_MSC_VER) || ((defined(_WIN32) || defined(_WIN64)) && defined(__INTEL_COMPILER))
#  include <intrin.h>
#elif defined(__GNUC__)
#  include <cpuid.h>
#endif

/* Just needed to look for the headers -- this just makes the compiler happy. */
int main(int argc, char *argv[])
{
    return 0;
}
