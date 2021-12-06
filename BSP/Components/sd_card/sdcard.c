/************************
 * SD Card Functions
 ************************/

#include "sdcard.h"
#include "fatfs.h"
#include "util_console.h"

#define DEBUG_SD 1
#define PRINT_SD_CARD(X) do{ if(DEBUG_SD>0) { X } }while(0);

static FRESULT res; 						/* FatFs function common result code */
static uint32_t byteswritten, bytesread; 	/* File write/read counts */


int size_list(FIL File){
	char frase_f[10] = { 0 };
	int index = 0;
	while(f_gets(frase_f, bytesread, &File) != 0){
		index++;
	}
	return index;
}

void mount_sd_card(){
	if(f_mount(&SDFatFS, (const TCHAR *)&SDPath, 1) != FR_OK)
	  {
		  // TODO Acionar flag ou alerta de ausencia de cartão ou erro de montagem
		  PRINT_SD_CARD(PRINTF("Erro ao montar o cartao\r\n");)
		 // Error_Handler();
	  }
}

void remove_and_open_again(const char* arq){

	res = f_unlink(arq);
	if(res == FR_LOCKED){
		f_close(&SDFile); 		// Fecha
		f_unlink(arq);			// Depois apaga
	}
	else if(res == FR_NO_FILE){
		return; 	// Não há nem arquivo existente com o nome informado
	}

	if(f_open(&SDFile, arq, FA_OPEN_APPEND | FA_READ | FA_WRITE) != FR_OK)
	{
		// TODO Imprimir os erros e tratar na uart
	  Error_Handler();
	}

	f_sync(&SDFile);
}

void verify_open(const char* arq){
	res = f_open(&SDFile, arq, FA_OPEN_APPEND | FA_READ | FA_WRITE) != FR_OK;
	if(res == FR_OK){
		PRINT_SD_CARD(PRINTF("FR_OK \n");)
		return;
	}
	else if(res == FR_LOCKED){
		PRINT_SD_CARD(PRINTF("FR_LOCKED \n");)
		return;
	}
	else if(res == FR_DISK_ERR){
		PRINT_SD_CARD(PRINTF("FR_DISK_ERR \n");)
		return;
	}
	else{
		PRINT_SD_CARD(PRINTF("Error to open the log file on the SD Card \n Reset the board \n");)
		return;
	}
}

void save_on_card(){
	//delayed_store_flag++; 	// Contagem de TAGs atrasadas ao envio

	// Se não há conexão entre o gateway, armazena no cartão SD para envio posterior
//	PRINT_SD_CARD(PRINTF("===> Escrita no cartao. Count = %d\r\n", delayed_store_flag);)
//	f_write(&SDFile, store_TAG[last_TAG].N_TAG, sizeof(store_TAG[last_TAG].N_TAG), (void *)&byteswritten);
//	f_sync(&SDFile);	// Um ou outro
//	f_close(&SDFile);
}

void remove_from_card(){
	// Remove do cartão SD e armazena estrutura para envio da Lora
	//TODO Generalizar a função colocando um argumento para receber o dado que estava no cartão

//	f_gets(buffer_tag, bytesread, &SDFile);
//	memcpy(tag_to_lora.N_TAG, buffer_tag, sizeof(buffer_tag));
//	delayed_store_flag--;
//	PRINT_SD_CARD(PRINTF("===> Removida do cartão. Count = %d\r\n", delayed_store_flag);)
}

/************* End of Sd card functions *****************/
