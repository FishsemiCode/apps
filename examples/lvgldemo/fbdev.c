/****************************************************************************
 * apps/examples/lvgldemo/fbdev.c
 *
 *   Copyright (C) 2018 Gregory Nutt. All rights reserved.
 *   Author: Gábor Kiss-Vámosi <kisvegabor@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "fbdev.h"

#include <nuttx/config.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <nuttx/nx/nx.h>
#include <nuttx/nx/nxglib.h>
#include <nuttx/video/fb.h>
#include <nuttx/video/rgbcolors.h>

#include <nuttx/lcd/ili9486_lcd_ioctl.h>


/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef FBDEV_PATH
#  define FBDEV_PATH  "/dev/ili9486"
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/
static int fd;
/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: fbdev_init
 *
 * Description:
 *
 * Input Parameters:
 *   None
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

int fbdev_init(void)
{
  fd = open(FBDEV_PATH, O_RDWR);
  if (fd < 0)
    {
      syslog(LOG_ERR, "Failed to open %s: %d\n", FBDEV_PATH, errno);
      return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

static int ili9486_lcd_flush(int len, uint16_t *matrix)
{
  ssize_t nwritten;

  nwritten = write(fd, (char *)matrix, len);
  if (nwritten < 0)
    {
      int errcode = errno;
      printf("write failed: %d\n", errcode);

      if (errcode != EINTR)
        {
          exit(EXIT_FAILURE);
        }
    }

  return OK;
}


/****************************************************************************
 * Name: fbdev_flush
 *
 * Description:
 *   Flush a buffer to the marked area
 *
 * Input Parameters:
 *   x1      - Left coordinate
 *   y1      - Top coordinate
 *   x2      - Right coordinate
 *   y2      - Bottom coordinate
 *   color_p - A n array of colors
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void fbdev_flush(struct _disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
  int ret;

  struct ili9486_lcd_matrix_s matrix;
  matrix.x = area->x1;
  matrix.y = area->y1;
  matrix.w = lv_area_get_width(area);
  matrix.h = lv_area_get_height(area);
  syslog(LOG_INFO, "fbdev_flush:%d,%d,%d,%d\n", matrix.x, matrix.y, matrix.w, matrix.h);
  ret = ioctl(fd, ILI9486_LCDIOC_SET_MATRIX, (unsigned long)&matrix);
  if (ret < 0)
    {
      printf("ioctl(ILI9486_LCDIOC_SET_MATRIX) failed: %d\n", errno);
      goto clean;
    }
  ili9486_lcd_flush(matrix.w * matrix.h, (uint16_t *)color_p);
clean:
  /* Tell the flushing is ready */
  lv_disp_flush_ready(disp_drv);
}
