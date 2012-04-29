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
#define X_OPT(n) m->n = (n##_t)GetProcAddress(hLib, #n)
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
    X_OPT(usb_reset_ex);

    X(usb_strerror);

    X(usb_init);
    X(usb_set_debug);
    X(usb_find_busses);
    X(usb_find_devices);
    X(usb_device);
    X(usb_get_busses);


    /* Windows specific functions */

    X(usb_install_service_np);
    X(usb_install_service_np_rundll);

    X(usb_uninstall_service_np);
    X(usb_uninstall_service_np_rundll);

    X(usb_install_driver_np);
    X(usb_install_driver_np_rundll);

    X(usb_touch_inf_file_np);
    X(usb_touch_inf_file_np_rundll);

    X(usb_install_needs_restart_np);

    X(usb_install_npW);
    X(usb_install_npA);
    X(usb_install_np_rundll);

    X(usb_get_version);

    X(usb_isochronous_setup_async);
    X(usb_bulk_setup_async);
    X(usb_interrupt_setup_async);

    X(usb_submit_async);
    X(usb_reap_async);
    X(usb_reap_async_nocancel);
    X(usb_cancel_async);
    X(usb_free_async);

#undef X
#undef X_OPT

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
