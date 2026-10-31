/*
 * Vulkan back end of HGVK.DLL: swapchain, one render pass, pipelines for the
 * game's handful of render states, a texture cache and the 2D overlay.
 * vulkan-1.dll is loaded at run time, so the SDK is not needed to build.
 */
#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <vulkan/vulkan.h>

#include "vk.h"

#include "hgvk_frag.spv.h"
#include "hgvk_vert.spv.h"

#define SCREEN_W 640
#define SCREEN_H 480
#define FRAMES 2
#define VERTEX_BUFFER_BYTES (24u * 1024u * 1024u)
#define MAX_TEXTURES 3000
#define HASH_SIZE 4096
#define MAX_SETS 4096

#define VK_FUNCTIONS(X) \
    X(vkDestroyInstance) X(vkEnumeratePhysicalDevices) X(vkGetPhysicalDeviceProperties) \
    X(vkGetPhysicalDeviceQueueFamilyProperties) X(vkGetPhysicalDeviceMemoryProperties) \
    X(vkCreateDevice) X(vkEnumerateDeviceExtensionProperties) X(vkDestroySurfaceKHR) \
    X(vkGetPhysicalDeviceSurfaceSupportKHR) X(vkGetPhysicalDeviceSurfaceCapabilitiesKHR) \
    X(vkGetPhysicalDeviceSurfaceFormatsKHR) X(vkGetPhysicalDeviceSurfacePresentModesKHR) \
    X(vkCreateWin32SurfaceKHR) X(vkDestroyDevice) X(vkGetDeviceQueue) \
    X(vkCreateSwapchainKHR) X(vkDestroySwapchainKHR) X(vkGetSwapchainImagesKHR) \
    X(vkAcquireNextImageKHR) X(vkQueuePresentKHR) X(vkCreateImageView) X(vkDestroyImageView) \
    X(vkCreateRenderPass) X(vkDestroyRenderPass) X(vkCreateFramebuffer) \
    X(vkDestroyFramebuffer) X(vkCreateShaderModule) X(vkDestroyShaderModule) \
    X(vkCreatePipelineLayout) X(vkDestroyPipelineLayout) X(vkCreateGraphicsPipelines) \
    X(vkDestroyPipeline) X(vkCreateDescriptorSetLayout) X(vkDestroyDescriptorSetLayout) \
    X(vkCreateDescriptorPool) X(vkDestroyDescriptorPool) X(vkAllocateDescriptorSets) \
    X(vkFreeDescriptorSets) X(vkUpdateDescriptorSets) X(vkCreateSampler) X(vkDestroySampler) \
    X(vkCreateBuffer) X(vkDestroyBuffer) X(vkGetBufferMemoryRequirements) \
    X(vkAllocateMemory) X(vkFreeMemory) X(vkBindBufferMemory) X(vkMapMemory) \
    X(vkUnmapMemory) X(vkCreateImage) X(vkDestroyImage) X(vkGetImageMemoryRequirements) \
    X(vkBindImageMemory) X(vkCreateCommandPool) X(vkDestroyCommandPool) \
    X(vkAllocateCommandBuffers) X(vkBeginCommandBuffer) X(vkEndCommandBuffer) \
    X(vkResetCommandBuffer) X(vkCmdBeginRenderPass) X(vkCmdEndRenderPass) \
    X(vkCmdBindPipeline) X(vkCmdBindVertexBuffers) X(vkCmdBindDescriptorSets) \
    X(vkCmdPushConstants) X(vkCmdDraw) X(vkCmdSetViewport) X(vkCmdSetScissor) \
    X(vkCmdPipelineBarrier) X(vkCmdCopyBufferToImage) X(vkQueueSubmit) X(vkQueueWaitIdle) \
    X(vkDeviceWaitIdle) X(vkCreateSemaphore) X(vkDestroySemaphore) X(vkCreateFence) \
    X(vkDestroyFence) X(vkWaitForFences) X(vkResetFences) \
    X(vkGetPhysicalDeviceFormatProperties)

static struct {
#define X(name) PFN_##name name;
    VK_FUNCTIONS(X)
#undef X
} vk;

struct HgvkTexture {
    const void *key;
    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;
    VkDescriptorSet set;
    int alpha;         /* made with HGVK_KEY_ALPHA */
    HgvkTexture *next; /* hash chain */
};

typedef struct Draw {
    uint32_t first, count;
    uint8_t topology, blend, sampler, keyed, painter;
    HgvkTexture *tex;
} Draw;

/* Matches the push_constant block of both shaders. */
typedef struct PushConstants {
    uint32_t flags;   /* bit 0: colour-keyed texture */
    float depthScale; /* iz * depthScale = depth in 0..1 */
} PushConstants;

typedef struct FrameResources {
    VkCommandBuffer cmd;
    VkFence fence;
    VkSemaphore imageAvailable;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexMemory;
    HgvkVertex *vertexMapped;
    VkBuffer hudBuffer;
    VkDeviceMemory hudMemory;
    uint32_t *hudMapped;
    VkBuffer hud2Buffer; /* the full-resolution layer, sized by EnsureHud2 */
    VkDeviceMemory hud2Memory;
    uint32_t *hud2Mapped;
} FrameResources;

static struct {
    HMODULE lib;
    PFN_vkGetInstanceProcAddr getProc;
    HWND hwnd;
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice gpu;
    VkPhysicalDeviceMemoryProperties memProps;
    VkDevice device;
    uint32_t queueFamily;
    VkQueue queue;

    VkSwapchainKHR swapchain;
    VkFormat format;
    VkExtent2D extent;
    LONG clientW, clientH; /* window size the swapchain was made for */
    uint32_t imageCount;
    VkImage *images;
    VkImageView *views;
    VkFramebuffer *framebuffers;
    VkSemaphore *renderFinished;
    VkRenderPass renderPass;

    /*
     * Optional W-buffer (hgrc.cfg "nozbuf" turns it off). The game only gives
     * a painter's order; the depth test corrects the overlaps it gets wrong,
     * and hgvk.c makes each later Render call slightly nearer so the game's
     * order still decides ties (decals on the ground and similar).
     */
    int depthOn;
    VkFormat depthFormat;
    VkImage depthImage;
    VkDeviceMemory depthMemory;
    VkImageView depthView;

    VkDescriptorSetLayout textureLayout, samplerLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipelines[3][2]; /* [topology][blend] */
    VkPipeline painterPipelines[3][2]; /* the same without depth test or write: 1.2's particles (round 32) */
    VkPipeline overlayPipeline;  /* 2D overlay: blended, no depth test */
    VkSampler samplers[4];
    VkDescriptorSet samplerSets[4];
    VkDescriptorPool pool;
    VkShaderModule vert, frag;

    VkCommandPool cmdPool;
    VkCommandBuffer uploadCmd;
    FrameResources frames[FRAMES];
    unsigned frameIndex;

    HgvkTexture white;
    HgvkTexture hud;
    int hudReady;
    HgvkTexture hud2;
    int hud2Ready;
    int hud2W, hud2H;

    HgvkTexture *hash[HASH_SIZE];
    int textureCount;

    HgvkVertex *verts;
    uint32_t vertCount, vertCap;
    Draw *draws;
    uint32_t drawCount, drawCap;

    int ready;
} g;

static const char *VkName(VkResult r)
{
    switch (r) {
    case VK_SUCCESS: return "VK_SUCCESS";
    case VK_NOT_READY: return "VK_NOT_READY";
    case VK_TIMEOUT: return "VK_TIMEOUT";
    case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
    default: return "VkResult(?)";
    }
}

