#include "../screen.h"

#define GRAPHICS_0_SCREEN_SIZE (40 * 25) // Defines the memory size in bytes
#define DISPLAY_LIST 0x0600              // Memory address to store DISPLAY_LIST.  0x0600 is the first address available for user space memory (1)
#define DISPLAY_MEMORY 0x7400            // Memory address to store DISPLAY_MEMORY.
#define FONT_MEMORY 0x7800

#define CH_FOLDER "\x04"      // Set the character folder to #
#define CH_SERVER "\x06"      // Set the server folder to atari heart.
#define CH_KEY_LABEL_L "\x99" // Left arrow on the keyboard
#define CH_KEY_LABEL_R "\x19" // Right arrow on the keyboard

// Inverse of internal character codes
#define CH_INV_MINUS "\xAd"
#define CH_INV_LT "\xBC"
#define CH_INV_GT "\xBE"

#define CH_INV_A "\xC1"
#define CH_INV_B "\xC2"
#define CH_INV_C "\xC3"
#define CH_INV_D "\xC4"
#define CH_INV_E "\xC5"
#define CH_INV_F "\xC6"
#define CH_INV_G "\xC7"
#define CH_INV_H "\xC8"
#define CH_INV_I "\xC9"
#define CH_INV_J "\xCA"
#define CH_INV_K "\xCB"
#define CH_INV_L "\xCC"
#define CH_INV_M "\xCD"
#define CH_INV_N "\xCE"
#define CH_INV_O "\xCF"
#define CH_INV_P "\xD0"
#define CH_INV_Q "\xD1"
#define CH_INV_R "\xD2"
#define CH_INV_S "\xD3"
#define CH_INV_T "\xD4"
#define CH_INV_U "\xD5"
#define CH_INV_V "\xD6"
#define CH_INV_W "\xD7"
#define CH_INV_X "\xD8"
#define CH_INV_Y "\xD9"
#define CH_INV_Z "\xDA"

#define CH_INV_1 "\xB1"
#define CH_INV_2 "\xB2"
#define CH_INV_3 "\xB3"
#define CH_INV_4 "\xB4"
#define CH_INV_5 "\xB5"
#define CH_INV_6 "\xB6"
#define CH_INV_7 "\xB7"
#define CH_INV_8 "\xB8"
#define CH_INV_9 "\xB9"
#define CH_INV_LEFT "\x9E"
#define CH_INV_RIGHT "\x9F"

#define CH_KEY_1TO8 CH_KEY_LABEL_L CH_INV_1 CH_INV_MINUS CH_INV_8 CH_KEY_LABEL_R
#define CH_KEY_ESC CH_KEY_LABEL_L CH_INV_E CH_INV_S CH_INV_C CH_KEY_LABEL_R
#define CH_KEY_TAB CH_KEY_LABEL_L CH_INV_T CH_INV_A CH_INV_B CH_KEY_LABEL_R
#define CH_KEY_DELETE CH_KEY_LABEL_L CH_INV_D CH_INV_E CH_INV_L CH_INV_E CH_INV_T CH_INV_E CH_KEY_LABEL_R
#define CH_KEY_LEFT CH_KEY_LABEL_L CH_INV_LEFT CH_KEY_LABEL_R
#define CH_KEY_RIGHT CH_KEY_LABEL_L CH_INV_RIGHT CH_KEY_LABEL_R
#define CH_KEY_RETURN CH_KEY_LABEL_L CH_INV_R CH_INV_E CH_INV_T CH_INV_U CH_INV_R CH_INV_N CH_KEY_LABEL_R
#define CH_KEY_OPTION CH_KEY_LABEL_L CH_INV_O CH_INV_P CH_INV_T CH_INV_I CH_INV_O CH_INV_N CH_KEY_LABEL_R
#define CH_KEY_C CH_KEY_LABEL_L CH_INV_C CH_KEY_LABEL_R
#define CH_KEY_N CH_KEY_LABEL_L CH_INV_N CH_KEY_LABEL_R
#define CH_KEY_F CH_KEY_LABEL_L CH_INV_F CH_KEY_LABEL_R
#define CH_KEY_LT CH_KEY_LABEL_L CH_INV_LT CH_KEY_LABEL_R
#define CH_KEY_GT CH_KEY_LABEL_L CH_INV_GT CH_KEY_LABEL_R
#define CH_KEY_ESC CH_KEY_LABEL_L CH_INV_E CH_INV_S CH_INV_C CH_KEY_LABEL_R

#define HORIZONTAL_LINE "\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12\x12"

extern void bar_setup_regs();
void fastcall bar_show(unsigned char y);
