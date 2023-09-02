# This makefile is for Opus MAKE
# Build with:
#       make -famx32
#
# For release version, use
#       make -a -famx32 NDEBUG=

DEFGEN = f:\tools\defgen\defgen
LIBASE = f:\tools\libase\libase

!ifdef NDEBUG
  # retail version
  C_DEBUG=-dNDEBUG
  L_DEBUG=d all op symf
!else
  # "work in progress"
  C_DEBUG=-hc
  L_DEBUG=d codeview op cvpack
!endif

INCLUDE=..

C_OPTS=-w4 -e25 $(C_DEBUG) -ei -zq -of+s -d2 -bd -bm -5s -bt=nt -mf -damx_Init=amx_InitAMX -dAMXAPI=__stdcall -dAMX_NATIVE_CALL=__stdcall
L_OPTS=$(L_DEBUG) SYS win95 dll op m exp =amx32 op maxe=25 op q

%.obj : %.c
    wcc386 $(.SOURCE) -i=$(INCLUDE) $(C_OPTS)

project : amx32.dll lib\amx32m.lib lib\amx32b.lib

amxdll.obj : amxdll.c balloon.h

amx.obj : ..\amx.c
  wcc386 $(.SOURCE) -i=$(INCLUDE) $(C_OPTS) -dASM32 -dSTACKARGS

amxcore.obj : ..\amxcore.c

amxcons.obj : ..\amxcons.c
  wcc386 $(.SOURCE) -i=$(INCLUDE) $(C_OPTS) -dAMX_TERMINAL

amxexec.obj : ..\amxexec.asm
  wasm $(.SOURCE) -i=$(INCLUDE) -d1 -mf -3s -w4 -zq -dSTDECL -dSTACKARGS

fixed.obj : ..\fixed.c

sdbg.obj : ..\sdbg.c

balloon.obj : balloon.c balloon.h

##### Linker files and LIB files #####

amx32.def + amx32.lbc + amx32.imp : amxdll.fed
    $(DEFGEN) -we=$(.TARGET,2) -g -win32 -o$(.TARGET,1) $(.SOURCE)
    $(DEFGEN) -wi=$(.TARGET,3) -s -win32 -oNUL $(.SOURCE)

lib\amx32m.lib + amx32m.def : amxdll.fed
  $(DEFGEN) -g -win32 -o$(.TARGET,2) $(.SOURCE)
  f:\tools\lib32\lib /machine:ix86 /def:$(.TARGET,2) /out:$(.TARGET,1)

lib\amx32b.lib : amx32.dll
  f:\tools\implib5\implib $(.TARGET) amx32.dll

amx32.dll : amxdll.obj amx.obj amxcore.obj amxcons.obj amxexec.obj fixed.obj \
            sdbg.obj balloon.obj amxdll.rc amx32.def
  wlink $(L_OPTS) @<<
    NAME $(.TARGET)
    FIL amxdll,amx,amxcore,amxcons,amxexec,fixed,sdbg,balloon
    lib user32.lib,gdi32.lib,kernel32.lib
  <<
  wrc -bt=nt -dWIN32 -i=$(INCLUDE) -q amxdll.rc $(.TARGET)
  $(LIBASE) $(.TARGET)
  wlib -n -b -ii -fo -io lib\amx32w.lib @amx32.imp
!ifdef NDEBUG
    upx $(.TARGET)
!endif

