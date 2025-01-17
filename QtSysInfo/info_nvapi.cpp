//
// Getting Nvidia GPU Usage
//
// Reference: Open Hardware Monitor (http://code.google.com/p/open-hardware-monitor)
//

#include <windows.h>
#include <iostream>

#if defined(_M_X64) || defined(__amd64__)
#define NVAPI_DLL "nvapi64.dll"
#else
#define NVAPI_DLL "nvapi.dll"
#endif

// magic numbers, do not change them
#define NVAPI_MAX_PHYSICAL_GPUS 64
#define NVAPI_MAX_USAGES_PER_GPU 34

// struct
typedef struct
{
    unsigned long version;                   //!< Structure version
    unsigned long long dedicatedVideoMemory; //!< Size(in bytes) of the physical framebuffer.
    unsigned long long
        availableDedicatedVideoMemory; //!< Size(in bytes) of the available physical framebuffer for allocating video memory surfaces.
    unsigned long long systemVideoMemory; //!< Size(in bytes) of system memory the driver allocates at load time.
    unsigned long long
        sharedSystemMemory; //!< Size(in bytes) of shared system memory that driver is allowed to commit for surfaces across all allocations.
    unsigned long long
        curAvailableDedicatedVideoMemory; //!< Size(in bytes) of the current available physical framebuffer for allocating video memory surfaces.
    unsigned long long
        dedicatedVideoMemoryEvictionsSize; //!< Size(in bytes) of the total size of memory released as a result of the evictions.
    unsigned long long
        dedicatedVideoMemoryEvictionCount; //!< Indicates the number of eviction events that caused an allocation to be removed from dedicated video memory to free GPU //!< video memory to make room for other allocations.
    unsigned long long
        dedicatedVideoMemoryPromotionsSize; //!< Size(in bytes) of the total size of memory allocated as a result of the promotions.
    unsigned long long
        dedicatedVideoMemoryPromotionCount; //!< Indicates the number of promotion events that caused an allocation to be promoted to dedicated video memory
} NV_GPU_MEMORY_INFO_EX_V1;
typedef NV_GPU_MEMORY_INFO_EX_V1 NV_GPU_MEMORY_INFO_EX;
#define MAKE_NVAPI_VERSION(typeName, ver) (unsigned long)(sizeof(typeName) | ((ver) << 16))
#define NV_GPU_MEMORY_INFO_EX_VER_1 MAKE_NVAPI_VERSION(NV_GPU_MEMORY_INFO_EX_V1, 1)
#define NV_GPU_MEMORY_INFO_EX_VER NV_GPU_MEMORY_INFO_EX_VER_1

// function pointer types
typedef int* (*NvAPI_QueryInterface_t)(unsigned int offset);
typedef int (*NvAPI_Initialize_t)();
typedef int (*NvAPI_Unload_t)();
typedef int (*NvAPI_EnumPhysicalGPUs_t)(int** handles, int* count);
typedef int (*NvAPI_GPU_GetUsages_t)(int* handle, unsigned int* usages);
typedef int (*NvAPI_GPU_GetMemoryInfoEx_t)(int* handle, NV_GPU_MEMORY_INFO_EX* memory_info);

int main()
{
    HMODULE hmod = LoadLibraryA(NVAPI_DLL);
    if (hmod == NULL)
    {
        std::cerr << "Couldn't find " << NVAPI_DLL << std::endl;
        return 1;
    }

    // nvapi.dll internal function pointers
    NvAPI_QueryInterface_t NvAPI_QueryInterface = NULL;
    NvAPI_Initialize_t NvAPI_Initialize = NULL;
    NvAPI_Unload_t NvAPI_Unload = NULL;
    NvAPI_EnumPhysicalGPUs_t NvAPI_EnumPhysicalGPUs = NULL;
    NvAPI_GPU_GetUsages_t NvAPI_GPU_GetUsages = NULL;
    NvAPI_GPU_GetMemoryInfoEx_t NvAPI_GPU_GetMemoryInfoEx = NULL;

    // nvapi_QueryInterface is a function used to retrieve other internal functions in nvapi.dll
    NvAPI_QueryInterface = (NvAPI_QueryInterface_t)GetProcAddress(hmod, "nvapi_QueryInterface");

    // some useful internal functions that aren't exported by nvapi.dll
    // those magic numbers can be found here: https://github.com/NVIDIA/nvapi/blob/main/nvapi_interface.h
    NvAPI_Initialize = (NvAPI_Initialize_t)(*NvAPI_QueryInterface)(0x0150E828);
    NvAPI_Unload = (NvAPI_Unload_t)(*NvAPI_QueryInterface)(0xd22bdd7e);
    NvAPI_EnumPhysicalGPUs = (NvAPI_EnumPhysicalGPUs_t)(*NvAPI_QueryInterface)(0xE5AC921F);
    NvAPI_GPU_GetUsages = (NvAPI_GPU_GetUsages_t)(*NvAPI_QueryInterface)(0x189A1FDF);
    NvAPI_GPU_GetMemoryInfoEx = (NvAPI_GPU_GetMemoryInfoEx_t)(*NvAPI_QueryInterface)(0xc0599498);

    if (NvAPI_Initialize == NULL || NvAPI_EnumPhysicalGPUs == NULL || NvAPI_EnumPhysicalGPUs == NULL ||
        NvAPI_GPU_GetUsages == NULL || NvAPI_GPU_GetMemoryInfoEx == NULL)
    {
        std::cerr << "Couldn't get functions in nvapi.dll" << std::endl;
        return 2;
    }

    // initialize NvAPI library, call it once before calling any other NvAPI functions
    (*NvAPI_Initialize)();

    int gpuCount = 0;
    int* gpuHandles[NVAPI_MAX_PHYSICAL_GPUS] = {NULL};
    unsigned int gpuUsages[NVAPI_MAX_USAGES_PER_GPU] = {0};

    // gpuUsages[0] must be this value, otherwise NvAPI_GPU_GetUsages won't work
    gpuUsages[0] = (NVAPI_MAX_USAGES_PER_GPU * 4) | 0x10000;

    (*NvAPI_EnumPhysicalGPUs)(gpuHandles, &gpuCount);

    // print GPU usage every second
    for (int i = 0; i < 10; i++)
    {
        (*NvAPI_GPU_GetUsages)(gpuHandles[0], gpuUsages);
        int usage = gpuUsages[3];
        NV_GPU_MEMORY_INFO_EX memory_info;
        memory_info.version = NV_GPU_MEMORY_INFO_EX_VER;
        (*NvAPI_GPU_GetMemoryInfoEx)(gpuHandles[0], &memory_info);
        auto gpuVramUsed = (memory_info.availableDedicatedVideoMemory - memory_info.curAvailableDedicatedVideoMemory);
        auto gpuVramUsage =
            static_cast<int>((static_cast<double>(gpuVramUsed) / memory_info.availableDedicatedVideoMemory) * 100);
        std::cout << "GPU Usage: " << usage << "%; GPU Memory Usage: " << gpuVramUsage << "%" << std::endl;
        Sleep(1000);
    }

    (*NvAPI_Unload)();

    return 0;
}