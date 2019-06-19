/****************************************************************************
 *
 * apps/system/nxrecorder/nxrecorder_main.c
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
#include <nuttx/audio/audio.h>

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include "system/readline.h"
#include "system/nxrecorder.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define NXRECORDER_VER          "1.00"

#ifdef CONFIG_NXRECORDER_INCLUDE_HELP
#  define NXRECORDER_HELP_TEXT(x)  #x
#else
#  define NXRECORDER_HELP_TEXT(x)
#endif

/****************************************************************************
 * Private Type Declarations
 ****************************************************************************/

struct mp_cmd_s
{
  const char      *cmd;       /* The command text */
  const char      *arghelp;   /* Text describing the args */
  nxrecorder_func pFunc;      /* Pointer to command handler */
  const char      *help;      /* The help text */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int nxrecorder_cmd_quit(FAR struct nxrecorder_s *pRecorder, char *parg);
static int nxrecorder_cmd_mw(FAR struct nxrecorder_s *pRecorder, char *parg);
static int nxrecorder_cmd_recordraw(FAR struct nxrecorder_s *pRecorder, char *parg);
static int nxrecorder_cmd_device(FAR struct nxrecorder_s *pRecorder, char *parg);

#ifndef CONFIG_AUDIO_EXCLUDE_PAUSE_RESUME
static int nxrecorder_cmd_pause(FAR struct nxrecorder_s *pRecorder, char *parg);
static int nxrecorder_cmd_resume(FAR struct nxrecorder_s *pRecorder, char *parg);
#endif

#ifndef CONFIG_AUDIO_EXCLUDE_STOP
static int nxrecorder_cmd_stop(FAR struct nxrecorder_s *pRecorder, char *parg);
#endif

#ifdef CONFIG_NXRECORDER_INCLUDE_HELP
static int nxrecorder_cmd_help(FAR struct nxrecorder_s *pRecorder, char *parg);
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct mp_cmd_s g_nxrecorder_cmds[] =
{
  { "device",    "devfile",  nxrecorder_cmd_device,    NXRECORDER_HELP_TEXT(Specify a preferred audio device) },
#ifdef CONFIG_NXRECORDER_INCLUDE_HELP
  { "h",         "",         nxrecorder_cmd_help,      NXRECORDER_HELP_TEXT(Display help for commands) },
  { "help",      "",         nxrecorder_cmd_help,      NXRECORDER_HELP_TEXT(Display help for commands) },
#endif
  { "mw",       "...",       nxrecorder_cmd_mw,         NXRECORDER_HELP_TEXT(WR registers) },
  { "recordraw", "filename", nxrecorder_cmd_recordraw, NXRECORDER_HELP_TEXT(Record a pcm raw file) },
#ifndef CONFIG_AUDIO_EXCLUDE_PAUSE_RESUME
  { "pause",     "",         nxrecorder_cmd_pause,     NXRECORDER_HELP_TEXT(Pause record) },
  { "resume",    "",         nxrecorder_cmd_resume,    NXRECORDER_HELP_TEXT(Resume record) },
#endif
#ifndef CONFIG_AUDIO_EXCLUDE_STOP
  { "stop",      "",         nxrecorder_cmd_stop,      NXRECORDER_HELP_TEXT(Stop record) },
#endif
  { "q",         "",         nxrecorder_cmd_quit,      NXRECORDER_HELP_TEXT(Exit NxRecorder) },
  { "quit",      "",         nxrecorder_cmd_quit,      NXRECORDER_HELP_TEXT(Exit NxRecorder) },
};
static const int g_nxrecorder_cmd_count = sizeof(g_nxrecorder_cmds) / sizeof(struct mp_cmd_s);


/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: nxplayer_cmd_mw
 *
 *   nxplayer_cmd_mw() read/write registers.
 *
 ****************************************************************************/

struct dbgmem_s
{
  bool         dm_write;  /* true: perfrom write operation */
  FAR void    *dm_addr;   /* Address to access */
  uint32_t     dm_value;  /* Value to write */
  unsigned int dm_count;  /* The number of bytes to access */
};

static int mem_parse(int argc, FAR char **argv, FAR struct dbgmem_s *mem)
{
  FAR char *pcvalue = strchr(argv[1], '=');
  unsigned long lvalue = 0;

  /* Check if we are writing a value */

  if (pcvalue)
    {
      *pcvalue = '\0';
      pcvalue++;

      lvalue = strtoul(pcvalue, NULL, 16);
      if (lvalue > 0xffffffffL)
        {
          return -EINVAL;
        }

      mem->dm_write = true;
      mem->dm_value = (uint32_t)lvalue;
    }
  else
    {
      mem->dm_write = false;
      mem->dm_value = 0;
    }

  /* Get the address to be accessed */

  mem->dm_addr = (FAR void *)((uintptr_t)strtoul(argv[1], NULL, 16));

  /* Get the number of bytes to access */

  if (argc > 2)
    {
      mem->dm_count = (unsigned int)strtoul(argv[2], NULL, 16);
    }
  else
    {
      mem->dm_count = 1;
    }

  return OK;
}

static int nxrecorder_cmd_mw(FAR struct nxrecorder_s *pRecorder, char *parg)
{
  struct dbgmem_s mem;
  FAR volatile uint32_t *ptr;
  unsigned int i;
  char *argv[3];
  int argc;
  int ret;

  argc = 1;
  argv[0] = "mw";

  while (argc <= 3 && *parg != '\0')
    {
      argv[argc] = parg;
      argc++;

      parg = strchr(parg, ' ');
      if (!parg)
        {
          break;
        }

      *parg++ = '\0';
    }

  ret = mem_parse(argc, argv, &mem);
  if (ret == 0)
    {
      /* Loop for the number of requested bytes */

      for (i = 0, ptr = (volatile uint32_t*)mem.dm_addr; i < mem.dm_count; i += 4, ptr++)
        {
          /* Print the value at the address */

          printf("  %p = 0x%08x", ptr, *ptr);

          /* Are we supposed to write a value to this address? */

          if (mem.dm_write)
            {
              /* Write the value and re-read the address so that we print its
               * current value (if the address is a process address, then the
               * value read might not necessarily be the value written).
               */

              *ptr = mem.dm_value;
              printf(" -> 0x%08x", *ptr);
            }

          /* Make sure we end it with a newline */

          printf("\n", *ptr);
        }
    }

  return ret;
}

/****************************************************************************
 * Name: nxrecorder_cmd_recordraw
 *
 *   nxrecorder_cmd_recordraw() records the raw data file using the nxrecorder
 *   context.
 *
 ****************************************************************************/

static int nxrecorder_cmd_recordraw(FAR struct nxrecorder_s *pRecorder, char *parg)
{
  int ret;
  int channels = 0;
  int bpsamp = 0;
  int samprate = 0;
  char filename[128];

  sscanf(parg, "%s %d %d %d", filename, &channels, &bpsamp, &samprate);

  /* Try to record the file specified */

  ret = nxrecorder_recordraw(pRecorder, filename, channels, bpsamp, samprate);

  /* nxrecorder_recordfile returned values:
   *
   *   OK         File is being recorded
   *   -EBUSY     The media device is busy
   *   -ENOSYS    The media file is an unsupported type
   *   -ENODEV    No audio device suitable to record the media type
   *   -ENOENT    The media file was not found
   */

  switch (-ret)
    {
      case OK:
        break;

      case ENODEV:
        printf("No suitable Audio Device found\n");
        break;

      case EBUSY:
        printf("Audio device busy\n");
        break;

      case ENOENT:
        printf("File %s not found\n", filename);
        break;

      case ENOSYS:
        printf("Unknown audio format\n");
        break;

      default:
        printf("Error recording file: %d\n", -ret);
        break;
    }

  return ret;
}

/****************************************************************************
 * Name: nxrecorder_cmd_stop
 *
 *   nxrecorder_cmd_stop() stops record of currently recording file
 *   context.
 *
 ****************************************************************************/

#ifndef CONFIG_AUDIO_EXCLUDE_STOP
static int nxrecorder_cmd_stop(FAR struct nxrecorder_s *pRecorder, char *parg)
{
  /* Stop the record */

  nxrecorder_stop(pRecorder);

  return OK;
}
#endif

/****************************************************************************
 * Name: nxrecorder_cmd_pause
 *
 *   nxrecorder_cmd_pause() pauses record of currently recording file
 *   context.
 *
 ****************************************************************************/

#ifndef CONFIG_AUDIO_EXCLUDE_PAUSE_RESUME
static int nxrecorder_cmd_pause(FAR struct nxrecorder_s *pRecorder, char *parg)
{
  /* Pause the record */

  nxrecorder_pause(pRecorder);

  return OK;
}
#endif

/****************************************************************************
 * Name: nxrecorder_cmd_resume
 *
 *   nxrecorder_cmd_resume() resumes record of currently recording file
 *   context.
 *
 ****************************************************************************/

#ifndef CONFIG_AUDIO_EXCLUDE_PAUSE_RESUME
static int nxrecorder_cmd_resume(FAR struct nxrecorder_s *pRecorder, char *parg)
{
  /* Resume the record */

  nxrecorder_resume(pRecorder);

  return OK;
}
#endif

/****************************************************************************
 * Name: nxrecorder_cmd_device
 *
 *   nxrecorder_cmd_device() sets the preferred audio device for record
 *
 ****************************************************************************/

static int nxrecorder_cmd_device(FAR struct nxrecorder_s *pRecorder, char *parg)
{
  int     ret;
  char    path[32];

  /* First try to open the file directly */

  ret = nxrecorder_setdevice(pRecorder, parg);
  if (ret == -ENOENT)
    {
      /* Append the /dev/audio path and try again */

#ifdef CONFIG_AUDIO_CUSTOM_DEV_PATH
#ifdef CONFIG_AUDIO_DEV_ROOT
      snprintf(path,  sizeof(path), "/dev/%s", parg);
#else
      snprintf(path,  sizeof(path), CONFIG_AUDIO_DEV_PATH "/%s", parg);
#endif
#else
      snprintf(path, sizeof(path), "/dev/audio/%s", parg);
#endif
      ret = nxrecorder_setdevice(pRecorder, path);
    }

  /* Test if the device file exists */

  if (ret == -ENOENT)
    {
      /* Device doesn't exit.  Report error */

      printf("Device %s not found\n", parg);
      return ret;
    }

  /* Test if is is an audio device */

  if (ret == -ENODEV)
    {
      printf("Device %s is not an audio device\n", parg);
      return ret;
    }

  if (ret < 0)
    {
      return ret;
    }

  /* Device set successfully */

  return OK;
}

/****************************************************************************
 * Name: nxrecorder_cmd_quit
 *
 *   nxrecorder_cmd_quit() terminates the application
 ****************************************************************************/

static int nxrecorder_cmd_quit(FAR struct nxrecorder_s *pRecorder, char *parg)
{
  /* Stop the record if any */

#ifndef CONFIG_AUDIO_EXCLUDE_STOP
  nxrecorder_stop(pRecorder);
#endif

  return OK;
}

/****************************************************************************
 * Name: nxrecorder_cmd_help
 *
 *   nxrecorder_cmd_help() display the application's help information on
 *   supported commands and command syntax.
 ****************************************************************************/

#ifdef CONFIG_NXRECORDER_INCLUDE_HELP
static int nxrecorder_cmd_help(FAR struct nxrecorder_s *pRecorder, char *parg)
{
  int   x, len, maxlen = 0;
  int   c;

  /* Calculate length of longest cmd + arghelp */

  for (x = 0; x < g_nxrecorder_cmd_count; x++)
    {
      len = strlen(g_nxrecorder_cmds[x].cmd) + strlen(g_nxrecorder_cmds[x].arghelp);
      if (len > maxlen)
        {
          maxlen = len;
        }
    }

  printf("NxRecorder commands\n================\n");
  for (x = 0; x < g_nxrecorder_cmd_count; x++)
    {
      /* Print the command and it's arguments */

      printf("  %s %s", g_nxrecorder_cmds[x].cmd, g_nxrecorder_cmds[x].arghelp);

      /* Calculate number of spaces to print before the help text */

      len = maxlen - (strlen(g_nxrecorder_cmds[x].cmd) + strlen(g_nxrecorder_cmds[x].arghelp));
      for (c = 0; c < len; c++)
        {
          printf(" ");
        }

      printf("  : %s\n", g_nxrecorder_cmds[x].help);
    }

  return OK;
}
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: nxrecorder
 *
 *   nxrecorder() reads in commands from the console using the readline
 *   system add-in and implemets a command-line based pcm raw data recorder
 *   that uses the NuttX audio system to record pcm raw data files read in
 *   from the audio device.  Commands are provided for setting volume, base and
 *   other audio features, as well as for pausing and stopping the
 *   record.
 *
 * Input Parameters:
 *   buf       - The user allocated buffer to be filled.
 *   buflen    - the size of the buffer.
 *   instream  - The stream to read characters from
 *   outstream - The stream to each characters to.
 *
 * Returned values:
 *   On success, the (positive) number of bytes transferred is returned.
 *   EOF is returned to indicate either an end of file condition or a
 *   failure.
 *
 ****************************************************************************/

#ifdef BUILD_MODULE
int main(int argc, FAR char *argv[])
#else
int nxrecorder_main(int argc, char *argv[])
#endif
{
  char                    buffer[64];
  int                     len, x, running;
  char                    *cmd, *arg;
  FAR struct nxrecorder_s *pRecorder;

  printf("NxRecorder version " NXRECORDER_VER "\n");
  printf("h for commands, q to exit\n");
  printf("\n");

  /* Initialize our NxRecorder context */

  pRecorder = nxrecorder_create();
  if (pRecorder == NULL)
    {
      printf("Error:  Out of RAM\n");
      return -ENOMEM;
    }

  /* Loop until the user exits */

  running = TRUE;
  while (running)
    {
      /* Print a prompt */

      printf("nxrecorder> ");
      fflush(stdout);

      /* Read a line from the terminal */

      len = readline(buffer, sizeof(buffer), stdin, stdout);
      buffer[len] = '\0';
      if (len > 0)
        {
          if (buffer[len-1] == '\n')
            {
              buffer[len-1] = '\0';
            }

          /* Parse the command from the argument */

          cmd = strtok_r(buffer, " \n", &arg);
          if (cmd == NULL)
            {
              continue;
            }

          /* Remove leading spaces from arg */

          while (*arg == ' ')
            {
              arg++;
            }

          /* Find the command in our cmd array */

          for (x = 0; x < g_nxrecorder_cmd_count; x++)
            {
              if (strcmp(cmd, g_nxrecorder_cmds[x].cmd) == 0)
                {
                  /* Command found.  Call it's handler if not NULL */

                  if (g_nxrecorder_cmds[x].pFunc != NULL)
                    {
                      g_nxrecorder_cmds[x].pFunc(pRecorder, arg);
                    }

                  /* Test if it is a quit command */

                  if (g_nxrecorder_cmds[x].pFunc == nxrecorder_cmd_quit)
                    {
                     running = FALSE;
                    }

                  break;
                }
            }

          /* Test for Unknown command */

          if (x == g_nxrecorder_cmd_count)
            {
              printf("%s:  unknown nxrecorder command\n", buffer);
            }
        }
    }

  /* Release the NxRecorder context */

  nxrecorder_release(pRecorder);

  return OK;
}
