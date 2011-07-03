project( QSerialDevice )
cmake_minimum_required( VERSION 2.6.0 )
find_package( Qt4 REQUIRED )

include( ${QT_USE_FILE} )
include_directories( ${CMAKE_CURRENT_BINARY_DIR}
                     ${QT_INCLUDES} 
)

# LIB_INCLUDE_DIRS - pathes to the header files of the library,
#                    this paths is cached and useful in the future to build the /examples.
set ( LIB_INCLUDE_DIRS 
    ${PROJECT_SOURCE_DIR}/qserialdevice
    ${PROJECT_SOURCE_DIR}/qserialdeviceenumerator
    CACHE string "Paths to include library headers."
)

if(UNIX)
    set( LIB_INCLUDE_DIRS
        ${LIB_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/unix
        CACHE string "Paths to include library headers."
    )
endif()

# LIBRARY_OUTPUT_PATH - path to contain the build output binary library,
#                       this path is cached and useful in the future to build the /examples.
# LIB_TARGET - target name of result output binary library file,
#                       this name of the library is stored and cached for the build of /examples.
# All depends on whether DEBUG or RELEASE.
if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    set ( LIBRARY_OUTPUT_PATH 
        ${PROJECT_BINARY_DIR}/debug
        CACHE string "Path to output library binaries."
    )
    set( LIB_TARGET "qserialdeviced" CACHE string "Name of the output library binaries." )
else()
    set ( LIBRARY_OUTPUT_PATH 
        ${PROJECT_BINARY_DIR}/release
        CACHE string "Path to output library binaries."
    )
    set( LIB_TARGET "qserialdevice" CACHE string "Name of the output library binaries." )
endif()

# Disable Qt GUI and add definition.
set( QT_DONT_USE_QTGUI 1 )
set( QT_DEFINITIONS
    -DUNICODE
    -DQT_NO_DEBUG
    -DQT_THREAD_SUPPORT
)
add_definitions( ${QT_DEFINITIONS} )

# Library source files.
set( QSERIALDEVICE_SRCS
    qserialdevice/abstractserial.cpp
    qserialdevice/abstractserialengine.cpp
    qserialdevice/nativeserialengine.cpp
    qserialdevice/abstractserialnotifier.cpp

    qserialdeviceenumerator/serialdeviceenumerator.cpp
)

# Hooking up a platform-dependent *.h/*.cpp files   
# and include additional system libraries.
# ADDITIONAL_LIBRARY - additional platform-dependent system libraries set.
if(WIN32)
    set( QSERIALDEVICE_SRCS
        ${QSERIALDEVICE_SRCS}
        qserialdevice/nativeserialengine_win.cpp
        qserialdevice/nativeserialnotifier_win.cpp

        qserialdeviceenumerator/serialdeviceenumerator_p_win.cpp
    )
    set( ADDITIONAL_LIBRARY setupapi uuid advapi32 )
elseif(UNIX)
    set( QSERIALDEVICE_SRCS
        ${QSERIALDEVICE_SRCS}
        qserialdevice/nativeserialengine_unix.cpp
        unix/ttylocker.cpp
        qserialdevice/nativeserialnotifier_unix.cpp
    )
    if(APPLE)
        set( QSERIALDEVICE_SRCS
            ${QSERIALDEVICE_SRCS}
            qserialdeviceenumerator/serialdeviceenumerator_p_mac.cpp
        )
    else(APPLE)
        set( QSERIALDEVICE_SRCS
            ${QSERIALDEVICE_SRCS}
            qserialdeviceenumerator/serialdeviceenumerator_p_unix.cpp
        )
        set( ADDITIONAL_LIBRARY udev )
    endif(APPLE)
else()
    message( FATAL_ERROR "This operating system is not supported library QSerialDevice" )
endif()

qt4_generate_moc( qserialdevice/abstractserial.h ${CMAKE_CURRENT_BINARY_DIR}/moc_abstractserial.cpp )
qt4_generate_moc( qserialdevice/abstractserialengine.h ${CMAKE_CURRENT_BINARY_DIR}/moc_abstractserialengine.cpp )
qt4_generate_moc( qserialdevice/nativeserialengine.h ${CMAKE_CURRENT_BINARY_DIR}/moc_nativeserialengine.cpp )
qt4_generate_moc( qserialdeviceenumerator/serialdeviceenumerator.h ${CMAKE_CURRENT_BINARY_DIR}/moc_serialdeviceenumerator.cpp )

set_source_files_properties( qserialdevice/abstractserial.cpp OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/moc_abstractserial.cpp )
set_source_files_properties( qserialdevice/abstractserialengine.cpp OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/moc_abstractserialengine.cpp )
set_source_files_properties( qserialdevice/nativeserialengine.cpp OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/moc_nativeserialengine.cpp )
set_source_files_properties( qserialdeviceenumerator/serialdeviceenumerator.cpp OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/moc_serialdeviceenumerator.cpp )

# Set shared/static build library.
if(BUILD_SHARED)
    add_library( ${LIB_TARGET} SHARED ${QSERIALDEVICE_SRCS} ${QSERIALDEVICE_MOCS} )
else()
    add_library( ${LIB_TARGET} STATIC ${QSERIALDEVICE_SRCS} ${QSERIALDEVICE_MOCS} )
endif()

target_link_libraries( ${LIB_TARGET}  ${QT_QTCORE_LIBRARY} ${ADDITIONAL_LIBRARY} )
