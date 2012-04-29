#ifdef Q_WIN
#include "lusb0_usb_win.h"
#else
#include <usb.h>
#endif

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
typedef int (*usb_bulk_write_t)(usb_dev_handle *dev, int ep, char const *bytes, int size,
                   int timeout);
typedef int (*usb_bulk_read_t)(usb_dev_handle *dev, int ep, char *bytes, int size,
                  int timeout);
typedef int (*usb_interrupt_write_t)(usb_dev_handle *dev, int ep, char const *bytes, int size,
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

typedef char *(*usb_strerror_t)(void);

typedef void (*usb_init_t)(void);
typedef void (*usb_set_debug_t)(int level);
typedef int (*usb_find_busses_t)(void);
typedef int (*usb_find_devices_t)(void);
typedef struct usb_device *(*usb_device_t)(usb_dev_handle *dev);
typedef struct usb_bus *(*usb_get_busses_t)(void);


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

    usb_strerror_t usb_strerror;

    usb_init_t usb_init;
    usb_set_debug_t usb_set_debug;
    usb_find_busses_t usb_find_busses;
    usb_find_devices_t usb_find_devices;
    usb_device_t usb_device;
    usb_get_busses_t usb_get_busses;
};

libusb0_methods * libusb0_dyn_init();
void libusb0_dyn_destroy(struct libusb0_methods * m);

#ifdef __cplusplus
}
#endif
