-- premake4.lua - meta-buildscript for the Tiny Little Mooltipass library
--
-- Copyright (c) 2015 Philipp Geyer (Nistur) nistur@gmail.com
-- 
-- This software is provided 'as-is', without any express or implied
-- warranty. In no event will the authors be held liable for any damages
-- arising from the use of this software.
-- 
-- Permission is granted to anyone to use this software for any purpose,
-- including commercial applications, and to alter it and redistribute it
-- freely, subject to the following restrictions:
-- 
-- 1. The origin of this software must not be misrepresented; you must not
--    claim that you wrote the original software. If you use this software
--    in a product, an acknowledgement in the product documentation would be
--    appreciated but is not required.
-- 2. Altered source versions must be plainly marked as such, and must not be
--    misrepresented as being the original software.
-- 3. This notice may not be removed or altered from any source distribution.

solution "tlmp"
    language "C++"
    configurations { "Debug", "Release" }
    includedirs { "include", "src/include" }
    files { "include/**.h" }

    defines { "TLMP_BUILD" }
    buildoptions { "`pkg-config --cflags libusb-1.0`" }
    linkoptions { "`pkg-config --libs libusb-1.0`" }

    configuration "Debug"
        defines { "DEBUG" }
        flags { "Symbols" }
        targetdir "build/debug"

    configuration "Release"
        defines { "NDEBUG" }
        flags { "OptimizeSpeed",
        	"ExtraWarnings",
        	"FatalWarnings",
        	"NoFramePointer" }
        targetdir "build/release"

    project "tlmp"
        kind "StaticLib"
        files { "src/**.c", "src/**.cpp" }

        --[[
        project "tlmp-dynamic"
        kind "SharedLib"
        files { "src/**.c", "src/**.cpp" }
        targetname "tlmp"
        --]]

    project "tlmp-log"
        kind "ConsoleApp"
        files { "tlmp-log/**.c" }
        links { "tlmp", "pthread" }

    project "tests"
        kind "ConsoleApp"
        files { "tests/**.cpp" }
        links { "tlmp" }
        configuration "Debug"
            postbuildcommands("build/debug/tests")
        configuration "Release"
            postbuildcommands("build/release/tests")
