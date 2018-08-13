/****************************************************************************
 * apps/system/regmap/regmap_main.c
 *   Copyright (C) 2017 Pinecone Inc. All rights reserved.
 *   Author: Zhong An <zhongan@pinecone.net>
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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <nuttx/regmap/regmap.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

void show_usage(void)
{
  printf("Useage: regmap store/load audio_dev out/input_file\n");
  printf("        regmap store/load audio_dev out/input_file reg_num\n");
  printf("        regmap set audio_dev reg_index reg_value\n");
  printf("        regmap get audio_dev reg_index\n");
  printf("        store: store registers' value in a batch run to a file\n");
  printf("        load: load registers' value in a batch run from a file\n");
  printf("        reg_num: the num of registers to set/get\n");
  printf("        set: set one register's value to regvalue\n");
  printf("        get: show one register's value\n");
  printf("        reg_index: the index of the register to be set/get\n");
  printf("        reg_value: the register's value to set\n");
}

static int batch_num(bool store, int dev_fd, FILE *file, uint32_t reg_num)
{
  struct regmap_arg_s regmap_arg;
  int i,ret;
  char pair[20];

  regmap_arg.reg_num = reg_num;
  regmap_arg.offsets = (uintptr_t *)malloc(reg_num);
  if (!regmap_arg.offsets)
    {
      regmap_arg.offsets = NULL;
      goto err_nomem_batch_num;
    }
  regmap_arg.values = (uint32_t *)malloc(reg_num);
  if (!regmap_arg.values)
    {
err_nomem_batch_num:
      ret = -ENOMEM;
      printf("Not Enough memory!\n");
      goto done_batch_num2;
    }

  if (store)
    {
      for (i = 0; i < reg_num; ++i)
        regmap_arg.offsets[i] = i;
      ret = ioctl(dev_fd, REGMAPIOC_GETREGVALUE, (unsigned long) &regmap_arg);
      if (ret)
        goto done_batch_num2;
      fseek(file, 0, SEEK_SET);
      for (i = 0; i < reg_num; ++i)
        {
          sprintf(pair, "%lx %lx\n", regmap_arg.offsets[i], regmap_arg.values[i]);
          fwrite(pair, strlen(pair), 1, file);
        }
      goto done_batch_num1;
    }
  else
    {
      for (i = 0; i < reg_num; ++i)
        {
          if (!fgets(pair, 20, file))
            break;
          if (sscanf(pair, "%lx %lx\n", &regmap_arg.offsets[i],
                 &regmap_arg.values[i]) != 2)
            {
              i--;
              break;
            }
        }
      if (i < reg_num)
        regmap_arg.reg_num = i;
      ret = ioctl(dev_fd, REGMAPIOC_SETREGVALUE, (unsigned long) &regmap_arg);
    }

done_batch_num1:
  if (ret)
    printf("Ioctl Failed!\n");
  else
    printf("Done");
done_batch_num2:
  free(regmap_arg.offsets);
  free(regmap_arg.values);
  fclose(file);
  close(dev_fd);
  return ret;
}

static int batch(bool store, int dev_fd, FILE *file)
{
  struct regmap_arg_s regmap_arg;
  int i,ret;
  char pair[20];

  regmap_arg.reg_num = 0;

  while(fgets(pair, 20, file))
    regmap_arg.reg_num++;

  regmap_arg.offsets = (uintptr_t *)malloc(regmap_arg.reg_num);
  if (!regmap_arg.offsets)
    {
      regmap_arg.offsets = NULL;
      goto err_nomem_batch;
    }
  regmap_arg.values = (uint32_t *)malloc(regmap_arg.reg_num);
  if (!regmap_arg.values)
    {
err_nomem_batch:
      ret = -ENOMEM;
      printf("Not Enough memory!\n");
      goto done_batch1;
    }
  fseek(file, 0, SEEK_SET);
  for (i = 0; i < regmap_arg.reg_num; ++i)
    {
      if (store)
        {
          if (sscanf(pair, "%lx\n", &regmap_arg.offsets[i]) != 1)
            break;
        }
      else
        {
          if (sscanf(pair, "%lx %lx\n", &regmap_arg.offsets[i],
            &regmap_arg.values[i]) != 2)
            break;
        }
    }
  if (i < regmap_arg.reg_num)
    regmap_arg.reg_num = i;

  if (store)
    {
      ret = ioctl(dev_fd, REGMAPIOC_GETREGVALUE, (unsigned long) &regmap_arg);
      if (ret)
        goto done_batch2;
      fseek(file, 0, SEEK_SET);
      for (i = 0; i < regmap_arg.reg_num; ++i)
        {
          sprintf(pair, "%lx %lx\n", regmap_arg.offsets[i], regmap_arg.values[i]);
          fwrite(pair, strlen(pair), 1, file);
        }
    }
  else
    ret = ioctl(dev_fd, REGMAPIOC_SETREGVALUE, (unsigned long) &regmap_arg);

done_batch1:
  if (ret)
    printf("Ioctl error!\n");
  else
    printf("Done!\n");
done_batch2:
  free(regmap_arg.offsets);
  free(regmap_arg.values);
  fclose(file);
  close(dev_fd);
  return ret;
}

static int single(bool set, int dev_fd, uintptr_t offset, uint32_t value)
{
  struct regmap_arg_s regmap_arg;
  int ret;

  regmap_arg.reg_num = 1;
  regmap_arg.offsets = &offset;
  regmap_arg.values = &value;

  if (set)
    ret = ioctl(dev_fd, REGMAPIOC_SETREGVALUE, (unsigned long) &regmap_arg);
  else
    ret = ioctl(dev_fd, REGMAPIOC_GETREGVALUE, (unsigned long) &regmap_arg);

  if (ret)
    printf("Ioctl error!\n");
  else
    printf("0x%lx = 0x%lx\n", offset, value);

  close(dev_fd);
  return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int regmap_main(int argc, char *argv[])
#endif
{
  int dev_fd;
  uint32_t reg_num, value;
  uintptr_t offset;
  FILE *file;
  char dev_name[30];

  reg_num = 0;
  if (argc < 4)
    goto error_arg;

  sprintf(dev_name, "/dev/audio/%s", argv[2]);

  dev_fd = open(dev_name, O_RDONLY);
  if (dev_fd < 0)
    {
      dev_fd = open(argv[2], O_RDONLY);
      if (dev_fd < 0)
        goto error_arg;
    }

  if (!strcmp(argv[1], "store"))
    {
      file = fopen(argv[3], "w");
      if (!file)
        goto error;
      if (argc == 4)
         return batch(true, dev_fd, file);
      else if (argc == 5)
        {
          if (sscanf(argv[4], "%lu", &reg_num) != 1)
            goto error;
          return batch_num(true, dev_fd, file, reg_num);
        }
    }

  if (!strcmp(argv[1], "load"))
    {
      file = fopen(argv[3], "r");
      if (!file)
        goto error;
      if (argc == 4)
        return batch(false, dev_fd, file);
      else if (argc == 5)
        {
          if (sscanf(argv[4], "%lu", &reg_num) != 1)
            goto error;
          return batch_num(false, dev_fd, file, reg_num);
        }
    }

  if (!strcmp(argv[1], "set"))
    {
      if (argc != 5)
        goto error;
      if (sscanf(argv[3], "%lx", &offset) != 1)
        goto error;
      if (sscanf(argv[4], "%lx", &value) != 1)
        goto error;
      return single(true, dev_fd, offset, value);
    }

  if (!strcmp(argv[1], "get"))
    {
      if (argc != 4)
        goto error;
      if (sscanf(argv[3], "%lx", &offset) != 1)
        goto error;
      return single(false, dev_fd, offset, value);
    }

error:
  close(dev_fd);
error_arg:
  show_usage();
  return -EINVAL;
}
