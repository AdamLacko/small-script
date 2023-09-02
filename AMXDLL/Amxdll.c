/*  Abstract Machine DLL interface functions
 *
 *  Copyright (c) ITB CompuPhase, 1999-2003
 *  This file may be freely used. No warranties of any kind.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <ctype.h>      /* for isdigit() */
#include <limits.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>     /* for atoi() */
#include <string.h>

/* redirect amx_Init() to a special version */
#if !defined amx_Init
  #error "amx_Init" must be defined when compiling these sources
#endif
#undef amx_Init /* redirection is over here */

#include "osdefs.h"
#include "balloon.h"
#include "amx.h"


int AMXAPI amx_InitAMX(AMX *amx, void *program);
static cell AMX_NATIVE_CALL _messagebox(AMX *amx,cell *params);
static cell AMX_NATIVE_CALL _iswin32(AMX *amx,cell *params);
static cell AMX_NATIVE_CALL CallDLL(AMX *amx,cell *params);
static cell AMX_NATIVE_CALL LoadDLL(AMX *amx,cell *params);
static cell AMX_NATIVE_CALL FreeDLL(AMX *amx,cell *params);
static cell AMX_NATIVE_CALL BalloonSet(AMX *amx,cell *params);
static cell AMX_NATIVE_CALL BalloonFont(AMX *amx,cell *params);

#if defined _Windows
  static HINSTANCE hinstAMX;
  static HWND hwndConsole;
#endif

AMX_NATIVE_INFO dll_Natives[] = {
  { "messagebox", _messagebox },
  { "iswin32", _iswin32 },
  { "calldll", CallDLL },
  { "loaddll", LoadDLL },
  { "freedll", FreeDLL },
#if defined __WIN32__ || defined __NT__ || defined _WIN32
  { "balloon", BalloonSet },
  { "balloonfont", BalloonFont },
#endif
  { NULL, NULL }        /* terminator */
};

int amx_CoreInit(AMX *amx);
int amx_ConsoleInit(AMX *amx);
int amx_FixedInit(AMX *amx);


int AMXAPI amx_Init(AMX *amx, void *program)
{
  int err;

  if ((err=amx_InitAMX(amx,program)) != AMX_ERR_NONE)
    return err;

  /* load standard libraries */
  amx_CoreInit(amx);
  amx_ConsoleeInit(amx);
  amx_FixedInit(amx);
  err = amx_Register(amx,dll_Natives,-1);

  return err;
}

#if defined _Windows

#define NUMLINES        30
#define NUMCOLUMNS      80
#define DEFWIDTH        640
#define DEFHEIGHT       480
#define U_PRINT         (WM_USER+1)
#define U_GETCHAR       (WM_USER+2)
#define U_PUTCHAR       (WM_USER+3)

static void scrollup(char **lines,int numlines)
{
  int ln;

  for (ln=1; ln<numlines; ln++)
    strcpy(lines[ln-1],lines[ln]);
  lines[numlines-1][0]='\0';
}

#define KEYQUEUE_SIZE 20
static WORD keyqueue[KEYQUEUE_SIZE];
static int key_start, key_end;

#define CSR_STACKSIZE 4