#define TRY(call)                                                              \
    do {                                                                       \
        VkResult r_ = (call);                                                  \
        if (r_ != VK_SUCCESS) {                                                \
            HgvkLog("Vulkan: %s failed: %s (%d)", #call, VkName(r_), (int)r_); \
            goto fail;                                                         \
        }                                                                      \
    } while (0)

static int FindMemoryType(uint32_t typeBits, VkMemoryPropertyFlags want)
{
    uint32_t i;

    for (i = 0; i < g.memProps.memoryTypeCount; i++) {
        if ((typeBits & (1u << i)) &&
            (g.memProps.memoryTypes[i].propertyFlags & want) == want)
            return (int)i;
    }
    return -1;
}

static int CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer *buffer,
                        VkDeviceMemory *memory, void **mapped)
{
    VkBufferCreateInfo bi = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    VkMemoryRequirements req;
    VkMemoryAllocateInfo ai = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    int type;

    bi.size = size;
    bi.usage = usage;
    bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    TRY(vk.vkCreateBuffer(g.device, &bi, NULL, buffer));
    vk.vkGetBufferMemoryRequirements(g.device, *buffer, &req);
    type = FindMemoryType(req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (type < 0) {
        HgvkLog("Vulkan: no host-visible memory type");
        goto fail;
    }
    ai.allocationSize = req.size;
    ai.memoryTypeIndex = (uint32_t)type;
    TRY(vk.vkAllocateMemory(g.device, &ai, NULL, memory));
    TRY(vk.vkBindBufferMemory(g.device, *buffer, *memory, 0));
    TRY(vk.vkMapMemory(g.device, *memory, 0, VK_WHOLE_SIZE, 0, mapped));
    return 0;
fail:
    return 1;
}

static void DestroyBuffer(VkBuffer *buffer, VkDeviceMemory *memory, void **mapped)
{
    if (*mapped != NULL)
        vk.vkUnmapMemory(g.device, *memory);
    if (*buffer != VK_NULL_HANDLE)
        vk.vkDestroyBuffer(g.device, *buffer, NULL);
    if (*memory != VK_NULL_HANDLE)
        vk.vkFreeMemory(g.device, *memory, NULL);
    *buffer = VK_NULL_HANDLE;
    *memory = VK_NULL_HANDLE;
    *mapped = NULL;
}

static void ImageBarrier(VkCommandBuffer cmd, VkImage image, VkImageLayout from,
                         VkImageLayout to, VkAccessFlags srcAccess, VkAccessFlags dstAccess,
                         VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage)
{
    VkImageMemoryBarrier b = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};

    b.srcAccessMask = srcAccess;
    b.dstAccessMask = dstAccess;
    b.oldLayout = from;
    b.newLayout = to;
    b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.image = image;
    b.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    b.subresourceRange.levelCount = 1;
    b.subresourceRange.layerCount = 1;
    vk.vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, NULL, 0, NULL, 1, &b);
}

