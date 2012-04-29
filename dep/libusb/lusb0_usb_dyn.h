#include "lusb0_usb.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* usb.c */
typedef usb_dev_handle *(*usb_open_t)(struct usb_device *dev);
typedef int (*usb_close_t)(usb_dev_handle *dev);
typedef int (*usb_get_string_t)(usb_dev_handle *dev, int index, int langid, char *buf,
                   size_t buflen);
typedef int (*usb_get_string_simple_t)(usb_dev_handle *dev, int index, char *buf,
                          size_t buflen);

/* descriptors.c */
typedef int (*usb_get_descriptor_by_endpoint_t)(usb_dev_handle *udev, int ep,
                                   unsigned char type, unsigned char index,
                                   void *buf, int size);
typedef int (*usb_get_descriptor_t)(usb_dev_handle *udev, unsigned char type,
                       unsigned char index, void *buf, int size);

/* <arch>.c */
typedef int (*usb_bulk_write_t)(usb_dev_handle *dev, int ep, char *bytes, int size,
                   int timeout);
typedef int (*usb_bulk_read_t)(usb_dev_handle *dev, int ep, char *bytes, int size,
                  int timeout);
typedef int (*usb_interrupt_write_t)(usb_dev_handle *dev, int ep, char *bytes, int size,
                        int timeout);
typedef int (*usb_interrupt_read_t)(usb_dev_handle *dev, int ep, char *bytes, int size,
                       int timeout);
typedef int (*usb_control_msg_t)(usb_dev_handle *dev, int requesttype, int request,
                    int value, int index, char *bytes, int size,
                    int timeout);
typedef int (*usb_set_configuration_t)(usb_dev_handle *dev, int configuration);
typedef int (*usb_claim_interface_t)(usb_dev_handle *dev, int interface);
typedef int (*usb_release_interface_t)(usb_dev_handle *dev, int interface);
typedef int (*usb_set_altinterface_t)(usb_dev_handle *dev, int alternate);
typedef int (*usb_resetep_t)(usb_dev_handle *dev, unsigned int ep);
typedef int (*usb_clear_halt_t)(usb_dev_handle *dev, unsigned int ep);
typedef int (*usb_reset_t)(usb_dev_handle *dev);
typedef int (*usb_reset_ex_t)(usb_dev_handle *dev, unsigned int reset_type);

typedef char *(*usb_strerror_t)(void);

typedef void (*usb_init_t)(void);
typedef void (*usb_set_debug_t)(int level);
typedef int (*usb_find_busses_t)(void);
typedef int (*usb_find_devices_t)(void);
typedef struct usb_device *(*usb_device_t)(usb_dev_handle *dev);
typedef struct usb_bus *(*usb_get_busses_t)(void);


/* Windows specific functions */

typedef int (*usb_install_service_np_t)(void);
typedef void (CALLBACK* usb_install_service_np_rundll_t)(HWND wnd, HINSTANCE instance,
        LPSTR cmd_line, int cmd_show);

typedef int (*usb_uninstall_service_np_t)(void);
typedef void (CALLBACK* usb_uninstall_service_np_rundll_t)(HWND wnd, HINSTANCE instance,
        LPSTR cmd_line, int cmd_show);

typedef int (*usb_install_driver_np_t)(const char *inf_file);
typedef void (CALLBACK* usb_install_driver_np_rundll_t)(HWND wnd, HINSTANCE instance,
        LPSTR cmd_line, int cmd_show);

typedef int (*usb_touch_inf_file_np_t)(const char *inf_file);
typedef void (CALLBACK* usb_touch_inf_file_np_rundll_t)(HWND wnd, HINSTANCE instance,
        LPSTR cmd_line, int cmd_show);

typedef int (*usb_install_needs_restart_np_t)(void);