long CALLBACK _export AMXConsoleFunc(HWND hwnd,unsigned message,WPARAM wParam,
                                     LPARAM lParam)
{
static char **lines;
static int x,y;
static int csr_stack[CSR_STACKSIZE][2];
static int csr_stacktop;
static BOOL WrapMode;
static HFONT hfont; //, hfontBold;
static BOOL showcaret;
  PAINTSTRUCT ps;
  RECT rect;
  char *line;
  int ln,height;
  WORD key;
  HFONT hfontOrg;

  switch (message) {
  case WM_CHAR:
  case U_PUTCHAR:
    /* store in a queue */
    if ((key_end+1)%KEYQUEUE_SIZE==key_start) {
      MessageBeep(MB_OK);
      break;
    } /* if */
    keyqueue[key_end]=(WORD)wParam;
    key_end=(key_end+1)%KEYQUEUE_SIZE;
    break;

  case WM_CREATE:
    /* allocate memory for the lines */
    lines=(char **)malloc(NUMLINES*sizeof(char *));
    if (lines==NULL) {
      DestroyWindow(hwnd);
      break;
    } /* if */
    for (y=0; y<NUMLINES; y++)
      lines[y]=NULL;    /* preset */
    for (y=0; y<NUMLINES; y++) {
      lines[y]=(char *)malloc((NUMCOLUMNS+1)*sizeof(char));
      if (lines[y]==NULL) {
        DestroyWindow(hwnd);
        break;
      } /* if */
      lines[y][0]='\0';
    } /* for */
    /* create (monospaced) fonts */
    hfont=CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, 0, 0,
                     DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                     CLIP_CHARACTER_PRECIS, PROOF_QUALITY,
                     FIXED_PITCH|FF_DONTCARE, "Courier New");
    #if 0
      hfontBold=CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, 0, 0,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                           CLIP_CHARACTER_PRECIS, PROOF_QUALITY,
                           FIXED_PITCH|FF_DONTCARE, "Courier New");
    #endif
    /* set initial cursor position */
    x=y=0;
    csr_stacktop=0;
    /* create a caret */
    GetClientRect(hwnd,&rect);
    CreateCaret(hwnd,NULL,rect.right/NUMCOLUMNS,2);
    showcaret=FALSE;
    WrapMode=TRUE;
    key_start=key_end=0;
    break;

  case WM_DESTROY:
    if (lines!=NULL) {
      for (y=0; y<NUMLINES; y++)
        if (lines[y]!=NULL)
          free(lines[y]);
      free(lines);
    } /* if */
    if (hfont!=0)
      DeleteObject(hfont);
    #if 0
      if (hfontBold!=0)
        DeleteObject(hfontBold);
    #endif
    DestroyCaret();
    break;

  case WM_SYSKEYDOWN:
  case WM_KEYDOWN: {
    char str[20];
    int i;
    switch (LOWORD(wParam)) {
    case VK_F1:
    case VK_F2:
    case VK_F3:
    case VK_F4:
    case VK_F5:
    case VK_F6:
    case VK_F7:
    case VK_F8:
    case VK_F9:
    case VK_F10:
    case VK_F11:
    case VK_F12:
      wsprintf(str,"\033F%d\n",LOWORD(wParam)-VK_F1+1);
      for (i=0; str[i]; i++)
        SendMessage(hwnd,U_PUTCHAR,str[i],0L);
      break;
    default:
      return DefWindowProc(hwnd,message,wParam,lParam);
    } /* switch */
    break;
  } /* case */

  case WM_PAINT:
    if (showcaret)
      HideCaret(hwnd);
    GetClientRect(hwnd,&rect);
    height=rect.bottom/NUMLINES;
    BeginPaint(hwnd, &ps);
    hfontOrg=SelectObject(ps.hdc,hfont);
    for (ln=0; ln<NUMLINES; ln++) {
      SetRect(&rect,0,ln*height,rect.right,(ln+1)*height);
      ExtTextOut(ps.hdc,0,ln*height,ETO_OPAQUE|ETO_CLIPPED,&rect,
                 lines[ln],strlen(lines[ln]),NULL);
    } /* for */
    SelectObject(ps.hdc,hfontOrg);
    EndPaint(hwnd, &ps);
    if (showcaret)
      ShowCaret(hwnd);
    break;

  case WM_SIZE:
    if (hfont!=0)
      DeleteObject(hfont);
    /* make a new font, redraw everything */
    GetClientRect(hwnd,&rect);
    hfont=CreateFont(rect.bottom/NUMLINES, rect.right/NUMCOLUMNS,
                     0, 0, FW_NORMAL, FALSE, 0, 0, DEFAULT_CHARSET,
                     OUT_DEFAULT_PRECIS, CLIP_CHARACTER_PRECIS, PROOF_QUALITY,
                     FIXED_PITCH|FF_DONTCARE, "Courier New");
    InvalidateRect(hwnd,NULL,FALSE);
    if (showcaret)
      SetCaretPos((rect.right/NUMCOLUMNS)*x,(rect.bottom/NUMLINES)*(y+1)-2);
    break;

  case U_PRINT:
    line=(char *)lParam;
    while (*line!='\0') {
      assert(x<NUMCOLUMNS);
      assert(y<NUMLINES);
      switch (*line) {
      case '\b':
        if (x>0)
          x--;
        lines[y][x]='\0';
        break;
      case '\t':
        if ((x/8+1)*8 < NUMCOLUMNS) {
          lines[y][x++]=' ';
          while (x%8!=0)
            lines[y][x++]=' ';
          lines[y][x]='\0';
        } /* if */
        break;
      case '\n':
        x=0; y++;
        break;
      case '\033': {
        int x2,y2;
        /* parse the text output, support ANSI escape sequences used by the debugger */
        if (strncmp(line,"\033[2J",4)==0) {                 /* clear screen */
          line+=3;
          for (y=0; y<NUMLINES; y++)
            lines[y][0]='\0';
          x=y=0;
        } else if (strncmp(line,"\033[K",3)==0) {           /* clear eol */
          line+=2;
          lines[y][x]='\0';
        } else if (sscanf(line,"\033[%d;%dH",&y2,&x2)==2) { /* set cursor position */
          while (*line!='H')
            line++;
          x=x2-1;       /* (1,1) is upper left */
          y=y2-1;
        } else if (strncmp(line,"\033[6n",4)==0) {          /* get cursor position */
          char str[20];
          line+=3;
          sprintf(str,"\033[%d;%dR\r",y+1,x+1);
          for (x2=0; str[x2]; x2++)
            SendMessage(hwnd,U_PUTCHAR,str[x2],0L);
        } else if (strncmp(line,"\033[s",3)==0) {           /* save cursor position */
          line+=2;
          if (csr_stacktop < CSR_STACKSIZE) {
            csr_stack[csr_stacktop][0]=x;
            csr_stack[csr_stacktop][1]=y;
            csr_stacktop++;
          } /* if */
        } else if (strncmp(line,"\033[u",3)==0) {           /* restore cursor position */
          line+=2;
          if (csr_stacktop > 0) {
            csr_stacktop--;
            x=csr_stack[csr_stacktop][0];
            y=csr_stack[csr_stacktop][1];
          } /* if */
        } else if (strncmp(line,"\033[0m",4)==0) {          /* reset character style */
          // ??? reset font
        } else if (strncmp(line,"\033[0m",4)==0) {          /* reset character style */
          // ??? select bold font
        } else if (strncmp(line,"\033[?7l",5)==0) {         /* disable wrap-around */
          WrapMode=FALSE;
        // ??? code to set wrap-around back on
        // ??? code to select different window on SI/SO codes
        // ??? code to set a window
        } else {
          if (x<NUMCOLUMNS) {
            lines[y][x++]=*line;
            lines[y][x]='\0';
          } /* if */
        } /* if */
        break;
      } /* case */
      default:
        if (x<NUMCOLUMNS) {
          lines[y][x++]=*line;
          lines[y][x]='\0';
        } /* if */
      } /* if */
      line++;
      /* handle wrap-around and scrolling */
      if (WrapMode && x>=NUMCOLUMNS) {
        x=0; y++;
      } /* if */
      if (y>=NUMLINES) {
        scrollup(lines,NUMLINES);
        y--;
        assert(y<NUMLINES);
      } /* if */
    } /* while */
    InvalidateRect(hwnd,NULL,FALSE);
    UpdateWindow(hwnd);
    break;

  case U_GETCHAR:
    showcaret=TRUE;
    ShowCaret(hwnd);
    GetClientRect(hwnd,&rect);
    SetCaretPos((rect.right/NUMCOLUMNS)*x,(rect.bottom/NUMLINES)*(y+1)-2);
    while (key_start==key_end) {
      MSG msg;
      if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      } /* if */
    } /* while */
    showcaret=FALSE;
    HideCaret(hwnd);
    key=keyqueue[key_start];
    key_start=(key_start+1)%KEYQUEUE_SIZE;
    return key;

  default:
    return DefWindowProc(hwnd,message,wParam,lParam);
  } /* switch */
  return 0L;
}

