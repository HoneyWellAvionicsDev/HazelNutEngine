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
		"vendors/glm/glm/**.inl",
        "vendors/ImGuizmo/ImGuizmo.h",
        "vendors/ImGuizmo/ImGuizmo.cpp"
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
        "%{IncludeDir.Box2D}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.glfw}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.imGui}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.yaml_cpp}",
        "%{IncludeDir.VulkanSDK}"
    }

    links
    {
        "Box2D",
        "glfw",
        "Glad",
        "imGui",
        "opengl32.lib",
        "yaml-cpp"
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

        links
		{
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}"
		}

    filter "configurations:Release"
        defines "HZ_RELEASE"
        runtime "Release"
        optimize "on"

        links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}
    
    filter "configurations:Dist"
        defines "HZ_DIST"
        runtime "Release"
        optimize "on"

        links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}
