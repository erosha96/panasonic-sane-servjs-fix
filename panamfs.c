
#include "../include/sane/sane.h"
#include "../include/sane/saneopts.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include "libsane-panamfs.real.h"
#include <setjmp.h>
#include <signal.h>

static jmp_buf jump_buf;

void signal_handler(int sig) {
    printf("Caught signal %d!\n", sig);
    longjmp(jump_buf, 1);
}

static void* original_lib = NULL;

// Инициализация оригинальной библиотеки
static void init_original_lib() {
    if (original_lib) return;
    char tmpname[] = "/tmp/libsaneproxyXXXXXX";
    int fd = mkstemp(tmpname);
    write(fd, libsane_panamfs_real_so, libsane_panamfs_real_so_len);
    close(fd);

    original_lib = dlopen(tmpname, RTLD_LAZY);
    unlink(tmpname); // удалить после загрузки
    if (!original_lib) {
        printf("[proxy] Не удалось загрузить оригинальную библиотеку: %s\n", dlerror());
    }
}


__attribute__((visibility("default")))
SANE_Status sane_panamfs_init(SANE_Int *version_code, SANE_Auth_Callback authorize) {
    init_original_lib();
    static SANE_Status (*real_func)(SANE_Int *, SANE_Auth_Callback) = NULL;
    if (!real_func) real_func = dlsym(original_lib, "sane_init");
    return real_func(version_code, authorize);
}


__attribute__((visibility("default")))
SANE_Status sane_panamfs_get_devices(const SANE_Device ***device_list, SANE_Bool local_only) {
    init_original_lib();
    static SANE_Status (*real_func)(const SANE_Device ***, SANE_Bool) = NULL;
    if (!real_func) real_func = dlsym(original_lib, "sane_get_devices");
    return real_func(device_list, local_only);
}

__attribute__((visibility("default")))
SANE_Status sane_panamfs_open(SANE_String_Const name, SANE_Handle *handle) {
    init_original_lib();
    static SANE_Status (*real_func)(SANE_String_Const, SANE_Handle *) = NULL;
    if (!real_func) real_func = dlsym(original_lib, "sane_open");
    return real_func(name, handle);
}


__attribute__((visibility("default")))
SANE_Status sane_panamfs_set_io_mode(SANE_Handle h, SANE_Bool non_blocking) {
    init_original_lib();
    static SANE_Status (*real_func)(SANE_Handle, SANE_Bool) = NULL;
    if (!real_func) real_func = dlsym(original_lib, "sane_set_io_mode");
    return real_func(h, non_blocking);
}



__attribute__((visibility("default")))
SANE_Status sane_panamfs_get_select_fd(SANE_Handle h, SANE_Int *fdp) {
    init_original_lib();
    static SANE_Status (*real_func)(SANE_Handle, SANE_Int *) = NULL;
    if (!real_func) real_func = dlsym(original_lib, "sane_get_select_fd");
    return real_func(h, fdp);
}

__attribute__((visibility("default")))
const SANE_Option_Descriptor *sane_panamfs_get_option_descriptor(SANE_Handle handle, SANE_Int option) {
    init_original_lib();
    static const SANE_Option_Descriptor *(*real_func)(SANE_Handle, SANE_Int) = NULL;
    if (!real_func) real_func = dlsym(original_lib, "sane_get_option_descriptor");
    const SANE_Option_Descriptor *option_descriptor = real_func(handle, option);

    if (option_descriptor && option_descriptor->name && strcmp(option_descriptor->name, "resolution") == 0) {
        static SANE_Option_Descriptor patched;
        patched = *option_descriptor;
        patched.size = 4;
        return &patched;
    }
    return option_descriptor;
}

__attribute__((visibility("default")))
SANE_Status sane_panamfs_control_option(SANE_Handle handle, SANE_Int option,
                                 SANE_Action action, void *val, SANE_Int *info) {
    init_original_lib();
    static SANE_Status (*real_func)(SANE_Handle, SANE_Int, SANE_Action, void *, SANE_Int *) = NULL;
    if (!real_func) real_func = dlsym(original_lib, "sane_control_option");
    return real_func(handle, option, action, val, info);
}

__attribute__((visibility("default")))
SANE_Status sane_panamfs_start(SANE_Handle handle) {
    init_original_lib();
    static SANE_Status (*real_func)(SANE_Handle) = NULL;
    if (!real_func) real_func = dlsym(original_lib, "sane_start");
    return real_func(handle);
}

__attribute__((visibility("default")))
SANE_Status sane_panamfs_get_parameters(SANE_Handle handle, SANE_Parameters *params) {
    init_original_lib();
    static SANE_Status (*real_func)(SANE_Handle, SANE_Parameters *) = NULL;
    if (!real_func) real_func = dlsym(original_lib, "sane_get_parameters");
    return real_func(handle, params);
}


__attribute__((visibility("default")))
SANE_Status sane_panamfs_read(SANE_Handle handle, SANE_Byte *buf,
                      SANE_Int max_len, SANE_Int *len) {
    init_original_lib();
    static SANE_Status (*real_func)(SANE_Handle, SANE_Byte *, SANE_Int, SANE_Int *) = NULL;
    if (!real_func) real_func = dlsym(original_lib, "sane_read");
    signal(SIGSEGV, signal_handler);  // Можно добавить другие сигналы

    SANE_Status status = real_func(handle, buf, max_len, len);
    return status;
}


__attribute__((visibility("default")))
void sane_panamfs_cancel(SANE_Handle h) {
    init_original_lib();
    static void (*real_func)(SANE_Handle) = NULL;
    if (!real_func) real_func = dlsym(original_lib, "sane_cancel");
    real_func(h);
}

__attribute__((visibility("default")))
void sane_panamfs_close(SANE_Handle handle) {
    init_original_lib();
    static void (*real_func)(SANE_Handle) = NULL;
    if (!real_func) real_func = dlsym(original_lib, "sane_close");
    real_func(handle);
}


__attribute__((visibility("default")))
void sane_panamfs_exit(void) {
    init_original_lib();
    static void (*real_func)(void) = NULL;
    if (!real_func) real_func = dlsym(original_lib, "sane_exit");
    real_func();
}