static BOOL InitWindowClass(void)
{
  WNDCLASS wc;

  wc.style=CS_GLOBALCLASS|CS_NOCLOSE;
  wc.lpfnWndProc=(WNDPROC)AMXConsoleFunc;
  wc.cbClsExtra=0;
  wc.cbWndExtra=0;
  wc.hInstance=hinstAMX;
  wc.hIcon=LoadIcon(hinstAMX,"amxdll");
  wc.hCursor=LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
  wc.lpszMenuName=(LPSTR)NULL;
  wc.lpszClassName="AMX_console";
  return RegisterClass(&wc);
}

static BOOL MakeConsoleWindow(void)
{
  if (!IsWindow(hwndConsole)) {
    RECT rect;
    SetRect(&rect,0,0,DEFWIDTH,DEFHEIGHT);
    AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW,FALSE);
    hwndConsole=CreateWindow("AMX_console","Small console",WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                             CW_USEDEFAULT,CW_USEDEFAULT,rect.right-rect.left,rect.bottom-rect.top,
                             HWND_DESKTOP,NULL,hinstAMX,NULL);
  } /* if */
  return IsWindow(hwndConsole);
}

#pragma argsused
int amx_fflush(void)
{
  return 0;
}

void amx_putchar(int ch)
{
  char str[]={0,0};
  str[0]=(char)ch;
  if (MakeConsoleWindow())
    SendMessage(hwndConsole,U_PRINT,0,(LPARAM)(LPSTR)str);
}

