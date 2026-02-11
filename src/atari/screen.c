#ifdef BUILD_ATARI

/**
 * FujiNet Configuration Program: screen functions
 * The screen functions are a common set of graphics and text string functions to display information to the video output device.
 * These screen functions is comprised of two files; screen.c and screen.h.  Screen.c sets up the display dimensions, memory and
 * initializes the display as well as provide various text manipulations functions for proper display.  The screen.h file include
 * defines for various items such as keyboard codes, functions and screen memory addresses.
 *
 **/

#include "atari_screen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <stdint.h>
#include <peekpoke.h>
#include "../globals.h"
#include "../input.h"
#include "../constants.h"

unsigned char *video_ptr;  // a pointer to the memory address containing the screen contents
unsigned char *cursor_ptr; // a pointer to the current cursor position on the screen
char _visibleEntries;
extern bool copy_mode;
char text_empty[] = "Empty";
char fn[256];
char d[256];
unsigned char byte0;
unsigned char byte1;
unsigned char byte2;
unsigned char byte3;
uint32_t s;

// The currently active screen (basically each time a different displaylist is used that will be considered a screen)
// Used by 'set_cursor' so it knows the layout of the screen and can properly figure out the memory offset needed whengiven an (x, y) coordinate pair.
//
_screen active_screen = SCREEN_HOSTS_AND_DEVICES;

// Patch to the character set to add things like the folder icon and the wifi-signal-strength bars.
// Each new character is 8-bytes.
//
unsigned char fontPatch[48] = {
    0, 0, 0, 0, 0, 0, 3, 51,
    0, 0, 3, 3, 51, 51, 51, 51,
    48, 48, 48, 48, 48, 48, 48, 48,
    0, 120, 135, 255, 255, 255, 255, 0,
    0x00, 0x78, 0x87, 0xff, 0xff, 0xff, 0xff, 0x00,
    0, 48, 120, 252, 48, 48, 48, 0};


void set_active_screen(unsigned char screen)
{
  active_screen = screen;
}

void screen_puts(unsigned char x, unsigned char y, const char *s)
{
  cputsxy(x,y,s);
}

void font_init()
{
  // Copy ROM font
  memcpy((unsigned char *)FONT_MEMORY, (unsigned char *)0xE000, 1024);

  // And patch it.
  memcpy((unsigned char *) FONT_MEMORY + 520, &fontPatch, sizeof(fontPatch));

  OS.chbas = FONT_MEMORY >> 8; // use the charset
}

void screen_mount_and_boot()
{
  set_active_screen(SCREEN_MOUNT_AND_BOOT);
  screen_clear();
  bar_clear(false);
}

void screen_set_wifi_extended(AdapterConfigExtended *ac)
{
  unsigned char i = 0;
  unsigned char x = 13;

  set_active_screen(SCREEN_SET_WIFI);
  screen_clear();
  bar_clear(false);
  cputsxy(0, 0, "WELCOME TO FUJINET!");
  cputsxy(0, 23, "SCANNING NETWORKS...");
  cputsxy(0, 2, "MAC Address:");
  cputsxy(13, 2, ac->sMacAddress);
}

void screen_set_wifi_print_rssi(SSIDInfo *s, unsigned char i)
{
  char out[4] = {0x20, 0x20, 0x20, 0x00};

  if (s->rssi > -40)
  {
    out[0] = 0x01;
    out[1] = 0x02;
    out[2] = 0x03;
  }
  else if (s->rssi > -60)
  {
    out[0] = 0x01;
    out[1] = 0x02;
  }
  else
  {
    out[0] = 0x01;
  }

  cputsxy(35, i + NETWORKS_START_Y, out);
}

void screen_set_wifi_display_ssid(char n, SSIDInfo *s)
{
  cputsxy(2, n + NETWORKS_START_Y, (char *)s->ssid);
  screen_set_wifi_print_rssi(s, n);
}

