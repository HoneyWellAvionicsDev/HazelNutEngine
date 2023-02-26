-- Dependencies

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["stb_image"] = "%{wks.location}/Hazel/vendors/stb_image"
IncludeDir["yaml_cpp"] = "%{wks.location}/Hazel/vendors/yaml-cpp/include"
IncludeDir["Box2D"] = "%{wks.location}/Hazel/vendors/Box2D/include"
IncludeDir["glfw"] = "%{wks.location}/Hazel/vendors/glfw/include"
IncludeDir["Glad"] = "%{wks.location}/Hazel/vendors/Glad/include"
IncludeDir["imGui"] = "%{wks.location}/Hazel/vendors/imGui"
IncludeDir["ImGuizmo"] = "%{wks.location}/Hazel/vendors/ImGuizmo"
IncludeDir["glm"] = "%{wks.location}/Hazel/vendors/glm"
IncludeDir["entt"] = "%{wks.location}/Hazel/vendors/entt/include"
IncludeDir["shaderc"] = "%{wks.location}/Hazel/vendors/shaderc/include"
IncludeDir["SPIRV_Cross"] = "%{wks.location}/Hazel/vendors/SPIRV-Cross"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["googletest"] = "%{wks.location}/Hazel/vendors/googletest/googletest/include"



LibraryDir = {}

LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Debug"] = "%{wks.location}/Hazel/vendors/VulkanSDK/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"


