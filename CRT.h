/* Do not edit this file. It was automatically generated. */

#ifndef HEADER_CRT
#define HEADER_CRT
/*
htop - CRT.h
(C) 2004-2011 Hisham H. Muhammad
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#if HAVE_SETUID_ENABLED
#endif

#define ColorIndex(i,j) ((7-i)*8+j)

#define ColorPair(i,j) COLOR_PAIR(ColorIndex(i,j))

#define Black COLOR_BLACK
#define Red COLOR_RED
#define Green COLOR_GREEN
#define Yellow COLOR_YELLOW
#define Blue COLOR_BLUE
#define Magenta COLOR_MAGENTA
#define Cyan COLOR_CYAN
#define White COLOR_WHITE

#define ColorPairGrayBlack ColorPair(Magenta,Magenta)
#define ColorIndexGrayBlack ColorIndex(Magenta,Magenta)

#define KEY_WHEELUP KEY_F(20)
#define KEY_WHEELDOWN KEY_F(21)
#define KEY_RECLICK KEY_F(22)
#define KEY_SHIFT_TAB KEY_F(23)

//#link curses

#include <stdbool.h>

typedef enum TreeStr_ {
   TREE_STR_HORZ,
   TREE_STR_VERT,
   TREE_STR_RTEE,
   TREE_STR_BEND,
   TREE_STR_TEND,
   TREE_STR_OPEN,
   TREE_STR_SHUT,
   TREE_STR_COUNT
} TreeStr;

typedef enum ColorSchemes_ {
   COLORSCHEME_DEFAULT = 0,
   COLORSCHEME_MONOCHROME = 1,
   COLORSCHEME_BLACKONWHITE = 2,
   COLORSCHEME_LIGHTTERMINAL = 3,
   COLORSCHEME_MIDNIGHT = 4,
   COLORSCHEME_BLACKNIGHT = 5,
   COLORSCHEME_BROKENGRAY = 6,
   LAST_COLORSCHEME = 7,
} ColorSchemes;

typedef enum ColorElements_ {
   COLOR_RESET_COLOR,
   COLOR_DEFAULT_COLOR,
   COLOR_FUNCTION_BAR,
   COLOR_FUNCTION_KEY,
   COLOR_FAILED_SEARCH,
   COLOR_PANEL_HEADER_FOCUS,
   COLOR_PANEL_HEADER_UNFOCUS,
   COLOR_PANEL_SELECTION_FOCUS,
   COLOR_PANEL_SELECTION_FOLLOW,
   COLOR_PANEL_SELECTION_UNFOCUS,
   COLOR_LARGE_NUMBER,
   COLOR_METER_TEXT,
   COLOR_METER_VALUE,
   COLOR_LED_COLOR,
   COLOR_UPTIME,
   COLOR_BATTERY,
   COLOR_TASKS_RUNNING,
   COLOR_SWAP,
   COLOR_PROCESS,
   COLOR_PROCESS_SHADOW,
   COLOR_PROCESS_TAG,
   COLOR_PROCESS_MEGABYTES,
   COLOR_PROCESS_TREE,
   COLOR_PROCESS_R_STATE,
   COLOR_PROCESS_D_STATE,
   COLOR_PROCESS_BASENAME,
   COLOR_PROCESS_HIGH_PRIORITY,
   COLOR_PROCESS_LOW_PRIORITY,
   COLOR_PROCESS_THREAD,
   COLOR_PROCESS_THREAD_BASENAME,
   COLOR_BAR_BORDER,
   COLOR_BAR_SHADOW,
   COLOR_GRAPH_1,
   COLOR_GRAPH_2,
   COLOR_MEMORY_USED,
   COLOR_MEMORY_BUFFERS,
   COLOR_MEMORY_BUFFERS_TEXT,
   COLOR_MEMORY_CACHE,
   COLOR_LOAD,
   COLOR_LOAD_AVERAGE_FIFTEEN,
   COLOR_LOAD_AVERAGE_FIVE,
   COLOR_LOAD_AVERAGE_ONE,
   COLOR_CHECK_BOX,
   COLOR_CHECK_MARK,
   COLOR_CHECK_TEXT,
   COLOR_CLOCK,
   COLOR_HELP_BOLD,
   COLOR_HOSTNAME,
   COLOR_CPU_NICE,
   COLOR_CPU_NICE_TEXT,
   COLOR_CPU_NORMAL,
   COLOR_CPU_KERNEL,
   COLOR_CPU_IOWAIT,
   COLOR_CPU_IRQ,
   COLOR_CPU_SOFTIRQ,
   COLOR_CPU_STEAL,
   COLOR_CPU_GUEST,
   COLOR_PANEL_EDIT,
   COLOR_SCREENS_OTH_BORDER,
   COLOR_SCREENS_OTH_TEXT,
   COLOR_SCREENS_CUR_BORDER,
   COLOR_SCREENS_CUR_TEXT,
   LAST_COLORELEMENT
} ColorElements;

void CRT_fatalError(const char* note) __attribute__ ((noreturn));

void CRT_handleSIGSEGV(int sgn);

#define KEY_ALT(x) (KEY_F(64 - 26) + (x - 'A'))


extern const char *CRT_treeStrAscii[TREE_STR_COUNT];

#ifdef HAVE_LIBNCURSESW

extern const char *CRT_treeStrUtf8[TREE_STR_COUNT];

extern bool CRT_utf8;

#endif

extern const char **CRT_treeStr;

extern int CRT_delay;

int* CRT_colors;

extern int CRT_colorSchemes[LAST_COLORSCHEME][LAST_COLORELEMENT];

extern int CRT_scrollHAmount;

extern int CRT_scrollWheelVAmount;

char* CRT_termType;

// TODO move color scheme to Settings, perhaps?

extern int CRT_colorScheme;

void *backtraceArray[128];

#if HAVE_SETUID_ENABLED

#define DIE(msg) do { CRT_done(); fprintf(stderr, msg); exit(1); } while(0)

void CRT_dropPrivileges();

void CRT_restorePrivileges();

#else

/* Turn setuid operations into NOPs */

#ifndef CRT_dropPrivileges
#define CRT_dropPrivileges()
#define CRT_restorePrivileges()
#endif

#endif

// TODO: pass an instance of Settings instead.

void CRT_init(int delay, int colorScheme);

void CRT_done();

void CRT_fatalError(const char* note);

int CRT_readKey();

void CRT_disableDelay();

void CRT_enableDelay();

void CRT_setColors(int colorScheme);

#endif
