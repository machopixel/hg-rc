# HGVK.DLL: replacement renderer plugin (see docs/renderer.md).
# The Vulkan headers are vendored in third_party/vulkan; vulkan-1.dll is loaded at run time.
add_library(HGVK SHARED hgvk.c vk.c hud2d.c hgvk.def)
set_target_properties(HGVK PROPERTIES PREFIX "")
target_include_directories(HGVK PRIVATE ${CMAKE_SOURCE_DIR}/third_party/vulkan/include
                                        ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(HGVK PRIVATE user32 gdi32)
if(NOT MSVC)
    target_link_libraries(HGVK PRIVATE m)
endif()
