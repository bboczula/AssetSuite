-- premake5.lua
workspace "AssetSuite"
    configurations { "Debug", "Release" }
	system "Windows"
    architecture "x86_64"
	location "build" 
	group "UnitTests"
		project "UnitTest"
	group "AssetSuite"
		project "AssetSuite"
		project "bmp"
		project "png"
		project "ppm"
		project "bypass"
		project "wavefront"
		project "bitstream"
	group "Demo"
		project "DemoApplication"

project "AssetSuite"
    kind "SharedLib"
    language "C++"
	cppdialect "C++20"
    targetdir "bin/%{cfg.buildcfg}"
	defines { "ASSETSUITE_EXPORTS" }
	links { "bmp", "png", "ppm", "bypass", "wavefront", "bitstream" }
	vpaths { ["Images"] = "bmp" }
    files {
		"source/common/**.h", "source/common/**.cpp"
	}
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
	
project "bmp"
	kind "StaticLib"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"
	files { "source/bmp/**.h", "source/bmp/**.cpp" }
	filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
		
project "png"
	kind "StaticLib"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"
	files { "source/png/**.h", "source/png/**.cpp" }
	filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
		
project "ppm"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	targetdir "bin/%{cfg.buildcfg}"
	files { "source/ppm/**.h", "source/ppm/**.cpp" }
	filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
		
project "bypass"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	targetdir "bin/%{cfg.buildcfg}"
	files { "source/bypass/**.h", "source/bypass/**.cpp" }
	filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
		
project "wavefront"
	kind "StaticLib"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"
	files { "source/wavefront/**.h", "source/wavefront/**.cpp" }
	filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
		
project "bitstream"
	kind "StaticLib"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"
	files { "source/bitstream/**.h", "source/bitstream/**.cpp" }
	filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
	
project "DemoApplication"
	kind "ConsoleApp"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"
	files { "source/demo/**.h", "source/demo/**.cpp" }
	links { "AssetSuite" }
	filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
		
project "UnitTest"
	kind "SharedLib"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"
	files { "unit_tests/**.h", "unit_tests/**.cpp" }
	links { "bmp", "png", "ppm", "wavefront", "bitstream" }
	filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"