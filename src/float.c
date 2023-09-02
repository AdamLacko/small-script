/*  Float arithmetic for the Small AMX engine
 *
 *  Copyright (c) Artran, Inc. 1999
 *  Written by Greg Garner (gmg@artran.com)
 *  This file may be freely used. No warranties of any kind.
 *
 * CHANGES -
 * 2002-08-27: Basic conversion of source from C++ to C by Adam D. Moss
 *             <adam@gimp.org> <aspirin@icculus.org>
 * 2003-08-29: Removal of the dynamic memory allocation and replacing two
 *             type conversion functions by macros, by Thiadmer Riemersma
 * 2003-09-22: Moved the type conversion macros to AMX.H, and simplifications
 *             of some routines, by Thiadmer Riemersma
 */
#include <stdlib.h>     /* for atof() */
#include <stdio.h>      /* for NULL */
#include <assert.h>
#include <math.h>
#include "amx.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/******************************************************************/
/* Private function to
   allocate and fill a C style string from a small type string.   */
static char *pcCreateAndFillStringFromCell(AMX *amx,cell params,char *buffer,int buflen)
{
    int nLen;
    cell *pString;

    /* Get the real address of the string. */
    amx_GetAddr(amx,params,&pString);

    /* Find out how long the string is in characters. */
    amx_StrLen(pString, &nLen);
    if (nLen >= buflen)
        return NULL;

    /* Now convert the Small String into a C type null terminated string */
    amx_GetString(buffer, pString);

    return buffer;
}

#if defined __BORLANDC__ || defined __WATCOMC__
  #pragma argsused
#endif
/******************************************************************/
static cell _float(AMX *amx,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = long value to convert to a float
    */
    float fValue;

    /* Convert to a float. Calls the compilers long to float conversion. */
    fValue = (float) params[1];

    /* Return the cell. */
    return amx_ftoc(fValue);
}

/******************************************************************/
#if defined __BORLANDC__ || defined __WATCOMC__
  #pragma argsused
#endif
static cell _floatstr(AMX *amx,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = virtual string address to convert to a float
    */
    char szSource[32];
    float fNum;
    long lCells;

    lCells = params[0]/sizeof(cell);

    /* They should have sent us 1 cell. */
    assert(lCells==1);

    /* Convert the Small string to a C style string. */
    pcCreateAndFillStringFromCell(amx, params[1], szSource, sizeof szSource);

    /* Now convert this to a float. */
    fNum = (float)atof(szSource);

    return amx_ftoc(fNum);
}

#if defined __BORLANDC__ || defined __WATCOMC__
  #pragma argsused
#endif
/******************************************************************/
static cell _floatmul(AMX *amx,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = float operand 1
    *   params[2] = float operand 2
    */
    float fRes = amx_ctof(params[1]) * amx_ctof(params[2]);
    return amx_ftoc(fRes);
}

#if defined __BORLANDC__ || defined __WATCOMC__
  #pragma argsused
#endif
/******************************************************************/
static cell _floatdiv(AMX *amx,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = float dividend (top)
    *   params[2] = float divisor (bottom)
    */
    float fRes = amx_ctof(params[1]) / amx_ctof(params[2]);
    return amx_ftoc(fRes);
}

#if defined __BORLANDC__ || defined __WATCOMC__
  #pragma argsused
#endif
/******************************************************************/
static cell _floatadd(AMX *amx,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = float operand 1
    *   params[2] = float operand 2
    */
    float fRes = amx_ctof(params[1]) + amx_ctof(params[2]);
    return amx_ftoc(fRes);
}

#if defined __BORLANDC__ || defined __WATCOMC__
  #pragma argsused
#endif
/******************************************************************/
static cell _floatsub(AMX *amx,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = float operand 1
    *   params[2] = float operand 2
    */
    float fRes = amx_ctof(params[1]) - amx_ctof(params[2]);
    return amx_ftoc(fRes);
}

#if defined __BORLANDC__ || defined __WATCOMC__
  #pragma argsused
#endif
/******************************************************************/
/* Return fractional part of float */
static cell _floatfract(AMX *amx,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = float operand
    */
    float fA = amx_ctof(params[1]);
    fA = fA - (float)(floor((double)fA));
    return amx_ftoc(fA);
}

#if defined __BORLANDC__ || defined __WATCOMC__
  #pragma argsused
#endif
/******************************************************************/
/* Return integer part of float, rounded */
static cell _floatround(AMX *amx,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = float operand
    *   params[2] = Type of rounding (long)
    */
    float fA = amx_ctof(params[1]);

    switch (params[2])
    {
        case 1:       /* round downwards (truncate) */
            fA = (float)(floor((double)fA));
            break;
        case 2:       /* round upwards */
            fA = (float)(ceil((double)fA));
            break;
        case 3:       /* round towards zero */
            if ( fA>=0.0 )
                fA = (float)(floor((double)fA));
            else
                fA = (float)(ceil((double)fA));
            break;
        default:      /* standard, round to nearest */
            fA = (float)(floor((double)fA+.5));
            break;
    }

    return (long)fA;
}

#if defined __BORLANDC__ || defined __WATCOMC__
  #pragma argsused
#endif
/******************************************************************/
static cell _floatcmp(AMX *amx,cell *params)
{
    /*
    *   params[0] = number of bytes
    *   params[1] = float operand 1
    *   params[2] = float operand 2
    */
    float fA, fB;

    fA = amx_ctof(params[1]);
    fB = amx_ctof(params[2]);
    if (fA == fB)
        return 0;
    else if (fA>fB)
        return 1;
    else
        return -1;

}

AMX_NATIVE_INFO float_Natives[] = {
  { "float",         _float },
  { "floatstr",      _floatstr },
  { "floatmul",      _floatmul },
  { "floatdiv",      _floatdiv },
  { "floatadd",      _floatadd },
  { "floatsub",      _floatsub },
  { "floatfract",    _floatfract},
  { "floatround",    _floatround},
  { "floatcmp",      _floatcmp},
  { NULL, NULL }        /* terminator */
};


#ifdef __cplusplus
}
#endif /* __cplusplus */