void screen_set_wifi_select_network(unsigned char)
{
  screen_clear_line(numNetworks + NETWORKS_START_Y);
  cputsxy(2, NETWORKS_START_Y + numNetworks, "<Enter a specific SSID>");

  cputsxy(0, 23, " SELECT NET, S SKIP "
                     "   ESC TO RE-SCAN   ");

  bar_show(NETWORKS_START_Y);
}

void screen_set_wifi_custom(void)
{
  screen_clear_line(20);
  cputsxy(0, 22, " ENTER NETWORK NAME "
                     "  AND PRESS return  ");
}

void screen_set_wifi_password(void)
{
  screen_clear_line(23);
  screen_clear_line(24);
  cputsxy(0, 23, "    ENTER PASSWORD");
}


/*
 * Display the 'info' screen
 */
void screen_show_info_extended(bool, AdapterConfigExtended *ac)
{
  set_active_screen(SCREEN_SHOW_INFO);
  screen_clear();
  bar_clear(false);

  cputsxy(3, 5, "FUJINET CONFIG");
  cputsxy(7, 17,
              CH_KEY_LABEL_L CH_INV_C CH_KEY_LABEL_R "RECONNECT " CH_KEY_LABEL_L CH_INV_S CH_KEY_LABEL_R "CHANGE SSID");
  cputsxy(9, 19, "Any other key to return");
  cputsxy(5, 7, "      SSID:");
  cputsxy(5, 8, "  Hostname:");
  cputsxy(5, 9, "IP Address:");
  cputsxy(5, 10, "   Gateway:");
  cputsxy(5, 11, "       DNS:");
  cputsxy(5, 12, "   Netmask:");
  cputsxy(5, 13, "       MAC:");
  cputsxy(5, 14, "     BSSID:");
  cputsxy(5, 15, "   Version:");

  cputsxy(17, 7, ac->ssid);
  cputsxy(17, 8, ac->hostname);
  cputsxy(17, 9, ac->sLocalIP);
  cputsxy(17, 10, ac->sGateway);
  cputsxy(17, 11, ac->sDnsIP);
  cputsxy(17, 12, ac->sNetmask);
  cputsxy(17, 13, ac->sMacAddress);
  cputsxy(17, 14, ac->sBssid);
  cputsxy(17, 15, ac->fn_version);
}

void screen_select_slot(const char *e)
{
  unsigned long *s;
  set_active_screen(SCREEN_SELECT_SLOT);

  screen_clear();

  cputsxy(0, 22,
              CH_KEY_1TO8 "Slot" CH_KEY_RETURN "Select" CH_KEY_LABEL_L CH_INV_E CH_KEY_LABEL_R "ject" CH_KEY_ESC "Abort");

  cputsxy(0, 0, "MOUNT TO DRIVE SLOT");

  // Show file details if it's an existing file only.
  if ( create == false )
  {
    memset(d, 0, sizeof(d));

    // Modified time
    sprintf(d, "%8s %04u-%02u-%02u %02u:%02u:%02u", "MTIME:", (*e++) + 1970, *e++, *e++, *e++, *e++, *e++);
    cputsxy(0, DEVICES_END_MOUNT_Y + 3, d);

    
    // File size
    s=(unsigned long *)e; // Cast the next four bytes as a long integer.

    sprintf(d, "%8s %lu K", "SIZE:", *s >> 10);
    cputsxy(0, DEVICES_END_MOUNT_Y + 4, d);

     e += sizeof(unsigned long) + 3; // Skip isdir, trunc, type
     
    // Filename
    cputsxy(3, DEVICES_END_MOUNT_Y + 2, "FILE:");
    cputsxy(9, DEVICES_END_MOUNT_Y + 2, e);
  }

  screen_hosts_and_devices_device_slots(DEVICES_START_MOUNT_Y, deviceSlots, deviceEnabled);

  bar_show(DEVICES_START_MOUNT_Y);
}

void screen_select_slot_mode(void)
{
  screen_clear_line(21);
  cputsxy(0, 22,
              CH_KEY_RETURN "Read Only" CH_KEY_LABEL_L CH_INV_W CH_KEY_LABEL_R "Read/Write" CH_KEY_ESC "Abort");
}

