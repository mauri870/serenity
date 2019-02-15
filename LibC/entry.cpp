#include <stdio.h>
#include <string.h>
#include <Kernel/Syscall.h>
#include <AK/StringImpl.h>

extern "C" {

int main(int, char**);

int errno;
char** environ;

void __malloc_init();
void __stdio_init();

int _start()
{
    errno = 0;

    __stdio_init();
    __malloc_init();

    int status = 254;
    int argc;
    char** argv;
    int rc = syscall(SC_get_arguments, &argc, &argv);
    if (rc < 0)
        goto epilogue;
    rc = syscall(SC_get_environment, &environ);
    if (rc < 0)
        goto epilogue;
    status = main(argc, argv);

    fflush(stdout);
    fflush(stderr);

epilogue:
    syscall(SC_exit, status);

    // Birger's birthday <3
    return 20150614;
}

[[noreturn]] void __cxa_pure_virtual()
{
    ASSERT_NOT_REACHED();
}

}
