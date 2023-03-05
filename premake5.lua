include "Dependencies.lua"

workspace "Hazel"
    architecture "x64"
    startproject "Hazelnut"

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

group "Dependencies"
    include "vendor/premake"
    include "Hazel/vendors/Box2D"
    include "Hazel/vendors/Glad"
    include "Hazel/vendors/glfw"
    include "Hazel/vendors/imGui"
    include "Hazel/vendors/yaml-cpp"
group ""


include "Hazel"
include "DevGround"
include "Hazelnut"