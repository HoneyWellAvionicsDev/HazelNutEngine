project "Test"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{wks.location}/Test/**.h",
        "%{wks.location}/Test/**.cpp"
    }

    includedirs
    {
        "%{wks.location}/Hazel/vendors/spdlog/include",
		"%{wks.location}/Hazel/Source",
        "%{IncludeDir.googletest}"
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
		optimize "on"