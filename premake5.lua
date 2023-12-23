-- premake5.lua

-- Global Variables
CREATE_LIB_DIRECTORY = "{MKDIR} %{cfg.targetdir}/../lib"
CREATE_INC_DIRECTORY = "{MKDIR} %{cfg.targetdir}/../inc"
COPY_RELEASE_LIB_FILE = "{COPY} %{cfg.targetdir}/assetsuite_r.lib %{cfg.targetdir}/../lib"
COPY_DEBUG_LIB_FILE = "{COPY} %{cfg.targetdir}/assetsuite_d.lib %{cfg.targetdir}/../lib"
COPY_HEADER_FILES = "{COPY} %{cfg.targetdir}/../../../source/common/*.h %{cfg.targetdir}/../inc"
LOCATION_DIRECTORY_NAME = "build"

-- Global Functions
function SetDebugFilters()
	filter "configurations:Debug"
    	defines { "DEBUG" }
    	symbols "On"
end

function SetReleaseFilters()
	filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
		symbols "On"
end

-- Workspace
workspace "AssetSuite"
    configurations {
		"Debug",
		"Release"
	}
	system "Windows"
    architecture "x86_64"
	location(LOCATION_DIRECTORY_NAME)
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
    targetdir "bin/%{cfg.buildcfg}/bin"
	defines { "ASSETSUITE_EXPORTS" }
	links { "bmp", "png", "ppm", "bypass", "wavefront", "bitstream" }
	vpaths { ["Images"] = "bmp" }
	-- Copy some files over to have a full DLL release
	postbuildcommands {
		CREATE_LIB_DIRECTORY,
		CREATE_INC_DIRECTORY,
		COPY_HEADER_FILES
	}
    files {
		"source/common/**.h", "source/common/**.cpp"
	}
	SetDebugFilters()
	SetReleaseFilters()
	filter "configurations:Debug"
		targetname "assetsuite_d"
		postbuildcommands { COPY_DEBUG_LIB_FILE }
	filter "configurations:Release"
		targetname "assetsuite_r"
		postbuildcommands { COPY_RELEASE_LIB_FILE }
project "bmp"
	kind "StaticLib"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}/bin"
	files { "source/bmp/**.h", "source/bmp/**.cpp" }
	SetDebugFilters()
	SetReleaseFilters()
		
project "png"
	kind "StaticLib"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}/bin"
	files { "source/png/**.h", "source/png/**.cpp" }
	SetDebugFilters()
	SetReleaseFilters()
		
project "ppm"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	targetdir "bin/%{cfg.buildcfg}/bin"
	files { "source/ppm/**.h", "source/ppm/**.cpp" }
	SetDebugFilters()
	SetReleaseFilters()
		
project "bypass"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	targetdir "bin/%{cfg.buildcfg}/bin"
	files { "source/bypass/**.h", "source/bypass/**.cpp" }
	SetDebugFilters()
	SetReleaseFilters()
		
project "wavefront"
	kind "StaticLib"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}/bin"
	files { "source/wavefront/**.h", "source/wavefront/**.cpp" }
	SetDebugFilters()
	SetReleaseFilters()
		
project "bitstream"
	kind "StaticLib"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}/bin"
	files { "source/bitstream/**.h", "source/bitstream/**.cpp" }
	SetDebugFilters()
	SetReleaseFilters()
	
project "DemoApplication"
	kind "ConsoleApp"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}/demo"
	files { "source/demo/**.h", "source/demo/**.cpp" }
	links { "AssetSuite" }
	SetDebugFilters()
	SetReleaseFilters()
	filter "configurations:Debug"
		postbuildcommands { "{COPY} %{cfg.targetdir}/../bin/assetsuite_d.dll %{cfg.targetdir}" }
	filter "configurations:Release"
		postbuildcommands { "{COPY} %{cfg.targetdir}/../bin/assetsuite_r.dll %{cfg.targetdir}" }
		
project "UnitTest"
	kind "SharedLib"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}/tests"
	files { "unit_tests/**.h", "unit_tests/**.cpp" }
	links { "AssetSuite", "bmp", "png", "ppm", "wavefront", "bitstream" }
	SetDebugFilters()
	SetReleaseFilters()
	filter "configurations:Debug"
		postbuildcommands { "{COPY} %{cfg.targetdir}/../bin/assetsuite_d.dll %{cfg.targetdir}" }
		postbuildcommands { "{COPY} %{cfg.targetdir}/../../../test_images/* %{cfg.targetdir}" }
	filter "configurations:Release"
		postbuildcommands { "{COPY} %{cfg.targetdir}/../bin/assetsuite_r.dll %{cfg.targetdir}" }
		postbuildcommands { "{COPY} %{cfg.targetdir}/../../../test_images/* %{cfg.targetdir}" }