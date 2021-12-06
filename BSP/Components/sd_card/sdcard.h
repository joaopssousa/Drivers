#ifndef __SDCARD_H
#define __SDCARD_H


#define SD_DET_CARD_Pin GPIO_PIN_7
#define SD_DET_CARD_GPIO_Port GPIOC

/************************* SD CArd Function prototypes *****************************************/
void mount_sd_card();
void remove_and_open_again(const char* arq);
void verify_open(const char* arq);
void save_on_card();
void remove_from_card();
/***********************************************************************************************/

#endif