void screen_select_slot_choose(void)
{
}

void screen_select_slot_eject(unsigned char)
{
}

void screen_select_slot_build_eos_directory(void)
{
}

void screen_select_slot_build_eos_directory_label(void)
{
}

void screen_select_slot_build_eos_directory_creating(void)
{
}

void screen_select_file(void)
{
  set_active_screen(SCREEN_SELECT_FILE);
  screen_clear();
  bar_clear(false);

//cputsxy(4, 0, "DISK IMAGES");
cputsxy(0, FILES_START_Y - 1, HORIZONTAL_LINE);
cputsxy(0, FILES_START_Y + ENTRIES_PER_PAGE + ADDTL_ENTRIES, HORIZONTAL_LINE);


  if (copy_mode == false)
  {
    cputsxy(0, 22,
                CH_KEY_LEFT CH_KEY_DELETE "Up Dir" CH_KEY_N "ew" CH_KEY_F "ilter" CH_KEY_C "opy");
  }
  else
  {
    cputsxy(0, 22,
                CH_KEY_LEFT CH_KEY_DELETE "Up Dir" CH_KEY_N "ew" CH_KEY_F "ilter" CH_KEY_C "Do It!");
  }
  cputsxy(0, 23,
              CH_KEY_RIGHT CH_KEY_RETURN "Choose" CH_KEY_OPTION "Boot" CH_KEY_ESC "Abort");
}

void screen_select_file_display(char *p, char *f)
{
  unsigned char i;

cputsxy(0, FILES_START_Y - 1, HORIZONTAL_LINE);
cputsxy(0, FILES_START_Y + ENTRIES_PER_PAGE + ADDTL_ENTRIES, HORIZONTAL_LINE);

  // Host
  cputsxy(0, 0, "Host:");
  cputsxy(5, 0, selected_host_name);

  // Filter
  cputsxy(0, 1, "Fltr:");
  cputsxy(5, 1, f);

  // Path - the path can wrap to line 4 (maybe 5?) so clear both to be safe.
  screen_clear_line(2);
  screen_clear_line(3);
  cputsxy(0, 2, "Path:");
  cputsxy(5, 2, p);

  // Clear out the file area
  for (i = FILES_START_Y; i < FILES_START_Y + ENTRIES_PER_PAGE + ADDTL_ENTRIES; i++)
  {
    screen_clear_line(i);
  }

  // clear Prev/next page lines. Sometimes they're left on the screen during directory devance.
//  screen_clear_line(FILES_START_Y + ENTRIES_PER_PAGE + ADDTL_ENTRIES);
//  screen_clear_line(FILES_START_Y - 1);
}

void screen_select_file_display_long_filename(const char *e)
{
  cputsxy(0, 24, e);
}

void screen_select_file_clear_long_filename(void)
{
  screen_clear_line(24);
  screen_clear_line(25);

}

void screen_select_file_filter(void)
{
  // No need to display anything additional, we're already showing the filter on the screen.
}

void screen_select_file_next(void)
{
  if (dir_eof == false)
  {
    cputsxy(37, FILES_START_Y - 1, ">>");
  }
  if (pos == 0)
  {
    // We're on the first page, clear the line with the "Previous Page" text.
  }
}

void screen_select_file_prev(void)
{
  if (pos > 0)
  {
    cputsxy(1, FILES_START_Y - 1, "<<");
  }

  if (dir_eof == true)
  {
    // We're on the last page, clear the line with the "Next Page" text.
//    screen_clear_line(FILES_START_Y + ENTRIES_PER_PAGE + ADDTL_ENTRIES);
  }
}

