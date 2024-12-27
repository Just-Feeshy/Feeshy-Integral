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
        defines { "N_FSEEKO" }
        warnings "off"

        files {
            "../third_party/zlib/*.c",
            "../third_party/zlib/*.h"
        }

        filter "system:windows"
            defines { "_WINDOWS" }
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

        files {
            "../include/**.h",
            "../" .. PROJECT_BACKEND .. "_backend/**.c",
            "../src/**.c",
            "../third_party/hashmap/hashmap.c",
            "../third_party/hashmap/hashmap.h",
            "../third_party/cglm/src/**.c",
            "../third_party/cglm/include/**.h",
            "../third_party/stb/stb_image.h"
        }

        includedirs {
            "../include",
            "../third_party/SDL/include",
            "../third_party/hashmap",
            "../third_party/cglm/include",
            "../third_party/stb"
        }

        -- Third Party Libraries
        links {
            "zlib-lib"
        }

        defines {
            "STB_IMAGE_IMPLEMENTATION",
            "STBI_SUPPORT_ZLIB"
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
end

solution_config()
third_party_config()
project_config()
