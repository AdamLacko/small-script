/*  Command-line shell for the "Small" Abstract Machine using JIT compiler.
 *
 *  Copyright (c) ITB CompuPhase, 1997-2003
 *  Copyright (c) Mark Peter, 1998-1999
 *
 *  This file may be freely used. No warranties of any kind.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>     /* for memset() (on some compilers) */
#include "amx.h"

#include <time.h>
#if !defined CLOCKS_PER_SEC
  #define CLOCKS_PER_SEC CLK_TCK
#endif

#define CONSOLE
/*#define FIXEDPOINT*/

#if defined CONSOLE
  extern int amx_ConsoleInit(AMX *amx);
  extern int amx_ConsoleCleanup(AMX *amx);
#endif
#if defined DATETIME
  extern int amx_TimeInit(AMX *amx);
  extern int amx_TimeCleanup(AMX *amx);
#endif
#if defined FIXEDPOINT
  extern int amx_FixedInit(AMX *amx);
  extern int amx_FixedCleanp(AMX *amx);
#endif
#if defined FLOATPOINT
  extern int amx_FloatInit(AMX *amx);
  extern int amx_FloatCleanp(AMX *amx);
#endif
extern int amx_CoreInit(AMX *amx);
extern int amx_CoredCleanp(AMX *amx);

extern int amx_LogFileInit(AMX *amx);


static int abortflagged = 0;
void sigabort(int sig)
{
  abortflagged=1;
  signal(sig,sigabort); /* re-install the signal handler */
}

typedef struct tagSTACKINFO {
  long maxstack, maxheap;
} STACKINFO;

int AMXAPI amx_Monitor(AMX *amx)
{
  int err;
  unsigned short flags;
  STACKINFO *stackinfo;

  switch (amx->dbgcode) {
  case DBG_INIT:
    amx_Flags(amx,&flags);
    return (flags & AMX_FLAG_NOCHECKS) ? AMX_ERR_DEBUG : AMX_ERR_NONE;

  case DBG_LINE:
    /* record the heap and stack usage */
    err = amx_GetUserData(amx, AMX_USERTAG('S','t','c','k'), (void**)&stackinfo);
    if (err == AMX_ERR_NONE) {
      if (amx->stp - amx->stk > stackinfo->maxstack)
        stackinfo->maxstack = amx->stp - amx->stk;
      if (amx->hea - amx->hlw > stackinfo->maxheap)
        stackinfo->maxstack = amx->stp - amx->stk;
    } /* if */

    /* check whether an "abort" was requested */
    return abortflagged ? AMX_ERR_EXIT : AMX_ERR_NONE;

  default:
    return AMX_ERR_DEBUG;
  } /* switch */
}

void *LoadProgram(AMX *amx, char *filename)
{
  FILE *fp;
  AMX_HEADER hdr;
  void *program;
  #if defined JIT
    void *np, *rt;       // np = new machine; rt = relocation table
  #endif

  if ( (fp = fopen( filename, "rb" )) != NULL )
  {
    fread(&hdr, sizeof hdr, 1, fp);
    amx_Align32((uint32_t *)&hdr.stp);
	amx_Align32((uint32_t *)&hdr.size);
    if ( (program = malloc( (int)hdr.stp )) != NULL )
    {
      rewind( fp );
      fread( program, 1, (int)hdr.size, fp );
      fclose( fp );
      memset(amx, 0, sizeof *amx);
      amx_SetDebugHook(amx, amx_Monitor);
      if ( amx_Init( amx, program ) != AMX_ERR_NONE )
      {
         free( program );
         return NULL;
      }

      #if defined JIT
        /* Now try to run the JIT compiler... */
        np = malloc( amx->code_size );
        rt = malloc( amx->reloc_size );

        if ( !np || (!rt && amx->reloc_size>0) )
        {
           free( program );
           free( np );
           free( rt );
           return program;
        }

        /* JIT rulz! (TM) */
        if (amx_InitJIT(amx, rt, np) == AMX_ERR_NONE) {
          /* The compiled code is relocatable, since only relative jumps are
           * used for destinations within the generated code and absoulute
           * addresses for jumps into the runtime, which is fixed in memory.
           */
          amx->base = realloc( np, amx->code_size );
          free( program );
          free( rt );
          return np;
        }
        else    /* MP: added case where JIT failed */
        {
          free( np );
          free( program );
          free( rt );
          printf("JIT compiler failed\n");
          return NULL;
        } /* if */
      #else
        /* standard virtual machine (no JIT) */
        return program;
      #endif
    } /* if */
  } /* if */
  return NULL;
}


void ExitOnError(AMX *amx, int error)
{
  if (error != AMX_ERR_NONE) {
    printf("Run time error %d: \"%s\" on line %ld\n",
           error, amx_StrError(error), amx->curline);
    exit(1);
  } /* if */
}

void Usage(char *program)
{
  printf("Usage: %s <filename>\n", program);
  exit(1);
}

static int register_natives(AMX *amx)
{
  #if defined CONSOLE
    amx_ConsoleInit(amx);
  #endif
  #if defined DATETIME
    amx_TimeInit(amx);
  #endif
  #if defined FIXEDPOINT
    amx_FixedInit(amx);
  #endif
  #if defined FLOATPOINT
    amx_FloatInit(amx);
  #endif
  amx_LogFileInit(amx);
  return amx_CoreInit(amx);
}


int main(int argc,char *argv[])
{
  AMX amx;
  cell ret = 0;
  int err;
  void *program;
  clock_t start,end;
  STACKINFO stackinfo;

  if (argc != 2)
    Usage(argv[0]);     /* function "usage" aborts the program */

  if ((program = LoadProgram(&amx,argv[1])) == NULL) {
    /* try adding an extension */
    char filename[_MAX_PATH];
    strcpy(filename, argv[1]);
    strcat(filename, ".amx");
    if ((program = LoadProgram(&amx,filename)) == NULL)
      Usage(argv[0]);
  } /* if */
  signal(SIGINT,sigabort);

  err = register_natives(&amx);
  ExitOnError(&amx, err);

  memset(&stackinfo, 0, sizeof stackinfo);
  err = amx_SetUserData(&amx, AMX_USERTAG('S','t','c','k'), &stackinfo);
  ExitOnError(&amx, err);

  start=clock();
  err = amx_Exec(&amx, &ret, AMX_EXEC_MAIN, 0);
  while (err == AMX_ERR_SLEEP)
    err = amx_Exec(&amx, &ret, AMX_EXEC_CONT, 0);
  end=clock();
  ExitOnError(&amx, err);

  free(program);

  if (ret!=0)
    printf("\nReturn value: %ld\n", (long)ret);

  printf("\nRun time:     %.2f seconds\n",(double)(end-start)/CLOCKS_PER_SEC);
  if (stackinfo.maxstack != 0 || stackinfo.maxheap != 0) {
    printf("Stack usage:  %ld cells (%ld bytes)\n",
           stackinfo.maxstack / sizeof(cell), stackinfo.maxstack);
    printf("Heap usage:   %ld cells (%ld bytes)\n",
           stackinfo.maxheap / sizeof(cell), stackinfo.maxheap);
  } else {
    printf("No stack/heap usage information available\n");
  } /* if */

  return 0;
}