void screen_select_file_display_entry(unsigned char y, const char *e, unsigned)
{

/*
  if (e[strlen(e)-1]=='/')
    cputsxy(0,FILES_START_Y+y,CH_FOLDER);
  else if (e[0]=='=')
    cputsxy(0,FILES_START_Y+y,CH_SERVER);
  else
  */

  if (e[strlen(e)-1]=='/')
  {
    cputsxy(0,FILES_START_Y+y,CH_FOLDER);
  }
  else if (e[0]=='=')
  {
    cputsxy(0,FILES_START_Y+y,CH_SERVER);
  }

  cputsxy(3, FILES_START_Y + y, e);

}

void screen_select_file_choose(char visibleEntries)
{
  bar_show(FILES_START_Y);
  _visibleEntries = visibleEntries;
}

void screen_select_file_new_type(void)
{
  // Not used on Atari
}

void screen_select_file_new_size(unsigned char)
{
  screen_clear_line(20);
  screen_clear_line(21);

  cputsxy(0, 20, "Size?" CH_KEY_LABEL_L CH_INV_1 CH_KEY_LABEL_R "90K  " CH_KEY_LABEL_L CH_INV_2 CH_KEY_LABEL_R "130K  " CH_KEY_LABEL_L CH_INV_3 CH_KEY_LABEL_R "180K  " CH_KEY_LABEL_L CH_INV_4 CH_KEY_LABEL_R "360K  ");
  cputsxy(5, 21,
              CH_KEY_LABEL_L CH_INV_5 CH_KEY_LABEL_R "720K " CH_KEY_LABEL_L CH_INV_6 CH_KEY_LABEL_R "1440K " CH_KEY_LABEL_L CH_INV_7 CH_KEY_LABEL_R "Custom ?");
}

void screen_select_file_new_custom(void)
{
  screen_clear_line(20);
  screen_clear_line(21);

  cputsxy(0, 20, "# Sectors?");
  cputsxy(0, 21, "Sector Size (128/256/512)?");
}

void screen_select_file_new_name(void)
{
  screen_clear_line(20);
  screen_clear_line(21);
  cputsxy(0, 20, "Enter name of new disk image file");
}

void screen_select_file_new_creating(void)
{
  screen_clear();
  cputsxy(3, 0, "Creating File");
}

void screen_clear_line(unsigned char y)
{
cclearxy(0,y,40);
//  set_cursor(0, y);          // move the cursor X position 0 and Y position.
//  memset(cursor_ptr, 0, 40); // fill the memory with spaces.  The zeros in the memory addresses are interpreted as spaces on the console
}

void screen_error(const char *msg)
{
  screen_clear_line(24);
  cputsxy(0, 24, msg);
}

void screen_hosts_and_devices(HostSlot *, DeviceSlot *, unsigned char *)
{
  set_active_screen(SCREEN_HOSTS_AND_DEVICES);

  screen_clear();
  bar_clear(false);


  cputsxy(5, 0, "HOST LIST");
  cputsxy(4, 11, "DRIVE SLOTS");

  screen_hosts_and_devices_host_slots(&hostSlots[0]);

  screen_hosts_and_devices_device_slots(DEVICES_START_Y, &deviceSlots[0], (bool *) "");
}

void screen_clear()
{
clrscr();
  //cursor_ptr = video_ptr;                       // Assign the current position of the cursor position address to the address of the video screen
  //memset(video_ptr, 0, GRAPHICS_0_SCREEN_SIZE); // Fill the memory address with blanks of the size of the screen; in this case Atari graphics mode 0 size.
}

// Show the keys that are applicable when we are on the Hosts portion of the screen.
void screen_hosts_and_devices_hosts(void)
{
  screen_clear_line(22);
  screen_clear_line(23);
  cputsxy(0, 22,
              CH_KEY_1TO8 "Slot" CH_KEY_LABEL_L CH_INV_E CH_KEY_LABEL_R "dit" CH_KEY_RETURN "Browse" CH_KEY_LABEL_L CH_INV_L CH_KEY_LABEL_R "obby");
  cputsxy(2, 23,
              CH_KEY_LABEL_L CH_INV_C CH_KEY_LABEL_R "onfig" CH_KEY_TAB "Drive Slots" CH_KEY_OPTION "Boot");

  // bar_show(2);
  bar_show(selected_host_slot + HOSTS_START_Y);
}

