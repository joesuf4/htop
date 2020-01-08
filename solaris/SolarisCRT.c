/*
htop - SolarisCRT.c
(C) 2014 Hisham H. Muhammad
(C) 2018 Guy M. Broome
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "config.h"
#include "CRT.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif

// Solaris and illumos can pass a psinfo struct to the target of proc_walk
// and yet the process has Gone Away(TM) before the psinfo_t info can be read.
// This is only a serious problem for pointers in the psinfo_t struct, where attempting
// to read them causes a SIGSEGV.  This would be fixed by grabbing a handle, but we have
// to be superuser or process owner for that.  So, we'll implement a dirty hack here to
// catch SIGSEGV in that specific case and work around it.
extern bool   protected_str_read;
extern char*  protected_str_target;
extern size_t protected_str_tlen;

void CRT_handleSIGSEGV(int sgn) {
   (void) sgn;
   if (!protected_str_read) {
      CRT_done();
      fprintf(stderr, "\n\nhtop " VERSION " aborting. Please report bug at http://hisham.hm/htop\n");
#ifdef HAVE_EXECINFO_H
      size_t size = backtrace(backtraceArray, sizeof(backtraceArray) / sizeof(void *));
      fprintf(stderr, "\n Please include in your report the following backtrace: \n");
      backtrace_symbols_fd(backtraceArray, size, 2);
      fprintf(stderr, "\nAdditionally, in order to make the above backtrace useful,");
      fprintf(stderr, "\nplease also run the following command to generate a disassembly of your binary:");
      fprintf(stderr, "\n\n   objdump -d `which htop` > ~/htop.objdump");
      fprintf(stderr, "\n\nand then attach the file ~/htop.objdump to your bug report.");
      fprintf(stderr, "\n\nThank you for helping to improve htop!\n\n");
#endif
      abort();
   } else {
      protected_str_target[0] = NULL;
      protected_str_tlen = 0;
   }
}
