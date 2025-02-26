include "emscripten_gmake.lua"

LANG = "C"
SOLUTION_NAME = "program"
SOLUTION_DIR = ".."
PROJECT_NAME = "program"
PROJECT_BACKEND = "kinc"
PROJECT_DIR = "../"
PROJECT_KIND = "ConsoleApp"
OBJ_DIR = "../bin/obj"
TARGET_DIR = "../bin"
OPTIMIZE = "Off"
LIBRARY_DIR = "../third_party/libraries"

ENABLE_VSYNC = true

function third_party_config()

    -- ZLIB
    project "zlib-lib"
        language "C"
        kind "StaticLib"
        defines { "N_FSEEKO", "_LARGEFILE64_SOURCE" }
        warnings "off"

        files {
            "../third_party/zlib/*.c",
            "../third_party/zlib/*.h"
        }

        filter "system:windows"
            defines { "_WINDOWS" }


    -- CURL
    project "curl-lib"
        language "C"
        kind "StaticLib"
        defines { "CURL_STATICLIB", "NEED_THREAD_SAFE=1", "BUILDING_LIBCURL", "HAVE_SOCKET=1" }

        defines {
            "CURL_STATICLIB",
            "_FILE_OFFSET_BITS=64",   -- Ensures curl_off_t is 64-bit
            "_LARGEFILE64_SOURCE",    -- Large file support
            "SIZEOF_CURL_OFF_T=8",    -- Explicitly define curl_off_t as 64-bit
            "HAVE_STRUCT_TIMEVAL",    -- Prevents timeval redefinition
            "HAVE_SYS_SOCKET_H",      -- Ensures sockaddr_in and socket functions
            "HAVE_NETINET_IN_H",      -- Ensures netinet/in.h is included
            "HAVE_ARPA_INET_H",       -- Ensures arpa/inet.h is included
            "HAVE_UNISTD_H",          -- Ensures unistd.h is included (for close())
            "HAVE_SYS_SELECT_H",      -- Ensures select() is available
            "HAVE_SYS_TYPES_H",       -- Ensures sys/types.h is included
            "HAVE_SYS_STAT_H",        -- Ensures sys/stat.h is included
            "HAVE_FCNTL_H",           -- Ensures fcntl.h is included (for non-blocking sockets)
            "HAVE_ERRNO_H",           -- Ensures errno.h is included
            "HAVE_POLL_H",            -- Ensures poll.h is included (alternative to select)
            "sread=read",
            "swrite=write"
        }


        warnings "off"

        includedirs {
            "../third_party/curl/lib",
            "../third_party/curl/include"
        }

        files {
            "../third_party/curl/lib/easy.c",
            "../third_party/curl/lib/http.c",
            "../third_party/curl/lib/url.c",
            "../third_party/curl/lib/connect.c",
            "../third_party/curl/lib/sendf.c",
            "../third_party/curl/lib/getinfo.c",
            "../third_party/curl/lib/transfer.c",
            -- "../third_party/cjson/cJSON.c",
            -- "../third_party/liboauth/oauth.c"
        }

        filter "not system:windows"

        filter "system:windows"
            defines { "_WINDOWS", "ALLOW_MSVC6_WITHOUT_PSDK", "HAVE_CONFIG_H",  }
            links { "ws2_32", "wldap32" }

        filter "system:linux"
            links { "pthread" }

        filter "system:macosx"
            links { "pthread" }
end

function solution_config()
    solution(SOLUTION_NAME)
        location(SOLUTION_DIR)
        systemversion "latest"
        language(LANG)
        configurations { "Debug", "Release" }

        -- Platforms + Architecture
        if os.target() ==  "windows" then
            platforms { "windows" }
            system "windows"
            architecture "x86_64"
        elseif os.target() == "linux" then
            platforms { "linux" }
            system "linux"
            architecture "x86_64"
        elseif os.target() == "macosx" then
            platforms { "macosx" }
            system "macosx"
            architecture "arm64"
        end

        flags { "MultiProcessorCompile" }
        optimize(OPTIMIZE)

        filter "system:macosx"
            buildoptions { "-mmacosx-version-min=13.1" }
            linkoptions { "-mmacosx-version-min=13.1" }

        filter "configurations:Debug"
            defines { "DEBUG" }
            symbols "On"

        filter "configurations:Release"
            symbols "Off"
end

function project_config()
    project(PROJECT_NAME)
        location(PROJECT_DIR)
        objdir(OBJ_DIR)
        kind(PROJECT_KIND)

        if ENABLE_VSYNC then
            defines { "ENABLE_VSYNC=1" }
        end

        defines {
            "STB_IMAGE_IMPLEMENTATION",
            "STBI_SUPPORT_ZLIB",
            "NK_IMPLEMENTATION",
        }

        files {
            "../include/**.h",
            "../" .. PROJECT_BACKEND .. "_backend/**.c",
            "../src/**.c",
            "../third_party/hashmap/hashmap.c",
            "../third_party/hashmap/hashmap.h",
            "../third_party/cglm/src/**.c",
            "../third_party/cglm/include/**.h",
            "../third_party/stb/stb_image.h",
            "../third_party/nuklear/nuklear.h",
        }

        includedirs {
            "../include",
            "../third_party/SDL/include",
            "../third_party/hashmap",
            "../third_party/cglm/include",
            "../third_party/stb",
            "../third_party/zlib",
            "../third_party/nuklear",
        }

        -- Third Party Libraries
        links {
            "zlib-lib"
        }

        targetdir(TARGET_DIR)
        libdirs (LIBRARY_DIR)

        filter "configurations:Release"
            links { "SDL2" }

        filter "configurations:Debug"
            links { "SDL2" }
            debugdir(TARGET_DIR)

        filter "system:not windows"
            defines { "HAVE_UNISTD_H" }

        filter { "system:macosx" }
            defines { "USE_EGL" }

            linkoptions {
                "-framework IOKit",
                "-framework CoreFoundation",
                "-framework CoreGraphics",
                "-framework AudioToolbox",
                "-framework QuartzCore",
                "-framework AppKit",
                "-framework Carbon",
                "-framework Cocoa",
                "-framework CoreServices",
                "-framework Metal",
                "-framework OpenGL"
            }

        filter { "system:windows" }
            defines { "WINDOWS" }

        filter { "system:emscripten" }
            defines { "EMSCRIPTEN" }
            linkoptions { "-s ALLOW_MEMORY_GROWTH=1" }
end

solution_config()
third_party_config()
project_config()
