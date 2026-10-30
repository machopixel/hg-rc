# Vulkan headers

Copies of `vk_platform.h`, `vulkan.h`, `vulkan_core.h`, `vulkan_win32.h` and the `vk_video` folder from the
Khronos Vulkan-Headers (version 1.3.275, Apache-2.0, see the SPDX line at the top of
each file). They are only needed to compile HGVK; the plugin loads `vulkan-1.dll`
at run time, so the Vulkan SDK is not required to build or run the game.