// this doesn't get declared in stdio.h if _WINDOWS is defined
// although it is in the Windows libraries!
int vsprintf(char *, const char *, va_list);

int amx_printf(const char * strFmt, ...)
{
  va_list marker;
  int len=0;

  if (MakeConsoleWindow()) {
    char str[1024]={0};
    va_start(marker,strFmt);
    len = vsprintf(str,strFmt,marker);
    SendMessage(hwndConsole,U_PRINT,0,(LPARAM)(LPSTR)str);
  } /* if */
  return len;
}

int amx_getch(void)
{
  if (MakeConsoleWindow())
    return (int)SendMessage(hwndConsole,U_GETCHAR,0,0L);
  return 0;
}

char *amx_gets(char *str,int length)
{
  if (str==NULL)
    return NULL;
  if (MakeConsoleWindow() && length>=2) {
    int i=0,show=TRUE;
    int c;

    do {
      c=(int)SendMessage(hwndConsole,U_GETCHAR,0,0L);
      if (c=='\r') {
        c='\n';
      } else if (c=='\b' && i>0) {
        str[--i]='\0';
      } else if (c=='\033') {
        i=0;
        show=FALSE;
      } else if (c!='\t' && (c<32 || c>255)) {
        continue;
      } /* if */
      if (show)
        amx_putchar(c);
      if (c!='\b')
        str[i++]=c;
    } while (c!='\n' && i<length-1);
    str[i]='\0';
  } /* if */
  return str;
}

#if 0
/* enum timezone
 *     {
 *     localtime,  // local time
 *     gmtime      // Greenwich mean time
 *     }
 * time(&hour, &minute, &second, timezone:timezone=localtime);
 * date
 * native timestring(string[], timezone:timezone=localtime, bool:pack=false);
 * datestring
 */
