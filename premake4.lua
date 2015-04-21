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
