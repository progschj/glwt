include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(GLWT_SOURCES common/glwt.c)

if(WIN32)
    list(APPEND GLWT_SOURCES
        win32/glwt_win32.c
        win32/window.c
        win32/events.c
        )

    if(NOT GLWT_USE_EGL)
        list(APPEND GLWT_SOURCES
            wgl/glwt_wgl.c
            wgl/window.c
            )

        list(APPEND GLWT_DEPENDENCY_LIBRARIES glxw_wgl)
    endif()

elseif (APPLE)
    list(APPEND GLWT_SOURCES
        osx/glwt_osx.m
        osx/window.m
        )

    # For some reason, CMake doesn't know about .m
    set_source_files_properties(${GLWT_SOURCES} PROPERTIES LANGUAGE C)

    find_library(COCOA_LIBRARY Cocoa)
    list(APPEND GLWT_DEPENDENCY_LIBRARIES ${COCOA_LIBRARY})

    find_path(COCOA_INCLUDE_DIR Cocoa/Cocoa.h)
    include_directories(${COCOA_INCLUDE_DIR})
elseif (RASPBERRYPI)
    list(APPEND GLWT_SOURCES
        rpi/glwt_rpi.c
        rpi/window.c
        rpi/events.c
        )

    list(APPEND GLWT_DEPENDENCY_LIBRARIES ${BROADCOM_LIBRARY})

    find_path(BROADCOM_INCLUDE bcm_host.h HINTS /opt/vc/include)
    find_path(VCOS_INCLUDE vcos_platform_types.h HINTS /opt/vc/include/interface/vcos/pthreads)
    include_directories(${BROADCOM_INCLUDE} ${VCOS_INCLUDE})

    find_path(VCHOST_CONFIG_INCLUDE vchost_config.h HINTS /opt/vc/include/interface/vmcs_host/linux)

    if(VCHOST_CONFIG_INCLUDE)
        include_directories(${VCHOST_CONFIG_INCLUDE})
    endif()

    find_library(GLESv2_LIBRARY GLESv2 HINTS /opt/vc/lib)
    mark_as_advanced(GLESv2_LIBRARY)
    if(GLESv2_LIBRARY)
        message("found GLESv2")
        list(APPEND GLWT_DEPENDENCY_LIBRARIES ${GLESV2_LIBRARY})
    endif()

    add_definitions(-DRASPBERRYPI)

    list(APPEND GLWT_SOURCES
        linux/glwt_linux_input.c
        )

    add_definitions(-DGLWT_USE_LINUX_INPUT)

else()
    find_package(X11 REQUIRED)
    list(APPEND GLWT_DEPENDENCY_LIBRARIES ${X11_LIBRARIES})
    include_directories(${X11_X11_INCLUDE_PATH})

    list(APPEND GLWT_SOURCES
        x11/glwt_x11.c
        x11/window.c
        x11/events.c
        )

    if(NOT GLWT_USE_EGL)
        list(APPEND GLWT_SOURCES
            glx/glwt_glx.c
            glx/window.c
            )
        list(APPEND GLWT_DEPENDENCY_LIBRARIES glxw_glx)
    endif()

    find_library(DL_LIBRARY dl)
    mark_as_advanced(DL_LIBRARY)
    if(DL_LIBRARY)
        list(APPEND GLWT_DEPENDENCY_LIBRARIES ${DL_LIBRARY})
    endif()

endif()

if(GLWT_USE_EGL)
    find_library(EGL_LIBRARY EGL HINTS /opt/vc/lib)
    mark_as_advanced(EGL_LIBRARY)
    list(APPEND GLWT_DEPENDENCY_LIBRARIES ${EGL_LIBRARY})
    add_definitions(-DGLWT_USE_EGL)

    list(APPEND GLWT_SOURCES
        egl/glwt_egl.c
        egl/window.c
        )
    list(APPEND GLWT_DEPENDENCY_LIBRARIES glxw_egl)
else()
    find_package(OpenGL REQUIRED)
    include_directories(${OPENGL_INCLUDE_DIR})
    list(APPEND GLWT_DEPENDENCY_LIBRARIES ${OPENGL_gl_LIBRARY})
endif()

add_library(glwt ${GLWT_SOURCES})
target_link_libraries(glwt ${GLWT_DEPENDENCY_LIBRARIES})
