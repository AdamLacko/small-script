/*  Small compiler - Error message system
 *  In fact a very simple system, using only 'panic mode'.
 *
 *  Copyright (c) ITB CompuPhase, 1997-2003
 *  This file may be freely used. No warranties of any kind.
 *
 *  Version: $Id: Sc5.c 2358 2003-10-16 12:00:51Z thiadmer $
 */
#include <assert.h>
#if defined	__WIN32__ || defined _WIN32 || defined __MSDOS__
  #include <io.h>
#endif
#if defined LINUX || defined __GNUC__
  #include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>     /* ANSI standardized variable argument list functions */
#include <string.h>
#if defined FORTIFY
  #include "fortify.h"
#endif
#include "sc.h"

#if defined _MSC_VER
  #pragma warning(push)
  #pragma warning(disable:4125)  /* decimal digit terminates octal escape sequence */
#endif

#include "sc5.scp"

#if defined _MSC_VER
  #pragma warning(pop)
#endif

static int errflag;
static int errstart;    /* line number at which the instruction started */

/*  error
 *
 *  Outputs an error message (note: msg is passed optionally).
 *  If an error is found, the variable "errflag" is set and subsequent
 *  errors are ignored until lex() finds a semicolumn or a keyword
 *  (lex() resets "errflag" in that case).
 *
 *  Global references: inpfname   (reffered to only)
 *                     fline      (reffered to only)
 *                     fcurrent   (reffered to only)
 *                     errflag    (altered)
 */
SC_FUNC int error(int number,...)
{
static char *prefix[3]={ "error", "fatal error", "warning" };
static int lastline,lastfile,errorcount;
  char *msg,*pre;
  va_list argptr;
  char string[128];

  /* errflag is reset on each semicolon.
   * In a two-pass compiler, an error should not be reported twice. Therefore
   * the error reporting is enabled only in the second pass (and only when
   * actually producing output). Fatal errors may never be ignored.
   */
  if ((errflag || sc_status!=statWRITE) && (number<100 || number>=200))
    return 0;

  if (number<100){
    msg=errmsg[number-1];
    pre=prefix[0];
    errflag=TRUE;       /* set errflag (skip rest of erroneous expression) */
    errnum++;
  } else if (number<200){
    msg=fatalmsg[number-100];
    pre=prefix[1];
    errnum++;           /* a fatal error also counts as an error */
  } else {
    msg=warnmsg[number-200];
    pre=prefix[2];
    warnnum++;
  } /* if */

  strexpand(string,(unsigned char *)msg,sizeof string,SCPACK_TABLE);

  assert(errstart<=fline);
  va_start(argptr,number);
  if (strlen(errfname)==0) {
    int start= (errstart==fline) ? -1 : errstart;
    if (sc_error(number,string,inpfname,start,fline,argptr)) {
      sc_closeasm(outf,TRUE);
      outf=NULL;
      longjmp(errbuf,3);        /* user abort */
    } /* if */
  } else {
    FILE *fp=fopen(errfname,"at");
    if (fp!=NULL) {
      if (errstart>=0 && errstart!=fline)
        fprintf(fp,"%s(%d -- %d) : %s %03d: ",inpfname,errstart,fline,pre,number);
      else
        fprintf(fp,"%s(%d) : %s %03d: ",inpfname,fline,pre,number);
      vfprintf(fp,string,argptr);
      fclose(fp);
    } /* if */
  } /* if */
  va_end(argptr);

  if (number>=100 && number<200 || errnum>25){
    if (strlen(errfname)==0) {
      va_start(argptr,number);
      sc_error(0,"\nCompilation aborted.",NULL,0,0,argptr);
      va_end(argptr);
    } /* if */
    if (outf!=NULL) {
      sc_closeasm(outf,TRUE);
      outf=NULL;
    } /* if */
    longjmp(errbuf,2);          /* fatal error, quit */
  } /* if */

  /* check whether we are seeing many errors on the same line */
  if ((errstart<0 && lastline!=fline) || lastline<errstart || lastline>fline || fcurrent!=lastfile)
    errorcount=0;
  lastline=fline;
  lastfile=fcurrent;
  if (number<200)
    errorcount++;
  if (errorcount>=3)
    error(107);         /* too many error/warning messages on one line */

  return 0;
}

SC_FUNC void errorset(int code)
{
  switch (code) {
  case sRESET:
    errflag=FALSE;      /* start reporting errors */
    break;
  case sFORCESET:
    errflag=TRUE;       /* stop reporting errors */
    break;
  case sEXPRMARK:
    errstart=fline;     /* save start line number */
    break;
  case sEXPRRELEASE:
    errstart=-1;        /* forget start line number */
    break;
  } /* switch */
}

#undef SCPACK_TABLE
