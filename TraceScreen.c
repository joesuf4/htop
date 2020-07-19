/*
htop - TraceScreen.c
(C) 2005-2006 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "TraceScreen.h"

#include "CRT.h"
#include "InfoScreen.h"
#include "ProcessList.h"
#include "ListItem.h"
#include "IncSet.h"
#include "StringUtils.h"
#include "FunctionBar.h"
#include "solaris/SolarisProcess.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

/*{
#include "InfoScreen.h"

typedef struct TraceScreen_ {
   InfoScreen super;
   bool tracing;
   int fdpair[2];
   int child;
   bool contLine;
   bool follow;
} TraceScreen;

}*/

static const char* const TraceScreenFunctions[] = {"Search ", "Filter ", "AutoScroll ", "Stop Tracing   ", "Done   ", NULL};

static const char* const TraceScreenKeys[] = {"F3", "F4", "F8", "F9", "Esc"};

static int TraceScreenEvents[] = {KEY_F(3), KEY_F(4), KEY_F(8), KEY_F(9), 27};

InfoScreenClass TraceScreen_class = {
   .super = {
      .extends = Class(Object),
      .delete = TraceScreen_delete
   },
   .draw = TraceScreen_draw,
   .onErr = TraceScreen_updateTrace,
   .onKey = TraceScreen_onKey,
};

TraceScreen* TraceScreen_new(Process* process) {
   TraceScreen* this = xMalloc(sizeof(TraceScreen));
   Object_setClass(this, Class(TraceScreen));
   this->tracing = true;
   this->contLine = false;
   this->follow = true;
   FunctionBar* fuBar = FunctionBar_new(TraceScreenFunctions, TraceScreenKeys, TraceScreenEvents);
   CRT_disableDelay();
   return (TraceScreen*) InfoScreen_init(&this->super, process, fuBar, LINES-2, "");
}

void TraceScreen_delete(Object* cast) {
   TraceScreen* this = (TraceScreen*) cast;
   if (this->child > 0) {
      kill(this->child, SIGTERM);
      waitpid(this->child, NULL, 0);
      close(this->fdpair[0]);
      close(this->fdpair[1]);
   }
   CRT_enableDelay();
   free(InfoScreen_done((InfoScreen*)cast));
}

void TraceScreen_draw(InfoScreen* this) {
   attrset(CRT_colors[COLOR_PANEL_HEADER_FOCUS]);
   mvhline(0, 0, ' ', COLS);
   SolarisProcess *sp = (SolarisProcess *)this->process;
   mvprintw(0, 0, "Trace of process %d/%d - %s", sp->realpid, sp->lwpid, sp->super.comm);
   attrset(CRT_colors[COLOR_DEFAULT_COLOR]);
   IncSet_drawBar(this->inc);
}

bool TraceScreen_forkTracer(TraceScreen* this) {
   char buffer[1001];
   int error = pipe2(this->fdpair, O_NONBLOCK);
   if (error == -1) return false;
   this->child = fork();
   if (this->child == -1) return false;
   if (this->child == 0) {
      CRT_dropPrivileges();
      dup2(this->fdpair[1], STDERR_FILENO);
      SolarisProcess *sp = (SolarisProcess *)this->super.process;
      xSnprintf(buffer, sizeof(buffer), "%d/%d", sp->realpid, sp->lwpid);
      execlp("truss", "truss", "-vall", "-p", buffer, NULL);
      const char* message = "Could not execute 'truss'. Please make sure it is available in your $PATH.";
      ssize_t written = write(this->fdpair[1], message, strlen(message));
      (void) written;
      exit(1);
   }

   return true;
}

void TraceScreen_updateTrace(InfoScreen* super) {
   TraceScreen* this = (TraceScreen*) super;
   char buffer[10001];
   fd_set fds;
   FD_ZERO(&fds);
   //   FD_SET(STDIN_FILENO, &fds);
   FD_SET(this->fdpair[0], &fds);
   struct timeval tv;
   tv.tv_sec = 0; tv.tv_usec = 500;
   int ready = select(this->fdpair[0]+1, &fds, NULL, NULL, &tv);
   int nread = 0;
   if (ready > 0 && FD_ISSET(this->fdpair[0], &fds))
      nread = read(this->fdpair[0],buffer,10000);
   if (nread > 0 && this->tracing) {
      char* line = buffer;
      buffer[nread] = '\0';
      for (int i = 0; i < nread; i++) {
        if (buffer[i] == '\t')
            buffer[i] = ' ';
        if (buffer[i] == '\n') {
            buffer[i] = '\0';
            if (this->contLine) {
               InfoScreen_appendLine(&this->super, line);
               this->contLine = false;
            } else {
               InfoScreen_addLine(&this->super, line);
            }
            line = buffer+i+1;
         }
      }
      if (line < buffer+nread) {
         InfoScreen_addLine(&this->super, line);
         buffer[nread] = '\0';
         this->contLine = true;
      }
      if (this->follow)
         Panel_setSelected(this->super.display, Panel_size(this->super.display)-1);
   }
}

bool TraceScreen_onKey(InfoScreen* super, int ch) {
   TraceScreen* this = (TraceScreen*) super;
   switch(ch) {
      case 'f':
      case KEY_F(8):
         this->follow = !(this->follow);
         if (this->follow)
            Panel_setSelected(super->display, Panel_size(super->display)-1);
         return true;
      case 't':
      case KEY_F(9):
         this->tracing = !this->tracing;
         FunctionBar_setLabel(super->display->defaultBar, KEY_F(9), this->tracing?"Stop Tracing   ":"Resume Tracing ");
         InfoScreen_draw(this);
         return true;
   }
   //this->follow = false;
   return false;
}