static cell AMX_NATIVE_CALL _timestring(AMX *amx,cell *params)
{
  time_t t;
  struct tm *ct;
  cell *cptr;

  t = time(NULL);
  if (params[2]!=0)
    ct = gmtime(&t);
  else
    ct = localtime(&t);
  amx_GetAddr(amx,params[1],&cptr);
  amx_SetString(cptr,asctime(area),(int)params[3]);
}
#endif

/* enum { Ok, Okcancel, okCancel, Yesno, yesNo, Yesnocancel, yesNocancel, yesnoCancel }
 * enum { noicon, information, exclamation, question, stop }
 * messagebox(message[], caption[], buttons=Ok, icons=noicon, timeout=0)
 */
static cell AMX_NATIVE_CALL _messagebox(AMX *amx,cell *params)
{
static long btn_style[] = { MB_OK, MB_OKCANCEL, MB_YESNO, MB_RETRYCANCEL, MB_ABORTRETRYIGNORE };
static long icon_style[] = { MB_ICONINFORMATION, MB_ICONEXCLAMATION, MB_ICONQUESTION, MB_ICONSTOP };
  char message[256], caption[128];
  long style;
  cell *cptr;
  int len;
  HWND hwnd;

  amx_GetAddr(amx,params[1],&cptr);
  amx_StrLen(cptr,&len);
  if (len<256)
    amx_GetString(message,cptr);
  else
    lstrcpy(message,"(message too long)");

  amx_GetAddr(amx,params[2],&cptr);
  amx_StrLen(cptr,&len);
  if (len<128)
    amx_GetString(caption,cptr);
  else
    lstrcpy(caption,"(caption too long)");

  style=0;
  if (params[3]>=0 && params[3]<5)
    style|=btn_style[(int)params[3]];
  if (params[4]>=0 && params[4]<4)
    style|=btn_style[(int)params[4]];

  /* remove previous messagebox */
  if ((hwnd=FindWindow("#32770",caption))!=0)
    EndDialog(hwnd,IDCANCEL);

  return MessageBox(GetFocus(),message,caption,(unsigned)style);
}

#if defined __WIN32__ || defined __NT__ || defined _WIN32
/* balloon(Balloon:&handle, Message[]="", X=0, Y=0, Timeout=-1)
 */
static cell AMX_NATIVE_CALL BalloonSet(AMX *amx,cell *params)
{
  #if defined __WIN32__ || defined __NT__ || defined _WIN32
    char message[512];
  #else
    char message[256];
  #endif
  cell *cptr;
  int len;
  HWND hwnd;
  POINT pt;

  // get the balloon window handle, or create a new balloon
  amx_GetAddr(amx,params[1],&cptr);
  hwnd=(HWND)*cptr;
  if (hwnd==NULL) {
    hwnd=bm_Create(hinstAMX,NULL);
    *cptr=(cell)hwnd;
    bm_SetAlign(hwnd,BAA_BOTTOM);
    bm_SetTail(hwnd,16);
  } /* if */

  amx_GetAddr(amx,params[2],&cptr);
  amx_StrLen(cptr,&len);
  if (len==0)
    return bm_Set(hwnd,NULL,NULL);
  if (len<sizeof message)
    amx_GetString(message,cptr);
  else
    lstrcpy(message,"(message too long)");

  pt.x=params[3];
  pt.y=params[4];

  if (params[5]>=0)
    bm_SetTimeout(hwnd,(DWORD)params[5]);

  return bm_Set(hwnd,message,&pt);
}

/* balloonfont(Balloon:&handle, font[]="", height=16, weight=400, italic=0)
 */
