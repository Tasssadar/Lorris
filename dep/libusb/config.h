/* config.h.  Manual config for MSVC.  */

/* Default visibility */
#define DEFAULT_VISIBILITY /**/

/* Debug message logging */
//#define ENABLE_DEBUG_LOGGING 1

/* Message logging */
#define ENABLE_LOGGING 1

/* Windows backend */
#define OS_WINDOWS 1

/* type of second poll() argument */
#define POLL_NFDS_TYPE unsigned int

/* no way to run git describe from MSVC? */
#define LIBUSB_DESCRIBE ""
