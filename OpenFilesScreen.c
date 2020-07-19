/*
htop - OpenFilesScreen.c
(C) 2005-2006 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include "OpenFilesScreen.h"

#include "CRT.h"
#include "ProcessList.h"
#include "IncSet.h"
#include "StringUtils.h"
#include "FunctionBar.h"
#include "solaris/SolarisProcess.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

/*{
#include "InfoScreen.h"

typedef struct OpenFiles_Data_ {
   char* data[256];
} OpenFiles_Data;

typedef struct OpenFiles_ProcessData_ {
   OpenFiles_Data data;
   int error;
   struct OpenFiles_FileData_* files;
} OpenFiles_ProcessData;

typedef struct OpenFiles_FileData_ {
   OpenFiles_Data data;
   struct OpenFiles_FileData_* next;
} OpenFiles_FileData;

typedef struct OpenFilesScreen_ {
   InfoScreen super;
   pid_t pid;
} OpenFilesScreen;

}*/

InfoScreenClass OpenFilesScreen_class = {
   .super = {
      .extends = Class(Object),
      .delete = OpenFilesScreen_delete
   },
   .scan = OpenFilesScreen_scan,
   .draw = OpenFilesScreen_draw
};

OpenFilesScreen* OpenFilesScreen_new(Process* process) {
   OpenFilesScreen* this = xMalloc(sizeof(OpenFilesScreen));
   SolarisProcess *sp = (SolarisProcess *)process;
   Object_setClass(this, Class(OpenFilesScreen));
   if (Process_isThread(process))
      this->pid = sp->lwpid;
   else
      this->pid = sp->realpid;
   return (OpenFilesScreen*) InfoScreen_init(&this->super, process, NULL, LINES-3, "pfiles + pmap output");
}

void OpenFilesScreen_delete(Object* this) {
   free(InfoScreen_done((InfoScreen*)this));
}

void OpenFilesScreen_draw(InfoScreen* this) {
   InfoScreen_drawTitled(this, "Snapshot of files open, and virual memory maps, in process %d - %s", ((OpenFilesScreen*)this)->pid, this->process->comm);
}

static OpenFiles_ProcessData* OpenFilesScreen_getProcessData(pid_t pid) {
   char buffer[100025];
   xSnprintf(buffer, 100024, "%d", pid);
   OpenFiles_ProcessData* pdata = xCalloc(1, sizeof(OpenFiles_ProcessData));
   OpenFiles_Data* item = &(pdata->data);
   int fdpair[2];
   if (pipe(fdpair) == -1) {
      pdata->error = 1;
      return pdata;
   }
   pid_t child = fork();
   if (child == -1) {
      pdata->error = 1;
      return pdata;
   }
   if (child == 0) {
      close(fdpair[0]);
      dup2(fdpair[1], STDOUT_FILENO);
      close(fdpair[1]);
      int fdnull = open("/dev/null", O_WRONLY);
      if (fdnull < 0)
         exit(1);
      dup2(fdnull, STDERR_FILENO);
      close(fdnull);
      execlp("pfiles", "pfiles", buffer, NULL);
      exit(127);
   }


   pid_t child2 = fork();
   if (child2 == -1) {
      pdata->error = 1;
      return pdata;
   }
   if (child2 == 0) {
      close(fdpair[0]);
      dup2(fdpair[1], STDOUT_FILENO);
      close(fdpair[1]);
      int fdnull = open("/dev/null", O_WRONLY);
      if (fdnull < 0)
         exit(1);
      dup2(fdnull, STDERR_FILENO);
      close(fdnull);
      execlp("pmap", "pmap", buffer, NULL);
      exit(127);
   }

   sleep(1);
   close(fdpair[1]);
   int rv = read(fdpair[0], buffer, sizeof buffer - 1);

   close (fdpair[0]);

   if (rv <= 0) {
     pdata->error = 1;
     return pdata;
   }

   buffer[rv] = 0;

   item->data[0] = strdup(buffer);

   int wstatus;
   if (waitpid(child, &wstatus, 0) == -1) {
      pdata->error = 1;
      return pdata;
   }
   if (!WIFEXITED(wstatus)) {
      pdata->error = 1;
      return pdata;
   }
   else
      pdata->error = WEXITSTATUS(wstatus);

   if (waitpid(child2, &wstatus, 0) == -1) {
      pdata->error = 1;
      return pdata;
   }
   if (!WIFEXITED(wstatus)) {
      pdata->error = 1;
      return pdata;
   }
   else
      pdata->error = WEXITSTATUS(wstatus);

   return pdata;
}

static inline void OpenFiles_Data_clear(OpenFiles_Data* data) {
   for (int i = 0; i < 255; i++)
      if (data->data[i])
         free(data->data[i]);
}

void OpenFilesScreen_scan(InfoScreen* this) {
   Panel* panel = this->display;
   int idx = Panel_getSelectedIndex(panel);
   Panel_prune(panel);
   OpenFiles_ProcessData* pdata = OpenFilesScreen_getProcessData(((OpenFilesScreen*)this)->pid);
   if (pdata->error == 127) {
      InfoScreen_addLine(this, "Could not execute 'pfiles' or 'pmap'. Please make sure they are available in your $PATH.");
   } else if (pdata->error == 1) {
      InfoScreen_addLine(this, "Failed listing 'pfiles' and 'pmap' output (possibly truncated).");
   }
   char *data;
   if (pdata && ((data = pdata->data.data[0]) != NULL)) {
     char *nl = data, *line = data;
     while (*nl) {
       if (*nl == '\t')
         *nl = ' ';

       if (*nl == '\n') {
         *nl = 0;
         InfoScreen_addLine(this, line);
         line = nl + 1;
       }
       ++nl;
     }
     OpenFiles_Data_clear(&pdata->data);
   }
   free(pdata);
   //Vector_insertionSort(this->lines);
   //Vector_insertionSort(panel->items);
   Panel_setSelected(panel, idx);
}
