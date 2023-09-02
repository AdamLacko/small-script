/*  Small compiler - Peephole optimizer "sequences" strings (plain
 *                   and compressed formats)
 *
 *  Copyright (c) ITB CompuPhase, 2000-2003
 *  This file may be freely used. No warranties of any kind.
 *
 *  Version: $Id: sc7.sch 2355 2003-10-16 11:57:44Z thiadmer $
 */

SC_FUNC int strexpand(char *dest, unsigned char *source, int maxlen, unsigned char pairtable[128][2]);

#define SCPACK_TERMINATOR ,     /* end each section with a comma */

#define SCPACK_TABLE sequences_table
/*-*SCPACK start of pair table, do not change or remove this line */
unsigned char sequences_table[][2] = {
  {32,37}, {114,105}, {112,129}, {46,130}, {49,33}, {128,132}, {97,100}, {111,134}, {108,135}, {46,97}, {137,108}, {138,116}, {115,104}, {131,133}, {131,33}, {136,46},
  {117,140}, {112,144}, {139,33}, {50,33}, {131,128}, {145,142}, {115,116}, {143,115}, {112,146}, {110,150}, {111,153}, {99,154}, {148,147}, {134,100}, {111,152}, {112,158},
  {141,149}, {151,156}, {59,33}, {94,162}, {59,163}, {157,114}, {139,133}, {51,33}, {122,101}, {110,100}, {101,113}, {168,114}, {155,160}, {46,99}, {128,167}, {166,161},
  {157,33}, {108,173}, {105,33}, {117,169}, {115,174}, {159,176}, {111,179}, {140,177}, {182,180}, {98,184}, {105,100}, {186,120}, {131,32}, {143,178}, {170,33}, {171,111},
  {133,164}, {46,115}, {165,160}, {118,101}, {111,195}, {109,196}, {115,103}, {115,108}, {171,133}, {106,200}, {170,133}, {191,142}, {188,147}, {194,161}, {172,161}, {187,165},
  {155,175}, {165,175}, {204,181}, {108,187}, {183,210}, {46,98}, {52,33}, {172,172}, {136,141}, {151,141}, {183,148}, {136,156}, {33,159}, {103,33}, {33,164}, {97,169},
  {99,193}, {104,221}, {99,225}, {120,226}, {100,101}, {105,110}, {101,115}, {230,115}, {116,114}, {114,232}, {155,141}, {197,146}, {155,156}, {136,160}, {151,160}, {99,192},
  {224,192}, {190,201}, {199,231}, {198,233}, {106,110}, {197,142}, {245,149}, {48,33}, {207,33}, {211,33}, {155,166}, {205,185}, {206,185}, {208,185}, {209,185}
};
/*-*SCPACK end of pair table, do not change or remove this line */

