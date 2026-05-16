-- premake5.lua

-- Global Variables
CREATE_LIB_DIRECTORY = "{MKDIR} %{cfg.targetdir}/../lib"
CREATE_INC_DIRECTORY = "{MKDIR} %{cfg.targetdir}/../inc"
CREATE_PUBLIC_INC_DIRECTORY = "{MKDIR} %{cfg.targetdir}/../inc/AssetSuite"
CLEAN_INC_DIRECTORY = "powershell -NoProfile -ExecutionPolicy Bypass -Command \"Remove-Item -LiteralPath '%{cfg.targetdir}/../inc' -Recurse -Force -ErrorAction SilentlyContinue\""
COPY_RELEASE_LIB_FILE = "{COPY} %{cfg.targetdir}/assetsuite_r.lib %{cfg.targetdir}/../lib"
COPY_DEBUG_LIB_FILE = "{COPY} %{cfg.targetdir}/assetsuite_d.lib %{cfg.targetdir}/../lib"
COPY_PUBLIC_HEADER_FILES = "{COPY} %{cfg.targetdir}/../../../include/AssetSuite/*.h %{cfg.targetdir}/../inc/AssetSuite"
RUN_PUBLIC_HEADER_HYGIENE_CHECK = "powershell -NoProfile -ExecutionPolicy Bypass -File %{cfg.targetdir}/../../../validation/public_header_hygiene/PublicHeaderHygiene.ps1 -Roots include/AssetSuite,bin/%{cfg.buildcfg}/inc"
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
	group "Validation"
		project "PublicHeaderCompile"
	group "AssetSuite"
		project "AssetSuite"
		project "zlib"
		project "bmp"
		project "png"
		project "ppm"
		project "bypass"
		project "wavefront"
		project "bitstream"
	group "Demo"
		project "LegacyDemoApplication"

project "AssetSuite"
    kind "SharedLib"
    language "C++"
	cppdialect "C++20"
    targetdir "bin/%{cfg.buildcfg}/bin"
	defines { "ASSETSUITE_EXPORTS" }
	links { "zlib", "bmp", "png", "ppm", "bypass", "wavefront", "bitstream" }
	includedirs { "include" }
	vpaths { ["Images"] = "bmp" }
	-- Copy some files over to have a full DLL release
	postbuildcommands {
		CREATE_LIB_DIRECTORY,
		CLEAN_INC_DIRECTORY,
		CREATE_INC_DIRECTORY,
		CREATE_PUBLIC_INC_DIRECTORY,
		COPY_PUBLIC_HEADER_FILES
	}
    files {
		"include/AssetSuite/**.h",
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

project "zlib"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	targetdir "bin/%{cfg.buildcfg}/bin"
	files { "source/zlib/**.h", "source/zlib/**.cpp" }
	SetDebugFilters()
	SetReleaseFilters()

project "bmp"
	kind "StaticLib"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}/bin"
	links { "zlib" }
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
	cppdialect "C++17"
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
	
project "LegacyDemoApplication"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	targetdir "bin/%{cfg.buildcfg}/demo"
	files { "source/demo/**.h", "source/demo/**.cpp" }
	links { "AssetSuite" }
	includedirs { "include" }
	SetDebugFilters()
	SetReleaseFilters()
	filter "configurations:Debug"
		postbuildcommands { "{COPY} %{cfg.targetdir}/../bin/assetsuite_d.dll %{cfg.targetdir}" }
		postbuildcommands { "{COPY} %{cfg.targetdir}/../../../test_images/girl_with_pearl_earring.bmp %{cfg.targetdir}" }
		postbuildcommands { "{COPY} %{cfg.targetdir}/../../../test_images/wavefront_sample.obj %{cfg.targetdir}" }
	filter "configurations:Release"
		postbuildcommands { "{COPY} %{cfg.targetdir}/../bin/assetsuite_r.dll %{cfg.targetdir}" }
		postbuildcommands { "{COPY} %{cfg.targetdir}/../../../test_images/girl_with_pearl_earring.bmp %{cfg.targetdir}" }
		postbuildcommands { "{COPY} %{cfg.targetdir}/../../../test_images/wavefront_sample.obj %{cfg.targetdir}" }
		
project "UnitTest"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	targetdir "bin/%{cfg.buildcfg}/tests"
	files { "unit_tests/**.h", "unit_tests/**.cpp" }
	links { "AssetSuite", "zlib", "bmp", "png", "ppm", "wavefront", "bitstream" }
	includedirs { "include" }
	SetDebugFilters()
	SetReleaseFilters()
	filter "configurations:Debug"
		postbuildcommands { "{COPY} %{cfg.targetdir}/../bin/assetsuite_d.dll %{cfg.targetdir}" }
		postbuildcommands { "{COPY} %{cfg.targetdir}/../../../test_images/* %{cfg.targetdir}" }
	filter "configurations:Release"
		postbuildcommands { "{COPY} %{cfg.targetdir}/../bin/assetsuite_r.dll %{cfg.targetdir}" }
		postbuildcommands { "{COPY} %{cfg.targetdir}/../../../test_images/* %{cfg.targetdir}" }

project "PublicHeaderCompile"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	targetdir "bin/%{cfg.buildcfg}/validation"
	files { "validation/public_header_compile/**.cpp", "validation/public_header_hygiene/**.ps1" }
	includedirs { "bin/%{cfg.buildcfg}/inc" }
	dependson { "AssetSuite" }
	prebuildcommands { RUN_PUBLIC_HEADER_HYGIENE_CHECK }
	SetDebugFilters()
	SetReleaseFilters()
