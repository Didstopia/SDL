/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2006 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"

/* Dummy SDL video driver implementation; this is just enough to make an
 *  SDL-based application THINK it's got a working video driver, for
 *  applications that call SDL_Init(SDL_INIT_VIDEO) when they don't need it,
 *  and also for use as a collection of stubs when porting SDL to a new
 *  platform for which you haven't yet written a valid video driver.
 *
 * This is also a great way to determine bottlenecks: if you think that SDL
 *  is a performance problem for a given platform, enable this driver, and
 *  then see if your application runs faster without video overhead.
 *
 * Initial work by Ryan C. Gordon (icculus@icculus.org). A good portion
 *  of this was cut-and-pasted from Stephane Peter's work in the AAlib
 *  SDL video driver.  Renamed to "DUMMY" by Sam Lantinga.
 */

#include <stdio.h>
#include <stdlib.h>
#include <nds.h>

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "SDL_ndsvideo.h"
#include "SDL_ndsevents_c.h"
#include "SDL_ndsrender_c.h"

#define NDSVID_DRIVER_NAME "nds"

/* Initialization/Query functions */
static int NDS_VideoInit(_THIS);
static int NDS_SetDisplayMode(_THIS, SDL_DisplayMode * mode);
static void NDS_VideoQuit(_THIS);

/* DUMMY driver bootstrap functions */

static int
NDS_Available(void)
{
    const char *envr = SDL_getenv("SDL_VIDEODRIVER");
    /*printf("NDS_Available()\n"); */
    return (1);
}

static void
NDS_DeleteDevice(SDL_VideoDevice * device)
{
    SDL_free(device);
}

static SDL_VideoDevice *
NDS_CreateDevice(int devindex)
{
    SDL_VideoDevice *device;
    /*printf("NDS_CreateDevice(%d)\n", devindex); */

    /* Initialize all variables that we clean on shutdown */
    device = (SDL_VideoDevice *) SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (!device) {
        SDL_OutOfMemory();
        if (device) {
            SDL_free(device);
        }
        return (0);
    }

    /* Set the function pointers */
    device->VideoInit = NDS_VideoInit;
    device->VideoQuit = NDS_VideoQuit;
    device->SetDisplayMode = NDS_SetDisplayMode;
    device->PumpEvents = NDS_PumpEvents;

    device->num_displays = 2;   /* DS = dual screens */

    device->free = NDS_DeleteDevice;

    return device;
}

VideoBootStrap NDS_bootstrap = {
    NDSVID_DRIVER_NAME, "SDL NDS video driver",
    NDS_Available, NDS_CreateDevice
};

int
NDS_VideoInit(_THIS)
{
    SDL_DisplayMode mode;
    int i;

    /* simple 256x192x15x60 for now */
    mode.w = 256;
    mode.h = 192;
    mode.format = SDL_PIXELFORMAT_BGR555;
    mode.refresh_rate = 60;
    mode.driverdata = NULL;

    SDL_AddBasicVideoDisplay(&mode);
    SDL_AddRenderDriver(0, &NDS_RenderDriver);

    SDL_zero(mode);
    SDL_AddDisplayMode(0, &mode);

    /* hackish stuff to get things up and running for now, and for a console */
    powerON(POWER_ALL);
    videoSetMode(MODE_FB0);
    videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);    /* debug text on sub */
        vramSetBankA(VRAM_A_LCD);
    vramSetBankC(VRAM_C_SUB_BG);
        irqInit();
    irqEnable(IRQ_VBLANK);
    /* set up console for debug text 'n stuff */
    SUB_BG0_CR = BG_MAP_BASE(31);
    BG_PALETTE_SUB[255] = RGB15(31, 31, 31);
    consoleInitDefault((u16 *) SCREEN_BASE_BLOCK_SUB(31),
                       (u16 *) CHAR_BASE_BLOCK_SUB(0), 16);

    /*NDS_SetDisplayMode(_this, &mode); */
    return 0;
}

static int
NDS_SetDisplayMode(_THIS, SDL_DisplayMode * mode)
{
    /* right now this function is just hard-coded for 256x192 ABGR1555 */
#if 0
    videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);       /* display on main core */
    videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);    /* debug text on sub */
    vramSetMainBanks(VRAM_A_MAIN_BG_0x06000000, VRAM_B_LCD,
                     VRAM_C_SUB_BG, VRAM_D_LCD);

    /* maps well to the 256x192 screen anyway.  note: need VRAM_B for bigger */
    BG3_CR = BG_BMP16_256x256;
    /* affine transformation matrix.  nothing too fancy here */
    BG3_XDX = 0x100;
    BG3_XDY = 0;
    BG3_YDX = 0;
    BG3_YDY = 0x100;
    /* x/y position */
    BG3_CX = 0;
    BG3_CY = 0;
#endif
    return 0;
}

void
NDS_VideoQuit(_THIS)
{
}

/* vi: set ts=4 sw=4 expandtab: */
