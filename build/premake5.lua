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
LIBRARY_DIR = "third_party/libraries"

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

        filter "system:emscripten"
            GRAPHICS_API = "OPENGL_ES"

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

<<<<<<< Updated upstream
=======
        if GRAPHICS_API == "OPENGL_3" then
            includedirs {
                "../nuklear_bindings/gl3"
            }

        elseif GRAPHICS_API == "OPENGL_ES" then
            includedirs {
                "../nuklear_bindings/gles2"
            }

        end

>>>>>>> Stashed changes
        -- Third Party Libraries
        links {
            "zlib-lib"
        }

        targetdir(TARGET_DIR)
<<<<<<< Updated upstream
        libdirs (LIBRARY_DIR)
=======
        libdirs ("../third_party/curl/lib")
>>>>>>> Stashed changes

        filter "configurations:Release"
            links { "SDL2" }
            libdirs ("../" .. LIBRARY_DIR)
            linkoptions { LIBRARY_DIR .. "/libSDL2.a" }

        filter "configurations:Debug"
            links { "SDL2" }
            libdirs ("../" .. LIBRARY_DIR)
            linkoption LIBRARY_DIR .. "/libSDL2.a" }
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
<<<<<<< Updated upstream
            defines { "EMSCRIPTEN" }
            linkoptions { "-s ALLOW_MEMORY_GROWTH=1" }
=======
            defines {
                "USE_EGL3",
                "EMSCRIPTEN",
            }

            linkoptions { "-s ALLOW_MEMORY_GROWTH=1 -s FULL_ES3" }
>>>>>>> Stashed changes
end

solution_config()
third_party_config()
project_config()
