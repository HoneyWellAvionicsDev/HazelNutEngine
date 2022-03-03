workspace "Hazel"
    architecture "x64"
    startproject "DevGround"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["glfw"] = "Hazel/vendors/glfw/include"
IncludeDir["Glad"] = "Hazel/vendors/Glad/include"
IncludeDir["imGui"] = "Hazel/vendors/imGui"
IncludeDir["glm"] = "Hazel/vendors/glm"

include "Hazel/vendors/glfw"
include "Hazel/vendors/Glad"
include "Hazel/vendors/imGui"


project "Hazel"
    location "Hazel"
    kind "SharedLib"
    language "C++"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "hzpch.h"
    pchsource "Hazel/Source/hzpch.cpp"

    files
    {
        "%{prj.name}/Source/**.h",
        "%{prj.name}/Source/**.cpp",
        "%{prj.name}/vendor/glm/glm/**.hpp",                                --this can be removed to reduce clutter
        "%{prj.name}/vendor/glm/glm/**.inl"   
    }

    includedirs
    {
        "%{prj.name}/Source",
        "%{prj.name}/vendors/spdlog/include",
        "%{IncludeDir.glfw}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.imGui}",
        "%{IncludeDir.glm}"
    }

    links
    {
        "glfw",
        "Glad",
        "imGui",
        "opengl32.lib"
    }

    filter "system:windows"
        cppdialect "C++20"
        staticruntime "off"
        systemversion "latest"

        defines
        {
            "HZ_PLATFORM_WINDOWS",
            "HZ_BUILD_DLL",
            "_WINDLL",
            "HZ_ENABLE_ASSERTS",
            "GLFW_INCLUDE_NONE"
        }

        postbuildcommands
        {
            ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/DevGround")
        }
    
    filter "configurations:Debug"
        defines "HZ_DEBUG"
        staticruntime "off"
        symbols "On"

    filter "configurations:Release"
        defines "HZ_RELEASE"
        staticruntime "On"
        optimize "On"
    
    filter "configurations:Dist"
        defines "HZ_DIST"
        staticruntime "On"
        optimize "On"

    --filter { "system:windows" }
        --buildoptions "/MDd"
    
project "DevGround"
    location "DevGround"
    kind "ConsoleApp"
    language "C++"

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
        "%{IncludeDir.glm}"
    }

    links
    {
        "Hazel"
    }

    filter "system:windows"
        cppdialect "C++20"
        staticruntime "off"
        systemversion "latest"

        defines
        {
            "HZ_PLATFORM_WINDOWS"
        }
    
    filter "configurations:Debug"
        defines "HZ_DEBUG"
        staticruntime "off"
        symbols "On"

    filter "configurations:Release"
        defines "HZ_RELEASE"
        staticruntime "On"
        optimize "On"
    
    filter "configurations:Dist"
        defines "HZ_DIST"
        staticruntime "On"
        optimize "On"