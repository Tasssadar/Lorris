#include "lusb0_usb_dyn.h"
#include <windows.h>

struct libusb0_methods * libusb0_dyn_init()
{
    HMODULE hLib = LoadLibraryW(L"libusb0.dll");
    libusb0_methods * m = 0;

    if (!hLib)
        return 0;

    m = (libusb0_methods *)malloc(sizeof(libusb0_methods));

#define X(n) m->n = (n##_t)GetProcAddress(hLib, #n); if (!m->n) goto error
    X(usb_open);
    X(usb_close);
    X(usb_get_string);
    X(usb_get_string_simple);

    /* descriptors.c */
    X(usb_get_descriptor_by_endpoint);
    X(usb_get_descriptor);

    /* <arch>.c */
    X(usb_bulk_write);
    X(usb_bulk_read);
    X(usb_interrupt_write);
    X(usb_interrupt_read);
    X(usb_control_msg);
    X(usb_set_configuration);
    X(usb_claim_interface);
    X(usb_release_interface);
    X(usb_set_altinterface);
    X(usb_resetep);
    X(usb_clear_halt);
    X(usb_reset);

    X(usb_strerror);

    X(usb_init);
    X(usb_set_debug);
    X(usb_find_busses);
    X(usb_find_devices);
    X(usb_device);
    X(usb_get_busses);
#undef X

    return m;

error:
    free(m);
    FreeLibrary(hLib);
    return 0;
}

void libusb0_dyn_destroy(struct libusb0_methods * m)
{
    free(m);

    HMODULE hLib = GetModuleHandleW(L"libusb0.dll");
    FreeLibrary(hLib);
}
