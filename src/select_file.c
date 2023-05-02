/**
 * FujiNet for #Adam configuration program
 *
 * Select file from Host Slot
 */

#include <conio.h>
#include <string.h>
#include "select_file.h"

#ifdef BUILD_ADAM
#include "adam/fuji_typedefs.h"
#include "adam/screen.h"
#include "adam/io.h"
#include "adam/globals.h"
#include "adam/input.h"
#include "adam/bar.h"
#define DIR_MAX_LEN 31
#define ENTRIES_PER_PAGE 15
#endif /* BUILD_ADAM */

#ifdef BUILD_ATARI
#include "atari/fuji_typedefs.h"
#include "atari/screen.h"
#include "atari/io.h"
#include "atari/globals.h"
#include "atari/input.h"
#include "atari/bar.h"
#define DIR_MAX_LEN 36
#endif

#ifdef BUILD_APPLE2
#ifdef BUILD_A2CDA
#pragma cda "FujiNet Config" Start ShutDown
#endif /* BUILD_A2CDA */
#include "apple2/fuji_typedefs.h"
#include "apple2/screen.h"
#include "apple2/io.h"
#include "apple2/globals.h"
#include "apple2/input.h"
#include "apple2/bar.h"
#define DIR_MAX_LEN 40
#define ENTRIES_PER_PAGE 15
#endif /* BUILD_APPLE2 */

#ifdef BUILD_C64
#include "c64/fuji_typedefs.h"
#include "c64/screen.h"
#include "c64/io.h"
#include "c64/globals.h"
#include "c64/input.h"
#include "c64/bar.h"
#define ENTRIES_PER_PAGE 15
#endif /* BUILD_C64 */

#ifdef BUILD_PC8801
#include "pc8801/fuji_typedefs.h"
#include "pc8801/screen.h"
#include "pc8801/io.h"
#include "pc8801/globals.h"
#include "pc8801/input.h"
#include "pc8801/bar.h"
#define ENTRIES_PER_PAGE 15
#endif /* BUILD_PC8801 */

#ifdef BUILD_PC6001
#include "pc6001/fuji_typedefs.h"
#include "pc6001/screen.h"
#include "pc6001/io.h"
#include "pc6001/globals.h"
#include "pc6001/input.h"
#include "pc6001/bar.h"
#define ENTRIES_PER_PAGE 15
#endif /* BUILD_PC6001 */

SFSubState sf_subState;
char path[224];
char filter[32];
char source_path[224];
char source_filter[32];
char source_filename[128];
DirectoryPosition pos = 0;
bool dir_eof = false;
bool quick_boot = false;
unsigned long selected_size = 0;
unsigned char entry_size[ENTRIES_PER_PAGE];
unsigned short entry_timer = ENTRY_TIMER_DUR;
bool long_entry_displayed = false;
bool copy_mode = false;
unsigned char selected_file_type = 0;

extern unsigned char copy_host_slot;

void select_file_init(void)
{
  if (copy_mode == true)
  {
    strncpy(source_path, path, 224);
    strncpy(source_filter, filter, 32);
  }

  io_close_directory();
  pos = 0;
  memset(entry_size, 0, ENTRIES_PER_PAGE);
  memset(path, 0, 256);
  path[0] = '/';
  memset(filter, 0, 32);
  screen_select_file();
  sf_subState = SF_DISPLAY;
  quick_boot = dir_eof = false;
}

unsigned char select_file_display(void)
{
  char visibleEntries = 0;
  char i;
  char *e;

  io_mount_host_slot(selected_host_slot);

  if (io_error())
  {
    screen_error("  COULD NOT MOUNT HOST SLOT.");
    sf_subState = SF_DONE;
    state = HOSTS_AND_DEVICES;
    return 0;
  }

  screen_select_file_display(path, filter);

  io_open_directory(selected_host_slot, path, filter);

  if (io_error())
  {
    screen_error("  COULD NOT OPEN DIRECTORY.");
    sf_subState = SF_DONE;
    state = HOSTS_AND_DEVICES;
    return 0;
  }

  if (pos > 0)
    io_set_directory_position(pos);

  for (i = 0; i < ENTRIES_PER_PAGE; i++)
  {
    e = io_read_directory(39/*DIR_MAX_LEN*/, 0x40);
#ifdef BUILD_ADAM
#define FUDGE_OFFSET 2
#else
#define FUDGE_OFFSET 1
#endif
    if (e[FUDGE_OFFSET] == 0x7F) // I am truly ashamed of this, and will fix someday -thom
    {
      dir_eof = true;
      break;
    }
    else
    {
      entry_size[i] = strlen(e+1);
      visibleEntries++; // could filter on e[0] to deal with message entries like on FUJINET.PL
      screen_select_file_display_entry(i, e+1, e[0]);
    }
  }

  // Do one more read to check EOF
  e = io_read_directory(39 /*DIR_MAX_LEN*/, 0x40);
  if (e[1] == 0x7F) // was e[2]
    dir_eof = true;

  io_close_directory();

  if (pos > 0)
    screen_select_file_prev();

  if (dir_eof != true)
    screen_select_file_next();

  sf_subState = SF_CHOOSE;
  return visibleEntries;
}

void select_file_set_source_filename(void)
{
  char entry[128];

  io_open_directory(selected_host_slot, path, filter);
  io_set_directory_position(pos);
  strcpy(entry, io_read_directory(128, 0));
  strcat(path, entry);
  strcpy(source_filename, entry);
}

