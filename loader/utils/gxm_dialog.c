#include <psp2/display.h>
#include <psp2/gxm.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/message_dialog.h>


#define VITA_GXM_SCREEN_WIDTH     960
#define VITA_GXM_SCREEN_HEIGHT    544
#define VITA_GXM_SCREEN_STRIDE    960

#define VITA_GXM_PIXEL_FORMAT    SCE_DISPLAY_PIXELFORMAT_A8B8G8R8

#define VITA_GXM_BUFFERS          3
#define VITA_GXM_PENDING_SWAPS    2

#define ALIGN(x, a) (((x) + ((a)-1)) & ~((a)-1))

static unsigned int back_buffer_index_for_common_dialog = 0;
static unsigned int front_buffer_index_for_common_dialog = 0;

typedef struct
{
    void *address;
    unsigned int wait_vblank;
} VITA_GXM_DisplayData;

struct
{
    VITA_GXM_DisplayData displayData;
    SceGxmSyncObject* sync;
    SceGxmColorSurface surf;
    SceUID uid;
} _buffer_for_common_dialog[VITA_GXM_BUFFERS];

void _vita_mem_free(SceUID uid)
{
    void *mem = NULL;
    if (sceKernelGetMemBlockBase(uid, &mem) < 0)
        return;
    sceGxmUnmapMemory(mem);
    sceKernelFreeMemBlock(uid);
}

void *_vita_mem_alloc(unsigned int type, unsigned int size, unsigned int alignment, unsigned int attribs, SceUID *uid)
{
    void *mem;

    if (type == SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW) {
        size = ALIGN(size, 256*1024);
    } else {
        size = ALIGN(size, 4*1024);
    }

    *uid = sceKernelAllocMemBlock("gpu_mem", type, size, NULL);

    if (*uid < 0)
        return NULL;

    if (sceKernelGetMemBlockBase(*uid, &mem) < 0)
        return NULL;

    if (sceGxmMapMemory(mem, size, attribs) < 0)
        return NULL;

    return mem;
}

static void display_callback(const void *callback_data)
{
    SceDisplayFrameBuf framebuf;
    const VITA_GXM_DisplayData *display_data = (const VITA_GXM_DisplayData *)callback_data;

    memset(&framebuf, 0x00, sizeof(SceDisplayFrameBuf));
    framebuf.size        = sizeof(SceDisplayFrameBuf);
    framebuf.base        = display_data->address;
    framebuf.pitch       = VITA_GXM_SCREEN_STRIDE;
    framebuf.pixelformat = VITA_GXM_PIXEL_FORMAT;
    framebuf.width       = VITA_GXM_SCREEN_WIDTH;
    framebuf.height      = VITA_GXM_SCREEN_HEIGHT;
    sceDisplaySetFrameBuf(&framebuf, SCE_DISPLAY_SETBUF_NEXTFRAME);

    if (display_data->wait_vblank) {
        sceDisplayWaitVblankStart();
    }
}

void _gxm_minimal_init_for_common_dialog(void)
{
    SceGxmInitializeParams initializeParams;
    memset(&initializeParams, 0, sizeof(SceGxmInitializeParams));
    initializeParams.flags                          = 0;
    initializeParams.displayQueueMaxPendingCount    = VITA_GXM_PENDING_SWAPS;
    initializeParams.displayQueueCallback           = display_callback;
    initializeParams.displayQueueCallbackDataSize   = sizeof(VITA_GXM_DisplayData);
    initializeParams.parameterBufferSize            = SCE_GXM_DEFAULT_PARAMETER_BUFFER_SIZE;
    sceGxmInitialize(&initializeParams);
}

void _gxm_minimal_term_for_common_dialog(void)
{
    sceGxmTerminate();
}

void _gxm_init_for_common_dialog(void)
{
    for (int i = 0; i < VITA_GXM_BUFFERS; i += 1)
    {
        _buffer_for_common_dialog[i].displayData.wait_vblank = 1;
        _buffer_for_common_dialog[i].displayData.address = _vita_mem_alloc(
            SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,
            4 * VITA_GXM_SCREEN_STRIDE * VITA_GXM_SCREEN_HEIGHT,
            SCE_GXM_COLOR_SURFACE_ALIGNMENT,
            SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE,
            &_buffer_for_common_dialog[i].uid);
        sceGxmColorSurfaceInit(
            &_buffer_for_common_dialog[i].surf,
            VITA_GXM_PIXEL_FORMAT,
            SCE_GXM_COLOR_SURFACE_LINEAR,
            SCE_GXM_COLOR_SURFACE_SCALE_NONE,
            SCE_GXM_OUTPUT_REGISTER_SIZE_32BIT,
            VITA_GXM_SCREEN_WIDTH,
            VITA_GXM_SCREEN_HEIGHT,
            VITA_GXM_SCREEN_STRIDE,
            _buffer_for_common_dialog[i].displayData.address
        );
        sceGxmSyncObjectCreate(&_buffer_for_common_dialog[i].sync);
    }
    sceGxmDisplayQueueFinish();
}

void _gxm_swap_for_common_dialog(void)
{
    SceCommonDialogUpdateParam updateParam;
    memset(&updateParam, 0, sizeof(SceCommonDialogUpdateParam));
    updateParam.renderTarget.colorFormat    = VITA_GXM_PIXEL_FORMAT;
    updateParam.renderTarget.surfaceType    = SCE_GXM_COLOR_SURFACE_LINEAR;
    updateParam.renderTarget.width          = VITA_GXM_SCREEN_WIDTH;
    updateParam.renderTarget.height         = VITA_GXM_SCREEN_HEIGHT;
    updateParam.renderTarget.strideInPixels = VITA_GXM_SCREEN_STRIDE;

    updateParam.renderTarget.colorSurfaceData = _buffer_for_common_dialog[back_buffer_index_for_common_dialog].displayData.address;

    updateParam.displaySyncObject = _buffer_for_common_dialog[back_buffer_index_for_common_dialog].sync;
    memset(_buffer_for_common_dialog[back_buffer_index_for_common_dialog].displayData.address, 0, 4 * VITA_GXM_SCREEN_STRIDE * VITA_GXM_SCREEN_HEIGHT);
    sceCommonDialogUpdate(&updateParam);

    sceGxmDisplayQueueAddEntry(_buffer_for_common_dialog[front_buffer_index_for_common_dialog].sync, _buffer_for_common_dialog[back_buffer_index_for_common_dialog].sync, &_buffer_for_common_dialog[back_buffer_index_for_common_dialog].displayData);
    front_buffer_index_for_common_dialog = back_buffer_index_for_common_dialog;
    back_buffer_index_for_common_dialog = (back_buffer_index_for_common_dialog + 1) % VITA_GXM_BUFFERS;
}

void _gxm_term_for_common_dialog(void)
{
    sceGxmDisplayQueueFinish();
    for (int i = 0; i < VITA_GXM_BUFFERS; i += 1)
    {
        _vita_mem_free(_buffer_for_common_dialog[i].uid);
        sceGxmSyncObjectDestroy(_buffer_for_common_dialog[i].sync);
    }
}
