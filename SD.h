/*Deklaracje funkcji ubslugujacych operacje na karcie SD*/
#include "SPI.c"
#include "LCD.c"
#include "diskio.h"

#define CMD0	(0)			/* IDZ DO STANU BEZCZYNNOSCI */
#define CMD1	(1)			/* SEND_OP_COND (MMC) */
#define CMD8	(8)			/* SEND_IF_COND */
#define CMD9	(9)			/* SEND_CSD */
#define CMD10	(10)		/* SEND_CID */
#define CMD12	(12)		/* STOP_TRANSMISSION */
#define ACMD13	(0x80+13)	/* SD_STATUS (SDC) */
#define CMD16	(16)		/* SET_BLOCKLEN */
#define CMD17	(17)		/* READ_SINGLE_BLOCK */
#define CMD18	(18)		/* READ_MULTIPLE_BLOCK */
#define CMD23	(23)		/* SET_BLOCK_COUNT (MMC) */
#define	ACMD23	(0x80+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(24)		/* WRITE_BLOCK */
#define CMD25	(25)		/* WRITE_MULTIPLE_BLOCK */
#define CMD32	(32)		/* ERASE_ER_BLK_START */
#define CMD33	(33)		/* ERASE_ER_BLK_END */
#define CMD38	(38)		/* ERASE */
#define	ACMD41	(0x80+41)	/* SEND_OP_COND (SDC) */
#define	CMD48	(48)		/* READ_EXTR_SINGLE */
#define	CMD49	(49)		/* WRITE_EXTR_SINGLE */
#define CMD55	(55)		/* APP_CMD */
#define CMD58	(58)		/* READ_OCR */

BYTE SD_wyslij_komende(BYTE cmd, DWORD argument);
DSTATUS SD_disk_status (void);
static int SD_odbierz_blok_danych(BYTE *bufor,	UINT bajty);
void SD_cofnij_wybor (void);
static int SD_wybierz(void);
static int SD_czekaj_na_gotowosc(UINT czekaj);
DRESULT SD_disk_ioctl(BYTE kod_kontrolny, void *bufor);