// Show the keys that are applicable when we are on the Devices portion of the screen.
void screen_hosts_and_devices_devices(void)
{
  screen_clear_line(22);
  screen_clear_line(23);

  screen_clear_line(11);
  cputsxy(4, 11, "DRIVE SLOTS");

  cputsxy(0, 22,
              CH_KEY_1TO8 "Slot" CH_KEY_LABEL_L CH_INV_E CH_KEY_LABEL_R "ject" CH_KEY_LABEL_L CH_INV_C CH_INV_L CH_INV_E CH_INV_A CH_INV_R CH_KEY_LABEL_R "All Slots" CH_KEY_LABEL_L CH_INV_L CH_KEY_LABEL_R "obby");
  cputsxy(3, 23,
              CH_KEY_TAB "Hosts" CH_KEY_LABEL_L CH_INV_R CH_KEY_LABEL_R "ead " CH_KEY_LABEL_L CH_INV_W CH_KEY_LABEL_R "rite" CH_KEY_LABEL_L CH_INV_C CH_KEY_LABEL_R "onfig");
  bar_show(selected_device_slot + DEVICES_START_Y);
}

void screen_hosts_and_devices_host_slots(HostSlot *)
{
  unsigned char slotNum;

  for (slotNum = 0; slotNum < NUM_HOST_SLOTS; slotNum++)
  {
    gotoxy(2, slotNum + HOSTS_START_Y);
    cputc(slotNum + '1');
    gotoxy(5, slotNum + HOSTS_START_Y);
    //cursor_ptr += 2;

    cputs((hostSlots[slotNum][0] != 0x00) ? (char *)hostSlots[slotNum] : text_empty);
  }
}

// Since 'deviceSlots' is a global, do we need to access the input parameter at all?
// Maybe globals.h wasn't supposed in be part of screen? I needed it for something..
void screen_hosts_and_devices_device_slots(uint8_t y, DeviceSlot *, const bool *)
{
  unsigned char slotNum;
  unsigned char dinfo[6];

  // Get full filename for device slot 8
  if (deviceSlots[7].file[0] != 0x00)
    fuji_get_device_filename(7, fn);

  // Display device slots
  for (slotNum = 0; slotNum < NUM_DEVICE_SLOTS; slotNum++)
  {
    dinfo[1] = 0x20;
    dinfo[2] = (slotNum == 7 && strstr(fn, ".cas") != NULL) ? 'C' : (0x31 + slotNum);
    dinfo[4] = 0x20;
    dinfo[5] = 0x00;

    if (deviceSlots[slotNum].file[0] != 0x00)
    {
      dinfo[0] = deviceSlots[slotNum].hostSlot + 0x31;
      dinfo[3] = (deviceSlots[slotNum].mode == 0x02 ? 'W' : 'R');
    }
    else
    {
      dinfo[0] = 0x20;
      dinfo[3] = 0x20;
    }

    cputsxy(0, slotNum + y, (char *) dinfo);

    cputs(deviceSlots[slotNum].file[0] != 0x00 ? (char *)deviceSlots[slotNum].file : text_empty);
    //screen_append(deviceSlots[slotNum].file[0] != 0x00 ? (char *)deviceSlots[slotNum].file : text_empty);
  }
}

void screen_hosts_and_devices_devices_clear_all(void)
{
  screen_clear_line(11);
  cputsxy(0, 11, "EJECTING ALL.. WAIT");
}

void screen_hosts_and_devices_clear_host_slot(unsigned char)
{
  // nothing to do, edit_line handles clearing correct space on screen, and doesn't touch the list numbers
}

void screen_hosts_and_devices_edit_host_slot(unsigned char)
{
  // nothing to do, edit_line handles clearing correct space on screen, and doesn't touch the list numbers
}