/* An RGBA8 sampled image with its view and descriptor set. */
static int CreateImageTexture(HgvkTexture *t, int w, int h, const uint32_t *rgba,
                              int uploadNow)
{
    VkImageCreateInfo ii = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    VkMemoryRequirements req;
    VkMemoryAllocateInfo ai = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    VkImageViewCreateInfo vi = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    VkDescriptorSetAllocateInfo di = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    VkDescriptorImageInfo imageInfo;
    VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    VkBuffer staging = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
    void *stagingMapped = NULL;
    int type;

    ii.imageType = VK_IMAGE_TYPE_2D;
    ii.format = VK_FORMAT_R8G8B8A8_UNORM;
    ii.extent.width = (uint32_t)w;
    ii.extent.height = (uint32_t)h;
    ii.extent.depth = 1;
    ii.mipLevels = 1;
    ii.arrayLayers = 1;
    ii.samples = VK_SAMPLE_COUNT_1_BIT;
    ii.tiling = VK_IMAGE_TILING_OPTIMAL;
    ii.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ii.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ii.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    TRY(vk.vkCreateImage(g.device, &ii, NULL, &t->image));
    vk.vkGetImageMemoryRequirements(g.device, t->image, &req);
    type = FindMemoryType(req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (type < 0)
        type = FindMemoryType(req.memoryTypeBits, 0);
    if (type < 0)
        goto fail;
    ai.allocationSize = req.size;
    ai.memoryTypeIndex = (uint32_t)type;
    TRY(vk.vkAllocateMemory(g.device, &ai, NULL, &t->memory));
    TRY(vk.vkBindImageMemory(g.device, t->image, t->memory, 0));

    vi.image = t->image;
    vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vi.format = VK_FORMAT_R8G8B8A8_UNORM;
    vi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vi.subresourceRange.levelCount = 1;
    vi.subresourceRange.layerCount = 1;
    TRY(vk.vkCreateImageView(g.device, &vi, NULL, &t->view));

    di.descriptorPool = g.pool;
    di.descriptorSetCount = 1;
    di.pSetLayouts = &g.textureLayout;
    TRY(vk.vkAllocateDescriptorSets(g.device, &di, &t->set));
    imageInfo.sampler = VK_NULL_HANDLE;
    imageInfo.imageView = t->view;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    write.dstSet = t->set;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    write.pImageInfo = &imageInfo;
    vk.vkUpdateDescriptorSets(g.device, 1, &write, 0, NULL);

    if (uploadNow) {
        VkCommandBufferBeginInfo bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        VkBufferImageCopy copy;
        VkSubmitInfo si = {VK_STRUCTURE_TYPE_SUBMIT_INFO};

        if (CreateBuffer((VkDeviceSize)w * h * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &staging,
                         &stagingMemory, &stagingMapped))
            goto fail;
        memcpy(stagingMapped, rgba, (size_t)w * h * 4);

        bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        TRY(vk.vkResetCommandBuffer(g.uploadCmd, 0));
        TRY(vk.vkBeginCommandBuffer(g.uploadCmd, &bi));
        ImageBarrier(g.uploadCmd, t->image, VK_IMAGE_LAYOUT_UNDEFINED,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
                     VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
        memset(&copy, 0, sizeof(copy));
        copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy.imageSubresource.layerCount = 1;
        copy.imageExtent.width = (uint32_t)w;
        copy.imageExtent.height = (uint32_t)h;
        copy.imageExtent.depth = 1;
        vk.vkCmdCopyBufferToImage(g.uploadCmd, staging, t->image,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
        ImageBarrier(g.uploadCmd, t->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT,
                     VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
        TRY(vk.vkEndCommandBuffer(g.uploadCmd));
        si.commandBufferCount = 1;
        si.pCommandBuffers = &g.uploadCmd;
        TRY(vk.vkQueueSubmit(g.queue, 1, &si, VK_NULL_HANDLE));
        TRY(vk.vkQueueWaitIdle(g.queue));
        DestroyBuffer(&staging, &stagingMemory, &stagingMapped);
    }
    return 0;
fail:
    DestroyBuffer(&staging, &stagingMemory, &stagingMapped);
    return 1;
}

static void DestroyImageTexture(HgvkTexture *t)
{
    if (t->set != VK_NULL_HANDLE)
        vk.vkFreeDescriptorSets(g.device, g.pool, 1, &t->set);
    if (t->view != VK_NULL_HANDLE)
        vk.vkDestroyImageView(g.device, t->view, NULL);
    if (t->image != VK_NULL_HANDLE)
        vk.vkDestroyImage(g.device, t->image, NULL);
    if (t->memory != VK_NULL_HANDLE)
        vk.vkFreeMemory(g.device, t->memory, NULL);
    t->set = VK_NULL_HANDLE;
    t->view = VK_NULL_HANDLE;
    t->image = VK_NULL_HANDLE;
    t->memory = VK_NULL_HANDLE;
}

/* ---- Texture cache ------------------------------------------------------ */

static unsigned HashKey(const void *key)
{
    return (unsigned)(((uintptr_t)key >> 3) * 2654435761u) % HASH_SIZE;
}

HgvkTexture *Vk_FindTexture(const void *key)
{
    HgvkTexture *t;

    if (!g.ready)
        return NULL;
    for (t = g.hash[HashKey(key)]; t != NULL; t = t->next) {
        if (t->key == key)
            return t;
    }
    return NULL;
}

int Vk_TextureHasAlpha(const HgvkTexture *t)
{
    return t != NULL && t->alpha;
}

HgvkTexture *Vk_CreateTexture(const void *key, int w, int h, const uint8_t *pixels,
                              const uint32_t *palette, int keyIndex)
{
    HgvkTexture *t;
    uint32_t *rgba;
    int i;

    if (!g.ready || w <= 0 || h <= 0 || w > 4096 || h > 4096 || g.textureCount >= MAX_TEXTURES)
        return NULL;
    rgba = (uint32_t *)malloc((size_t)w * h * 4);
    t = (HgvkTexture *)calloc(1, sizeof(*t));
    if (rgba == NULL || t == NULL) {
        free(rgba);
        free(t);
        return NULL;
    }
    for (i = 0; i < w * h; i++) {
        unsigned idx = pixels[i];
        uint32_t c = palette[idx];

        if (keyIndex == HGVK_KEY_ALPHA)
            rgba[i] = ((c >> 16) & 0xff) | (c & 0xff00) | ((c & 0xff) << 16) |
                      ((15u - ((c >> 28) & 15u)) * 17u << 24);
        else
            rgba[i] = ((c >> 16) & 0xff) | (c & 0xff00) | ((c & 0xff) << 16) |
                      (idx == (unsigned)keyIndex ? 0u : 0xff000000u);
    }
    t->key = key;
    t->alpha = keyIndex == HGVK_KEY_ALPHA;
    if (CreateImageTexture(t, w, h, rgba, 1)) {
        DestroyImageTexture(t);
        free(t);
        free(rgba);
        return NULL;
    }
    free(rgba);
    t->next = g.hash[HashKey(key)];
    g.hash[HashKey(key)] = t;
    g.textureCount++;
    return t;
}

static void FreeTexture(HgvkTexture *t)
{
    DestroyImageTexture(t);
    free(t);
    g.textureCount--;
}

void Vk_DropTexture(const void *key)
{
    HgvkTexture **link;

    if (!g.ready)
        return;
    vk.vkDeviceWaitIdle(g.device);
    for (link = &g.hash[HashKey(key)]; *link != NULL; link = &(*link)->next) {
        if ((*link)->key == key) {
            HgvkTexture *dead = *link;

            *link = dead->next;
            FreeTexture(dead);
            return;
        }
    }
}

static int g_dropAll;

void Vk_RequestDropAllTextures(void)
{
    g_dropAll = 1;
}

static void DropAllTextures(void)
{
    int i;

    for (i = 0; i < HASH_SIZE; i++) {
        while (g.hash[i] != NULL) {
            HgvkTexture *dead = g.hash[i];

            g.hash[i] = dead->next;
            FreeTexture(dead);
        }
    }
}

/* ---- Swapchain ---------------------------------------------------------- */

static void DestroyDepth(void)
{
    if (g.depthView != VK_NULL_HANDLE)
        vk.vkDestroyImageView(g.device, g.depthView, NULL);
    if (g.depthImage != VK_NULL_HANDLE)
        vk.vkDestroyImage(g.device, g.depthImage, NULL);
    if (g.depthMemory != VK_NULL_HANDLE)
        vk.vkFreeMemory(g.device, g.depthMemory, NULL);
    g.depthView = VK_NULL_HANDLE;
    g.depthImage = VK_NULL_HANDLE;
    g.depthMemory = VK_NULL_HANDLE;
}

static int CreateDepth(void)
{
    VkImageCreateInfo ii = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    VkMemoryRequirements req;
    VkMemoryAllocateInfo ai = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    VkImageViewCreateInfo vi = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    int type;

    ii.imageType = VK_IMAGE_TYPE_2D;
    ii.format = g.depthFormat;
    ii.extent.width = g.extent.width;
    ii.extent.height = g.extent.height;
    ii.extent.depth = 1;
    ii.mipLevels = 1;
    ii.arrayLayers = 1;
    ii.samples = VK_SAMPLE_COUNT_1_BIT;
    ii.tiling = VK_IMAGE_TILING_OPTIMAL;
    ii.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    ii.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ii.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    TRY(vk.vkCreateImage(g.device, &ii, NULL, &g.depthImage));
    vk.vkGetImageMemoryRequirements(g.device, g.depthImage, &req);
    type = FindMemoryType(req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (type < 0)
        type = FindMemoryType(req.memoryTypeBits, 0);
    if (type < 0)
        goto fail;
    ai.allocationSize = req.size;
    ai.memoryTypeIndex = (uint32_t)type;
    TRY(vk.vkAllocateMemory(g.device, &ai, NULL, &g.depthMemory));
    TRY(vk.vkBindImageMemory(g.device, g.depthImage, g.depthMemory, 0));
    vi.image = g.depthImage;
    vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vi.format = g.depthFormat;
    vi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    vi.subresourceRange.levelCount = 1;
    vi.subresourceRange.layerCount = 1;
    TRY(vk.vkCreateImageView(g.device, &vi, NULL, &g.depthView));
    return 0;
fail:
    return 1;
}

/* The first depth format the GPU can render to, or VK_FORMAT_UNDEFINED. */
static VkFormat PickDepthFormat(void)
{
    static const VkFormat candidates[3] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_X8_D24_UNORM_PACK32,
                                           VK_FORMAT_D16_UNORM};
    int i;

    for (i = 0; i < 3; i++) {
        VkFormatProperties fp;

        vk.vkGetPhysicalDeviceFormatProperties(g.gpu, candidates[i], &fp);
        if (fp.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            return candidates[i];
    }
    return VK_FORMAT_UNDEFINED;
}

static void DestroySwapchain(int keepHandle)
{
    uint32_t i;

    for (i = 0; i < g.imageCount; i++) {
        if (g.framebuffers != NULL && g.framebuffers[i] != VK_NULL_HANDLE)
            vk.vkDestroyFramebuffer(g.device, g.framebuffers[i], NULL);
        if (g.views != NULL && g.views[i] != VK_NULL_HANDLE)
            vk.vkDestroyImageView(g.device, g.views[i], NULL);
        if (g.renderFinished != NULL && g.renderFinished[i] != VK_NULL_HANDLE)
            vk.vkDestroySemaphore(g.device, g.renderFinished[i], NULL);
    }
    DestroyDepth();
    free(g.framebuffers);
    free(g.views);
    free(g.renderFinished);
    free(g.images);
    g.framebuffers = NULL;
    g.views = NULL;
    g.renderFinished = NULL;
    g.images = NULL;
    g.imageCount = 0;
    if (!keepHandle && g.swapchain != VK_NULL_HANDLE) {
        vk.vkDestroySwapchainKHR(g.device, g.swapchain, NULL);
        g.swapchain = VK_NULL_HANDLE;
    }
}

static int CreateSwapchain(void)
{
    VkSurfaceCapabilitiesKHR caps;
    VkSurfaceFormatKHR formats[32];
    uint32_t formatCount = 32, i, wantImages;
    VkSwapchainCreateInfoKHR sci = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    VkSwapchainKHR old = g.swapchain;
    RECT rc;
    VkSurfaceFormatKHR chosen;

    GetClientRect(g.hwnd, &rc);
    g.clientW = rc.right;
    g.clientH = rc.bottom;
    TRY(vk.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g.gpu, g.surface, &caps));
    TRY(vk.vkGetPhysicalDeviceSurfaceFormatsKHR(g.gpu, g.surface, &formatCount, formats));
    chosen = formats[0];
    for (i = 0; i < formatCount; i++) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM &&
            formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            chosen = formats[i];
            break;
        }
    }
    if (caps.currentExtent.width != 0xffffffffu) {
        g.extent = caps.currentExtent;
    } else {
        g.extent.width = (uint32_t)rc.right;
        g.extent.height = (uint32_t)rc.bottom;
    }
    if (g.extent.width == 0 || g.extent.height == 0)
        return 2; /* minimised: try again next frame */

    wantImages = caps.minImageCount + 1;
    if (caps.maxImageCount != 0 && wantImages > caps.maxImageCount)
        wantImages = caps.maxImageCount;
    sci.surface = g.surface;
    sci.minImageCount = wantImages;
    sci.imageFormat = chosen.format;
    sci.imageColorSpace = chosen.colorSpace;
    sci.imageExtent = g.extent;
    sci.imageArrayLayers = 1;
    sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    sci.preTransform = caps.currentTransform;
    sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    sci.clipped = VK_TRUE;
    sci.oldSwapchain = old;
    TRY(vk.vkCreateSwapchainKHR(g.device, &sci, NULL, &g.swapchain));
    if (old != VK_NULL_HANDLE)
        vk.vkDestroySwapchainKHR(g.device, old, NULL);
    g.format = chosen.format;

    TRY(vk.vkGetSwapchainImagesKHR(g.device, g.swapchain, &g.imageCount, NULL));
    g.images = (VkImage *)calloc(g.imageCount, sizeof(VkImage));
    g.views = (VkImageView *)calloc(g.imageCount, sizeof(VkImageView));
    g.framebuffers = (VkFramebuffer *)calloc(g.imageCount, sizeof(VkFramebuffer));
    g.renderFinished = (VkSemaphore *)calloc(g.imageCount, sizeof(VkSemaphore));
    TRY(vk.vkGetSwapchainImagesKHR(g.device, g.swapchain, &g.imageCount, g.images));

    if (g.renderPass == VK_NULL_HANDLE) {
        VkAttachmentDescription att, attDepth;
        VkAttachmentDescription atts[2];
        VkAttachmentReference ref, refDepth;
        VkSubpassDescription sub;
        VkSubpassDependency dep;
        VkRenderPassCreateInfo rpi = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};

        memset(&att, 0, sizeof(att));
        att.format = g.format;
        att.samples = VK_SAMPLE_COUNT_1_BIT;
        att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        ref.attachment = 0;
        ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        memset(&sub, 0, sizeof(sub));
        sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        sub.colorAttachmentCount = 1;
        sub.pColorAttachments = &ref;
        memset(&dep, 0, sizeof(dep));
        dep.srcSubpass = VK_SUBPASS_EXTERNAL;
        dep.dstSubpass = 0;
        dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        atts[0] = att;
        rpi.attachmentCount = 1;
        if (g.depthOn) {
            memset(&attDepth, 0, sizeof(attDepth));
            attDepth.format = g.depthFormat;
            attDepth.samples = VK_SAMPLE_COUNT_1_BIT;
            attDepth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attDepth.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attDepth.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attDepth.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attDepth.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attDepth.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            refDepth.attachment = 1;
            refDepth.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            sub.pDepthStencilAttachment = &refDepth;
            dep.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            dep.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            dep.srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dep.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                 VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            atts[1] = attDepth;
            rpi.attachmentCount = 2;
        }
        rpi.pAttachments = atts;
        rpi.subpassCount = 1;
        rpi.pSubpasses = &sub;
        rpi.dependencyCount = 1;
        rpi.pDependencies = &dep;
        TRY(vk.vkCreateRenderPass(g.device, &rpi, NULL, &g.renderPass));
    }

    if (g.depthOn && CreateDepth())
        goto fail;
    for (i = 0; i < g.imageCount; i++) {
        VkImageViewCreateInfo vi = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        VkImageView fbViews[2];
        VkFramebufferCreateInfo fi = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        VkSemaphoreCreateInfo si = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

        vi.image = g.images[i];
        vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vi.format = g.format;
        vi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        vi.subresourceRange.levelCount = 1;
        vi.subresourceRange.layerCount = 1;
        TRY(vk.vkCreateImageView(g.device, &vi, NULL, &g.views[i]));
        fbViews[0] = g.views[i];
        fbViews[1] = g.depthView;
        fi.renderPass = g.renderPass;
        fi.attachmentCount = g.depthOn ? 2 : 1;
        fi.pAttachments = fbViews;
        fi.width = g.extent.width;
        fi.height = g.extent.height;
        fi.layers = 1;
        TRY(vk.vkCreateFramebuffer(g.device, &fi, NULL, &g.framebuffers[i]));
        TRY(vk.vkCreateSemaphore(g.device, &si, NULL, &g.renderFinished[i]));
    }
    HgvkLog("Vulkan: swapchain %ux%u, %u images, format %d", g.extent.width, g.extent.height,
            g.imageCount, (int)g.format);
    return 0;
fail:
    return 1;
}

static int RecreateSwapchain(void)
{
    vk.vkDeviceWaitIdle(g.device);
    DestroySwapchain(1);
    return CreateSwapchain();
}

/* ---- Setup and teardown ------------------------------------------------- */

static int LoadFunctions(void)
{
#define X(name)                                                                 \
    vk.name = (PFN_##name)g.getProc(g.instance, #name);                         \
    if (vk.name == NULL) {                                                      \
        HgvkLog("Vulkan: missing function %s", #name);                          \
        return 1;                                                               \
    }
    VK_FUNCTIONS(X)
#undef X
    return 0;
}

static int CreatePipelines(void)
{
    VkPipelineShaderStageCreateInfo stages[2];
    VkVertexInputBindingDescription binding;
    VkVertexInputAttributeDescription attrs[4];
    VkPipelineVertexInputStateCreateInfo vin = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    VkPipelineInputAssemblyStateCreateInfo ia = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    VkPipelineViewportStateCreateInfo vp = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    VkPipelineRasterizationStateCreateInfo rs = {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    VkPipelineMultisampleStateCreateInfo ms = {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    VkPipelineColorBlendAttachmentState blendAtt;
    VkPipelineColorBlendStateCreateInfo cb = {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    VkDynamicState dynStates[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dyn = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    VkPipelineDepthStencilStateCreateInfo ds = {
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    static const VkPrimitiveTopology topo[3] = {VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                                                VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
                                                VK_PRIMITIVE_TOPOLOGY_POINT_LIST};
    int t, b;

    memset(stages, 0, sizeof(stages));
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = g.vert;
    stages[0].pName = "main";
    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = g.frag;
    stages[1].pName = "main";

    binding.binding = 0;
    binding.stride = sizeof(HgvkVertex);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    attrs[0].location = 0;
    attrs[0].binding = 0;
    attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attrs[0].offset = offsetof(HgvkVertex, x);
    attrs[1].location = 1;
    attrs[1].binding = 0;
    attrs[1].format = VK_FORMAT_R8G8B8A8_UNORM;
    attrs[1].offset = offsetof(HgvkVertex, rgba);
    attrs[2].location = 2;
    attrs[2].binding = 0;
    attrs[2].format = VK_FORMAT_R32G32_SFLOAT;
    attrs[2].offset = offsetof(HgvkVertex, u);
    attrs[3].location = 3;
    attrs[3].binding = 0;
    attrs[3].format = VK_FORMAT_R32_SFLOAT;
    attrs[3].offset = offsetof(HgvkVertex, iz);
    vin.vertexBindingDescriptionCount = 1;
    vin.pVertexBindingDescriptions = &binding;
    vin.vertexAttributeDescriptionCount = 4;
    vin.pVertexAttributeDescriptions = attrs;

    vp.viewportCount = 1;
    vp.scissorCount = 1;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_NONE;
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.lineWidth = 1.0f;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    dyn.dynamicStateCount = 2;
    dyn.pDynamicStates = dynStates;

    memset(&blendAtt, 0, sizeof(blendAtt));
    blendAtt.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    cb.attachmentCount = 1;
    cb.pAttachments = &blendAtt;

    /* Larger depth = nearer (depth is rhw scaled), so the clear is 0. */
    ds.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;

    for (t = 0; t < 4; t++) {
        for (b = 0; b < 2; b++) {
            VkGraphicsPipelineCreateInfo pi = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
            int overlay = t == 3;

            if (overlay && !b)
                continue;

            ia.topology = topo[overlay ? 0 : t];
            ds.depthTestEnable = g.depthOn && !overlay ? VK_TRUE : VK_FALSE;
            /* Translucent polygons are tested but do not hide what follows. */
            ds.depthWriteEnable = g.depthOn && !overlay && !b ? VK_TRUE : VK_FALSE;
            blendAtt.blendEnable = b ? VK_TRUE : VK_FALSE;
            /* The overlays hold premultiplied colours. */
            blendAtt.srcColorBlendFactor =
                overlay ? VK_BLEND_FACTOR_ONE : VK_BLEND_FACTOR_SRC_ALPHA;
            blendAtt.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            blendAtt.colorBlendOp = VK_BLEND_OP_ADD;
            blendAtt.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            blendAtt.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            blendAtt.alphaBlendOp = VK_BLEND_OP_ADD;
            pi.stageCount = 2;
            pi.pStages = stages;
            pi.pVertexInputState = &vin;
            pi.pInputAssemblyState = &ia;
            pi.pViewportState = &vp;
            pi.pRasterizationState = &rs;
            pi.pMultisampleState = &ms;
            pi.pColorBlendState = &cb;
            pi.pDepthStencilState = &ds;
            pi.pDynamicState = &dyn;
            pi.layout = g.pipelineLayout;
            pi.renderPass = g.renderPass;
            TRY(vk.vkCreateGraphicsPipelines(g.device, VK_NULL_HANDLE, 1, &pi, NULL,
                                             overlay ? &g.overlayPipeline : &g.pipelines[t][b]));
            if (!overlay) {
                /* painter's order, as 1.2's renderer drew everything (it had no depth buffer): for 1.2's particle
                   squares, which carry no depth (round 32) */
                ds.depthTestEnable = VK_FALSE;
                ds.depthWriteEnable = VK_FALSE;
                TRY(vk.vkCreateGraphicsPipelines(g.device, VK_NULL_HANDLE, 1, &pi, NULL, &g.painterPipelines[t][b]));
            }
        }
    }
    return 0;
fail:
    return 1;
}

static int PickDevice(void)
{
    VkPhysicalDevice gpus[16];
    uint32_t count = 16, i, q;
    int best = -1, bestScore = -1;
    uint32_t bestFamily = 0;

    TRY(vk.vkEnumeratePhysicalDevices(g.instance, &count, gpus));
    for (i = 0; i < count; i++) {
        VkPhysicalDeviceProperties props;
        VkQueueFamilyProperties fam[16];
        uint32_t famCount = 16;
        VkExtensionProperties exts[256];
        uint32_t extCount = 256, e;
        int hasSwapchain = 0, score;

        vk.vkGetPhysicalDeviceProperties(gpus[i], &props);
        vk.vkEnumerateDeviceExtensionProperties(gpus[i], NULL, &extCount, exts);
        for (e = 0; e < extCount; e++) {
            if (strcmp(exts[e].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
                hasSwapchain = 1;
        }
        vk.vkGetPhysicalDeviceQueueFamilyProperties(gpus[i], &famCount, fam);
        for (q = 0; q < famCount && hasSwapchain; q++) {
            VkBool32 present = VK_FALSE;

            vk.vkGetPhysicalDeviceSurfaceSupportKHR(gpus[i], q, g.surface, &present);
            if ((fam[q].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present) {
                score = props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 2 : 1;
                HgvkLog("Vulkan: GPU %u '%s' type %d queue family %u", i, props.deviceName,
                        (int)props.deviceType, q);
                if (score > bestScore) {
                    bestScore = score;
                    best = (int)i;
                    bestFamily = q;
                }
                break;
            }
        }
    }
    if (best < 0) {
        HgvkLog("Vulkan: no GPU with graphics, present and swapchain support");
        return 1;
    }
    g.gpu = gpus[best];
    g.queueFamily = bestFamily;
    vk.vkGetPhysicalDeviceMemoryProperties(g.gpu, &g.memProps);
    return 0;
fail:
    return 1;
}

void Vk_Shutdown(void)
{
    unsigned i;

    if (g.device != VK_NULL_HANDLE) {
        vk.vkDeviceWaitIdle(g.device);
        DropAllTextures();
        for (i = 0; i < FRAMES; i++) {
            FrameResources *f = &g.frames[i];
            void *vm = f->vertexMapped, *hm = f->hudMapped, *h2m = f->hud2Mapped;

            DestroyBuffer(&f->vertexBuffer, &f->vertexMemory, &vm);
            DestroyBuffer(&f->hudBuffer, &f->hudMemory, &hm);
            DestroyBuffer(&f->hud2Buffer, &f->hud2Memory, &h2m);
            f->hud2Mapped = NULL;
            if (f->fence != VK_NULL_HANDLE)
                vk.vkDestroyFence(g.device, f->fence, NULL);
            if (f->imageAvailable != VK_NULL_HANDLE)
                vk.vkDestroySemaphore(g.device, f->imageAvailable, NULL);
        }
        DestroyImageTexture(&g.white);
        DestroyImageTexture(&g.hud);
        DestroyImageTexture(&g.hud2);
        DestroySwapchain(0);
        for (i = 0; i < 6; i++) {
            if (g.pipelines[i / 2][i % 2] != VK_NULL_HANDLE)
                vk.vkDestroyPipeline(g.device, g.pipelines[i / 2][i % 2], NULL);
            if (g.painterPipelines[i / 2][i % 2] != VK_NULL_HANDLE)
                vk.vkDestroyPipeline(g.device, g.painterPipelines[i / 2][i % 2], NULL);
        }
        if (g.overlayPipeline != VK_NULL_HANDLE)
            vk.vkDestroyPipeline(g.device, g.overlayPipeline, NULL);
        for (i = 0; i < 4; i++) {
            if (g.samplers[i] != VK_NULL_HANDLE)
                vk.vkDestroySampler(g.device, g.samplers[i], NULL);
        }
        if (g.pool != VK_NULL_HANDLE)
            vk.vkDestroyDescriptorPool(g.device, g.pool, NULL);
        if (g.pipelineLayout != VK_NULL_HANDLE)
            vk.vkDestroyPipelineLayout(g.device, g.pipelineLayout, NULL);
        if (g.textureLayout != VK_NULL_HANDLE)
            vk.vkDestroyDescriptorSetLayout(g.device, g.textureLayout, NULL);
        if (g.samplerLayout != VK_NULL_HANDLE)
            vk.vkDestroyDescriptorSetLayout(g.device, g.samplerLayout, NULL);
        if (g.vert != VK_NULL_HANDLE)
            vk.vkDestroyShaderModule(g.device, g.vert, NULL);
        if (g.frag != VK_NULL_HANDLE)
            vk.vkDestroyShaderModule(g.device, g.frag, NULL);
        if (g.renderPass != VK_NULL_HANDLE)
            vk.vkDestroyRenderPass(g.device, g.renderPass, NULL);
        if (g.cmdPool != VK_NULL_HANDLE)
            vk.vkDestroyCommandPool(g.device, g.cmdPool, NULL);
        vk.vkDestroyDevice(g.device, NULL);
    }
    if (g.instance != VK_NULL_HANDLE) {
        if (g.surface != VK_NULL_HANDLE)
            vk.vkDestroySurfaceKHR(g.instance, g.surface, NULL);
        vk.vkDestroyInstance(g.instance, NULL);
    }
    free(g.verts);
    free(g.draws);
    if (g.lib != NULL)
        FreeLibrary(g.lib);
    memset(&g, 0, sizeof(g));
}

int Vk_IsReady(void)
{
    return g.ready;
}

int Vk_Init(HWND hwnd, int depthBuffer)
{
    VkApplicationInfo app = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    VkInstanceCreateInfo ici = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    const char *instExts[2] = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
    PFN_vkCreateInstance createInstance;
    VkWin32SurfaceCreateInfoKHR wsi = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    float priority = 1.0f;
    VkDeviceQueueCreateInfo qci = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    const char *devExts[1] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkDeviceCreateInfo dci = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    VkCommandPoolCreateInfo cpi = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    VkCommandBufferAllocateInfo cai = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    VkDescriptorSetLayoutBinding lb;
    VkDescriptorSetLayoutCreateInfo lci = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    VkPushConstantRange pcr;
    unsigned i;
    int rc;

    memset(&g, 0, sizeof(g));
    g.hwnd = hwnd;
    g.lib = LoadLibraryA("vulkan-1.dll");
    if (g.lib == NULL) {
        HgvkLog("Vulkan: vulkan-1.dll not found");
        return 1;
    }
    g.getProc = (PFN_vkGetInstanceProcAddr)GetProcAddress(g.lib, "vkGetInstanceProcAddr");
    createInstance = g.getProc ? (PFN_vkCreateInstance)g.getProc(NULL, "vkCreateInstance") : NULL;
    if (createInstance == NULL) {
        HgvkLog("Vulkan: vkCreateInstance not available");
        goto fail;
    }
    app.pApplicationName = "Heavy Gear";
    app.apiVersion = VK_API_VERSION_1_0;
    ici.pApplicationInfo = &app;
    ici.enabledExtensionCount = 2;
    ici.ppEnabledExtensionNames = instExts;
    HgvkLog("Vulkan: creating instance");
    TRY(createInstance(&ici, NULL, &g.instance));
    if (LoadFunctions())
        goto fail;

    HgvkLog("Vulkan: creating surface");
    wsi.hinstance = GetModuleHandleA(NULL);
    wsi.hwnd = hwnd;
    TRY(vk.vkCreateWin32SurfaceKHR(g.instance, &wsi, NULL, &g.surface));
    if (PickDevice())
        goto fail;
    if (depthBuffer) {
        g.depthFormat = PickDepthFormat();
        g.depthOn = g.depthFormat != VK_FORMAT_UNDEFINED;
        HgvkLog("Vulkan: depth buffer %s (format %d)", g.depthOn ? "on" : "unavailable",
                (int)g.depthFormat);
    } else {
        HgvkLog("Vulkan: depth buffer off (nozbuf)");
    }

    HgvkLog("Vulkan: creating device");
    qci.queueFamilyIndex = g.queueFamily;
    qci.queueCount = 1;
    qci.pQueuePriorities = &priority;
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos = &qci;
    dci.enabledExtensionCount = 1;
    dci.ppEnabledExtensionNames = devExts;
    TRY(vk.vkCreateDevice(g.gpu, &dci, NULL, &g.device));
    vk.vkGetDeviceQueue(g.device, g.queueFamily, 0, &g.queue);

    HgvkLog("Vulkan: device created, setting up pools and shaders");
    cpi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cpi.queueFamilyIndex = g.queueFamily;
    TRY(vk.vkCreateCommandPool(g.device, &cpi, NULL, &g.cmdPool));
    cai.commandPool = g.cmdPool;
    cai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cai.commandBufferCount = 1;
    TRY(vk.vkAllocateCommandBuffers(g.device, &cai, &g.uploadCmd));

    {
        VkShaderModuleCreateInfo smi = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};

        smi.codeSize = sizeof(hgvk_vert_spv);
        smi.pCode = hgvk_vert_spv;
        TRY(vk.vkCreateShaderModule(g.device, &smi, NULL, &g.vert));
        smi.codeSize = sizeof(hgvk_frag_spv);
        smi.pCode = hgvk_frag_spv;
        TRY(vk.vkCreateShaderModule(g.device, &smi, NULL, &g.frag));
    }

    memset(&lb, 0, sizeof(lb));
    lb.binding = 0;
    lb.descriptorCount = 1;
    lb.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    lb.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    lci.bindingCount = 1;
    lci.pBindings = &lb;
    TRY(vk.vkCreateDescriptorSetLayout(g.device, &lci, NULL, &g.textureLayout));
    lb.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    TRY(vk.vkCreateDescriptorSetLayout(g.device, &lci, NULL, &g.samplerLayout));
    {
        VkDescriptorSetLayout layouts[2];
        VkPipelineLayoutCreateInfo pli = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

        layouts[0] = g.textureLayout;
        layouts[1] = g.samplerLayout;
        pcr.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pcr.offset = 0;
        pcr.size = sizeof(PushConstants);
        pli.setLayoutCount = 2;
        pli.pSetLayouts = layouts;
        pli.pushConstantRangeCount = 1;
        pli.pPushConstantRanges = &pcr;
        TRY(vk.vkCreatePipelineLayout(g.device, &pli, NULL, &g.pipelineLayout));
    }
    {
        VkDescriptorPoolSize sizes[2];
        VkDescriptorPoolCreateInfo dpi = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};

        sizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        sizes[0].descriptorCount = MAX_SETS;
        sizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
        sizes[1].descriptorCount = 8;
        dpi.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        dpi.maxSets = MAX_SETS + 8;
        dpi.poolSizeCount = 2;
        dpi.pPoolSizes = sizes;
        TRY(vk.vkCreateDescriptorPool(g.device, &dpi, NULL, &g.pool));
    }
    for (i = 0; i < 4; i++) {
        VkSamplerCreateInfo sci = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        VkDescriptorSetAllocateInfo dai = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        VkDescriptorImageInfo info;
        VkWriteDescriptorSet w = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};

        sci.magFilter = sci.minFilter = (i & 2) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        sci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        sci.addressModeU = sci.addressModeV = sci.addressModeW =
            (i & 1) ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : VK_SAMPLER_ADDRESS_MODE_REPEAT;
        TRY(vk.vkCreateSampler(g.device, &sci, NULL, &g.samplers[i]));
        dai.descriptorPool = g.pool;
        dai.descriptorSetCount = 1;
        dai.pSetLayouts = &g.samplerLayout;
        TRY(vk.vkAllocateDescriptorSets(g.device, &dai, &g.samplerSets[i]));
        info.sampler = g.samplers[i];
        info.imageView = VK_NULL_HANDLE;
        info.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        w.dstSet = g.samplerSets[i];
        w.descriptorCount = 1;
        w.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        w.pImageInfo = &info;
        vk.vkUpdateDescriptorSets(g.device, 1, &w, 0, NULL);
    }

    HgvkLog("Vulkan: creating swapchain");
    rc = CreateSwapchain();
    if (rc == 2)
        rc = 0; /* minimised at start: the swapchain is made on the first frame */
    if (rc)
        goto fail;
    if (g.renderPass == VK_NULL_HANDLE) {
        HgvkLog("Vulkan: window has no size yet, cannot create the render pass");
        goto fail;
    }
    HgvkLog("Vulkan: creating pipelines");
    if (CreatePipelines())
        goto fail;

    for (i = 0; i < FRAMES; i++) {
        FrameResources *f = &g.frames[i];
        VkCommandBufferAllocateInfo fai = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        VkFenceCreateInfo fci = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        VkSemaphoreCreateInfo sei = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        void *mapped = NULL;

        fai.commandPool = g.cmdPool;
        fai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        fai.commandBufferCount = 1;
        TRY(vk.vkAllocateCommandBuffers(g.device, &fai, &f->cmd));
        fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        TRY(vk.vkCreateFence(g.device, &fci, NULL, &f->fence));
        TRY(vk.vkCreateSemaphore(g.device, &sei, NULL, &f->imageAvailable));
        if (CreateBuffer(VERTEX_BUFFER_BYTES, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                         &f->vertexBuffer, &f->vertexMemory, &mapped))
            goto fail;
        f->vertexMapped = (HgvkVertex *)mapped;
        mapped = NULL;
        if (CreateBuffer((VkDeviceSize)SCREEN_W * SCREEN_H * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         &f->hudBuffer, &f->hudMemory, &mapped))
            goto fail;
        f->hudMapped = (uint32_t *)mapped;
    }

    HgvkLog("Vulkan: creating per-frame resources done, uploading helper textures");
    {
        uint32_t whitePixel = 0xffffffffu;

        if (CreateImageTexture(&g.white, 1, 1, &whitePixel, 1))
            goto fail;
        if (CreateImageTexture(&g.hud, SCREEN_W, SCREEN_H, NULL, 0))
            goto fail;
    }
    g.ready = 1;
    HgvkLog("Vulkan: ready");
    return 0;
fail:
    HgvkLog("Vulkan: initialisation failed");
    Vk_Shutdown();
    return 1;
}

/* ---- Frame -------------------------------------------------------------- */

void Vk_AddDraw(int topology, int blend, int sampler, int keyed, int painter, HgvkTexture *tex,
                const HgvkVertex *verts, int count)
{
    Draw *last;
    const uint32_t maxVerts = VERTEX_BUFFER_BYTES / sizeof(HgvkVertex) - 16;

    if (!g.ready || count <= 0)
        return;
    if (g.vertCount + (uint32_t)count > maxVerts)
        return; /* more than one frame can hold: drop the rest */
    if (g.vertCount + (uint32_t)count > g.vertCap) {
        uint32_t cap = g.vertCap ? g.vertCap * 2 : 65536;
        HgvkVertex *grown;

        while (cap < g.vertCount + (uint32_t)count)
            cap *= 2;
        if (cap > maxVerts)
            cap = maxVerts;
        grown = (HgvkVertex *)realloc(g.verts, (size_t)cap * sizeof(HgvkVertex));
        if (grown == NULL)
            return;
        g.verts = grown;
        g.vertCap = cap;
    }
    memcpy(g.verts + g.vertCount, verts, (size_t)count * sizeof(HgvkVertex));

    last = g.drawCount ? &g.draws[g.drawCount - 1] : NULL;
    if (last != NULL && last->topology == topology && last->blend == blend &&
        last->sampler == sampler && last->keyed == keyed && last->painter == painter && last->tex == tex) {
        last->count += (uint32_t)count;
    } else {
        Draw *d;

        if (g.drawCount == g.drawCap) {
            uint32_t cap = g.drawCap ? g.drawCap * 2 : 4096;
            Draw *grown = (Draw *)realloc(g.draws, (size_t)cap * sizeof(Draw));

            if (grown == NULL)
                return;
            g.draws = grown;
            g.drawCap = cap;
        }
        d = &g.draws[g.drawCount++];
        d->first = g.vertCount;
        d->count = (uint32_t)count;
        d->topology = (uint8_t)topology;
        d->blend = (uint8_t)blend;
        d->sampler = (uint8_t)sampler;
        d->keyed = (uint8_t)keyed;
        d->painter = (uint8_t)(painter != 0);
        d->tex = tex;
    }
    g.vertCount += (uint32_t)count;
}

static void ResetBatch(void)
{
    g.vertCount = 0;
    g.drawCount = 0;
}

static uint8_t expand5[32], expand6[64];
static int expandReady;

/* (Re)creates the full-resolution layer's image and upload buffers for w x h. 0: ready. */
static int EnsureHud2(int w, int h)
{
    unsigned i;

    if (g.hud2W == w && g.hud2H == h && g.hud2.image != VK_NULL_HANDLE)
        return 0;
    vk.vkDeviceWaitIdle(g.device);
    DestroyImageTexture(&g.hud2);
    for (i = 0; i < FRAMES; i++) {
        FrameResources *f = &g.frames[i];
        void *m = f->hud2Mapped;

        DestroyBuffer(&f->hud2Buffer, &f->hud2Memory, &m);
        f->hud2Mapped = NULL;
    }
    g.hud2W = g.hud2H = 0;
    g.hud2Ready = 0;
    if (CreateImageTexture(&g.hud2, w, h, NULL, 0))
        goto fail;
    for (i = 0; i < FRAMES; i++) {
        FrameResources *f = &g.frames[i];
        void *m = NULL;

        if (CreateBuffer((VkDeviceSize)w * (VkDeviceSize)h * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         &f->hud2Buffer, &f->hud2Memory, &m))
            goto fail;
        f->hud2Mapped = (uint32_t *)m;
    }
    g.hud2W = w;
    g.hud2H = h;
    HgvkLog("Vulkan: full-resolution HUD layer %dx%d", w, h);
    return 0;
fail:
    HgvkLog("Vulkan: full-resolution HUD layer %dx%d could not be created", w, h);
    DestroyImageTexture(&g.hud2);
    for (i = 0; i < FRAMES; i++) {
        FrameResources *f = &g.frames[i];
        void *m = f->hud2Mapped;

        DestroyBuffer(&f->hud2Buffer, &f->hud2Memory, &m);
        f->hud2Mapped = NULL;
    }
    return 1;
}

void Vk_Present(const uint16_t *hud, int hudStrideBytes, const uint32_t *hud2, int hud2W,
                int hud2H)
{
    FrameResources *f = &g.frames[g.frameIndex];
    uint32_t imageIndex, i, quadFirst;
    VkResult r;
    VkCommandBufferBeginInfo bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    VkRenderPassBeginInfo rbi = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    VkClearValue clear[2];
    PushConstants pc;
    float maxIz = 0.0f;
    VkViewport viewport;
    VkRect2D scissor;
    VkDeviceSize zero = 0;
    VkPipeline bound = VK_NULL_HANDLE;
    HgvkTexture *boundTex = NULL;
    int boundSampler = -1;
    RECT rc;
    uint32_t drawVerts = g.vertCount;
    VkSubmitInfo si = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkPresentInfoKHR pi = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    VkBufferImageCopy copy;
    uint32_t px, py;
    float scale, vw, vh;
    int useHud2 = 0;

    if (!g.ready)
        return;

    /* Follow window size changes. */
    GetClientRect(g.hwnd, &rc);
    if (g.swapchain == VK_NULL_HANDLE || rc.right != g.clientW || rc.bottom != g.clientH) {
        if (g.swapchain != VK_NULL_HANDLE) {
            int rcs = RecreateSwapchain();

            if (rcs == 1) {
                ResetBatch();
                return;
            }
        } else if (CreateSwapchain() != 0) {
            ResetBatch();
            return;
        }
    }
    if (g.imageCount == 0) {
        ResetBatch();
        return;
    }

    r = vk.vkWaitForFences(g.device, 1, &f->fence, VK_TRUE, UINT64_MAX);
    if (r != VK_SUCCESS) {
        HgvkLog("Vulkan: vkWaitForFences: %s", VkName(r));
        ResetBatch();
        return;
    }
    r = vk.vkAcquireNextImageKHR(g.device, g.swapchain, UINT64_MAX, f->imageAvailable,
                                 VK_NULL_HANDLE, &imageIndex);
    if (r == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapchain();
        ResetBatch();
        return;
    }
    if (r != VK_SUCCESS && r != VK_SUBOPTIMAL_KHR) {
        HgvkLog("Vulkan: vkAcquireNextImageKHR: %s", VkName(r));
        ResetBatch();
        return;
    }
    vk.vkResetFences(g.device, 1, &f->fence);

    /* Vertices, then the two overlay quads (sim's 2D, full-resolution 2D) after them. */
    memcpy(f->vertexMapped, g.verts, (size_t)drawVerts * sizeof(HgvkVertex));
    quadFirst = drawVerts;
    {
        static const float qx[6] = {0, 640, 640, 0, 640, 0};
        static const float qy[6] = {0, 0, 480, 0, 480, 480};

        for (i = 0; i < 12; i++) {
            HgvkVertex *v = &f->vertexMapped[quadFirst + i];

            v->x = qx[i % 6];
            v->y = qy[i % 6];
            v->rhw = -1.0f; /* screen space */
            v->iz = -1.0f;  /* no depth */
            v->rgba = 0xffffffffu;
            v->u = qx[i % 6] / 640.0f;
            v->v = qy[i % 6] / 480.0f;
        }
    }
    if (hud2 != NULL && hud2W > 0 && hud2H > 0 && EnsureHud2(hud2W, hud2H) == 0) {
        memcpy(f->hud2Mapped, hud2, (size_t)hud2W * (size_t)hud2H * 4);
        useHud2 = 1;
    }

    /* RGB565 overlay to RGBA8, 0x0000 is transparent. */
    if (!expandReady) {
        for (px = 0; px < 64; px++) {
            expand5[px & 31] = (uint8_t)((px & 31) * 255 / 31);
            expand6[px] = (uint8_t)(px * 255 / 63);
        }
        expandReady = 1;
    }
    for (py = 0; py < SCREEN_H; py++) {
        const uint16_t *src = (const uint16_t *)((const uint8_t *)hud + (size_t)py * hudStrideBytes);
        uint32_t *dst = f->hudMapped + (size_t)py * SCREEN_W;

        for (px = 0; px < SCREEN_W; px++) {
            uint32_t p = src[px];

            if (p == 0) {
                dst[px] = 0;
            } else {
                dst[px] = expand5[p >> 11] | ((uint32_t)expand6[(p >> 5) & 63] << 8) |
                          ((uint32_t)expand5[p & 31] << 16) | 0xff000000u;
            }
        }
    }

    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vk.vkResetCommandBuffer(f->cmd, 0);
    vk.vkBeginCommandBuffer(f->cmd, &bi);

    ImageBarrier(f->cmd, g.hud.image, g.hudReady ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                                 : VK_IMAGE_LAYOUT_UNDEFINED,
                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                 g.hudReady ? VK_ACCESS_SHADER_READ_BIT : 0, VK_ACCESS_TRANSFER_WRITE_BIT,
                 g.hudReady ? VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
                            : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                 VK_PIPELINE_STAGE_TRANSFER_BIT);
    memset(&copy, 0, sizeof(copy));
    copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy.imageSubresource.layerCount = 1;
    copy.imageExtent.width = SCREEN_W;
    copy.imageExtent.height = SCREEN_H;
    copy.imageExtent.depth = 1;
    vk.vkCmdCopyBufferToImage(f->cmd, f->hudBuffer, g.hud.image,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
    ImageBarrier(f->cmd, g.hud.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT,
                 VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    g.hudReady = 1;
    if (useHud2) {
        ImageBarrier(f->cmd, g.hud2.image, g.hud2Ready ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                                       : VK_IMAGE_LAYOUT_UNDEFINED,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     g.hud2Ready ? VK_ACCESS_SHADER_READ_BIT : 0, VK_ACCESS_TRANSFER_WRITE_BIT,
                     g.hud2Ready ? VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
                                 : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                     VK_PIPELINE_STAGE_TRANSFER_BIT);
        copy.imageExtent.width = (uint32_t)g.hud2W;
        copy.imageExtent.height = (uint32_t)g.hud2H;
        vk.vkCmdCopyBufferToImage(f->cmd, f->hud2Buffer, g.hud2.image,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
        ImageBarrier(f->cmd, g.hud2.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT,
                     VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
        g.hud2Ready = 1;
    }

    memset(clear, 0, sizeof(clear));
    clear[0].color.float32[3] = 1.0f;
    clear[1].depthStencil.depth = 0.0f; /* farthest */
    rbi.renderPass = g.renderPass;
    rbi.framebuffer = g.framebuffers[imageIndex];
    rbi.renderArea.extent = g.extent;
    rbi.clearValueCount = g.depthOn ? 2 : 1;
    rbi.pClearValues = clear;

    /* Scale this frame's depth range to 0..1. */
    for (i = 0; i < drawVerts; i++) {
        if (g.verts[i].iz > maxIz)
            maxIz = g.verts[i].iz;
    }
    pc.depthScale = maxIz > 0.0f ? 1.0f / (maxIz * 1.0001f) : 1.0f;
    vk.vkCmdBeginRenderPass(f->cmd, &rbi, VK_SUBPASS_CONTENTS_INLINE);

    /* 4:3 picture centred in the window. */
    scale = (float)g.extent.width / SCREEN_W;
    if ((float)g.extent.height / SCREEN_H < scale)
        scale = (float)g.extent.height / SCREEN_H;
    vw = SCREEN_W * scale;
    vh = SCREEN_H * scale;
    viewport.x = ((float)g.extent.width - vw) * 0.5f;
    viewport.y = ((float)g.extent.height - vh) * 0.5f;
    viewport.width = vw;
    viewport.height = vh;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    scissor.offset.x = (int32_t)viewport.x;
    scissor.offset.y = (int32_t)viewport.y;
    scissor.extent.width = (uint32_t)vw;
    scissor.extent.height = (uint32_t)vh;
    vk.vkCmdSetViewport(f->cmd, 0, 1, &viewport);
    vk.vkCmdSetScissor(f->cmd, 0, 1, &scissor);
    vk.vkCmdBindVertexBuffers(f->cmd, 0, 1, &f->vertexBuffer, &zero);

    for (i = 0; i < g.drawCount; i++) {
        const Draw *d = &g.draws[i];
        VkPipeline pipe = d->painter ? g.painterPipelines[d->topology][d->blend] : g.pipelines[d->topology][d->blend];
        HgvkTexture *tex = d->tex != NULL ? d->tex : &g.white;

        if (pipe != bound) {
            vk.vkCmdBindPipeline(f->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
            bound = pipe;
        }
        if (tex != boundTex) {
            vk.vkCmdBindDescriptorSets(f->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.pipelineLayout,
                                       0, 1, &tex->set, 0, NULL);
            boundTex = tex;
        }
        if (d->sampler != boundSampler) {
            vk.vkCmdBindDescriptorSets(f->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.pipelineLayout,
                                       1, 1, &g.samplerSets[d->sampler], 0, NULL);
            boundSampler = d->sampler;
        }
        pc.flags = d->keyed ? 1u : 0u;
        vk.vkCmdPushConstants(f->cmd, g.pipelineLayout,
                              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                              sizeof(pc), &pc);
        vk.vkCmdDraw(f->cmd, d->count, 1, d->first, 0);
    }

    /* Overlay. */
    {
        pc.flags = 0;
        vk.vkCmdBindPipeline(f->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.overlayPipeline);
        vk.vkCmdBindDescriptorSets(f->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.pipelineLayout, 0,
                                   1, &g.hud.set, 0, NULL);
        vk.vkCmdBindDescriptorSets(f->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.pipelineLayout, 1,
                                   1, &g.samplerSets[1], 0, NULL);
        vk.vkCmdPushConstants(f->cmd, g.pipelineLayout,
                              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                              sizeof(pc), &pc);
        vk.vkCmdDraw(f->cmd, 6, 1, quadFirst, 0);
    }
    /* Full-resolution layer, 1:1 over the picture. */
    if (useHud2) {
        vk.vkCmdBindDescriptorSets(f->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.pipelineLayout, 0,
                                   1, &g.hud2.set, 0, NULL);
        vk.vkCmdBindDescriptorSets(f->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.pipelineLayout, 1,
                                   1, &g.samplerSets[1], 0, NULL);
        vk.vkCmdDraw(f->cmd, 6, 1, quadFirst + 6, 0);
    }
    vk.vkCmdEndRenderPass(f->cmd);
    vk.vkEndCommandBuffer(f->cmd);

    si.waitSemaphoreCount = 1;
    si.pWaitSemaphores = &f->imageAvailable;
    si.pWaitDstStageMask = &waitStage;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &f->cmd;
    si.signalSemaphoreCount = 1;
    si.pSignalSemaphores = &g.renderFinished[imageIndex];
    r = vk.vkQueueSubmit(g.queue, 1, &si, f->fence);
    if (r != VK_SUCCESS)
        HgvkLog("Vulkan: vkQueueSubmit: %s", VkName(r));

    pi.waitSemaphoreCount = 1;
    pi.pWaitSemaphores = &g.renderFinished[imageIndex];
    pi.swapchainCount = 1;
    pi.pSwapchains = &g.swapchain;
    pi.pImageIndices = &imageIndex;
    r = vk.vkQueuePresentKHR(g.queue, &pi);
    if (r == VK_ERROR_OUT_OF_DATE_KHR || r == VK_SUBOPTIMAL_KHR)
        RecreateSwapchain();
    else if (r != VK_SUCCESS)
        HgvkLog("Vulkan: vkQueuePresentKHR: %s", VkName(r));

    g.frameIndex = (g.frameIndex + 1) % FRAMES;
    ResetBatch();

    /* Keep the texture count (and descriptor sets) bounded; also after a palette change (Vk_RequestDropAllTextures). */
    if (g.textureCount >= MAX_TEXTURES - 8 || g_dropAll) {
        g_dropAll = 0;
        HgvkLog("Vulkan: dropping %d cached textures", g.textureCount);
        vk.vkDeviceWaitIdle(g.device);
        DropAllTextures();
    }
}