void select_display_long_filename(void)
{
  char *e;

#ifdef BUILD_ATARI
  if ((entry_size[bar_get() - FILES_START_Y] > LONG_FILENAME) && (entry_timer == 0))
#else
  if ((entry_size[bar_get()] > 30) && (entry_timer == 0))
#endif
  {
    if (long_entry_displayed == false)
    {
      io_open_directory(selected_host_slot, path, filter);
#ifdef BUILD_ATARI      
      io_set_directory_position(pos + bar_get() - FILES_START_Y);
#else
      io_set_directory_position(pos + bar_get());
#endif
      e = io_read_directory(64, 0);
      screen_select_file_display_long_filename(e);
      io_close_directory();
      long_entry_displayed = true;
    }
  }
  else
  {
    long_entry_displayed = false;
    screen_select_file_clear_long_filename();
  }
}

void select_next_page(void)
{
  bar_clear(false);
  pos += ENTRIES_PER_PAGE;
  sf_subState = SF_DISPLAY;
  dir_eof = false;
}

void select_prev_page(void)
{
  bar_clear(false);
  pos -= ENTRIES_PER_PAGE;
  sf_subState = SF_DISPLAY;
  dir_eof = false;
}

void select_file_filter(void)
{
  screen_select_file_filter();
  input_line_filter(filter);
  dir_eof = quick_boot = false;
  pos = 0;
  sf_subState = SF_DISPLAY;
}

void select_file_choose(char visibleEntries)
{
  screen_select_file_choose(visibleEntries);

  while (sf_subState == SF_CHOOSE)
  {
    sf_subState = input_select_file_choose();
    if (sf_subState == SF_SELECTED) {
      pos += (bar_get() - FILES_START_Y);
      selected_file_type = select_file_type();
      if (selected_file_type == 0) 
      {
        sf_subState = SF_CHOOSE;
        pos -= (bar_get() - FILES_START_Y);
      }
//      else if (selected_file_type==3) sf_subState = SF_LINK;
      else if (selected_file_type==1) sf_subState = SF_ADVANCE_FOLDER;
      else sf_subState = SF_DONE;
    }
  }
}

void select_file_advance(void)
{
  char *e;

  bar_clear(false);

  io_open_directory(selected_host_slot, path, filter);

  io_set_directory_position(pos);

  e = io_read_directory(128, 1);

  strcat(path, e); // append directory entry to end of current path

  io_close_directory(); // have to use "e" before calling another io command, otherwise e gets wiped out

  pos = 0;
  dir_eof = quick_boot = false;

  sf_subState = SF_DISPLAY; // and display the result.
}

void select_file_devance(void)
{
  int i;
  char *p = strrchr(path, '/'); // find end of directory string (last /)

  bar_clear(false);

  while (*--p != '/')
    ; // scoot backward until we reach next /

  p++;

  *p = 0; // truncate string.

  for ( i = strlen (path); i < 224; i++ )
  {
    path[i] = 0;
  }

  pos = 0;
  dir_eof = quick_boot = false;

  sf_subState = SF_DISPLAY; // And display the result.
}

unsigned char select_file_is_folder(void)
{
    unsigned char result = select_file_type();
    if (result == 1) return true;
    else return false;
}

unsigned char select_file_type(void)
{
  char *e;
  unsigned char result;

  io_open_directory(selected_host_slot, path, filter);

  io_set_directory_position(pos);

  e = io_read_directory(16, 0x40); // 0x40 -> get type info
  result = e[0];

  io_close_directory();

  return result; // Offset 10 = directory flag.
}

void select_file_new(void)
{
#ifdef __ORCAC__
  static char f[128];
#else
  char f[128];
#endif
  char k;

  memset(f, 0, 128);

  screen_select_file_new_type();
  k = input_select_file_new_type();
  if (k == 0)
  {
    sf_subState = SF_CHOOSE;
    return;
  }

  screen_select_file_new_size(k);
  selected_size = input_select_file_new_size(k);

  if (selected_size == 1) // User selected custom
  {
    screen_select_file_new_custom();
    selected_size = input_select_file_new_custom();
  }

  if (selected_size == 0) // Aborted from size
  {
    sf_subState = SF_CHOOSE;
    return;
  }

  screen_select_file_new_name();
  input_select_file_new_name(f);

  if (f[0] == 0x00)
  {
    sf_subState = SF_CHOOSE;
    return;
  }
  else
  {
    create = true;
    strcat(path, f);
    state = SELECT_SLOT;
  }
}

void select_file_copy(void)
{
  sf_subState = SF_DONE;
  state = DESTINATION_HOST_SLOT;
}

void select_file_done(void)
{
  if (copy_mode == true)
    state = PERFORM_COPY;
  else
    state = SELECT_SLOT;
}

void select_file(void)
{
  char visibleEntries = 0;

  sf_subState = SF_INIT;

  while (state == SELECT_FILE)
  {
    switch (sf_subState)
    {
    case SF_INIT:
      select_file_init(); // get things ready, clear screen, status area
      break;
    case SF_DISPLAY:
      visibleEntries = select_file_display(); // put the list of files on the screen
      break;
    case SF_NEXT_PAGE:
      select_next_page();
      break;
    case SF_PREV_PAGE:
      select_prev_page();
      break;
    case SF_CHOOSE:
      select_file_choose(visibleEntries); // allow user to pick a file
      break;
    case SF_FILTER:
      select_file_filter();
      break;
    case SF_ADVANCE_FOLDER:
      select_file_advance();
      break;
    case SF_DEVANCE_FOLDER:
      select_file_devance();
      break;
    case SF_NEW:
      select_file_new();
      break;
    case SF_COPY:
      select_file_copy();
      break;
    case SF_DONE:
      select_file_done();
      break;
    }
  }
}
