project "DevGround"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "Source/**.h",
        "Source/**.cpp"
    }

    includedirs
    {
        "%{wks.location}/Hazel/vendors/spdlog/include",
        "%{wks.location}/Hazel/vendors/imGui",
        "%{wks.location}/Hazel/Source",
        "%{wks.location}/Hazel/vendors",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}"
    }

    links
    {
        "Hazel"
    }

    filter "system:windows"
        systemversion "latest"
  
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



