/****************************************************************************
 * examples/ili9486_lcd/ili9486_lcd_main.c
 *
 *   Copyright (C) 2019 Fishsemi Inc. All rights reserved.
 *   Author: clyde <liuyan@fishsemi.com>
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

#include <nuttx/config.h>

#include <sys/ioctl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <nuttx/lcd/ili9486_lcd_ioctl.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_EXAMPLES_ILI9486_LCD_DEVNAME
#  define CONFIG_EXAMPLES_ILI9486_LCD_DEVNAME "/dev/ili9486"
#endif

#ifndef MIN
#  define MIN(a,b)                    (((a) < (b)) ? (a) : (b))
#endif

/****************************************************************************
 * Private Typs
 ****************************************************************************/
/* All state information for this test is kept within the following structure
 * in order create a namespace and to minimize the possibility of name
 * collisions.
 *
 * NOTE: stream must be the first element of struct ili9486_lcd_test_s to support
 * casting between these two types.
 */

struct ili9486_lcd_test_s
{
  struct ili9486_lcd_attributes_s attr;  /* Size of the Ili9486 LCD (rows x columns) */
  struct ili9486_lcd_matrix_s matrix;    /* Matrix to draw */
  int fd;                                /* File descriptor or the open ILI9486 LCD device */

  char *buffer;                          /* Drawing string buffer */
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct ili9486_lcd_test_s g_ili9486_lcdtest;

/****************************************************************************
 * Private Functions
 ****************************************************************************/


/****************************************************************************
 * Name: set_color
 ****************************************************************************/

static uint16_t set_color(uint8_t r, uint8_t g, uint8_t b)
{
  uint16_t color;

  color  = MIN((uint16_t) r, 0x1F) << 11;
  color |= MIN((uint16_t) g, 0x3F) << 5;
  color |= MIN((uint16_t) b, 0x1F);

  return color;
}

/****************************************************************************
 * Name: ili9486_lcd_flush
 ****************************************************************************/

static int ili9486_lcd_flush(FAR struct ili9486_lcd_test_s *priv, uint16_t *matrix)
{
  ssize_t nwritten;
  uint16_t len;

  len = priv->matrix.w * priv->matrix.h;

  nwritten = write(priv->fd, (char *)matrix, len);
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
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * ili9486_lcd_main
 ****************************************************************************/

#ifdef BUILD_MODULE
int main(int argc, FAR char *argv[])
#else
int ili9486_lcd_main(int argc, char *argv[])
#endif
{
  /* new process, only save fb id can't handle file struct. */
  FAR struct ili9486_lcd_test_s *priv = &g_ili9486_lcdtest;
  int ret;
  uint16_t *matrix;
  uint16_t color;
  int i;

  if (argc != 8)
    {
      printf("usage:%s matrix_x matrix_y matrix_w matrix_h color_r color_g color_b\n", argv[0]);
      return 0;
    }

  memset(priv, 0, sizeof(struct ili9486_lcd_test_s));

  priv->matrix.x = atoi(argv[1]);
  priv->matrix.y = atoi(argv[2]);
  priv->matrix.w = atoi(argv[3]);
  priv->matrix.h = atoi(argv[4]);
  color= set_color(atoi(argv[5]), atoi(argv[6]), atoi(argv[7]));
  matrix = (uint16_t *)malloc(priv->matrix.w * priv->matrix.h * 2);
  for (i = 0; i < priv->matrix.w * priv->matrix.h; i++)
    {
      *(matrix + i) = color;
    }

  printf("Opening %s for read/write access\n", CONFIG_EXAMPLES_ILI9486_LCD_DEVNAME);
  priv->fd = open(CONFIG_EXAMPLES_ILI9486_LCD_DEVNAME, O_RDWR);
  if (priv->fd < 0)
    {
      printf("Failed to open %s: %d\n", CONFIG_EXAMPLES_ILI9486_LCD_DEVNAME, errno);
      goto errout;
    }

  /* Get the attributes of the Ili9486 LCD device */
  ret = ioctl(priv->fd, ILI9486_LCDIOC_GETATTRIBUTES, (unsigned long)&priv->attr);
  if (ret < 0)
    {
      printf("ioctl(ILI9486_LCDIOC_GETATTRIBUTES) failed: %d\n", errno);
      goto errout_with_fd;
    }

  /* Clear screen */
  ret = ioctl(priv->fd, ILI9486_LCDIOC_CLEAR, (unsigned long)NULL);
  if (ret < 0)
    {
      printf("ioctl(ILI9486_LCDIOC_CLEAR) failed: %d\n", errno);
      goto errout_with_fd;
    }

  ret = ioctl(priv->fd, ILI9486_LCDIOC_SET_MATRIX, (unsigned long)&priv->matrix);
  if (ret < 0)
    {
      printf("ioctl(ILI9486_LCDIOC_SET_CURSOR) failed: %d\n", errno);
      goto errout_with_fd;
    }

  ili9486_lcd_flush(priv, matrix);
  close(priv->fd);

  free(matrix);

  printf("Test complete\n");
  return 0;

errout_with_fd:
   close(priv->fd);
errout:
   free(matrix);
   exit(EXIT_FAILURE);
}
