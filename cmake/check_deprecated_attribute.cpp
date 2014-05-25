#include <cstdio>

#if defined(TRY_ATTRIBUTE)
[[deprecated("derp_func is deprecated -- use func instead")]]
#elif defined(TRY_GCC_ATTR)
__attribute__((deprecated("derp_func is deprecated -- use func instead")))
#else
#   error "DERP"
#endif
void derp_func()
{
    // This is deprecated
    fputs("derp\n", stdout);
}

int main(int argc, char *argv[])
{
    derp_func();
    return 0;
}
