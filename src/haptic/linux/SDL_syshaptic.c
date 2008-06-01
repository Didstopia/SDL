/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 2008 Edgar Simo

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

#ifdef SDL_JOYSTICK_LINUX

#include "SDL_haptic.h"
#include "../SDL_haptic_c.h"
#include "../SDL_syshaptic.h"

#include <unistd.h> /* close */
#include <linux/input.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <string.h>


#include <stdio.h>


#define MAX_HAPTICS  32


static struct
{
   char *fname;
   SDL_Haptic *haptic;
} SDL_hapticlist[MAX_HAPTICS];

struct haptic_hwdata
{
   int fd;
};


#define test_bit(nr, addr) \
   (((1UL << ((nr) & 31)) & (((const unsigned int *) addr)[(nr) >> 5])) != 0)
#define EV_TEST(ev,f) \
   if (test_bit((ev), features)) ret |= (f);
static int
EV_IsHaptic(int fd)
{
   unsigned int ret;
   unsigned long features[1 + FF_MAX/sizeof(unsigned long)];

   ret = 0;

   ioctl(fd, EVIOCGBIT(EV_FF, sizeof(unsigned long) * 4), features);

   EV_TEST(FF_CONSTANT,   SDL_HAPTIC_CONSTANT);
   EV_TEST(FF_PERIODIC,   SDL_HAPTIC_PERIODIC);
   EV_TEST(FF_RAMP,       SDL_HAPTIC_RAMP);
   EV_TEST(FF_SPRING,     SDL_HAPTIC_SPRING);
   EV_TEST(FF_FRICTION,   SDL_HAPTIC_FRICTION);
   EV_TEST(FF_DAMPER,     SDL_HAPTIC_DAMPER);
   EV_TEST(FF_RUMBLE,     SDL_HAPTIC_RUMBLE);
   EV_TEST(FF_INERTIA,    SDL_HAPTIC_INERTIA);
   EV_TEST(FF_GAIN,       SDL_HAPTIC_GAIN);
   EV_TEST(FF_AUTOCENTER, SDL_HAPTIC_AUTOCENTER);

   return ret;
}

int
SDL_SYS_HapticInit(void)
{
   const char joydev_pattern[] = "/dev/input/event%d";
   dev_t dev_nums[MAX_HAPTICS];
   char path[PATH_MAX];
   struct stat sb;
   int fd;
   int i, j, k;
   int duplicate;
   int numhaptics;

   numhaptics = 0;

   i = 0;
   for (j = 0; j < MAX_HAPTICS; ++j) {

      snprintf(path, PATH_MAX, joydev_pattern, i++);

      /* check to see if file exists */
      if (stat(path,&sb) != 0)
         break;

      /* check for duplicates */
      duplicate = 0;
      for (k = 0; (k < numhaptics) && !duplicate; ++k) {
         if (sb.st_rdev == dev_nums[k]) {
            duplicate = 1;
         }                                                     
      }                                                         
      if (duplicate) {
         continue;
      }

      /* try to open */
      fd = open(path, O_RDWR, 0);
      if (fd < 0) continue;

#ifdef DEBUG_INPUT_EVENTS
      printf("Checking %s\n", path);
#endif          

      /* see if it works */
      if (EV_IsHaptic(fd)!=0) {
         SDL_hapticlist[numhaptics].fname = SDL_strdup(path);
         SDL_hapticlist[numhaptics].haptic = NULL;
         dev_nums[numhaptics] = sb.st_rdev;
         ++numhaptics;
      }
      close(fd);
   }

   return numhaptics;
}


const char *
SDL_SYS_HapticName(int index)
{
   int fd;
   static char namebuf[128];
   char *name;

   name = NULL;
   fd = open(SDL_hapticlist[index].fname, O_RDONLY, 0);
   if (fd >= 0) {
      if (ioctl(fd, EVIOCGNAME(sizeof(namebuf)), namebuf) <= 0) {
         name = SDL_hapticlist[index].fname;
      }
      else {
         name = namebuf;
      }
   }
   close(fd);

   return name;
}


int
SDL_SYS_HapticOpen(SDL_Haptic * haptic)
{
   /* TODO finish
   int fd;

   fd = open(SDL_hapticlist[haptic->index].fname, O_RDWR, 0);
   
   if (fd < 0) {
      SDL_SetError("Unable to open %s\n", SDL_hapticlist[haptic->index]);
      return (-1);
   }

   

   return 0;
   */
}


#endif /* SDL_HAPTIC_LINUX */
