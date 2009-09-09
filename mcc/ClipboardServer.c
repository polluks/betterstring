/***************************************************************************

 BetterString.mcc - A better String gadget MUI Custom Class
 Copyright (C) 1997-2000 Allan Odgaard
 Copyright (C) 2005-2009 by BetterString.mcc Open Source Team

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 BetterString class Support Site:  http://www.sf.net/projects/bstring-mcc/

 $Id$

***************************************************************************/

#include <string.h>
#include <stdlib.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>

// for iffparse.library no global variable definitions are needed
#define __NOLIBBASE__
#define __NOGLOBALIFACE__
#include <proto/iffparse.h>

#include "private.h"

#include "Debug.h"

static struct Library *IFFParseBase = NULL;
static struct IFFParseIFace *IIFFParse = NULL;
static struct SignalSemaphore *serverLock = NULL;
static struct Process *serverProcess = NULL;
static struct MsgPort *serverPort = NULL;
static struct MsgPort replyPort;
static struct Message msg;

struct ServerData
{
  ULONG sd_Command;
  STRPTR sd_String;
  LONG sd_Length;
};

#define SERVER_SHUTDOWN   0xdeadf00d
#define SERVER_WRITE      0x00000001
#define SERVER_READ       0x00000002

#define ID_FORM    MAKE_ID('F','O','R','M')
#define ID_FTXT    MAKE_ID('F','T','X','T')
#define ID_CHRS    MAKE_ID('C','H','R','S')
#define ID_CSET    MAKE_ID('C','S','E','T')

/// StringToClipboard
// copy a string to the clipboard, public callable function
void StringToClipboard(STRPTR str, LONG length)
{
  // lock out other tasks
  if(AttemptSemaphore(serverLock))
  {
    struct ServerData sd;

    // set up the data packet
    sd.sd_Command = SERVER_WRITE;
    sd.sd_String = str;
    sd.sd_Length = (length > 0) ? length : (LONG)strlen(str);

    if(sd.sd_Length > 0)
    {
      // set up the message, send it and wait for a reply
      msg.mn_Node.ln_Name = (STRPTR)&sd;
      replyPort.mp_SigTask = FindTask(NULL);

      PutMsg(serverPort, &msg);
      Remove((struct Node *)WaitPort(&replyPort));
    }
    else
      DisplayBeep(0);

    // allow other tasks again
    ReleaseSemaphore(serverLock);
  }
  else
    DisplayBeep(0);
}

///
/// ClipboardToString
// copy from the clipboard to a string, public callable function
// the string must be FreeVec()ed externally
void ClipboardToString(STRPTR *str, LONG *length)
{
  // lock out other tasks
  if(AttemptSemaphore(serverLock))
  {
    struct ServerData sd;

    // set up the data packet
    sd.sd_Command = SERVER_READ;

    // set up the message, send it and wait for a reply
    msg.mn_Node.ln_Name = (STRPTR)&sd;
    replyPort.mp_SigTask = FindTask(NULL);

    PutMsg(serverPort, &msg);
    Remove((struct Node *)WaitPort(&replyPort));

    // retrieve the values from the server
    *str = sd.sd_String;
    *length = sd.sd_Length;

    // allow other tasks again
    ReleaseSemaphore(serverLock);
  }
  else
    DisplayBeep(0);
}

///
/// WriteToClipboard
// write a given string via iffparse.library to the clipboard
// non-public server side function
static void WriteToClipboard(STRPTR str, LONG length)
{
  struct IFFHandle *iff;

  if((iff = AllocIFF()) != NULL)
  {
    if((iff->iff_Stream = (IPTR)OpenClipboard(0)) != 0)
    {
      InitIFFasClip(iff);

      if(OpenIFF(iff, IFFF_WRITE) == 0)
      {
        PushChunk(iff, ID_FTXT, ID_FORM, IFFSIZE_UNKNOWN);
        PushChunk(iff, 0, ID_CHRS, IFFSIZE_UNKNOWN);
        WriteChunkBytes(iff, str, length);
        PopChunk(iff);
        PopChunk(iff);

        CloseIFF(iff);
      }

      CloseClipboard((struct ClipboardHandle *)iff->iff_Stream);
    }

    FreeIFF(iff);
  }
}

