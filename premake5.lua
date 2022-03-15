workspace "Hazel"
    architecture "x64"
    startproject "DevGround"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    flags
    {
        "MultiProcessorCompile"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["glfw"] = "Hazel/vendors/glfw/include"
IncludeDir["Glad"] = "Hazel/vendors/Glad/include"
IncludeDir["imGui"] = "Hazel/vendors/imGui"
IncludeDir["glm"] = "Hazel/vendors/glm"
IncludeDir["stb_image"] = "Hazel/vendors/stb_image"

group "Dependencies"
    include "Hazel/vendors/glfw"
    include "Hazel/vendors/Glad"
    include "Hazel/vendors/imGui"

group ""

project "Hazel"
    location "Hazel"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "On"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "hzpch.h"
    pchsource "Hazel/Source/hzpch.cpp"

    files
    {
        "%{prj.name}/Source/**.h",
        "%{prj.name}/Source/**.cpp",
        "%{prj.name}/vendors/stb_image/**.h",
        "%{prj.name}/vendors/stb_image/**.cpp",
        "%{prj.name}/vendors/glm/glm/**.hpp",                                --this can be removed to reduce clutter
        "%{prj.name}/vendors/glm/glm/**.inl"   
    }

    includedirs
    {
        "%{prj.name}/Source",
        "%{prj.name}/vendors/spdlog/include",
        "%{IncludeDir.glfw}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.imGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}"

    }

    links
    {
        "glfw",
        "Glad",
        "imGui",
        "opengl32.lib"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "HZ_PLATFORM_WINDOWS",
            "HZ_BUILD_DLL",
            "_WINDLL",
            "HZ_ENABLE_ASSERTS",
            "GLFW_INCLUDE_NONE"
        }
    
    filter "configurations:Debug"
        defines "HZ_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "HZ_RELEASE"
        runtime "Release"
        optimize "on"
    
    filter "configurations:Dist"
        defines "HZ_DIST"
        runtime "Release"
        optimize "on"

    --filter { "system:windows" }
        --buildoptions "/MDd"
    
project "DevGround"
    location "DevGround"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/Source/**.h",
        "%{prj.name}/Source/**.cpp"
    }

    includedirs
    {
        "Hazel/vendors/spdlog/include",
        "Hazel/Source",
        "%{IncludeDir.glm}",
        "%{IncludeDir.imGui}"
    }

    links
    {
        "Hazel"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "HZ_PLATFORM_WINDOWS"
        }
    
    filter "configurations:Debug"
        defines "HZ_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "HZ_RELEASE"
        runtime "Release"
        optimize "on"
    
    filter "configurations:Dist"
        defines "HZ_DIST"
        runtime "Release"
        optimize "On"

    project "Hazel-Nut"
    location "Hazel-Nut"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/Source/**.h",
        "%{prj.name}/Source/**.cpp"
    }

    includedirs
    {
        "Hazel/vendors/spdlog/include",
        "Hazel/Source",
        "%{IncludeDir.glm}",
        "%{IncludeDir.imGui}"
    }

    links
    {
        "Hazel"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "HZ_PLATFORM_WINDOWS"
        }
    
    filter "configurations:Debug"
        defines "HZ_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "HZ_RELEASE"
        runtime "Release"
        optimize "on"
    
    filter "configurations:Dist"
        defines "HZ_DIST"
        runtime "Release"
        optimize "On"