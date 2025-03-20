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

function sdl_config()
    includedirs {
        "../third_party/SDL/include"
    }

    filter "configurations:Release"
        libdirs (LIBRARY_DIR)
        links { "SDL2" }

    filter "configurations:Debug"
        libdirs (LIBRARY_DIR)
        links { "SDL2" }
        debugdir(TARGET_DIR)
end

function third_party_config()
    local curl_config = require("config/curl_config")

    -- ZLIB
    project "zlib-lib"
        language "C"
        kind "StaticLib"
        defines { "N_FSEEKO", "_LARGEFILE64_SOURCE" }
        warnings "off"

        includedirs {
            "../third_party/zlib"
        }

        files {
            "../third_party/zlib/*.c",
        }

        filter "system:windows"
            defines { "_WINDOWS" }


    -- CURL
    project "curl-lib"
        language "C"
        kind "StaticLib"
        defines { "CURL_STATICLIB", "NEED_THREAD_SAFE=1", "BUILDING_LIBCURL" }
        warnings "off"

        curl_config.setup()

        files {
            "../third_party/curl/include/curl/curl.h",
            "../third_party/custom/curl/lib/curl_config.h",
            "../third_party/curl/include/curl/curlver.h",
            "../third_party/curl/include/curl/easy.h",
            "../third_party/curl/include/curl/mprintf.h",
            "../third_party/curl/include/curl/multi.h",
            "../third_party/curl/include/curl/stdcheaders.h",
            "../third_party/curl/include/curl/typecheck-gcc.h"
        }

        includedirs {
            "../third_party/curl/lib",
            "../third_party/curl/include",
            "../third_party/custom/curl/lib",
            "../third_party/zlib"
        }

        files {
            "../third_party/curl/lib/altsvc.c",
            "../third_party/curl/lib/asyn-thread.c",
            "../third_party/curl/lib/base64.c",
            "../third_party/curl/lib/bufref.c",
            "../third_party/curl/lib/cfilters.c",
            "../third_party/curl/lib/conncache.c",
            "../third_party/curl/lib/connect.c",
            "../third_party/curl/lib/content_encoding.c",
            "../third_party/curl/lib/cookie.c",
            "../third_party/curl/lib/curl_addrinfo.c",
            "../third_party/curl/lib/curl_des.c",
            "../third_party/curl/lib/curl_endian.c",
            "../third_party/curl/lib/curl_fnmatch.c",
            "../third_party/curl/lib/curl_get_line.c",
            "../third_party/curl/lib/curl_gethostname.c",
            "../third_party/curl/lib/curl_memrchr.c",
            "../third_party/curl/lib/curl_ntlm_core.c",
            "../third_party/curl/lib/curl_range.c",
            "../third_party/curl/lib/curl_sasl.c",
            "../third_party/curl/lib/curl_threads.c",
            "../third_party/curl/lib/dict.c",
            "../third_party/curl/lib/doh.c",
            "../third_party/curl/lib/dynbuf.c",
            "../third_party/curl/lib/easy.c",
            "../third_party/curl/lib/escape.c",
            "../third_party/curl/lib/file.c",
            "../third_party/curl/lib/fileinfo.c",
            "../third_party/curl/lib/fopen.c",
            "../third_party/curl/lib/formdata.c",
            "../third_party/curl/lib/ftp.c",
            "../third_party/curl/lib/ftplistparser.c",
            "../third_party/curl/lib/getenv.c",
            "../third_party/curl/lib/getinfo.c",
            "../third_party/curl/lib/gopher.c",
            "../third_party/curl/lib/hash.c",
            "../third_party/curl/lib/headers.c",
            "../third_party/curl/lib/hmac.c",
            "../third_party/curl/lib/hostasyn.c",
            "../third_party/curl/lib/hostip.c",
            "../third_party/curl/lib/hostip4.c",
            "../third_party/curl/lib/hostip6.c",
            "../third_party/curl/lib/hostsyn.c",
            "../third_party/curl/lib/hsts.c",
            "../third_party/curl/lib/http_aws_sigv4.c",
            "../third_party/curl/lib/http_chunks.c",
            "../third_party/curl/lib/http_digest.c",
            "../third_party/curl/lib/http_ntlm.c",
            "../third_party/curl/lib/http_proxy.c",
            "../third_party/curl/lib/http.c",
            "../third_party/curl/lib/if2ip.c",
            "../third_party/curl/lib/idn.c",
            "../third_party/curl/lib/imap.c",
            "../third_party/curl/lib/llist.c",
            "../third_party/curl/lib/md4.c",
            "../third_party/curl/lib/md5.c",
            "../third_party/curl/lib/mime.c",
            "../third_party/curl/lib/mprintf.c",
            "../third_party/curl/lib/mqtt.c",
            "../third_party/curl/lib/multi.c",
            "../third_party/curl/lib/netrc.c",
            "../third_party/curl/lib/strtok.c",
            "../third_party/curl/lib/nonblock.c",
            "../third_party/curl/lib/noproxy.c",
            "../third_party/curl/lib/parsedate.c",
            "../third_party/curl/lib/pingpong.c",
            "../third_party/curl/lib/pop3.c",
            "../third_party/curl/lib/progress.c",
            "../third_party/curl/lib/rand.c",
            "../third_party/curl/lib/rename.c",
            "../third_party/curl/lib/rtsp.c",
            "../third_party/curl/lib/select.c",
            "../third_party/curl/lib/sendf.c",
            "../third_party/curl/lib/setopt.c",
            "../third_party/curl/lib/sha256.c",
            "../third_party/curl/lib/share.c",
            "../third_party/curl/lib/slist.c",
            "../third_party/curl/lib/smb.c",
            "../third_party/curl/lib/smtp.c",
            "../third_party/curl/lib/socks.c",
            "../third_party/curl/lib/speedcheck.c",
            "../third_party/curl/lib/splay.c",
            "../third_party/curl/lib/strcase.c",
            "../third_party/curl/lib/inet_ntop.c",
            "../third_party/curl/lib/inet_pton.c",
            "../third_party/curl/lib/strdup.c",
            "../third_party/curl/lib/strerror.c",
            "../third_party/curl/lib/strtoofft.c",
            "../third_party/curl/lib/telnet.c",
            "../third_party/curl/lib/tftp.c",
            "../third_party/curl/lib/timediff.c",
            "../third_party/curl/lib/timeval.c",
            "../third_party/curl/lib/transfer.c",
            "../third_party/curl/lib/url.c",
            "../third_party/curl/lib/urlapi.c",
            "../third_party/curl/lib/vauth/cleartext.c",
            "../third_party/curl/lib/vauth/cram.c",
            "../third_party/curl/lib/vauth/digest_sspi.c",
            "../third_party/curl/lib/vauth/digest.c",
            "../third_party/curl/lib/vauth/krb5_gssapi.c",
            "../third_party/curl/lib/vauth/krb5_sspi.c",
            "../third_party/curl/lib/vauth/ntlm_sspi.c",
            "../third_party/curl/lib/vauth/ntlm.c",
            "../third_party/curl/lib/vauth/oauth2.c",
            "../third_party/curl/lib/vauth/spnego_gssapi.c",
            "../third_party/curl/lib/vauth/spnego_sspi.c",
            "../third_party/curl/lib/vauth/vauth.c",
            "../third_party/curl/lib/version.c",
            "../third_party/curl/lib/vtls/hostcheck.c",
            "../third_party/curl/lib/vtls/vtls.c",
            "../third_party/curl/lib/warnless.c",
            "../third_party/curl/lib/wildcard.c"
        }

        filter "system:emscripten"
            links { "zlib" }
            buildoptions { "-sUSE_ZLIB=1" }
            defines { "USE_CURL" }

        filter "system:windows"
            defines { "_WINDOWS", "ALLOW_MSVC6_WITHOUT_PSDK", "HAVE_CONFIG_H",  }
            links { "ws2_32", "wldap32" }

            files {
                "../third_party/curl/lib/asyn-ares.c",
                "../third_party/curl/lib/curl_multibyte.c",
                "../third_party/curl/lib/krb5.c",
                "../third_party/curl/lib/ldap.c",
                "../third_party/curl/lib/openldap.c",
                "../third_party/curl/lib/socketpair.c",
                "../third_party/curl/lib/system_win32.c",
                "../third_party/curl/lib/version_win32.c"
            }

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
            "../third_party/tinyfiledialogs/tinyfiledialogs.c",
            "../third_party/cglm/include/**.h",
            "../third_party/stb/stb_image.h",
        }

        includedirs {
            "../include",
            "../third_party/hashmap",
            "../third_party/cglm/include",
            "../third_party/stb",
            "../third_party/zlib",
            "../third_party/curl/include",
            "../third_party/nuklear",
            "../third_party/tinyfiledialogs",
            "../nuklear_bindings/gl3"
        }

        -- Third Party Libraries
        links {
            "zlib-lib",
            "curl-lib"
        }

        libdirs ("../third_party/curl/lib")

        if os.target() ~= "emscripten" then
            targetdir(TARGET_DIR)
            sdl_config()
        end

        filter { "system:not windows" }
            defines { "HAVE_UNISTD_H" }

        filter { "system:macosx" }
            defines { "MACOSX" }

            linkoptions {
                "-framework IOKit",
                "-framework SystemConfiguration",
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
            targetdir(TARGET_DIR .. "/Web")
            targetextension ".html"

            defines {
                "USE_GLES",
                "EMSCRIPTEN",
            }

            linkoptions { "-sWASM=1 -sFULL_ES3 -sMIN_WEBGL_VERSION=2 -sINITIAL_MEMORY=128MB -sMAXIMUM_MEMORY=512MB -sALLOW_MEMORY_GROWTH=1 -s-sMAX_WEBGL_VERSION=2 -sUSE_SDL=2 -sASSERTIONS=1 --preload-file shaders" }
end

solution_config()
third_party_config()
project_config()