static cell AMX_NATIVE_CALL BalloonFont(AMX *amx,cell *params)
{
  char name[128];
  cell *cptr;
  int len;
  HWND hwnd;
  HFONT hfont;

  // get the balloon window handle, or create a new balloon
  amx_GetAddr(amx,params[1],&cptr);
  hwnd=(HWND)*cptr;
  if (hwnd==NULL) {
    hwnd=bm_Create(hinstAMX,NULL);
    *cptr=(cell)hwnd;
    bm_SetAlign(hwnd,BAA_BOTTOM);
    bm_SetTail(hwnd,16);
  } /* if */

  amx_GetAddr(amx,params[2],&cptr);
  amx_StrLen(cptr,&len);
  if (len==0)
    return bm_Set(hwnd,NULL,NULL);
  if (len>=sizeof name)
    return FALSE;
  amx_GetString(name,cptr);

  hfont=CreateFont((int)params[3], 0, 0, 0, (int)params[4], (int)params[5],
                   0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                   CLIP_CHARACTER_PRECIS, PROOF_QUALITY, FF_DONTCARE, name);
  hfont=bm_SetFont(hwnd,hfont);
  DeleteObject(hfont);
  return TRUE;
}
#endif

#pragma argsused
static cell AMX_NATIVE_CALL _iswin32(AMX *amx,cell *params)
{
  #if defined __WIN32__ || defined __NT__ || defined _WIN32
    return 1;
  #else
    return 0;
  #endif
}


/*  push()
**
**  This function is the kind of programming trick that you don't even dare to
**  dream about! With the usual C calling convention, the caller cleans up the
**  stack after calling the function. This allows C functions to be flexible
**  with parameters, both in number and in type.
**  With the Pascal calling convention, used here, the callee (the function)
**  cleans up the stack. But here, function push() doesn't know about any
**  parameters. We neither declare any, nor indicate that the function has no
**  parameters (i.e. the function is not declared having 'void' parameters).
**  When we call function push(), the caller thinks the function cleans up the
**  stack (because of the Pascal calling convention), while the function does
**  not know that it has parameters, so it cannot clean them. As a result,
**  nobody cleans up the stack. Ergo, The parameter you pass to function push()
**  stays on the stack.
*/
static void PASCAL push() { }

FARPROC SearchProcAddress(char *dllname,char *functionname)
{
  HINSTANCE hinst;
  FARPROC lpfn;

  hinst=GetModuleHandle(dllname);
  if (hinst==NULL && strlen(dllname)<128-2) {
    char str[128];
    strcpy(str,dllname);
    #if defined __WIN32__ || defined __NT__ || defined _WIN32
      strcat(str,"32");
    #else
      strcat(str,"16");
    #endif
    hinst=GetModuleHandle(str);
  } /* if */
  if (hinst==NULL)
    return NULL;
  lpfn=GetProcAddress(hinst,functionname);
  #if defined __WIN32__
    if (lpfn==NULL && strlen(functionname)<128-1) {
      char str[128];
      strcpy(str,functionname);
      strcat(str,"A");
      lpfn=GetProcAddress(hinst,str);
    } /* if */
  #endif
  return lpfn;
}

#define MAXPARAMS 32
typedef int (CALLBACK* DLLPROC)();

