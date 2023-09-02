/*  Date/time module for the Small AMX
 *
 *  Copyright (c) ITB CompuPhase, 2001-2002
 *  This file may be freely used. No warranties of any kind.
 *
 *  Version: $Id: amxtime.c 2410 2003-11-07 09:38:37Z thiadmer $
 */
#include <time.h>
#include <assert.h>
#if defined __WIN32__ || defined _WIN32
  #include <windows.h>
  #include <mmsystem.h>
#endif
#include "amx.h"

#if !defined CLOCKS_PER_SEC
  #define CLOCKS_PER_SEC CLK_TCK
#endif

#if defined __BORLANDC__ || defined __WATCOMC__
  #pragma argsused
#endif
static cell AMX_NATIVE_CALL _time(AMX *amx, cell *params)
{
  time_t sec1970;
  struct tm gtm;
  cell *cptr;

  assert(params[0]==3*sizeof(cell));

  time(&sec1970);

  /* on DOS/Windows, the timezone is usually not set for the C run-time
   * library; in that case gmtime() and localtime() return the same value
   */
  gtm=*localtime(&sec1970);
  if (amx_GetAddr(amx,params[1],&cptr)==AMX_ERR_NONE)
    *cptr=gtm.tm_hour;
  if (amx_GetAddr(amx,params[2],&cptr)==AMX_ERR_NONE)
    *cptr=gtm.tm_min;
  if (amx_GetAddr(amx,params[3],&cptr)==AMX_ERR_NONE)
    *cptr=gtm.tm_sec;

  /* the time() function returns the number of seconds since January 1 1970
   * in Universal Coordinated Time (the successor to Greenwich Mean Time)
   */
  return sec1970;
}

#if defined __BORLANDC__ || defined __WATCOMC__
  #pragma argsused
#endif
static cell AMX_NATIVE_CALL _date(AMX *amx, cell *params)
{
  time_t sec1970;
  struct tm gtm;
  cell *cptr;

  assert(params[0]==3*sizeof(cell));

  time(&sec1970);

  gtm=*localtime(&sec1970);
  if (amx_GetAddr(amx,params[1],&cptr)==AMX_ERR_NONE)
    *cptr=gtm.tm_year+1900;
  if (amx_GetAddr(amx,params[2],&cptr)==AMX_ERR_NONE)
    *cptr=gtm.tm_mon+1;
  if (amx_GetAddr(amx,params[3],&cptr)==AMX_ERR_NONE)
    *cptr=gtm.tm_mday;

  return 0;
}

#if defined __BORLANDC__ || defined __WATCOMC__
  #pragma argsused
#endif
static cell AMX_NATIVE_CALL _tickcount(AMX *amx, cell *params)
{
  #if defined __WIN32__ || defined _WIN32
    static int timerset = 0;
  #endif
  cell value;

  assert(params[0]==sizeof(cell));

  #if defined __WIN32__ || defined _WIN32
    if (!timerset) {
      timeBeginPeriod(1);       /* timeGetTime() is more accurate on WindowsNT
                                 * if timeBeginPeriod(1) is set */
      timerset=1;
    } /* if */
    value=timeGetTime();        /* this value is already in milliseconds */
  #else
    value=(cell)clock();
    /* convert to milliseconds */
    value=(cell)((1000L * (value+CLOCKS_PER_SEC/2)) / CLOCKS_PER_SEC);
  #endif
  return value;
}


AMX_NATIVE_INFO time_Natives[] = {
  { "time",      _time },
  { "date",      _date },
  { "tickcount", _tickcount },
  { NULL, NULL }        /* terminator */
};

int amx_TimeInit(AMX *amx)
{
  return amx_Register(amx, time_Natives, -1);
}

#if defined __BORLANDC__ || defined __WATCOMC__
  #pragma argsused
#endif
int amx_TimeCleanup(AMX *amx)
{
  return AMX_ERR_NONE;
}
