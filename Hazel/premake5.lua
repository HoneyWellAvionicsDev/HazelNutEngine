project "Hazel"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "hzpch.h"
    pchsource "Source/hzpch.cpp"

    files
    {
        "Source/**.h",
		"Source/**.cpp",
		"vendors/stb_image/**.h",
		"vendors/stb_image/**.cpp",
		"vendors/glm/glm/**.hpp",
		"vendors/glm/glm/**.inl"
    }

    defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
	}

    includedirs
    {
        "Source",
        "vendors/spdlog/include",
        "%{IncludeDir.glfw}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.imGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.yaml_cpp}",
        "%{IncludeDir.ImGuizmo}"
    }

    links
    {
        "glfw",
        "Glad",
        "imGui",
        "yaml-cpp",
        "opengl32.lib"
    }

    filter "files:vendors/ImGuizmo/**.cpp"
	flags { "NoPCH" }

    filter "system:windows"
        systemversion "latest"

        defines
        {
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