void screen_hosts_and_devices_eject(unsigned char ds)
{
  char tmp[2] = {0, 0};
  unsigned char y;

  tmp[0] = ds + '1';

  if ( mounting )
  {
    y = DEVICES_START_MOUNT_Y;
  }
  else
  {
    y = DEVICES_START_Y;
  }
  bar_show(y + ds);
  screen_clear_line(y + ds);
  cputsxy(2, y + ds, tmp);
  cputsxy(5, y + ds, text_empty);
}

void screen_hosts_and_devices_host_slot_empty(unsigned char)
{
  // When this gets called it seems like the cursor is right where we want it to be.
  // so no need to move to a position first.
//screen_append(text_empty);
cputs(text_empty);
}

void screen_hosts_and_devices_long_filename(const char *)
{
  // show_line_nums();
}

void screen_init(void)
{
  //OS.noclik = 0xFF;
  //OS.shflok = 0;
  //OS.color0 = 0x9f;
  //OS.color1 = 0x0f;
  //OS.color2 = 0x90;
  //OS.color4 = 0x90;
  //OS.coldst = 1;
  //OS.sdmctl = 0; // Turn off screen

  //memcpy((void *)DISPLAY_LIST, &config_dlist, sizeof(config_dlist)); // copy display list to $0600
  //OS.sdlst = (void *)DISPLAY_LIST;                                   // and use it.
  //video_ptr = (unsigned char *)(DISPLAY_MEMORY);                     // assign the value of DISPLAY_MEMORY to video_ptr

  font_init();
  bar_setup_regs();
  bar_clear(false);

  selected_host_slot = 0;
  selected_device_slot = 0;
}

void screen_destination_host_slot(char *h, char *p)
{
  screen_clear();
  cputsxy(0, 22,
              CH_KEY_1TO8 "Slot" CH_KEY_RETURN "Select" CH_KEY_ESC "Abort");

  cputsxy(0, 18, h);
  cputsxy(0, 19, p);

  cputsxy(0, 0, "COPY TO HOST SLOT");
  bar_show(HOSTS_START_Y);
}

void screen_destination_host_slot_choose(void)
{
  // show_line_nums();
}

void screen_perform_copy(char *, char *, char *, char *)
{
  // show_line_nums();
  screen_clear();
  bar_clear(false);
  cputsxy(0, 0, "COPYING, PLEASE WAIT");
}

void screen_connect_wifi(NetConfig *nc)
{
  set_active_screen(SCREEN_CONNECT_WIFI);
  screen_clear();
  bar_clear(false);

  cputsxy(0, 0, "WELCOME TO FUJINET! CONNECTING TO NET");
  cputsxy(2, 3, nc->ssid);
  bar_show(3);
}


void screen_end(void)
{
  return;
}

#ifdef DEBUG
// Debugging function to show line #'s, used to test if the Y coordinate calculations are working.
void show_line_nums(void)
{
  cputsxy(0, 0, "0");
  cputsxy(0, 1, "1");
  cputsxy(0, 2, "2");
  cputsxy(0, 3, "3");
  cputsxy(0, 4, "4");
  cputsxy(0, 5, "5");
  cputsxy(0, 6, "6");
  cputsxy(0, 7, "7");
  cputsxy(0, 8, "8");
  cputsxy(0, 9, "9");
  cputsxy(0, 10, "10");
  cputsxy(0, 11, "11");
  cputsxy(0, 12, "12");
  cputsxy(0, 13, "13");
  cputsxy(0, 14, "14");
  cputsxy(0, 15, "15");
  cputsxy(0, 16, "16");
  cputsxy(0, 17, "17");
  cputsxy(0, 18, "18");
  cputsxy(0, 19, "19");
  cputsxy(0, 20, "20");
  cputsxy(0, 21, "21");
  cputsxy(0, 22, "22");
  cputsxy(0, 23, "23");
  cputsxy(0, 24, "24");
  cputsxy(0, 25, "25");

  while (!kbhit())
  {
  }
  cgetc();
  screen_clear();
}

void screen_debug(char *message)
{
  screen_clear_line(24);
  cputsxy(0, 24, message);
}
#endif //DEBUG
#endif
