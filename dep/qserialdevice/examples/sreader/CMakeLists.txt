project( SReader )
#cmake_minimum_required( VERSION 2.6.0 )
#find_package( Qt4 REQUIRED )

include( ${QT_USE_FILE} )
include_directories( ${CMAKE_CURRENT_BINARY_DIR}
                     ${QT_INCLUDES} 
                     ${LIB_INCLUDE_DIRS} 
)

# APP_TARGET - application target name.
# EXECUTABLE_OUTPUT_PATH - path to contain the build output binary application.
# All depends on whether DEBUG or RELEASE.
if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    set( APP_TARGET "sreaderd" )
    set ( EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/debug )
else()
    set( APP_TARGET "sreader" )
    set ( EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/release )
endif()

# Disable Qt GUI and add definition.
set( QT_DONT_USE_QTGUI 1 )
set( QT_DEFINITIONS
    -DUNICODE
    -DQT_NO_DEBUG
    -DQT_THREAD_SUPPORT
)
add_definitions( ${QT_DEFINITIONS} )

# Applicalion sources and headers files.
set( APP_SRCS main.cpp )
set( APP_HDRS sreader.h )

# Wrap Qt classes inherited from QObject. 
qt4_wrap_cpp( APP_MOCS ${APP_HDRS} )

# Wrap Qt classes inherited from QObject.
#qt4_wrap_cpp( APP_MOCS ${APP_HDRS} )

# LIBRARY_OUTPUT_PATH - link directory in which to store the output binary library QSerialDevice.
link_directories( ${LIBRARY_OUTPUT_PATH} )

add_executable( ${APP_TARGET} WIN32 ${APP_SRCS} ${APP_MOCS} )

# LIB_TARGET - name of the output binary library QSerialDevice.
target_link_libraries( ${APP_TARGET}  ${QT_QTCORE_LIBRARY} ${LIB_TARGET} )