///
/// ReadFromClipboard
// read a string via iffparse.library from the clipboard
// non-public server side function
static void ReadFromClipboard(STRPTR *str, LONG *length)
{
  struct IFFHandle *iff;

  *str = NULL;
  *length = 0;

  if((iff = AllocIFF()) != NULL)
  {
    if((iff->iff_Stream = (IPTR)OpenClipboard(0)) != 0)
    {
      InitIFFasClip(iff);

      if(OpenIFF(iff, IFFF_READ) == 0)
      {
        if(StopChunk(iff, ID_FTXT, ID_CHRS) == 0 && StopChunk(iff, ID_FTXT, ID_CSET) == 0)
        {
          LONG codeset = 0;

          while(TRUE)
          {
            LONG error;
            struct ContextNode *cn;

            error = ParseIFF(iff, IFFPARSE_SCAN);
            if(error == IFFERR_EOC)
              continue;
            else if(error != 0)
              break;

            if((cn = CurrentChunk(iff)) != NULL)
            {
              if(cn->cn_ID == ID_CSET)
              {
                if (cn->cn_Size >= 4)
                {
                  // Only the first four bytes are interesting
                  if(ReadChunkBytes(iff, &codeset, 4) != 4)
                    codeset = 0;
                }
              }
              else if(cn->cn_ID == ID_CHRS && cn->cn_Size > 0)
              {
                ULONG size = cn->cn_Size;
                STRPTR buffer;

                if((buffer = AllocVec(size + 1, MEMF_ANY)) != NULL)
                {
                  LONG readBytes;

                  // read the string from the clipboard
                  if((readBytes = ReadChunkBytes(iff, buffer, size)) > 0)
                  {
                    memset(buffer + readBytes, 0, size-readBytes+1);

                    #if defined(__MORPHOS__)
                    if (codeset == CODESET_UTF8)
                    {
                      if (IS_MORPHOS2)
                      {
                        utf8_to_ansi(buffer, buffer);
                        readBytes = strlen(buffer);
                      }
                    }
                    #endif

                    *str = buffer;
                    *length = readBytes;
                  }
                  else
                    E(DBF_ALWAYS, "ReadChunkBytes error! (%ld)", readBytes);
                }
              }
            }
          }
        }

        CloseIFF(iff);
      }

      CloseClipboard((struct ClipboardHandle *)iff->iff_Stream);
    }

    FreeIFF(iff);
  }
}

///
/// ClipboardServer
// the clipboard server process
static LONG ClipboardServer(UNUSED STRPTR args, UNUSED LONG length, struct ExecBase *sysBase)
{
  struct Process *me;
  struct Message *msg;
  #if defined(__amigaos4__)
  struct ExecIFace *IExec = (struct ExecIFace *)sysBase->MainInterface;
  #endif

  ENTER();

  me = (struct Process *)FindTask(NULL);
  WaitPort(&me->pr_MsgPort);
  msg = GetMsg(&me->pr_MsgPort);

  if((IFFParseBase = OpenLibrary("iffparse.library", 36)) != NULL)
  {
    #if defined(__amigaos4__)

    if((IIFFParse = (struct IFFParseIFace *)GetInterface(IFFParseBase, "main", 1, NULL)) != NULL)
    {
    #endif
    struct MsgPort *mp;

    #if defined(__amigaos4__)
    mp = AllocSysObjectTags(ASOT_PORT, TAG_DONE);
    #else
    mp = CreateMsgPort();
    #endif
    if(mp != NULL)
    {
      BOOL running = TRUE;

      // return something as a valid reply
      msg->mn_Node.ln_Name = (STRPTR)mp;
      ReplyMsg(msg);

      do
      {
        WaitPort(mp);

        while((msg = GetMsg(mp)) != NULL)
        {
          struct ServerData *sd = (struct ServerData *)msg->mn_Node.ln_Name;

          switch(sd->sd_Command)
          {
            case SERVER_SHUTDOWN:
            {
              running = FALSE;
            }
            break;

            case SERVER_WRITE:
            {
              WriteToClipboard(sd->sd_String, sd->sd_Length);
            }
            break;

            case SERVER_READ:
            {
              ReadFromClipboard(&sd->sd_String, &sd->sd_Length);
            }
            break;
          }

          ReplyMsg(msg);
        }
      }
      while(running == TRUE);

      #if defined(__amigaos4__)
      FreeSysObject(ASOT_PORT, mp);
      #else
      DeleteMsgPort(mp);
      #endif
    }

    #if defined(__amigaos4__)
    DropInterface((struct Interface *)IIFFParse);
    }
    #endif

    CloseLibrary(IFFParseBase);
  }

  Forbid();

  LEAVE();
  return 0;
}

