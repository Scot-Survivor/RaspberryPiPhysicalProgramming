/*
 * main.c
 *	Origin: 
 * 		From:	https://github.com/weiche/RC522 * 	
 * This:
 * 		Project			: RFID-MFRC522
 * 		Description		: Use MFRC522 Read and Write Mifare Card.
 * 		Author			: www.freenove.com
 * 		Date			: 2019/03/26
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include "mfrc522.h"
#define DISP_COMMANDLINE()	printf("RC522>")

#define MAX_LEN 16

int scan_loop(uint8_t *CardID, uint8_t *SectorKeyA, bool crack_mode);
int tag_select(uint8_t *CardID);

int main(int argc, char **argv) {
    setbuf(stdout, 0);
	MFRC522_Status_t ret;
	//Recognized card ID
	uint8_t CardID[5] = { 0x00, };
	uint8_t tagType[16] = {0x00,};
	static char command_buffer[1024];

	ret = MFRC522_Init('B');
	if (ret < 0) {
		printf("Failed to initialize.\r\nProgram exit.\r\n");
		exit(-1);
	}

	printf("User Space RC522 Application\r\n");
    bool crack_mode = false;
	while (1) {
		/*Main Loop Start*/
		DISP_COMMANDLINE();
        uint8_t SectorKeyA[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
		scanf("%s", command_buffer);
		if (strcmp(command_buffer, "scan") == 0) {
			puts("Scanning ... ");
			while (1) {
				ret = MFRC522_Request(PICC_REQIDL, tagType);
				if (ret == MI_OK) {
					printf("Card detected!\r\n");	
					ret = MFRC522_Anticoll(CardID);	
					if(ret == MI_OK){
						ret = tag_select(CardID);
						if (ret == MI_OK) {
							ret = scan_loop(CardID, SectorKeyA, crack_mode);
							if (ret < 0) {
								printf("Card error...\r\n");
								break;
							} else if (ret == 1) {
								puts("Halt...\r\n");
								break;
							}
						}
					}
					else{
						printf("Get Card ID failed!\r\n");
					}								
				}
				MFRC522_Halt();
			}
			MFRC522_Halt();
			MFRC522_Init('B');
		} else if (strcmp(command_buffer, "quit") == 0
				|| strcmp(command_buffer, "exit") == 0) {
			return 0;
		} else if (strcmp(command_buffer, "crack") == 0) {
            printf("Changing crack mode...\r\n");
            crack_mode = !crack_mode;
            printf("Crack mode: %s\r\n", crack_mode ? "ON" : "OFF");
        } else {
			puts("Unknown command");
			puts("scan:scan card and dump");
			puts("quit:exit program");
		}
		/*Main Loop End*/
	}
}
int scan_loop(uint8_t *CardID, uint8_t *SectorKeyA, bool crack_mode) {

	while (1) {

		char input[32];
		int block_start;
		DISP_COMMANDLINE();
		printf("%02X%02X%02X%02X>", CardID[0], CardID[1], CardID[2], CardID[3]);
		scanf("%s", input);
		puts((char*)input);
		if (strcmp(input, "halt") == 0) {
			MFRC522_Halt();
			return 1;
		} else if (strcmp(input, "crack") == 0) {
            printf("Changing crack mode...\r\n");
            crack_mode = !crack_mode;
            printf("Crack mode: %s\r\n", crack_mode ? "ON" : "OFF");
        } else if (strcmp(input, "brute") == 0) {
            printf("Brute force mode...\r\n");
            crack_mode = true;
            printf("Crack mode: %s\r\n", crack_mode ? "ON" : "OFF");
            int tries = 0;
            time_t start_time = time(NULL);
            for (int i = 0; i < 16; i++) {
                for (int j = 0; j < 16; j++) {
                    for (int k = 0; k < 16; k++) {
                        for (int l = 0; l < 16; l++) {
                            uint8_t new_key[6] = { i, j, k, l};
                            memcpy(SectorKeyA, new_key, 1+sizeof(&SectorKeyA)/sizeof(uint8_t));
                            tries++;
                            if (MFRC522_Auth(PICC_AUTHENT1A, 0, SectorKeyA, CardID) == MI_OK) {
                                printf("Key: %02X%02X%02X%02X%02X\r\n", SectorKeyA[0], SectorKeyA[1], SectorKeyA[2], SectorKeyA[3], SectorKeyA[4]);
                                return 0;
                            }
                            else {
                                time_t current_time = time(NULL);
                                double iterations_second;
                                iterations_second = (float)tries / (float)(current_time - start_time);
                                double remaining_time;
                                remaining_time = (float)((16*16*16*16) - tries) / iterations_second;
                                printf("\rCompleted %3d/%3d: %.2f iterations/s. ETA: %.2f seconds", tries, (int)floor(pow(16, 4)), iterations_second, remaining_time);
                                fflush(stdout);
                            }
                        }
                    }
                }
            }

        } else if (strcmp(input, "key") == 0) {
            printf("Current KeyA: %02X%02X%02X%02X%02X\r\n", SectorKeyA[0], SectorKeyA[1], SectorKeyA[2], SectorKeyA[3], SectorKeyA[4]);
            uint8_t *new_key = (uint8_t*)malloc(sizeof(&SectorKeyA)/sizeof(uint8_t));
            printf("Enter new Sector Key: ");
            scanf("%x %x %x %x %x", &new_key[0], &new_key[1], &new_key[2], &new_key[3], &new_key[4]);
            memcpy(SectorKeyA, new_key, 1+sizeof(&SectorKeyA)/sizeof(uint8_t));
            printf("New KeyA: %02X%02X%02X%02X%02X\r\n", SectorKeyA[0], SectorKeyA[1], SectorKeyA[2], SectorKeyA[3], SectorKeyA[4]);
        } else if (strcmp(input, "dump") == 0) {
			if (MFRC522_Debug_CardDump(CardID, SectorKeyA) < 0)
                if (!crack_mode) {
                    return -1;
                }
		} else if (strcmp(input, "read") == 0) {
			scanf("%d", &block_start);
			if (MFRC522_Debug_DumpSector(CardID, block_start, SectorKeyA) < 0) {
                if (!crack_mode) {
                    return -1;
                }
			}
		} else if(strcmp(input, "clean") == 0){
			char c;
			scanf("%d", &block_start);
			while ((c = getchar()) != '\n' && c != EOF)
				;
			if (MFRC522_Debug_Clean(CardID, block_start)) {
                if (!crack_mode) {
                    return -1;
                }
			}
			
		} else if (strcmp(input, "write") == 0) {
			char write_buffer[MAX_LEN*4] = {};
			size_t len = 0;
			scanf("%d ", &block_start);
			fgets(write_buffer, sizeof(write_buffer), stdin);
			if (len >= 0) {
				if (MFRC522_Debug_Write(CardID, block_start, write_buffer,
						strlen(write_buffer)) < 0) {
                    if (!crack_mode) {
                        return -1;
                    }
				}
			}
		} else {

			printf(
					"Usage:\r\n" "\tread <blockstart>\r\n" "\tdump\r\n" "\thalt\r\n" 
						"\tclean <blockaddr>\r\n" "\twrite <blockaddr> <data>\r\n" "\tkey\r\n" "\tcrack\r\n");
			//return 0;
		}
	}
	return 0;

}
int tag_select(uint8_t *CardID) {
	int ret_int;
	printf(
			"Card UID: 0x%02X 0x%02X 0x%02X 0x%02X, Check Sum = 0x%02X\r\n",
			CardID[0], CardID[1], CardID[2], CardID[3], CardID[4]);
	ret_int = MFRC522_SelectTag(CardID);
	if (ret_int == 0) {
		printf("Card Select Failed\r\n");
		return -1;
	} else {
		printf("Card Selected, Type:%s\r\n",
				MFRC522_TypeToString(MFRC522_ParseType(ret_int)));
	}
	ret_int = 0;
	return ret_int;
}
