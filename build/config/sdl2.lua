-- This project uses Premake, which is licensed under the BSD-3-Clause License.
-- Copyright (C) 2002-2023 Jason Perkins and the Premake project.
--
-- Redistribution and use in source and binary forms, with or without modification,
-- are permitted provided that the following conditions are met:
--
-- 1. Redistributions of source code must retain the above copyright notice,
--    this list of conditions, and the following disclaimer.
-- 2. Redistributions in binary form must reproduce the above copyright notice,
--    this list of conditions, and the following disclaimer in the documentation
--    and/or other materials provided with the distribution.
-- 3. Neither the name of the copyright holders nor the names of its contributors
--    may be used to endorse or promote products derived from this software
--    without specific prior written permission.
--
-- Full license: https://github.com/premake/premake-core/blob/master/LICENSE.txt


local NATIVE_PATH = "../../third_party"

project "SDL"
    kind "StaticLib"
    language "C"
    cdialect "gnu99"

    -- Include directories
    includedirs {
        NATIVE_PATH .. "/custom/sdl/include/",
        NATIVE_PATH .. "/sdl/include/",
        NATIVE_PATH .. "/sdl/src/hidapi/hidapi/",
        NATIVE_PATH .. "/sdl/src/video/khronos/"
    }

    -- Common defines
    defines {
        "HAVE_LIBC",
        "SDL_HIDAPI_DISABLED",
        "SDL_AUDIO=OFF"
    }

    -- Platform-specific configurations
    filter "system:linux"
        defines { "USING_GENERATED_CONFIG_H" }

    filter "system:winrt"
        defines {
            "__WINRT__",
            "UNICODE",
            "SDL_BUILDING_WINRT=1"
        }

    filter "system:windows"
        defines {
            "HAVE_LIBC",
            "SDL_HIDAPI_DISABLED",
            "SDL_AUDIO_DISABLED",
            "SDL_JOYSTICK_XINPUT",
            "SDL_JOYSTICK_DISABLED_DINPUT",
            "SDL_HAPTIC_XINPUT",
            "SDL_STATIC",
            "_WIN32",
            "WIN32",
            "HAVE_STDARG_H",
            "HAVE_STDINT_H",
            "HAVE_STDDEF_H",
            "HAVE_STDIO_H",
            "HAVE_STDLIB_H",
            "HAVE_STRING_H",
            "SDL_THREAD_WINDOWS",
            "SDL_TIMER_WINDOWS",
            "SDL_VIDEO_DRIVER_WINDOWS"
        }

    filter "system:linux"
        defines {
            "HAVE_LINUX_INPUT_H",
            "HAVE_LINUX_VERSION_H",
            "_REENTRANT",
            "NATIVE_TOOLKIT_SDL_X11_XINPUT2=1",
            "NATIVE_TOOLKIT_SDL_X11_XINPUT2_SUPPORTS_MULTITOUCH=1"
        }
        buildoptions {
            "-mmmx", "-msse", "-msse2", "-msse3", "-mssse3"
        }

    filter "system:macosx"
        defines {
            "TARGET_API_MAC_CARBON",
            "TARGET_API_MAC_OSX",
            "_THREAD_SAFE"
        }

    filter {}

    -- Common source files
    files {
        NATIVE_PATH .. "/sdl/src/atomic/SDL_atomic.c",
        NATIVE_PATH .. "/sdl/src/atomic/SDL_spinlock.c",
        NATIVE_PATH .. "/sdl/src/audio/disk/SDL_diskaudio.c",
        NATIVE_PATH .. "/sdl/src/audio/dummy/SDL_dummyaudio.c",
        NATIVE_PATH .. "/sdl/src/audio/SDL_audio.c",
        NATIVE_PATH .. "/sdl/src/audio/SDL_audiocvt.c",
        NATIVE_PATH .. "/sdl/src/audio/SDL_audiotypecvt.c",
        NATIVE_PATH .. "/sdl/src/audio/SDL_mixer.c",
        NATIVE_PATH .. "/sdl/src/audio/SDL_wave.c",
        NATIVE_PATH .. "/sdl/src/cpuinfo/SDL_cpuinfo.c",
        NATIVE_PATH .. "/sdl/src/dynapi/SDL_dynapi.c",
        NATIVE_PATH .. "/sdl/src/events/SDL_clipboardevents.c",
        NATIVE_PATH .. "/sdl/src/events/SDL_displayevents.c",
        NATIVE_PATH .. "/sdl/src/events/SDL_dropevents.c",
        NATIVE_PATH .. "/sdl/src/events/SDL_events.c",
        NATIVE_PATH .. "/sdl/src/events/SDL_gesture.c",
        NATIVE_PATH .. "/sdl/src/events/SDL_keyboard.c",
        NATIVE_PATH .. "/sdl/src/events/SDL_mouse.c",
        NATIVE_PATH .. "/sdl/src/events/SDL_quit.c",
        NATIVE_PATH .. "/sdl/src/events/SDL_touch.c",
        NATIVE_PATH .. "/sdl/src/events/SDL_windowevents.c",
        NATIVE_PATH .. "/sdl/src/events/SDL_scancode_tables.c",
        NATIVE_PATH .. "/sdl/src/events/SDL_keysym_to_scancode.c",
        NATIVE_PATH .. "/sdl/src/file/SDL_rwops.c",
        NATIVE_PATH .. "/sdl/src/haptic/SDL_haptic.c",
        NATIVE_PATH .. "/sdl/src/hidapi/SDL_hidapi.c",
        NATIVE_PATH .. "/sdl/src/joystick/controller_type.c",
        NATIVE_PATH .. "/sdl/src/joystick/steam/SDL_steamcontroller.c",
        NATIVE_PATH .. "/sdl/src/joystick/SDL_gamecontroller.c",
        NATIVE_PATH .. "/sdl/src/joystick/SDL_joystick.c",
        NATIVE_PATH .. "/sdl/src/joystick/SDL_steam_virtual_gamepad.c",
        NATIVE_PATH .. "/sdl/src/joystick/virtual/SDL_virtualjoystick.c",

        -- Math library
        NATIVE_PATH .. "/sdl/src/libm/**.c",

        NATIVE_PATH .. "/sdl/src/loadso/dlopen/SDL_sysloadso.c",
        NATIVE_PATH .. "/sdl/src/locale/SDL_locale.c",
        NATIVE_PATH .. "/sdl/src/misc/SDL_url.c",
        NATIVE_PATH .. "/sdl/src/power/SDL_power.c",

        -- Rendering
        NATIVE_PATH .. "/sdl/src/render/opengl/SDL_render_gl.c",
        NATIVE_PATH .. "/sdl/src/render/opengl/SDL_shaders_gl.c",
        NATIVE_PATH .. "/sdl/src/render/SDL_render.c",
        NATIVE_PATH .. "/sdl/src/render/SDL_yuv_sw.c",
        NATIVE_PATH .. "/sdl/src/render/software/**.c",

        -- Core SDL files
        NATIVE_PATH .. "/sdl/src/SDL_assert.c",
        NATIVE_PATH .. "/sdl/src/SDL_dataqueue.c",
        NATIVE_PATH .. "/sdl/src/SDL_error.c",
        NATIVE_PATH .. "/sdl/src/SDL_hints.c",
        NATIVE_PATH .. "/sdl/src/SDL_list.c",
        NATIVE_PATH .. "/sdl/src/SDL_log.c",
        NATIVE_PATH .. "/sdl/src/SDL_utils.c",
        NATIVE_PATH .. "/sdl/src/SDL_guid.c",
        NATIVE_PATH .. "/sdl/src/SDL.c",

        -- Sensors, stdlib, threads, timers
        NATIVE_PATH .. "/sdl/src/sensor/dummy/SDL_dummysensor.c",
        NATIVE_PATH .. "/sdl/src/sensor/SDL_sensor.c",
        NATIVE_PATH .. "/sdl/src/stdlib/SDL_crc16.c",
        NATIVE_PATH .. "/sdl/src/stdlib/SDL_crc32.c",
        NATIVE_PATH .. "/sdl/src/stdlib/SDL_getenv.c",
        NATIVE_PATH .. "/sdl/src/stdlib/SDL_iconv.c",
        NATIVE_PATH .. "/sdl/src/stdlib/SDL_malloc.c",
        NATIVE_PATH .. "/sdl/src/stdlib/SDL_qsort.c",
        NATIVE_PATH .. "/sdl/src/stdlib/SDL_stdlib.c",
        NATIVE_PATH .. "/sdl/src/stdlib/SDL_string.c",
        NATIVE_PATH .. "/sdl/src/stdlib/SDL_strtokr.c",
        NATIVE_PATH .. "/sdl/src/thread/SDL_thread.c",
        NATIVE_PATH .. "/sdl/src/timer/SDL_timer.c",

        -- Video subsystem
        NATIVE_PATH .. "/sdl/src/video/dummy/**.c",
        NATIVE_PATH .. "/sdl/src/video/SDL_blit*.c",
        NATIVE_PATH .. "/sdl/src/video/SDL_bmp.c",
        NATIVE_PATH .. "/sdl/src/video/SDL_clipboard.c",
        NATIVE_PATH .. "/sdl/src/video/SDL_fillrect.c",
        NATIVE_PATH .. "/sdl/src/video/SDL_pixels.c",
        NATIVE_PATH .. "/sdl/src/video/SDL_rect.c",
        NATIVE_PATH .. "/sdl/src/video/SDL_RLEaccel.c",
        NATIVE_PATH .. "/sdl/src/video/SDL_shape.c",
        NATIVE_PATH .. "/sdl/src/video/SDL_stretch.c",
        NATIVE_PATH .. "/sdl/src/video/SDL_surface.c",
        NATIVE_PATH .. "/sdl/src/video/SDL_video.c",
        NATIVE_PATH .. "/sdl/src/video/SDL_vulkan_utils.c",
        NATIVE_PATH .. "/sdl/src/video/SDL_yuv.c",
        NATIVE_PATH .. "/sdl/src/video/yuv2rgb/yuv_rgb_std.c",
        NATIVE_PATH .. "/sdl/src/video/yuv2rgb/yuv_rgb_sse.c",
        NATIVE_PATH .. "/sdl/src/video/yuv2rgb/yuv_rgb_lsx.c"
    }

    -- Non-Windows platforms (Unix-like)
    filter "not system:windows"
        files {
            NATIVE_PATH .. "/sdl/src/thread/pthread/SDL_syscond.c",
            NATIVE_PATH .. "/sdl/src/thread/pthread/SDL_sysmutex.c",
            NATIVE_PATH .. "/sdl/src/thread/pthread/SDL_syssem.c",
            NATIVE_PATH .. "/sdl/src/thread/pthread/SDL_systhread.c",
            NATIVE_PATH .. "/sdl/src/timer/unix/SDL_systimer.c"
        }

    -- OpenGL ES platforms (including macOS now)
    filter "system:windows"
        files {
            NATIVE_PATH .. "/sdl/src/render/opengles/SDL_render_gles.c",
            NATIVE_PATH .. "/sdl/src/render/opengles2/SDL_render_gles2.c",
            NATIVE_PATH .. "/sdl/src/render/opengles2/SDL_shaders_gles2.c"
        }

    -- Optional EGL for Windows and macOS (uncomment if you have EGL libraries)
    -- filter "system:windows or system:macosx"
    --     files { NATIVE_PATH .. "/sdl/src/video/SDL_egl.c" }
    --     defines { "SDL_VIDEO_DRIVER_EGL=1" }

    -- HID API platforms (expanded list)
    filter "system:windows or system:macosx"
        files {
            NATIVE_PATH .. "/sdl/src/joystick/hidapi/SDL_hidapi_gamecube.c",
            NATIVE_PATH .. "/sdl/src/joystick/hidapi/SDL_hidapi_luna.c",
            NATIVE_PATH .. "/sdl/src/joystick/hidapi/SDL_hidapi_ps3.c",
            NATIVE_PATH .. "/sdl/src/joystick/hidapi/SDL_hidapi_ps4.c",
            NATIVE_PATH .. "/sdl/src/joystick/hidapi/SDL_hidapi_ps5.c",
            NATIVE_PATH .. "/sdl/src/joystick/hidapi/SDL_hidapi_rumble.c",
            NATIVE_PATH .. "/sdl/src/joystick/hidapi/SDL_hidapi_stadia.c",
            NATIVE_PATH .. "/sdl/src/joystick/hidapi/SDL_hidapi_steam.c",
            NATIVE_PATH .. "/sdl/src/joystick/hidapi/SDL_hidapi_switch.c",
            NATIVE_PATH .. "/sdl/src/joystick/hidapi/SDL_hidapi_xbox360.c",
            NATIVE_PATH .. "/sdl/src/joystick/hidapi/SDL_hidapi_xbox360w.c",
            NATIVE_PATH .. "/sdl/src/joystick/hidapi/SDL_hidapi_xboxone.c",
            NATIVE_PATH .. "/sdl/src/joystick/hidapi/SDL_hidapi_shield.c",
            NATIVE_PATH .. "/sdl/src/joystick/hidapi/SDL_hidapi_combined.c",
            NATIVE_PATH .. "/sdl/src/joystick/hidapi/SDL_hidapi_wii.c",
            NATIVE_PATH .. "/sdl/src/joystick/hidapi/SDL_hidapi_steamdeck.c",
            NATIVE_PATH .. "/sdl/src/joystick/hidapi/SDL_hidapijoystick.c"
        }

    -- Linux-specific files
    filter "system:linux"
        files {
            NATIVE_PATH .. "/sdl/src/core/linux/**.c",
            NATIVE_PATH .. "/sdl/src/core/unix/SDL_poll.c",
            NATIVE_PATH .. "/sdl/src/events/imKStoUCS.c",
            NATIVE_PATH .. "/sdl/src/filesystem/unix/SDL_sysfilesystem.c",
            NATIVE_PATH .. "/sdl/src/haptic/linux/SDL_syshaptic.c",
            NATIVE_PATH .. "/sdl/src/joystick/linux/SDL_sysjoystick.c",
            NATIVE_PATH .. "/sdl/src/locale/unix/SDL_syslocale.c",
            NATIVE_PATH .. "/sdl/src/misc/unix/SDL_sysurl.c",
            NATIVE_PATH .. "/sdl/src/power/linux/SDL_syspower.c",
            NATIVE_PATH .. "/sdl/src/video/x11/**.c"
        }

    -- Windows-specific files
    filter "system:windows"
        files {
            NATIVE_PATH .. "/sdl/src/core/windows/SDL_windows.c",
            NATIVE_PATH .. "/sdl/src/core/windows/SDL_xinput.c",
            NATIVE_PATH .. "/sdl/src/core/windows/SDL_immdevice.c",
            NATIVE_PATH .. "/sdl/src/haptic/windows/**.c",
            NATIVE_PATH .. "/sdl/src/hidapi/windows/hid.c",
            NATIVE_PATH .. "/sdl/src/joystick/windows/**.c",
            NATIVE_PATH .. "/sdl/src/loadso/windows/SDL_sysloadso.c",
            NATIVE_PATH .. "/sdl/src/render/direct3d11/SDL_render_d3d11.c",
            NATIVE_PATH .. "/sdl/src/render/direct3d12/SDL_render_d3d12.c",
            NATIVE_PATH .. "/sdl/src/render/SDL_d3dmath.c",
            NATIVE_PATH .. "/sdl/src/thread/SDL_thread.c",
            NATIVE_PATH .. "/sdl/src/thread/windows/SDL_sysmutex.c",
            NATIVE_PATH .. "/sdl/src/thread/windows/SDL_systhread.c",
            NATIVE_PATH .. "/sdl/src/thread/windows/SDL_syssem.c",
            NATIVE_PATH .. "/sdl/src/thread/generic/SDL_syscond.c",
            NATIVE_PATH .. "/sdl/src/thread/windows/SDL_systls.c",
            NATIVE_PATH .. "/sdl/src/timer/windows/SDL_systimer.c"
        }

        files {
            NATIVE_PATH .. "/sdl/src/filesystem/windows/SDL_sysfilesystem.c",
            NATIVE_PATH .. "/sdl/src/locale/windows/SDL_syslocale.c",
            NATIVE_PATH .. "/sdl/src/misc/windows/SDL_sysurl.c",
            NATIVE_PATH .. "/sdl/src/power/windows/SDL_syspower.c",
            NATIVE_PATH .. "/sdl/src/render/direct3d/SDL_render_d3d.c",
            NATIVE_PATH .. "/sdl/src/video/windows/**.c"
        }

    -- WinRT-specific files
    filter "system:winrt"
        files {
            NATIVE_PATH .. "/sdl/src/core/winrt/**.cpp",
            NATIVE_PATH .. "/sdl/src/filesystem/winrt/SDL_sysfilesystem.cpp",
            NATIVE_PATH .. "/sdl/src/locale/winrt/SDL_syslocale.c",
            NATIVE_PATH .. "/sdl/src/misc/winrt/SDL_sysurl.cpp",
            NATIVE_PATH .. "/sdl/src/power/winrt/SDL_syspower.cpp",
            NATIVE_PATH .. "/sdl/src/render/direct3d11/SDL_render_d3d11.c",
            NATIVE_PATH .. "/sdl/src/render/direct3d12/SDL_render_d3d12.c",
            NATIVE_PATH .. "/sdl/src/render/direct3d11/SDL_render_winrt.cpp",
            NATIVE_PATH .. "/sdl/src/render/direct3d11/SDL_shaders_d3d11.c",
            NATIVE_PATH .. "/sdl/src/video/winrt/**.cpp"
        }

    -- macOS-specific files
    filter "system:macosx"
        buildoptions { "-fobjc-arc" }
        -- buildoptions { "-mmmx", "-msse", "-msse2", "-msse3", "-mssse3" }
        files {
            NATIVE_PATH .. "/sdl/src/thread/pthread/SDL_systls.c",
            NATIVE_PATH .. "/sdl/src/audio/coreaudio/SDL_coreaudio.m",
            NATIVE_PATH .. "/sdl/src/file/cocoa/SDL_rwopsbundlesupport.m",
            NATIVE_PATH .. "/sdl/src/filesystem/cocoa/SDL_sysfilesystem.m",
            NATIVE_PATH .. "/sdl/src/haptic/darwin/SDL_syshaptic.c",
            NATIVE_PATH .. "/sdl/src/hidapi/mac/hid.c",
            NATIVE_PATH .. "/sdl/src/joystick/darwin/SDL_iokitjoystick.c",
            NATIVE_PATH .. "/sdl/src/joystick/iphoneos/SDL_mfijoystick.m",
            NATIVE_PATH .. "/sdl/src/locale/macosx/SDL_syslocale.m",
            NATIVE_PATH .. "/sdl/src/misc/macosx/SDL_sysurl.m",
            NATIVE_PATH .. "/sdl/src/power/macosx/SDL_syspower.c",
            NATIVE_PATH .. "/sdl/src/video/cocoa/SDL_cocoaclipboard.m",
            NATIVE_PATH .. "/sdl/src/video/cocoa/SDL_cocoaevents.m",
            NATIVE_PATH .. "/sdl/src/video/cocoa/SDL_cocoakeyboard.m",
            NATIVE_PATH .. "/sdl/src/video/cocoa/SDL_cocoamessagebox.m",
            NATIVE_PATH .. "/sdl/src/video/cocoa/SDL_cocoametalview.m",
            NATIVE_PATH .. "/sdl/src/video/cocoa/SDL_cocoamodes.m",
            NATIVE_PATH .. "/sdl/src/video/cocoa/SDL_cocoamouse.m",
            NATIVE_PATH .. "/sdl/src/video/cocoa/SDL_cocoaopengl.m",
            NATIVE_PATH .. "/sdl/src/video/cocoa/SDL_cocoashape.m",
            NATIVE_PATH .. "/sdl/src/video/cocoa/SDL_cocoavideo.m",
            NATIVE_PATH .. "/sdl/src/video/cocoa/SDL_cocoavulkan.m",
            NATIVE_PATH .. "/sdl/src/video/cocoa/SDL_cocoawindow.m",
        }

    -- Emscripten-specific files
    filter "system:emscripten"
        files {
            NATIVE_PATH .. "/sdl/src/filesystem/emscripten/SDL_sysfilesystem.c",
            NATIVE_PATH .. "/sdl/src/joystick/emscripten/SDL_sysjoystick.c",
            NATIVE_PATH .. "/sdl/src/locale/emscripten/SDL_syslocale.c",
            NATIVE_PATH .. "/sdl/src/misc/emscripten/SDL_sysurl.c",
            NATIVE_PATH .. "/sdl/src/power/emscripten/SDL_syspower.c",
            NATIVE_PATH .. "/sdl/src/video/emscripten/**.c"
        }
