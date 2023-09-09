#include <vitasdk.h>
#include <vitaGL.h>
#include <stdio.h>
#include <string.h>

static char comm_id[12] = {0};
static char signature[160] = {0xb9,0xdd,0xe1,0x3b,0x01,0x00};

static int trp_ctx;
static int plat_id = -1;

typedef struct {
	int sdkVersion;
	SceCommonDialogParam commonParam;
	int context;
	int options;
	uint8_t reserved[128];
} SceNpTrophySetupDialogParam;

typedef struct {
	uint32_t unk[4];
} SceNpTrophyUnlockState;
SceNpTrophyUnlockState trophies_unlocks;

int sceNpTrophyInit(void *unk);
int sceNpTrophyCreateContext(int *context, char *commId, char *commSign, uint64_t options);
int sceNpTrophySetupDialogInit(SceNpTrophySetupDialogParam *param);
SceCommonDialogStatus sceNpTrophySetupDialogGetStatus();
int sceNpTrophySetupDialogTerm();
int sceNpTrophyCreateHandle(int *handle);
int sceNpTrophyDestroyHandle(int handle);
int sceNpTrophyUnlockTrophy(int ctx, int handle, int id, int *plat_id);
int sceNpTrophyGetTrophyUnlockState(int ctx, int handle, SceNpTrophyUnlockState *state, uint32_t *count);

int trophies_available = 0;

volatile int trp_id;
SceUID trp_request_mutex;
int trophies_unlocker(SceSize args, void *argp) {
	for (;;) {
		sceKernelWaitSema(trp_request_mutex, 1, NULL);
		int local_trp_id = trp_id;
		int trp_handle;
		sceNpTrophyCreateHandle(&trp_handle);
		sceNpTrophyUnlockTrophy(trp_ctx, trp_handle, local_trp_id, &plat_id);
		sceNpTrophyDestroyHandle(trp_handle);
	}
}

int trophies_init() {
#if GRAPHICS_API_VITAGL
	// Starting sceNpTrophy
	strcpy(comm_id, "HOMM00003");
	sceSysmoduleLoadModule(SCE_SYSMODULE_NP_TROPHY);
	sceNpTrophyInit(NULL);
	int res = sceNpTrophyCreateContext(&trp_ctx, comm_id, signature, 0);
	if (res < 0) {
		sceClibPrintf("sceNpTrophyCreateContext returned 0x%08X\n", res);
		return res;
	}
	SceNpTrophySetupDialogParam setupParam;
	sceClibMemset(&setupParam, 0, sizeof(SceNpTrophySetupDialogParam));
	_sceCommonDialogSetMagicNumber(&setupParam.commonParam);
	setupParam.sdkVersion = PSP2_SDK_VERSION;
	setupParam.options = 0;
	setupParam.context = trp_ctx;
	sceNpTrophySetupDialogInit(&setupParam);
	static int trophy_setup = SCE_COMMON_DIALOG_STATUS_RUNNING;
	while (trophy_setup == SCE_COMMON_DIALOG_STATUS_RUNNING) {
		trophy_setup = sceNpTrophySetupDialogGetStatus();
		vglSwapBuffers(GL_TRUE);
	}
	sceNpTrophySetupDialogTerm();
	
	// Starting trophy unlocker thread
	trp_request_mutex = sceKernelCreateSema("trps request", 0, 0, 1, NULL);
	SceUID tropies_unlocker_thd = sceKernelCreateThread("trophies unlocker", &trophies_unlocker, 0x10000100, 0x10000, 0, 0, NULL);
	sceClibPrintf("sceKernelCreateThread(trophies_unlocker): %X\n", tropies_unlocker_thd);
	sceClibPrintf("sceKernelStartThread(trophies_unlocker): %X\n", sceKernelStartThread(tropies_unlocker_thd, 0, NULL));
	
	// Getting current trophy unlocks state
	int trp_handle;
	uint32_t dummy;
	sceNpTrophyCreateHandle(&trp_handle);
	sceNpTrophyGetTrophyUnlockState(trp_ctx, trp_handle, &trophies_unlocks, &dummy);
	sceNpTrophyDestroyHandle(trp_handle);
	
	trophies_available = 1;
	return res;
#else
	return -1;
#endif
}

uint8_t trophies_is_unlocked(uint32_t id) {
	if (trophies_available) {
		return (trophies_unlocks.unk[id >> 5] & (1 << (id & 31))) > 0;
	}
	return 0;
}

void trophies_unlock(uint32_t id) {
	if (trophies_available && !trophies_is_unlocked(id)) {
		trophies_unlocks.unk[id >> 5] |= (1 << (id & 31));
		trp_id = id;
		sceKernelSignalSema(trp_request_mutex, 1);
	}
}