typedef int (*usb_install_npW_t)(HWND hwnd, HINSTANCE instance, LPCWSTR cmd_line, int starg_arg);
typedef int (*usb_install_npA_t)(HWND hwnd, HINSTANCE instance, LPCSTR cmd_line, int starg_arg);
typedef void (CALLBACK* usb_install_np_rundll_t)(HWND wnd, HINSTANCE instance,
        LPSTR cmd_line, int cmd_show);

typedef const struct usb_version *(*usb_get_version_t)(void);

typedef int (*usb_isochronous_setup_async_t)(usb_dev_handle *dev, void **context,
                                unsigned char ep, int pktsize);
typedef int (*usb_bulk_setup_async_t)(usb_dev_handle *dev, void **context,
                         unsigned char ep);
typedef int (*usb_interrupt_setup_async_t)(usb_dev_handle *dev, void **context,
                              unsigned char ep);

typedef int (*usb_submit_async_t)(void *context, char *bytes, int size);
typedef int (*usb_reap_async_t)(void *context, int timeout);
typedef int (*usb_reap_async_nocancel_t)(void *context, int timeout);
typedef int (*usb_cancel_async_t)(void *context);
typedef int (*usb_free_async_t)(void **context);

struct libusb0_methods
{
    /* usb.c */
    usb_open_t usb_open;
    usb_close_t usb_close;
    usb_get_string_t usb_get_string;
    usb_get_string_simple_t usb_get_string_simple;

    /* descriptors.c */
    usb_get_descriptor_by_endpoint_t usb_get_descriptor_by_endpoint;
    usb_get_descriptor_t usb_get_descriptor;

    /* <arch>.c */
    usb_bulk_write_t usb_bulk_write;
    usb_bulk_read_t usb_bulk_read;
    usb_interrupt_write_t usb_interrupt_write;
    usb_interrupt_read_t usb_interrupt_read;
    usb_control_msg_t usb_control_msg;
    usb_set_configuration_t usb_set_configuration;
    usb_claim_interface_t usb_claim_interface;
    usb_release_interface_t usb_release_interface;
    usb_set_altinterface_t usb_set_altinterface;
    usb_resetep_t usb_resetep;
    usb_clear_halt_t usb_clear_halt;
    usb_reset_t usb_reset;
    usb_reset_ex_t usb_reset_ex;

    usb_strerror_t usb_strerror;

    usb_init_t usb_init;
    usb_set_debug_t usb_set_debug;
    usb_find_busses_t usb_find_busses;
    usb_find_devices_t usb_find_devices;
    usb_device_t usb_device;
    usb_get_busses_t usb_get_busses;


    /* Windows specific functions */

    usb_install_service_np_t usb_install_service_np;
    usb_install_service_np_rundll_t usb_install_service_np_rundll;

    usb_uninstall_service_np_t usb_uninstall_service_np;
    usb_uninstall_service_np_rundll_t usb_uninstall_service_np_rundll;

    usb_install_driver_np_t usb_install_driver_np;
    usb_install_driver_np_rundll_t usb_install_driver_np_rundll;

    usb_touch_inf_file_np_t usb_touch_inf_file_np;
    usb_touch_inf_file_np_rundll_t usb_touch_inf_file_np_rundll;

    usb_install_needs_restart_np_t usb_install_needs_restart_np;

    usb_install_npW_t usb_install_npW;
    usb_install_npA_t usb_install_npA;
    usb_install_np_rundll_t usb_install_np_rundll;

    usb_get_version_t usb_get_version;

    usb_isochronous_setup_async_t usb_isochronous_setup_async;
    usb_bulk_setup_async_t usb_bulk_setup_async;
    usb_interrupt_setup_async_t usb_interrupt_setup_async;

    usb_submit_async_t usb_submit_async;
    usb_reap_async_t usb_reap_async;
    usb_reap_async_nocancel_t usb_reap_async_nocancel;
    usb_cancel_async_t usb_cancel_async;
    usb_free_async_t usb_free_async;
};

libusb0_methods * libusb0_dyn_init();
void libusb0_dyn_destroy(struct libusb0_methods * m);

#ifdef __cplusplus
}
#endif
