#ifdef BUILD_ATARI

#include "mount_and_boot.h"
#include "io.h"
#include "globals.h"
#include "screen.h"
#include "../die.h"
#include "atari_die.h"
#include <conio.h>

void mount_and_boot_lobby(void)
{
    screen_mount_and_boot();
    screen_clear();
    cputsxy(3, 0, "Booting Lobby");


//    if ( !io_mount_all() )
//    {
//        screen_error("ERROR MOUNTING ALL");
//        wait_a_moment();
//        state = HOSTS_AND_DEVICES;
//    }
//    else
    {
        while (!kbhit())
        {
        }
        cgetc();

        io_set_boot_mode(2);
        cold_start();
    }
}

void mount_and_boot(void)
{
    screen_mount_and_boot();

    io_get_device_slots(&deviceSlots[0]);
    if (io_error())
    {
        screen_error("ERROR READING DEVICE SLOTS");
        die();
    }

    io_get_host_slots(&hostSlots[0]);
    if (io_error())
    {
        screen_error("ERROR READING HOST SLOTS");
        die();
    }

    screen_clear();
    cputsxy(3, 0, "MOUNT AND BOOT");

    cputsxy(5, 0, "Mounting all Host and Device Slots");

    if ( !io_mount_all() )
    {
        screen_error("ERROR MOUNTING ALL");
        wait_a_moment();
        state = HOSTS_AND_DEVICES;
    }
    else
    {
        cputsxy(9, 22, "SUCCESSFUL! BOOTING");
        io_set_boot_config(0);
        cold_start();
    }

}

#endif
