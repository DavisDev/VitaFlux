/*
	Vita-Flux v1.0
	Screen filter plugin for PS Vita.
	
	Designed By DevDavisNunez.
	https://twitter.com/DevDavisNunez
	
	Licensed by Creative Commons Attribution-ShareAlike 4.0
	http://creativecommons.org/licenses/by-sa/4.0/	
	
	TODO:
	- Add config file!
	- Add time enable! (After some time enable / disable)
	- Add color of filter? xD
	
*/

#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/display.h>
#include <psp2/ctrl.h>
#include <psp2/power.h>
#include <taihen.h>
#include "blit.h"

uint8_t OverMenu = 0,
		enable = 1,
		a=0;
uint64_t crono = 0;

static tai_hook_ref_t sceDisplaySetFrameBuf_ref;
static SceUID sceDisplaySetFrameBuf_hook_uid = -1;
int sceDisplaySetFrameBuf_hook_func(const SceDisplayFrameBuf *pParam, int sync) {
	blit_set_frame_buf(pParam);
	if(a > 0) // Only draw filter if is necessary...
		blit_rect(0,0,960,544,RGBT(255,130,0,255-((a+3)/3)));
	
	if(sceKernelGetProcessTimeWide() - crono > 32){
		crono = sceKernelGetProcessTimeWide();
		if(enable == 1 && a < 150 ){
			a++;
		} else if(enable == 0 && a > 0){
			a--;
		}
	}
	
	if(OverMenu){ // If menu of plugin is open, draw xD
		blit_stringf(CENTER(24), 20, "= Vita Flux Screen Menu =");
		blit_stringf(CENTER(24), 40, "<-/-> Enable/Disable");
		blit_stringf(CENTER(24), 60, "Status: %s",enable == 1?"Enabled":"Disabled");
			
		//blit_stringf(CENTER(24), 80, "Alfa %d",a); // Only Debug :P
	}
	return TAI_CONTINUE(int, sceDisplaySetFrameBuf_ref, pParam, sync);
}   

uint32_t last_ctrls, press_ctrls;

int HandleCtrls(int port, tai_hook_ref_t ref_hook, SceCtrlData *ctrl, int count){
	int ret;
	if (ref_hook == 0) // Reference loss!? xD
		ret = 1;
	else {
		ret = TAI_CONTINUE(int, ref_hook, port, ctrl, count); // Read & Update Buttons states.
		if(OverMenu){ // Handle buttons in menu...
			press_ctrls = ctrl->buttons & ~last_ctrls;
			if((press_ctrls & SCE_CTRL_LEFT) || (press_ctrls & SCE_CTRL_RIGHT)){
				enable = !enable;
			}else if(press_ctrls & SCE_CTRL_CIRCLE){
				OverMenu = 0;
			}
			last_ctrls = ctrl->buttons;
			ctrl->buttons = 0; // If is over menu, the out state is Nothing!
		}else{ // Handle buttons in game...
			if ((ctrl->buttons & SCE_CTRL_SELECT) && (ctrl->buttons & SCE_CTRL_START)) // Combo to open menu...
				OverMenu = 1;
		}
	}
	return ret;
}

static tai_hook_ref_t sceCtrlPeekBufferPositive_ref;
static SceUID sceCtrlPeekBufferPositive_hook_uid = -1;
static int sceCtrlPeekBufferPositive_hook_func(int port, SceCtrlData *ctrl, int count) {
    return HandleCtrls(port, sceCtrlPeekBufferPositive_ref, ctrl, count);
}   

static tai_hook_ref_t sceCtrlPeekBufferPositive2_ref;
static SceUID sceCtrlPeekBufferPositive2_hook_uid = -1;
static int sceCtrlPeekBufferPositive2_hook_func(int port, SceCtrlData *ctrl, int count) {
    return HandleCtrls(port, sceCtrlPeekBufferPositive2_ref, ctrl, count);
}   

static tai_hook_ref_t sceCtrlReadBufferPositive_ref;
static SceUID sceCtrlReadBufferPositive_hook_uid = -1;
static int sceCtrlReadBufferPositive_hook_func(int port, SceCtrlData *ctrl, int count) {
    return HandleCtrls(port, sceCtrlReadBufferPositive_ref, ctrl, count);
}   

static tai_hook_ref_t sceCtrlReadBufferPositive2_ref;
static SceUID sceCtrlReadBufferPositive2_hook_uid = -1;
static int sceCtrlReadBufferPositive2_hook_func(int port, SceCtrlData *ctrl, int count) {
    return HandleCtrls(port, sceCtrlReadBufferPositive2_ref, ctrl, count);
}

void _start() __attribute__ ((weak, alias ("module_start")));

int module_start(SceSize argc, const void *args) {
	
	// sceDisplaySetFrameBuf
	sceDisplaySetFrameBuf_hook_uid = taiHookFunctionImport(&sceDisplaySetFrameBuf_ref, TAI_MAIN_MODULE, TAI_ANY_LIBRARY, 0x7A410B64, sceDisplaySetFrameBuf_hook_func);
	
	// sceCtrlPeekBufferPositive
	sceCtrlPeekBufferPositive_hook_uid = taiHookFunctionImport(&sceCtrlPeekBufferPositive_ref, TAI_MAIN_MODULE, TAI_ANY_LIBRARY, 0xA9C3CED6, sceCtrlPeekBufferPositive_hook_func);
	
	// sceCtrlPeekBufferPositive2
	sceCtrlPeekBufferPositive2_hook_uid = taiHookFunctionImport(&sceCtrlPeekBufferPositive2_ref, TAI_MAIN_MODULE, TAI_ANY_LIBRARY, 0x15F81E8C, sceCtrlPeekBufferPositive2_hook_func);
	
	// sceCtrlReadBufferPositive
	sceCtrlReadBufferPositive_hook_uid = taiHookFunctionImport(&sceCtrlReadBufferPositive_ref, TAI_MAIN_MODULE, TAI_ANY_LIBRARY, 0x67E7AB83, sceCtrlReadBufferPositive_hook_func);

	// sceCtrlReadBufferPositive2
	sceCtrlReadBufferPositive2_hook_uid = taiHookFunctionImport(&sceCtrlReadBufferPositive2_ref, TAI_MAIN_MODULE, TAI_ANY_LIBRARY, 0xC4226A3E, sceCtrlReadBufferPositive2_hook_func);
	
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args) {
	
	// We release the hooks, only if there are xD
	if (sceDisplaySetFrameBuf_hook_uid >= 0)
		taiHookRelease(sceDisplaySetFrameBuf_hook_uid, sceDisplaySetFrameBuf_ref);
	
	if (sceCtrlPeekBufferPositive_hook_uid >= 0)
		taiHookRelease(sceCtrlPeekBufferPositive_hook_uid, sceCtrlPeekBufferPositive_ref);

	if (sceCtrlPeekBufferPositive2_hook_uid >= 0)
		taiHookRelease(sceCtrlPeekBufferPositive2_hook_uid, sceCtrlPeekBufferPositive2_ref);
	
	if (sceCtrlReadBufferPositive_hook_uid >= 0)
		taiHookRelease(sceCtrlReadBufferPositive_hook_uid, sceCtrlReadBufferPositive_ref);

	if (sceCtrlReadBufferPositive2_hook_uid >= 0)
		taiHookRelease(sceCtrlReadBufferPositive2_hook_uid, sceCtrlReadBufferPositive2_ref);
	
	return SCE_KERNEL_STOP_SUCCESS;
}
