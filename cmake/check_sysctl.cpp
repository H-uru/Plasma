#include <sys/types.h>
#include <sys/sysctl.h>

int main()
{
    sysctl(nullptr, 0, nullptr, nullptr, nullptr, 0);
    return 0;
}