///
/// StartClipboardServer
// launch the clipboard server process
// we must use a separate process, because accessing the clipboard via iffparse.library
// allocates 2 signals for every instance of this class. Hence we will run out of signals
// sooner or later. The separate process avoids this situation.
BOOL StartClipboardServer(void)
{
  BOOL success = FALSE;

  ENTER();

  // create a semaphore to protect several concurrent tasks
  #if defined(__amigaos4__)
  serverLock = AllocSysObjectTags(ASOT_SEMAPHORE, TAG_DONE);
  #else
  serverLock = AllocVec(sizeof(*serverLock), MEMF_ANY);
  #endif
  if(serverLock != NULL)
  {
    #if !defined(__amigaos4__)
    InitSemaphore(serverLock);
    #endif

    // create the server process
    // this must *NOT* be a child process
    serverProcess = CreateNewProcTags(NP_Entry, ClipboardServer,
                                      NP_Name, "BetterString.mcc clipboard server",
                                      NP_Priority, 1,
                                      NP_StackSize, 16384,
                                      NP_WindowPtr, ~0L,
                                      #if defined(__amigaos4__)
                                      NP_Child, FALSE,
                                      #endif
                                      TAG_DONE);
    if(serverProcess !=  NULL)
    {
      // we use one global reply port with a static signal bit
      replyPort.mp_Node.ln_Type = NT_MSGPORT;
      NewList(&replyPort.mp_MsgList);
      replyPort.mp_SigBit = SIGB_SINGLE;
      replyPort.mp_SigTask = FindTask(NULL);

      msg.mn_ReplyPort = &replyPort;
      msg.mn_Node.ln_Name = (STRPTR)NULL;

      // send out the startup message and wait for a reply
      PutMsg(&serverProcess->pr_MsgPort, &msg);
      Remove((struct Node *)WaitPort(&replyPort));

      // check whether everything went ok
      if((serverPort = (struct MsgPort *)msg.mn_Node.ln_Name) != NULL)
      {
        success = TRUE;
      }
    }
  }

  RETURN(success);
  return success;
}

///
/// ShutdownClipboardServer
// shutdown the server process and clean up
void ShutdownClipboardServer(void)
{
  if(serverPort != NULL)
  {
    struct ServerData sd;

    sd.sd_Command = SERVER_SHUTDOWN;

    msg.mn_Node.ln_Name = (STRPTR)&sd;
    replyPort.mp_SigTask = FindTask(NULL);

    PutMsg(serverPort, &msg);
    WaitPort(&replyPort);

    serverPort = NULL;
  }

  if(serverLock != NULL)
  {
    #if defined(__amigaos4__)
    FreeSysObject(ASOT_SEMAPHORE, serverLock);
    #else
    FreeVec(serverLock);
    #endif

    serverLock = NULL;
  }
}

///