static cell AMX_NATIVE_CALL CallDLL(AMX *amx,cell *params)
{
  char dllname[128], functionname[128], typestring[MAXPARAMS];
  cell *cptr,result;
  int len,i,count,j;
  int numparams,paramidx;
  DWORD dllparms[MAXPARAMS];
  DLLPROC DLLproc;

  amx_GetAddr(amx,params[1],&cptr);
  amx_StrLen(cptr,&len);
  if (len>=sizeof dllname)
    return amx_RaiseError(amx, AMX_ERR_NATIVE);
  amx_GetString(dllname,cptr);

  amx_GetAddr(amx,params[2],&cptr);
  amx_StrLen(cptr,&len);
  if (len>=sizeof functionname)
    return amx_RaiseError(amx, AMX_ERR_NATIVE);
  amx_GetString(functionname,cptr);

  amx_GetAddr(amx,params[3],&cptr);
  amx_StrLen(cptr,&len);
  if (len>=sizeof typestring)
    return amx_RaiseError(amx, AMX_ERR_NATIVE);
  amx_GetString(typestring,cptr);

  /* find the function */
  DLLproc=(DLLPROC)SearchProcAddress(dllname,functionname);
  if (DLLproc==NULL)
    return 0;

  /* decode the parameters */
  numparams=0;
  paramidx=0;
  while (typestring[paramidx]!='\0') {
    amx_GetAddr(amx,params[numparams+4],&cptr);
    switch (typestring[paramidx]) {
    case 'w':
      dllparms[paramidx]=(WORD)*cptr;
      break;
    case 'i':
    case 'h':
      dllparms[paramidx]=(int)*cptr;
      break;
    case 'l':
      dllparms[paramidx]=(long)*cptr;
      break;
    case 'I':
    case 'H':
    case 'L':
    case 'W':
      dllparms[paramidx]=(DWORD)cptr;
      break;
    case 'p':
    case 'P':
    case 's':
    case 'S':
      amx_StrLen(cptr,&len);
      dllparms[paramidx]=(DWORD)(LPSTR)malloc(len+1);
      if (dllparms[paramidx]==0L)
        return amx_RaiseError(amx, AMX_ERR_NATIVE);
      amx_GetString((char *)dllparms[paramidx],cptr);
      break;
    default:
      return amx_RaiseError(amx, AMX_ERR_NATIVE);
    } /* switch */
    numparams++;
    paramidx++;
    /* skip digits */
    while (isdigit(typestring[paramidx]))
      paramidx++;
  } /* while */
  if ((params[0]/sizeof(cell)) - 3 != numparams)
    return amx_RaiseError(amx, AMX_ERR_NATIVE); /* format string does not match number of parameters */

  /* push the parameters to the stack (left-to-right in 16-bit; right-to-left
   * in 32-bit)
   */
  #if defined __WIN32__
    for (i=numparams-1; i>=0; i--) {
      /* push always 32-bits */
      push(dllparms[i]);
    } /* for */
  #else
    for (i=paramidx=0; i<numparams; i++) {
      switch (typestring[paramidx]) {
      case 'i':
      case 'h':
      case 'w':
        push((int)dllparms[i]);
        break;
      default:
        push(dllparms[i]);
        break;
      } /* switch */
      /* skip digits */
      paramidx++;
      while (isdigit(typestring[paramidx]))
        paramidx++;
    } /* for */
  #endif

  /* call the function; all parameters are already pushed to the stack (the
   * function should remove the parameters from the stack)
   */
  result=DLLproc();

  /* free allocated memory */
  for (i=paramidx=0; i<numparams; i++) {
    switch (typestring[paramidx]) {
    case 'p':
    case 's':
      free((char *)dllparms[i]);
      break;
    case 'P':
    case 'S':
      amx_GetAddr(amx,params[i+4],&cptr);
      amx_SetString(cptr,(char *)dllparms[i],typestring[paramidx]=='P');
      free((char *)dllparms[i]);
      break;
#if !defined __WIN32__
    case 'I':
      count=isdigit(typestring[paramidx+1]) ? atoi(&typestring[paramidx+1]) : 1;
      amx_GetAddr(amx,params[i+4],&cptr);
      for (j=count-1; j>=0; j--) {
        if ((j & 1)==0)
          cptr[j] = cptr[j/2];
        else
          cptr[j] = cptr[j/2] >> 16;
        cptr[j] = (long)((short)cptr[j]); /* sign-extent 16-bit integers */
      } /* for */
      break;
    case 'H':
      count=isdigit(typestring[paramidx+1]) ? atoi(&typestring[paramidx+1]) : 1;
      amx_GetAddr(amx,params[i+4],&cptr);
      for (j=count-1; j>=0; j--) {
        if ((j & 1)==0)
          cptr[j] = cptr[j/2];
        else
          cptr[j] = cptr[j/2] >> 16;
        cptr[j] &= 0x0000ffffL;           /* clear high bits of 16-bit words */
      } /* for */
      break;
#endif
    case 'W':
      count=isdigit(typestring[paramidx+1]) ? atoi(&typestring[paramidx+1]) : 1;
      amx_GetAddr(amx,params[i+4],&cptr);
      for (j=count-1; j>=0; j--) {
        if ((j & 1)==0)
          cptr[j] = cptr[j/2];
        else
          cptr[j] = cptr[j/2] >> 16;
        cptr[j] &= 0x0000ffffL;           /* clear high bits of 16-bit words */
      } /* for */
      break;
    } /* switch */
    /* skip digits */
    paramidx++;
    while (isdigit(typestring[paramidx]))
      paramidx++;
  } /* for */

  return result;
}

