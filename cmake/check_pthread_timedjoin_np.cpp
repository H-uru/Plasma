#include <pthread.h>
#include <time.h>

int main()
{
    pthread_t thread;
    struct timespec ts{};
    return pthread_timedjoin_np(thread, nullptr, &ts);
}
