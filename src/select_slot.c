/**
 * FujiNet for #Adam configuration program
 *
 * Select a Destination Device Slot
 */

#include <stdlib.h>
#include <string.h>
#include "select_slot.h"

#ifdef BUILD_ADAM
#include <eos.h>
#include <conio.h>
#include "adam/screen.h"
#include "adam/input.h"
#include "adam/globals.h"
#include "adam/io.h"
#include "adam/bar.h"
#endif /* BUILD_ADAM */

#ifdef BUILD_APPLE2
#include "apple2/screen.h"
#include "apple2/input.h"
#include "apple2/globals.h"
#include "apple2/io.h"
#include "apple2/bar.h"
#endif /* BUILD_APPLE2 */

#ifdef BUILD_C64
#include "c64/screen.h"
#include "c64/input.h"
#include "c64/globals.h"
#include "c64/io.h"
#include "c64/bar.h"
#endif /* BUILD_APPLE2 */

extern DeviceSlot deviceSlots[8];
extern bool quick_boot;

static enum
  {
   SF_INIT,
   SF_DISPLAY,
   SF_CHOOSE,
   SF_DONE,
   SF_ABORT
  } subState;

static char mode=0;
static char selected_device_slot=0;

bool create=false;

void select_slot_init()
{
  if (quick_boot==true)
    {
      mode=0;
      selected_device_slot=0;
      subState=SF_DONE;
    }
  else
    subState=SF_DISPLAY;
}

void select_slot_display()
{
  if (create==true)
    {
      char dispPath[42];
      memset(dispPath,0,42);
      strncpy(&dispPath[11],path,31);
      screen_select_slot(dispPath);
    }
  else
    {
      io_open_directory(selected_host_slot,path,filter);
      
      io_set_directory_position(pos);
      
      io_get_device_slots(&deviceSlots[0]);
      
      screen_select_slot(io_read_directory(42,0x80));
      
      io_close_directory();
    }
  
  subState=SF_CHOOSE;
}

void select_slot_eject(unsigned char ds)
{
  io_umount_disk_image(ds);
  memset(deviceSlots[ds].file,0,FILE_MAXLEN);
  deviceSlots[ds].hostSlot=0xFF;
  io_put_device_slots(&deviceSlots[0]);
  screen_select_slot_eject(ds);
}

void select_slot_choose()
{
  char k;
  
  screen_select_slot_choose();

  while (subState==SF_CHOOSE)
    {
      k=input();
      switch(k)
	{
	case 0x1B:
	  subState=SF_ABORT;
	  state=HOSTS_AND_DEVICES;
	  break;
	case '1':
	case '2':
	case '3':
	case '4':
	  bar_jump(k-0x31);
	  break;
	case 0x84:
	  select_slot_eject(bar_get());
	  break;
	case 0x0d:
	case 0x85:
	  selected_device_slot=bar_get();
	  mode=0;
	  subState=SF_DONE;
	  break;
	case 0x86:
	  selected_device_slot=bar_get();
	  mode=2;
	  subState=SF_DONE;
	case 0xA0:
	  bar_up();
	  break;
	case 0xA2:
	  bar_down();
	  break;
	}
    }
}

void select_slot_done()
{
  char filename[256];

  if (create==true)
    {
      create=false; // we're done with this until next time.
      screen_select_file_new_creating();
      io_create_new(selected_host_slot,selected_device_slot,selected_size,path);
      memcpy(deviceSlots[selected_device_slot].file,path,31);
      deviceSlots[selected_device_slot].mode=2;
      deviceSlots[selected_device_slot].hostSlot=selected_host_slot;
     
      io_put_device_slots(&deviceSlots[0]);
      io_set_device_filename(selected_device_slot,path);
#ifdef BUILD_ADAM
      screen_select_slot_build_eos_directory();
      if (input_select_slot_build_eos_directory())
	{
	  io_mount_disk_image(selected_device_slot,2); // R/W
	  screen_select_slot_build_eos_directory_label();
	  input_select_slot_build_eos_directory_label(filename);
	  screen_select_slot_build_eos_directory_creating();
	  io_build_directory(selected_device_slot,selected_size,filename);
	}
#endif
    }
  else
    {    
      strcat(filename,path);
      
      io_open_directory(selected_host_slot,path,filter);
      
      io_set_directory_position(pos);
      
      memcpy(deviceSlots[selected_device_slot].file,io_read_directory(31,0),31);
      deviceSlots[selected_device_slot].mode=mode;
      deviceSlots[selected_device_slot].hostSlot=selected_host_slot;
      
      io_put_device_slots(&deviceSlots[0]);
      
      io_set_directory_position(pos);
      
      strcat(filename,io_read_directory(255,0));
      
      io_set_device_filename(selected_device_slot,filename);
      
      io_close_directory();
      
    }
  state=HOSTS_AND_DEVICES;
}

void select_slot(void)
{
  subState=SF_INIT;
  
  while (state==SELECT_SLOT)
    {
      switch(subState)
	{
	case SF_INIT:
	  select_slot_init();
	  break;
	case SF_DISPLAY:
	  select_slot_display();
	  break;
	case SF_CHOOSE:
	  select_slot_choose();
	  break;
	case SF_DONE:
	  select_slot_done();
	case SF_ABORT:
	  break;
	}
    }
}