static HINSTANCE int_LoadLibrary(char *dllname)
{
  HINSTANCE hinstDLL;
  hinstDLL = LoadLibrary(dllname);
  #if !defined __WIN32__
    if ((WORD)hinstDLL<=32)
      hinstDLL=0;
  #endif
  return hinstDLL;
}

static cell AMX_NATIVE_CALL LoadDLL(AMX *amx,cell *params)
{
  char dllname[128];
  cell *cptr;
  int len;
  HINSTANCE hinstDLL;

  amx_GetAddr(amx,params[1],&cptr);
  amx_StrLen(cptr,&len);
  if (len>=sizeof dllname)
    return amx_RaiseError(amx, AMX_ERR_NATIVE);
  amx_GetString(dllname,cptr);

  hinstDLL = int_LoadLibrary(dllname);
  if (hinstDLL==0) {
    #if defined __WIN32__ || defined __NT__ || defined _WIN32
      strcat(dllname,"32");
    #else
      strcat(dllname,"16");
    #endif
    hinstDLL = int_LoadLibrary(dllname);
  } /* if */

  return hinstDLL!=0;
}

static cell AMX_NATIVE_CALL FreeDLL(AMX *amx,cell *params)
{
  char dllname[128];
  cell *cptr;
  int len;
  HINSTANCE hinstDLL;

  amx_GetAddr(amx,params[1],&cptr);
  amx_StrLen(cptr,&len);
  if (len>=sizeof dllname)
    return amx_RaiseError(amx, AMX_ERR_NATIVE);
  amx_GetString(dllname,cptr);

  hinstDLL=GetModuleHandle(dllname);
  if (hinstDLL==NULL) {
    #if defined __WIN32__ || defined __NT__ || defined _WIN32
      strcat(dllname,"32");
    #else
      strcat(dllname,"16");
    #endif
  } /* if */
  if (hinstDLL==NULL)
    return FALSE;

  #if defined __WIN32__
    return FreeLibrary(hinstDLL);
  #else
    FreeLibrary(hinstDLL);
    return TRUE;
  #endif
}

#endif // _Windows

#if defined(__WIN32__)

  #if defined(__WATCOMC__)
    #define DllMain(h,dw,lp)    LibMain(h,dw,lp)
  #endif

#pragma argsused
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpRes)
{
  switch (dwReason) {
  case DLL_PROCESS_ATTACH:
    hinstAMX=hinstDLL;
    InitWindowClass();  /* console window class */
    break;
  case DLL_PROCESS_DETACH:
    UnregisterClass("AMX_console",hinstAMX);
    break;
  } /* switch */
  return TRUE;
}

#else

#pragma argsused
int FAR PASCAL LibMain(HINSTANCE hinstDLL, WORD wDataSeg, WORD wHeapSize, LPSTR lpszCmdLine)
{
  hinstAMX=hinstDLL;
  InitWindowClass();  /* "AMX_console" class */
  return(1);
}

#pragma argsused
int FAR PASCAL _export WEP(int param)
{
  UnregisterClass("AMX_console",hinstAMX);
  return(1);
}

#endif