#define seqsize(o,p)    (opcodes(o)+opargs(p))
typedef struct {
  char *find;
  char *replace;
  int savesize;         /* number of bytes saved (in bytecode) */
} SEQUENCE;
static SEQUENCE sequences_cmp[] = {
  /* A very common sequence in four varieties
   *    load.s.pri n1           load.s.pri n2
   *    push.pri                load.s.alt n1
   *    load.s.pri n2           -
   *    pop.alt                 -
   *    --------------------------------------
   *    load.pri n1             load.s.pri n2
   *    push.pri                load.alt n1
   *    load.s.pri n2           -
   *    pop.alt                 -
   *    --------------------------------------
   *    load.s.pri n1           load.pri n2
   *    push.pri                load.s.alt n1
   *    load.pri n2             -
   *    pop.alt                 -
   *    --------------------------------------
   *    load.pri n1             load.pri n2
   *    push.pri                load.alt n1
   *    load.pri n2             -
   *    pop.alt                 -
   */
  {
    #ifdef SCPACK
      "load.s.pri %1!push.pri!load.s.pri %2!pop.alt!",
      "load.s.pri %2!load.s.alt %1!",
    #else
      "\356\241\237",
      "\227\257",
    #endif
    seqsize(4,2) - seqsize(2,2)
  },
  {
    #ifdef SCPACK
      "load.pri %1!push.pri!load.s.pri %2!pop.alt!",
      "load.s.pri %2!load.alt %1!",
    #else
      "\355\241\237",
      "\210\257",
    #endif
    seqsize(4,2) - seqsize(2,2)
  },
  {
    #ifdef SCPACK
      "load.s.pri %1!push.pri!load.pri %2!pop.alt!",
      "load.pri %2!load.s.alt %1!",
    #else
      "\356\333\237",
      "\227\246\333",
    #endif
    seqsize(4,2) - seqsize(2,2)
  },
  {
    #ifdef SCPACK
      "load.pri %1!push.pri!load.pri %2!pop.alt!",
      "load.pri %2!load.alt %1!",
    #else
      "\355\333\237",
      "\210\246\333",
    #endif
    seqsize(4,2) - seqsize(2,2)
  },
  /* (#1#) The above also occurs with "addr.pri" (array
   * indexing) as the first line; so that adds 2 cases.
   */
  {
    #ifdef SCPACK
      "addr.pri %1!push.pri!load.s.pri %2!pop.alt!",
      "addr.alt %1!load.s.pri %2!",
    #else
      "\315\237",
      "\321",
    #endif
    seqsize(4,2) - seqsize(2,2)
  },
  {
    #ifdef SCPACK
      "addr.pri %1!push.pri!load.pri %2!pop.alt!",
      "addr.alt %1!load.pri %2!",
    #else
      "\302\333\237",
      "\245\246\333",
    #endif
    seqsize(4,2) - seqsize(2,2)
  },
  /* And the same sequence with const.pri as either the first
   * or the second load instruction: four more cases.
   */
  {
    #ifdef SCPACK
      "const.pri %1!push.pri!load.s.pri %2!pop.alt!",
      "load.s.pri %2!const.alt %1!",
    #else
      "\316\237",
      "\320",
    #endif
    seqsize(4,2) - seqsize(2,2)
  },
  {
    #ifdef SCPACK
      "const.pri %1!push.pri!load.pri %2!pop.alt!",
      "load.pri %2!const.alt %1!",
    #else
      "\254\333\237",
      "\372\333",
    #endif
    seqsize(4,2) - seqsize(2,2)
  },
  {
    #ifdef SCPACK
      "load.s.pri %1!push.pri!const.pri %2!pop.alt!",
      "const.pri %2!load.s.alt %1!",
    #else
      "\356\354\237",
      "\227\246\354",
    #endif
    seqsize(4,2) - seqsize(2,2)
  },
  {
    #ifdef SCPACK
      "load.pri %1!push.pri!const.pri %2!pop.alt!",
      "const.pri %2!load.alt %1!",
    #else
      "\355\354\237",
      "\210\246\354",
    #endif
    seqsize(4,2) - seqsize(2,2)
  },
  /* The same as above, but now with "addr.pri" (array
   * indexing) on the first line and const.pri on
   * the second.
   */
  {
    #ifdef SCPACK
      "addr.pri %1!push.pri!const.pri %2!pop.alt!",
      "addr.alt %1!const.pri %2!",
    #else
      "\302\354\237",
      "\245\246\354",
    #endif
    seqsize(4,2) - seqsize(2,2)
  },
  /* ??? add references */
  /* Chained relational operators can contain sequences like:
   *    move.pri                load.s.pri n1
   *    push.pri                -
   *    load.s.pri n1           -
   *    pop.alt                 -
   * The above also accurs for "load.pri" and for "const.pri",
   * so add another two cases.
   */
  {
    #ifdef SCPACK
      "move.pri!push.pri!load.s.pri %1!pop.alt!",
      "load.s.pri %1!",
    #else
      "\366\331\237",
      "\331",
    #endif
    seqsize(4,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "move.pri!push.pri!load.pri %1!pop.alt!",
      "load.pri %1!",
    #else
      "\366\330\237",
      "\330",
    #endif
    seqsize(4,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "move.pri!push.pri!const.pri %1!pop.alt!",
      "const.pri %1!",
    #else
      "\366\352\237",
      "\352",
    #endif
    seqsize(4,1) - seqsize(1,1)
  },
  /* More optimizations for chained relational operators; the
   * continuation sequences can be simplified if they turn out
   * to be termination sequences:
   *    xchg                    sless       also for sless, sgeq and sleq
   *    sgrtr                   pop.alt
   *    swap.alt                and
   *    and                     ;$exp
   *    pop.alt                 -
   *    ;$exp                   -
   *    --------------------------------------
   *    xchg                    sless       also for sless, sgeq and sleq
   *    sgrtr                   pop.alt
   *    swap.alt                and
   *    and                     jzer n1
   *    pop.alt                 -
   *    jzer n1                 -
   *    --------------------------------------
   *    xchg                    jsgeq  n1   also for sless, sgeq and sleq
   *    sgrtr                   ;$exp       (occurs for non-chained comparisons)
   *    jzer n1                 -
   *    ;$exp                   -
   *    --------------------------------------
   *    xchg                    sless       also for sless, sgeq and sleq
   *    sgrtr                   ;$exp       (occurs for non-chained comparisons)
   *    ;$exp                   -
   */
  {
    #ifdef SCPACK
      "xchg!sgrtr!swap.alt!and!pop.alt!;$exp!",
      "sless!pop.alt!and!;$exp!",
    #else
      "\343\363!swa\230\337\334\244",
      "\362\334\337\336",
    #endif
    seqsize(5,0) - seqsize(3,0)
  },
  {
    #ifdef SCPACK
      "xchg!sless!swap.alt!and!pop.alt!;$exp!",
      "sgrtr!pop.alt!and!;$exp!",
    #else
      "\343\362!swa\230\337\334\244",
      "\363\334\337\336",
    #endif
    seqsize(5,0) - seqsize(3,0)
  },
  {
    #ifdef SCPACK
      "xchg!sgeq!swap.alt!and!pop.alt!;$exp!",
      "sleq!pop.alt!and!;$exp!",
    #else
      "\343\306\276swa\230\337\334\244",
      "\307\276\237\337\336",
    #endif
    seqsize(5,0) - seqsize(3,0)
  },
  {
    #ifdef SCPACK
      "xchg!sleq!swap.alt!and!pop.alt!;$exp!",
      "sgeq!pop.alt!and!;$exp!",
    #else
      "\343\307\276swa\230\337\334\244",
      "\306\276\237\337\336",
    #endif
    seqsize(5,0) - seqsize(3,0)
  },
  {
    #ifdef SCPACK
      "xchg!sgrtr!swap.alt!and!pop.alt!jzer %1!",
      "sless!pop.alt!and!jzer %1!",
    #else
      "\343\363!swa\230\337\334\244",
      "\362\334\337\336",
    #endif
    seqsize(5,0) - seqsize(3,0)
  },
  {
    #ifdef SCPACK
      "xchg!sless!swap.alt!and!pop.alt!jzer %1!",
      "sgrtr!pop.alt!and!jzer %1!",
    #else
      "\343\362!swa\230\337\334\244",
      "\363\334\337\336",
    #endif
    seqsize(5,0) - seqsize(3,0)
  },
  {
    #ifdef SCPACK
      "xchg!sgeq!swap.alt!and!pop.alt!jzer %1!",
      "sleq!pop.alt!and!jzer %1!",
    #else
      "\343\306\276swa\230\337\334\244",
      "\307\276\237\337\336",
    #endif
    seqsize(5,0) - seqsize(3,0)
  },
  {
    #ifdef SCPACK
      "xchg!sleq!swap.alt!and!pop.alt!jzer %1!",
      "sgeq!pop.alt!and!jzer %1!",
    #else
      "\343\307\276swa\230\337\334\244",
      "\306\276\237\337\336",
    #endif
    seqsize(5,0) - seqsize(3,0)
  },
  {
    #ifdef SCPACK
      "xchg!sgrtr!jzer %1!;$exp!",
      "jsgeq %1!;$exp!",
    #else
      "\343\363\336",
      "\362\336",
    #endif
    seqsize(3,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "xchg!sless!jzer %1!;$exp!",
      "jsleq %1!;$exp!",
    #else
      "\343\362\336",
      "\363\336",
    #endif
    seqsize(3,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "xchg!sgeq!jzer %1!;$exp!",
      "jsgrtr %1!;$exp!",
    #else
      "\343\306\276\244",
      "\307\276\244",
    #endif
    seqsize(3,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "xchg!sleq!jzer %1!;$exp!",
      "jsless %1!;$exp!",
    #else
      "\343\307\276\244",
      "\306\276\244",
    #endif
    seqsize(3,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "xchg!sgrtr!;$exp!",
      "sless!;$exp!",
    #else
      "\343\363\336",
      "\362\336",
    #endif
    seqsize(2,0) - seqsize(1,0)
  },
  {
    #ifdef SCPACK
      "xchg!sless!;$exp!",
      "sgrtr!;$exp!",
    #else
      "\343\362\336",
      "\363\336",
    #endif
    seqsize(2,0) - seqsize(1,0)
  },
  {
    #ifdef SCPACK
      "xchg!sgeq!;$exp!",
      "sleq!;$exp!",
    #else
      "\343\306\276\244",
      "\307\276\244",
    #endif
    seqsize(2,0) - seqsize(1,0)
  },
  {
    #ifdef SCPACK
      "xchg!sleq!;$exp!",
      "sgeq!;$exp!",
    #else
      "\343\307\276\244",
      "\306\276\244",
    #endif
    seqsize(2,0) - seqsize(1,0)
  },
  /* The entry to chained operators is also opt to optimization
   *    load.s.pri n1           load.s.pri n2
   *    load.s.alt n2           load.s.alt n1
   *    xchg                    -
   *    --------------------------------------
   *    load.s.pri n1           load.pri n2
   *    load.alt n2             load.s.alt n1
   *    xchg                    -
   *    --------------------------------------
   *    load.s.pri n1           const.pri n2
   *    const.alt n2            load.s.alt n1
   *    xchg                    -
   *    --------------------------------------
   * and all permutations...
   */
  {
    #ifdef SCPACK
      "load.s.pri %1!load.s.alt %2!xchg!",
      "load.s.pri %2!load.s.alt %1!",
    #else
      "\343\307\276\244",
      "\306\276\244",
    #endif
    seqsize(3,2) - seqsize(2,2)
  },
  {
    #ifdef SCPACK
      "load.s.pri %1!load.alt %2!xchg!",
      "load.pri %2!load.s.alt %1!",
    #else
      "\343\307\276\244",
      "\306\276\244",
    #endif
    seqsize(3,2) - seqsize(2,2)
  },
  {
    #ifdef SCPACK
      "load.s.pri %1!const.alt %2!xchg!",
      "const.pri %2!load.s.alt %1!",
    #else
      "\343\307\276\244",
      "\306\276\244",
    #endif
    seqsize(3,2) - seqsize(2,2)
  },
  {
    #ifdef SCPACK
      "load.pri %1!load.s.alt %2!xchg!",
      "load.s.pri %2!load.alt %1!",
    #else
      "\343\307\276\244",
      "\306\276\244",
    #endif
    seqsize(3,2) - seqsize(2,2)
  },
  {
    #ifdef SCPACK
      "load.pri %1!load.alt %2!xchg!",
      "load.pri %2!load.alt %1!",
    #else
      "\343\307\276\244",
      "\306\276\244",
    #endif
    seqsize(3,2) - seqsize(2,2)
  },
  {
    #ifdef SCPACK
      "load.pri %1!const.alt %2!xchg!",
      "const.pri %2!load.alt %1!",
    #else
      "\343\307\276\244",
      "\306\276\244",
    #endif
    seqsize(3,2) - seqsize(2,2)
  },
  {
    #ifdef SCPACK
      "const.pri %1!load.s.alt %2!xchg!",
      "load.s.pri %2!const.alt %1!",
    #else
      "\343\307\276\244",
      "\306\276\244",
    #endif
    seqsize(3,2) - seqsize(2,2)
  },
  {
    #ifdef SCPACK
      "const.pri %1!load.alt %2!xchg!",
      "load.pri %2!const.alt %1!",
    #else
      "\343\307\276\244",
      "\306\276\244",
    #endif
    seqsize(3,2) - seqsize(2,2)
  },
  /* Array indexing can merit from special instructions.
   * Simple indexed array lookup can be optimized quite
   * a bit.
   *    addr.pri n1             addr.alt n1
   *    push.pri                load.s.pri n2
   *    load.s.pri n2           bounds n3
   *    bounds n3               lidx.b n4
   *    shl.c.pri n4            -
   *    pop.alt                 -
   *    add                     -
   *    load.i                  -
   *
   * And to prepare for storing a value in an array
   *    addr.pri n1             addr.alt n1
   *    push.pri                load.s.pri n2
   *    load.s.pri n2           bounds n3
   *    bounds n3               idxaddr.b n4
   *    shl.c.pri n4            -
   *    pop.alt                 -
   *    add                     -
   *
   * Notes (additional cases):
   * 1. instruction addr.pri can also be const.pri (for
   *    global arrays)
   * 2. the bounds instruction can be absent
   * 3. when "n4" (the shift value) is the 2 (with 32-bit cels), use the
   *    even more optimal instructions LIDX and IDDXADDR
   *
   * If the array index is more complex, one can only optimize
   * the last four instructions:
   *    shl.c.pri n1            pop.alt
   *    pop.alt                 lidx.b n1
   *    add                     -
   *    loadi                   -
   *    --------------------------------------
   *    shl.c.pri n1            pop.alt
   *    pop.alt                 idxaddr.b n1
   *    add                     -
   */
#if !defined BIT16
  /* loading from array, "cell" shifted */
  {
    #ifdef SCPACK
      "addr.pri %1!push.pri!load.s.pri %2!bounds %3!shl.c.pri 2!pop.alt!add!load.i!",
      "addr.alt %1!load.s.pri %2!bounds %3!lidx!",
    #else
      "\373\324\275",
      "\376\371",
    #endif
    seqsize(8,4) - seqsize(4,3)
  },
  {
    #ifdef SCPACK
      "const.pri %1!push.pri!load.s.pri %2!bounds %3!shl.c.pri 2!pop.alt!add!load.i!",
      "const.alt %1!load.s.pri %2!bounds %3!lidx!",
    #else
      "\374\324\275",
      "\375\371",
    #endif
    seqsize(8,4) - seqsize(4,3)
  },
  {
    #ifdef SCPACK
      "addr.pri %1!push.pri!load.s.pri %2!shl.c.pri 2!pop.alt!add!load.i!",
      "addr.alt %1!load.s.pri %2!lidx!",
    #else
      "\315\324\275",
      "\321\371",
    #endif
    seqsize(7,3) - seqsize(3,2)
  },
  {
    #ifdef SCPACK
      "const.pri %1!push.pri!load.s.pri %2!shl.c.pri 2!pop.alt!add!load.i!",
      "const.alt %1!load.s.pri %2!lidx!",
    #else
      "\316\324\275",
      "\320\371",
    #endif
    seqsize(7,3) - seqsize(3,2)
  },
#endif
  /* loading from array, not "cell" shifted */
  {
    #ifdef SCPACK
      "addr.pri %1!push.pri!load.s.pri %2!bounds %3!shl.c.pri %4!pop.alt!add!load.i!",
      "addr.alt %1!load.s.pri %2!bounds %3!lidx.b %4!",
    #else
      "\373\332\326\265\275",
      "\376\323\325\200\326",
    #endif
    seqsize(8,4) - seqsize(4,4)
  },
  {
    #ifdef SCPACK
      "const.pri %1!push.pri!load.s.pri %2!bounds %3!shl.c.pri %4!pop.alt!add!load.i!",
      "const.alt %1!load.s.pri %2!bounds %3!lidx.b %4!",
    #else
      "\374\332\326\265\275",
      "\375\323\325\200\326",
    #endif
    seqsize(8,4) - seqsize(4,4)
  },
  {
    #ifdef SCPACK
      "addr.pri %1!push.pri!load.s.pri %2!shl.c.pri %3!pop.alt!add!load.i!",
      "addr.alt %1!load.s.pri %2!lidx.b %3!",
    #else
      "\315\332\247\265\275",
      "\321\323\325\256",
    #endif
    seqsize(7,3) - seqsize(3,3)
  },
  {
    #ifdef SCPACK
      "const.pri %1!push.pri!load.s.pri %2!shl.c.pri %3!pop.alt!add!load.i!",
      "const.alt %1!load.s.pri %2!lidx.b %3!",
    #else
      "\316\332\247\265\275",
      "\320\323\325\256",
    #endif
    seqsize(7,3) - seqsize(3,3)
  },
#if !defined BIT16
  /* array index calculation for storing a value, "cell" aligned */
  {
    #ifdef SCPACK
      "addr.pri %1!push.pri!load.s.pri %2!bounds %3!shl.c.pri 2!pop.alt!add!",
      "addr.alt %1!load.s.pri %2!bounds %3!idxaddr!",
    #else
      "\373\324",
      "\376\370",
    #endif
    seqsize(7,4) - seqsize(4,3)
  },
  {
    #ifdef SCPACK
      "const.pri %1!push.pri!load.s.pri %2!bounds %3!shl.c.pri 2!pop.alt!add!",
      "const.alt %1!load.s.pri %2!bounds %3!idxaddr!",
    #else
      "\374\324",
      "\375\370",
    #endif
    seqsize(7,4) - seqsize(4,3)
  },
  {
    #ifdef SCPACK
      "addr.pri %1!push.pri!load.s.pri %2!shl.c.pri 2!pop.alt!add!",
      "addr.alt %1!load.s.pri %2!idxaddr!",
    #else
      "\315\324",
      "\321\370",
    #endif
    seqsize(6,3) - seqsize(3,2)
  },
  {
    #ifdef SCPACK
      "const.pri %1!push.pri!load.s.pri %2!shl.c.pri 2!pop.alt!add!",
      "const.alt %1!load.s.pri %2!idxaddr!",
    #else
      "\316\324",
      "\320\370",
    #endif
    seqsize(6,3) - seqsize(3,2)
  },
#endif
  /* array index calculation for storing a value, not "cell" packed */
  {
    #ifdef SCPACK
      "addr.pri %1!push.pri!load.s.pri %2!bounds %3!shl.c.pri %4!pop.alt!add!",
      "addr.alt %1!load.s.pri %2!bounds %3!idxaddr.b %4!",
    #else
      "\373\332\326\265",
      "\376\317\325\200\326",
    #endif
    seqsize(7,4) - seqsize(4,4)
  },
  {
    #ifdef SCPACK
      "const.pri %1!push.pri!load.s.pri %2!bounds %3!shl.c.pri %4!pop.alt!add!",
      "const.alt %1!load.s.pri %2!bounds %3!idxaddr.b %4!",
    #else
      "\374\332\326\265",
      "\375\317\325\200\326",
    #endif
    seqsize(7,4) - seqsize(4,4)
  },
  {
    #ifdef SCPACK
      "addr.pri %1!push.pri!load.s.pri %2!shl.c.pri %3!pop.alt!add!",
      "addr.alt %1!load.s.pri %2!idxaddr.b %3!",
    #else
      "\315\332\247\265",
      "\321\317\325\256",
    #endif
    seqsize(6,3) - seqsize(3,3)
  },
  {
    #ifdef SCPACK
      "const.pri %1!push.pri!load.s.pri %2!shl.c.pri %3!pop.alt!add!",
      "const.alt %1!load.s.pri %2!idxaddr.b %3!",
    #else
      "\316\332\247\265",
      "\320\317\325\256",
    #endif
    seqsize(6,3) - seqsize(3,3)
  },
#if !defined BIT16
  /* the shorter array indexing sequences, see above for comments */
  {
    #ifdef SCPACK
      "shl.c.pri 2!pop.alt!add!loadi!",
      "pop.alt!lidx!",
    #else
      "\324\210\262",
      "\237\371",
    #endif
    seqsize(4,1) - seqsize(2,0)
  },
  {
    #ifdef SCPACK
      "shl.c.pri 2!pop.alt!add!",
      "pop.alt!idxaddr!",
    #else
      "\324",
      "\237\370",
    #endif
    seqsize(3,1) - seqsize(2,0)
  },
#endif
  {
    #ifdef SCPACK
      "shl.c.pri %1!pop.alt!add!loadi!",
      "pop.alt!lidx.b %1!",
    #else
      "\267\215\265\210\262",
      "\237\323\325\205",
    #endif
    seqsize(4,1) - seqsize(2,1)
  },
  {
    #ifdef SCPACK
      "shl.c.pri %1!pop.alt!add!",
      "pop.alt!idxaddr.b %1!",
    #else
      "\267\215\265",
      "\237\317\325\205",
    #endif
    seqsize(3,1) - seqsize(2,1)
  },
  /* For packed arrays, there is another case (packed arrays
   * do not take advantage of the LIDX or IDXADDR instructions).
   *    addr.pri n1             addr.alt n1
   *    push.pri                load.s.pri n2
   *    load.s.pri n2           bounds n3
   *    bounds n3               -
   *    pop.alt                 -
   *
   * Notes (additional cases):
   * 1. instruction addr.pri can also be const.pri (for
   *    global arrays)
   * 2. the bounds instruction can be absent, but that
   *    case is already handled (see #1#)
   */
  {
    #ifdef SCPACK
      "addr.pri %1!push.pri!load.s.pri %2!bounds %3!pop.alt!",
      "addr.alt %1!load.s.pri %2!bounds %3!",
    #else
      "\373\237",
      "\376",
    #endif
    seqsize(5,3) - seqsize(3,3)
  },
  {
    #ifdef SCPACK
      "const.pri %1!push.pri!load.s.pri %2!bounds %3!pop.alt!",
      "const.alt %1!load.s.pri %2!bounds %3!",
    #else
      "\374\237",
      "\375",
    #endif
    seqsize(5,3) - seqsize(3,3)
  },
  /* During a calculation, the intermediate result must sometimes
   * be moved from PRI to ALT, like in:
   *    push.pri                move.alt
   *    load.s.pri n1           load.s.pri n1
   *    pop.alt                 -
   *
   * The above also accurs for "load.pri" and for "const.pri",
   * so add another two cases.
   */
  {
    #ifdef SCPACK
      "push.pri!load.s.pri %1!pop.alt!",
      "move.alt!load.s.pri %1!",
    #else
      "\225\331\237",
      "\353\331",
    #endif
    seqsize(3,1) - seqsize(2,1)
  },
  {
    #ifdef SCPACK
      "push.pri!load.pri %1!pop.alt!",
      "move.alt!load.pri %1!",
    #else
      "\225\330\237",
      "\353\330",
    #endif
    seqsize(3,1) - seqsize(2,1)
  },
  {
    #ifdef SCPACK
      "push.pri!const.pri %1!pop.alt!",
      "move.alt!const.pri %1!",
    #else
      "\225\352\237",
      "\353\352",
    #endif
    seqsize(3,1) - seqsize(2,1)
  },
  {
    #ifdef SCPACK
      "push.pri!zero.pri!pop.alt!",
      "move.alt!zero.pri!",
    #else
      "\225\313\237",
      "\353\313",
    #endif
    seqsize(3,0) - seqsize(2,0)
  },
  /* saving PRI and then loading from its address
   * occurs when indexing a multi-dimensional array
   */
  {
    #ifdef SCPACK
      "push.pri!load.i!pop.alt!",
      "move.alt!load.i!",
    #else
      "\225\275\237",
      "\353\275",
    #endif
    seqsize(3,0) - seqsize(2,0)
  },
  /* An even simpler PUSH/POP optimization (occurs in
   * switch statements):
   *    push.pri                move.alt
   *    pop.alt                 -
   */
  {
    #ifdef SCPACK
      "push.pri!pop.alt!",
      "move.alt!",
    #else
      "\225\237",
      "\353",
    #endif
    seqsize(2,0) - seqsize(1,0)
  },
  /* And what to think of this PUSH/POP sequence, which occurs
   * due to the support for user-defined assignment operator):
   *    push.alt                -
   *    pop.alt                 -
   */
//???
//{
//  #ifdef SCPACK
//    "push.alt!pop.alt!",
//    ";$",     /* SCPACK cannot handle empty strings */
//  #else
//    "\225\237",
//    "\353",
//  #endif
//  seqsize(2,0) - seqsize(0,0)
//},
  /* Functions with many parameters with the same default
   * value have sequences like:
   *    push.c n1               const.pri n1
   *    ;$par                   push.r.pri n2   ; where n2 is the number of pushes
   *    push.c n1               ;$par
   *    ;$par                   -
   *    push.c n1               -
   *    ;$par                   -
   *    etc.                    etc.
   * The shortest matched sequence is 3, because a sequence of two can also be
   * optimized as two "push.c n1" instructions.
   * => this optimization does not work, because the argument re-ordering in
   *    a function call causes each argument to be optimized individually
   */
//{
//  #ifdef SCPACK
//    "const.pri %1!push.pri!;$par!const.pri %1!push.pri!;$par!const.pri %1!push.pri!;$par!const.pri %1!push.pri!;$par!const.pri %1!push.pri!;$par!",
//    "const.pri %1!push.r.pri 5!;$par!",
//  #else
//    "\327\327\254",
//    "\352\221.r\2745!",
//  #endif
//  seqsize(10,5) - seqsize(2,2)
//},
//{
//  #ifdef SCPACK
//    "const.pri %1!push.pri!;$par!const.pri %1!push.pri!;$par!const.pri %1!push.pri!;$par!const.pri %1!push.pri!;$par!",
//    "const.pri %1!push.r.pri 4!;$par!",
//  #else
//    "\327\327",
//    "\352\221.r\274\326",
//  #endif
//  seqsize(8,4) - seqsize(2,2)
//},
//{
//  #ifdef SCPACK
//    "const.pri %1!push.pri!;$par!const.pri %1!push.pri!;$par!const.pri %1!push.pri!;$par!",
//    "const.pri %1!push.r.pri 3!;$par!",
//  #else
//    "\327\254",
//    "\352\221.r\274\247",
//  #endif
//  seqsize(6,3) - seqsize(2,2)
//},
  /* User-defined operators first load the operands into registers and
   * then have them pushed onto the stack. This can give rise to sequences
   * like:
   *    const.pri n1            push.c n1
   *    const.alt n2            push.c n2
   *    push.pri                -
   *    push.alt                -
   * A similar sequence occurs with the two PUSH.pri/alt instructions inverted.
   * The first, second, or both CONST.pri/alt instructions can also be
   * LOAD.pri/alt.
   * This gives 2 x 4 cases.
   */
  {
    #ifdef SCPACK
      "const.pri %1!const.alt %2!push.pri!push.alt!",
      "push.c %1!push.c %2!",
    #else
    #endif
    seqsize(4,2) - seqsize(2,2)
  },
  {
    #ifdef SCPACK
      "const.pri %1!const.alt %2!push.alt!push.pri!",
      "push.c %2!push.c %1!",
    #else
    #endif
    seqsize(4,2) - seqsize(2,2)
  },
  {
    #ifdef SCPACK
      "const.pri %1!load.alt %2!push.pri!push.alt!",
      "push.c %1!push %2!",
    #else
    #endif
    seqsize(4,2) - seqsize(2,2)
  },
  {
    #ifdef SCPACK
      "const.pri %1!load.alt %2!push.alt!push.pri!",
      "push %2!push.c %1!",
    #else
    #endif
    seqsize(4,2) - seqsize(2,2)
  },
  {
    #ifdef SCPACK
      "load.pri %1!const.alt %2!push.pri!push.alt!",
      "push %1!push.c %2!",
    #else
    #endif
    seqsize(4,2) - seqsize(2,2)
  },
  {
    #ifdef SCPACK
      "load.pri %1!const.alt %2!push.alt!push.pri!",
      "push.c %2!push %1!",
    #else
    #endif
    seqsize(4,2) - seqsize(2,2)
  },
  {
    #ifdef SCPACK
      "load.pri %1!load.alt %2!push.pri!push.alt!",
      "push %1!push %2!",
    #else
    #endif
    seqsize(4,2) - seqsize(2,2)
  },
  {
    #ifdef SCPACK
      "load.pri %1!load.alt %2!push.alt!push.pri!",
      "push %2!push %1!",
    #else
    #endif
    seqsize(4,2) - seqsize(2,2)
  },
  /* Function calls (parameters are passed on the stack)
   *    load.s.pri n1           push.s n1
   *    push.pri                -
   *    --------------------------------------
   *    load.pri n1             push n1
   *    push.pri                -
   *    --------------------------------------
   *    const.pri n1            push.c n1
   *    push.pri                -
   *    --------------------------------------
   *    zero.pri                push.c 0
   *    push.pri                -
   *    --------------------------------------
   *    addr.pri n1             pushaddr n1
   *    push.pri                -
   *
   * However, PRI must not be needed after this instruction
   * if this shortcut is used. Check for the ;$par comment.
   */
  {
    #ifdef SCPACK
      "load.s.pri %1!push.pri!;$par!",
      "push.s %1!;$par!",
    #else
      "\356",
      "\221\301\205",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "load.pri %1!push.pri!;$par!",
      "push %1!;$par!",
    #else
      "\355",
      "\221\205",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "const.pri %1!push.pri!;$par!",
      "push.c %1!;$par!",
    #else
      "\254",
      "\221\255\205",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "zero.pri!push.pri!;$par!",
      "push.c 0!;$par!",
    #else
      "\313\225",
      "\221\255 \367",
    #endif
    seqsize(2,0) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "addr.pri %1!push.pri!;$par!",
      "pushaddr %1!;$par!",
    #else
      "\254",
      "\221\255\205",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  /* References with a default value generate new cells on the heap
   * dynamically. That code often ends with:
   *    move.pri                push.alt
   *    push.pri                -
   */
  {
    #ifdef SCPACK
      "move.pri!push.pri!",
      "push.alt!",
    #else
      "\366",
      "\221\222",
    #endif
    seqsize(2,0) - seqsize(1,0)
  },
  /* Simple arithmetic operations on constants. Noteworthy is the
   * subtraction of a constant, since it is converted to the addition
   * of the inverse value.
   *    const.alt n1            add.c n1
   *    add                     -
   *    --------------------------------------
   *    const.alt n1            add.c -n1
   *    sub                     -
   *    --------------------------------------
   *    const.alt n1            smul.c n1
   *    smul                    -
   *    --------------------------------------
   *    const.alt n1            eq.c.pri n1
   *    eq                      -
   */
  {
    #ifdef SCPACK
      "const.alt %1!add!",
      "add.c %1!",
    #else
      "\372\260",
      "\235\255\205",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "const.alt %1!sub!",
      "add.c -%1!",
    #else
      "\372sub!",
      "\235\255 -%\204",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "const.alt %1!smul!",
      "smul.c %1!",
    #else
      "\372smul!",
      "smu\261\205",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "const.alt %1!eq!",
      "eq.c.pri %1!",
    #else
      "\372\276",
      "\252\255\215",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  /* Some operations use the alternative subtraction operation --these
   * can also be optimized.
   *    const.pri n1            load.s.pri n2
   *    load.s.alt n2           add.c -n1
   *    sub.alt                 -
   *    --------------------------------------
   *    const.pri n1            load.pri n2
   *    load.alt n2             add.c -n1
   *    sub.alt                 -
   */
  {
    #ifdef SCPACK
      "const.pri %1!load.s.alt %2!sub.alt!",
      "load.s.pri %2!add.c -%1!",
    #else
      "\372\260",
      "\235\255\205",
    #endif
    seqsize(3,2) - seqsize(2,2)
  },
  {
    #ifdef SCPACK
      "const.pri %1!load.alt %2!sub.alt!",
      "load.pri %2!add.c -%1!",
    #else
      "\372\260",
      "\235\255\205",
    #endif
    seqsize(3,2) - seqsize(2,2)
  },
  /* Compare and jump
   *    eq                      jneq n1
   *    jzer n1                 -
   *    --------------------------------------
   *    eq                      jeq n1
   *    jnz n1                  -
   *    --------------------------------------
   *    neq                     jeq n1
   *    jzer n1                 -
   *    --------------------------------------
   *    neq                     jneq n1
   *    jnz n1                  -
   * Compares followed by jzer occur much more
   * often than compares followed with jnz. So we
   * take the easy route here.
   *    less                    jgeq n1
   *    jzer n1                 -
   *    --------------------------------------
   *    leq                     jgrtr n1
   *    jzer n1                 -
   *    --------------------------------------
   *    grtr                    jleq n1
   *    jzer n1                 -
   *    --------------------------------------
   *    geq                     jless n1
   *    jzer n1                 -
   *    --------------------------------------
   *    sless                   jsgeq n1
   *    jzer n1                 -
   *    --------------------------------------
   *    sleq                    jsgrtr n1
   *    jzer n1                 -
   *    --------------------------------------
   *    sgrtr                   jsleq n1
   *    jzer n1                 -
   *    --------------------------------------
   *    sgeq                    jsless n1
   *    jzer n1                 -
   */
  {
    #ifdef SCPACK
      "eq!jzer %1!",
      "jneq %1!",
    #else
      "\361",
      "\364\312",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "eq!jnz %1!",
      "jeq %1!",
    #else
      "\276\364z\205",
      "j\312",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "neq!jzer %1!",
      "jeq %1!",
    #else
      "n\361",
      "j\312",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "neq!jnz %1!",
      "jneq %1!",
    #else
      "n\276\364z\205",
      "\364\312",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "less!jzer %1!",
      "jgeq %1!",
    #else
      "l\347!\311",
      "jg\312",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "leq!jzer %1!",
      "jgrtr %1!",
    #else
      "l\361",
      "jg\351\205",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "grtr!jzer %1!",
      "jleq %1!",
    #else
      "g\351!\311",
      "jl\312",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "geq!jzer %1!",
      "jless %1!",
    #else
      "g\361",
      "jl\347\205",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "sless!jzer %1!",
      "jsgeq %1!",
    #else
      "\362!\311",
      "j\306\312",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "sleq!jzer %1!",
      "jsgrtr %1!",
    #else
      "\307\361",
      "j\363\205",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "sgrtr!jzer %1!",
      "jsleq %1!",
    #else
      "\363!\311",
      "j\307\312",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "sgeq!jzer %1!",
      "jsless %1!",
    #else
      "\306\361",
      "j\362\205",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  /* Test for zero (common case, especially for strings)
   * E.g. the test expression of: "for (i=0; str{i}!=0; ++i)"
   *
   *    zero.alt                jzer n1
   *    jeq n1                  -
   *    --------------------------------------
   *    zero.alt                jnz n1
   *    jneq n1                 -
   */
  {
    #ifdef SCPACK
      "zero.alt!jeq %1!",
      "jzer %1!",
    #else
      "\277\222j\312",
      "\311",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "zero.alt!jneq %1!",
      "jnz %1!",
    #else
      "\277\222\364\312",
      "\364z\205",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  /* Incrementing and decrementing leaves a value in
   * in PRI which may not be used (for example, as the
   * third expression in a "for" loop).
   *    inc n1                  inc n1  ; ++n
   *    load.pri n1             ;$exp
   *    ;$exp                   -
   *    --------------------------------------
   *    load.pri n1             inc n1  ; n++, e.g. "for (n=0; n<10; n++)"
   *    inc n1                  ;$exp
   *    ;$exp                   -
   * Plus the varieties for stack relative increments
   * and decrements.
   */
  {
    #ifdef SCPACK
      "inc %1!load.pri %1!;$exp!",
      "inc %1!;$exp!",
    #else
      "\345c\205\330\244",
      "\345\357",
    #endif
    seqsize(2,2) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "load.pri %1!inc %1!;$exp!",
      "inc %1!;$exp!",
    #else
      "\330\345\357",
      "\345\357",
    #endif
    seqsize(2,2) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "inc.s %1!load.s.pri %1!;$exp!",
      "inc.s %1!;$exp!",
    #else
      "\345\340\205\331\244",
      "\345\360",
    #endif
    seqsize(2,2) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "load.s.pri %1!inc.s %1!;$exp!",
      "inc.s %1!;$exp!",
    #else
      "\331\345\360",
      "\345\360",
    #endif
    seqsize(2,2) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "dec %1!load.pri %1!;$exp!",
      "dec %1!;$exp!",
    #else
      "\344c\205\330\244",
      "\344\357",
    #endif
    seqsize(2,2) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "load.pri %1!dec %1!;$exp!",
      "dec %1!;$exp!",
    #else
      "\330\344\357",
      "\344\357",
    #endif
    seqsize(2,2) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "dec.s %1!load.s.pri %1!;$exp!",
      "dec.s %1!;$exp!",
    #else
      "\344\340\205\331\244",
      "\344\360",
    #endif
    seqsize(2,2) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "load.s.pri %1!dec.s %1!;$exp!",
      "dec.s %1!;$exp!",
    #else
      "\331\344\360",
      "\344\360",
    #endif
    seqsize(2,2) - seqsize(1,1)
  },
  /* ??? the same (increments and decrements) for references */
  /* Loading the constant zero has a special opcode.
   * When storing zero in memory, the value of PRI must not be later on.
   *    const.pri 0             zero n1
   *    stor.pri n1             ;$exp
   *    ;$exp                   -
   *    --------------------------------------
   *    const.pri 0             zero.s n1
   *    stor.s.pri n1           ;$exp
   *    ;$exp                   -
   *    --------------------------------------
   *    zero.pri                zero n1
   *    stor.pri n1             ;$exp
   *    ;$exp                   -
   *    --------------------------------------
   *    zero.pri                zero.s n1
   *    stor.s.pri n1           ;$exp
   *    ;$exp                   -
   *    --------------------------------------
   *    const.pri 0             zero.pri
   *    --------------------------------------
   *    const.alt 0             zero.alt
   * The last two alternatives save more memory than they save
   * time, but anyway...
   */
  {
    #ifdef SCPACK
      "const.pri 0!stor.pri %1!;$exp!",
      "zero %1!;$exp!",
    #else
      "\233\274\367\226or\215\244",
      "\277\300",
    #endif
    seqsize(2,2) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "const.pri 0!stor.s.pri %1!;$exp!",
      "zero.s %1!;$exp!",
    #else
      "\233\274\367\226or\301\215\244",
      "\277\301\300",
    #endif
    seqsize(2,2) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "zero.pri!stor.pri %1!;$exp!",
      "zero %1!;$exp!",
    #else
      "\313\226or\215\244",
      "\277\300",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "zero.pri!stor.s.pri %1!;$exp!",
      "zero.s %1!;$exp!",
    #else
      "\313\226or\301\215\244",
      "\277\301\300",
    #endif
    seqsize(2,1) - seqsize(1,1)
  },
  {
    #ifdef SCPACK
      "const.pri 0!",
      "zero.pri!",
    #else
      "\233\274\367",
      "\313",
    #endif
    seqsize(1,1) - seqsize(1,0)
  },
  {
    #ifdef SCPACK
      "const.alt 0!",
      "zero.alt!",
    #else
      "\233\213 \367",
      "\277\222",
    #endif
    seqsize(1,1) - seqsize(1,0)
  },
  /* ----- */
  { NULL, NULL, 0 }
};
