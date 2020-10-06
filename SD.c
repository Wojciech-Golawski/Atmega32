/*Funkcje obslugujace podstawowe operacje na karcie SD, wykorzystano magistrale SPI*/
#include "SD.h"
#include "diskio.h"

static volatile DSTATUS Stat = STA_NOINIT;	//Status dysku
static BYTE CardType;

static volatile BYTE Timer1;
static volatile UINT Timer2;

char bufor[33]={"  "};	

DSTATUS SD_inicjalizacja(void)
{
	BYTE licznik_1=0, ocr[4], typ;
	typ=0;
	
	if(SD_disk_status() & STA_NODISK) return Stat;
	
	SPI_inicjalizacja();
	//for (Timer1 = 10; Timer1; );	// Oczekiwanie 100ms na ustabilizowanie sie zailania
	
	for(licznik_1=0; licznik_1<10; licznik_1++)	
	{
		SPI_wyslij_dane(0xFF);							//wyslanie 80 taktow zegara aby odpalic procesor karty SD
	}
	
	PORTB &= ~(1<<CS);
	if(SD_wyslij_komende(CMD0, 0)==1);					//CMD0
	{
		//strncpy(bufor, "CMD 0           \n               ", 33);
		//_delay_ms(1000);
		SPI_wyslij_dane(0xFF);
		if(SD_wyslij_komende(CMD8, 0x1AA)==1)				//CMD8
		{
			for(licznik_1=0; licznik_1<4; licznik_1++)
			{
				ocr[licznik_1]=SPI_wyslij_dane(0xFF);
			}
			if (ocr[2] == 0x01 && ocr[3] == 0xAA)
			{
// 				LCD_napis("CMD 8");
// 				_delay_ms(1000);
// 				LCD_komenda(0x01);
				while(SD_wyslij_komende(ACMD41, 1UL << 30));	//ACMD41
				
// 				LCD_komenda(0x01);
// 				LCD_napis("ACMD 41");
// 				_delay_ms(1000);
// 				LCD_komenda(0x01);
				SPI_wyslij_dane(0xFF);
				if(SD_wyslij_komende(CMD58, 0) == 0)	//CMD58
				{
					for(licznik_1=0; licznik_1<4; licznik_1++)
					{
						ocr[licznik_1] = SPI_wyslij_dane(0xFF);
					}
					if(ocr[0] & 0x40)
					{
						typ = CT_SD2 | CT_BLOCK; 
// 						LCD_komenda(0x01);
// 						LCD_napis("Karta typu SDHC");
// 						_delay_ms(1000);
// 						LCD_komenda(0x01);
					}
					else
					{
						typ = CT_SD2;
						LCD_komenda(0x01);
						LCD_napis("Karta typu SDXC");
						_delay_ms(1000);
						LCD_komenda(0x01);
					}
				}
			}
		}
	}
	CardType=typ;
	//SD_cofnij_wybor();
	if(typ) 
	{
		Stat &= ~STA_NOINIT;		// Wyczysc pole STA_NOINIT(znaczy ze karta zainicjalizowana poprawnie) 
		strncpy(bufor, "Zainicjalizowano\n               ", 33);
		_delay_ms(1000);
	}
	else 
	{
		strncpy(bufor, "Blad            \nInicjalizacji  ", 33);
		_delay_ms(1000);
	}
	return Stat;
}


BYTE SD_wyslij_komende(BYTE cmd, DWORD argument)
{
	BYTE odpowiedz, crc, licznik;
	
	if (cmd & 0x80)	// CMD55 aby nastepna komenda stala sie komenda aplikacji
	 {
		cmd &= 0x7F;	//maska bitowa aby uniewa¿niæ 1 bit:		1xxx xxxx
		SPI_wyslij_dane(0xFF);
		odpowiedz = SD_wyslij_komende(CMD55, 0);
		SPI_wyslij_dane(0xFF);
		if (odpowiedz > 1) return odpowiedz;
	 }
	
	SPI_wyslij_dane(0x40 | cmd);		// Wyslanie 1 bajtu identyfikujacego dana komende
	SPI_wyslij_dane(argument >> 24);	// Argument[31..24] 
	SPI_wyslij_dane(argument >> 16);	// Argument[23..16]
	SPI_wyslij_dane(argument >> 8);		// Argument[15..8]
	SPI_wyslij_dane(argument);			// Argument[7..0]
	
	crc = 0x01;							// Atrapa sumy kontrolnej
	if (cmd == CMD0) crc = 0x95;		// Wazne CRC dla komendy CMD0
	if (cmd == CMD8) crc = 0x87;		// Wazne CRC dla komendy CMD8
	SPI_wyslij_dane(crc);				// Wyslanie sumy kontrolnej
	licznik = 10;						// Czekaj na wazna odpowiedz(rozna od 0x00), podejmij do 10 prub
	
	do
	{
		odpowiedz = SPI_wyslij_dane(0xFF);
	}while ((odpowiedz & 0x80) && --licznik);

	return odpowiedz;			// Zwroc odpowiedz wraz z jej wartoscia	
}

static int SD_odbierz_blok_danych(BYTE *bufor,	UINT bajty)	// *bufor wskaznik gdzie dane maja byc zapisane  bajty: liczba bajtow do zapisania
{
	BYTE token;
	int licznik=0;
	
	do{							// Wait for data packet in timeout of 200ms
		token = SPI_wyslij_dane(0xFF);
	}while (token == 0xFF);
	if (token != 0xFE) return 0;
	
	for(licznik=0; licznik<bajty; licznik++)
	{
		bufor[licznik]=SPI_wyslij_dane(0xFF);
		//LCD_wyswietl_HEX(bufor[licznik]);
	}
	SPI_wyslij_dane(0xFF);					// Wys³anie atrapy CRC(sumy kontrolnej) //
	SPI_wyslij_dane(0xFF);

	return 1;
}

DRESULT SD_odczytaj_dane(BYTE *bufor_danych, LBA_t numer_sektora, UINT licznik_sektorow)
{
	BYTE cmd;
	DWORD sektor = (DWORD)numer_sektora;
	
	if (!licznik_sektorow) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	
	if (!(CardType & CT_BLOCK)) sektor *= 512;		// Konwertuj do bajtowej adresacji jesli potrzeba

	cmd = licznik_sektorow > 1 ? CMD18 : CMD17;		// Odczyt wielu blokow : Odczyt jednego bloku
	
	SPI_wyslij_dane(0xFF);
	PORTB &= ~(1<<CS);
	if(SD_wyslij_komende(cmd, sektor)==0)
	{
		do 
		{
			SPI_wyslij_dane(0xFF);
			if (!SD_odbierz_blok_danych(bufor_danych, 512)) break;
			bufor_danych += 512;
		}while (--licznik_sektorow);
		SPI_wyslij_dane(0xFF);
		if (cmd == CMD18) SD_wyslij_komende(CMD12, 0);	// Koniec transmisji 
		
		PORTB |= (1<<CS);
		//SPI_wyslij_dane(0xFF);
		
		return licznik_sektorow ? RES_ERROR : RES_OK;
	}
	else
	{
		strncpy(bufor, "Blad odczytu    \ndanych         ", 33);
		_delay_ms(1000);
		
		return RES_ERROR;
	}
}

static int SD_wyslij_blok_danych(const BYTE *bufor, BYTE token)	// 512 bajtowy blok danych do wyslania 
{
	BYTE odpowiedz;
	int licznik=0;
	
	
	SPI_wyslij_dane(token);					// Wyslij token danych 
	if (token == 0xFD) return 1;		// Nie wysylaj danych jesli token to StopTran(zatrzymaj transmisjie)
	
	for(licznik=0; licznik<512; licznik++)
	{
		SPI_wyslij_dane(bufor[licznik]);
		//LCD_wyswietl_HEX(bufor[licznik]);
	}
	SPI_wyslij_dane(0xFF);		// Wys³anie atrapy CRC(sumy kontrolnej) 
	SPI_wyslij_dane(0xFF);		// Wys³anie atrapy CRC(sumy kontrolnej) 
	
	odpowiedz = SPI_wyslij_dane(0xFF);	//Odbierz odpowiedz
	//SPI_wyslij_dane(0xFF);
	//LCD_wyswietl_HEX(odpowiedz);
	_delay_ms(200);
	
	return (odpowiedz & 0x1F) == 0x05 ? 1 : 0;	// Dane zostaly zaakceptowane przez karte albo nie
}

DRESULT SD_zapisz_dane(const BYTE *bufor_danych, LBA_t numer_sektora, UINT licznik_sektorow)
{
	DWORD sektor = (DWORD)numer_sektora;
	
	
	if (!licznik_sektorow) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	if (Stat & STA_PROTECT) return RES_WRPRT;
	
	if (!(CardType & CT_BLOCK)) sektor *= 512;
	
	SPI_wyslij_dane(0xFF);
	PORTB &= ~(1<<CS);
	
	if (licznik_sektorow == 1)	// Zapis jednego sektora 
	{	
		char druga;
		if((SD_wyslij_komende(CMD24, sektor) == 0) && (druga=SD_wyslij_blok_danych(bufor_danych, 0xFE)))	// Zapisz blok danych
		{
			licznik_sektorow = 0;
			SPI_wyslij_dane(0xFF);			
		}
		else
		{
			strncpy(bufor, "Blad zapisu     \njednego sektora ", 33);
			//LCD_napis("Blad zapisu\njednego sektora");
			_delay_ms(1000);
		}
	}
	else						// Zapis wielu sektorow
	{				
		if (CardType & CT_SDC) SD_wyslij_komende(ACMD23, licznik_sektorow);
		if (SD_wyslij_komende(CMD25, sektor) == 0)
		{	// WRITE_MULTIPLE_BLOCK 
			do
			{
				if (!SD_wyslij_blok_danych(bufor_danych, 0xFC)) break;
				bufor_danych += 512;
			}
			while(--licznik_sektorow);
			if (!SD_wyslij_blok_danych(0, 0xFD)) licznik_sektorow = 1;	// STOP_TRAN token 
		}
		else
		{
			strncpy(bufor, "Blad zapisu     \nwielu sektorow  ", 33);
			_delay_ms(1000);
		}
	}
	PORTB |= (1<<CS);
	
	return licznik_sektorow ? RES_ERROR : RES_OK;
}

DSTATUS SD_disk_status (void)
{
	DDRB &= ~(1<<PB0);	//Obsluga wykrywania obecnosci karty w slocie
	PORTB |= (1<<PB0);
	
	if(!(PINB & (1<<PB0)))
	{	
		strncpy(bufor, "BRAK KARTY      \n               ", 33);
		_delay_ms(1000);
		Stat |= STA_NODISK;
		Stat |= STA_NOINIT;
	}
	else
	{
		//LCD_komenda(0x01);
		//LCD_napis("Jest karta");
		Stat &= ~STA_NODISK;
	}
	
	return Stat;
}

DRESULT SD_disk_ioctl(BYTE kod_kontrolny, void *bufor)		//Bufor do wysylania/odbierania danych
{
	DRESULT odpowiedz;
	BYTE n, csd[16];//, *ptr = bufor;
	DWORD csize;
#if FF_USE_TRIM
	LBA_t *range;
	DWORD st, ed;
#endif
#if _USE_ISDIO
	SDIO_CTRL *sdi;
	BYTE rc, *bp;
	UINT dc;
#endif

	if (Stat & STA_NOINIT) return RES_NOTRDY;
	odpowiedz = RES_ERROR;
	switch (kod_kontrolny) 
	{
		case CTRL_SYNC :		//Upewnij sie ze nie trwa wlasnie zapis. Nie usuwaj tego albo nadpisany sektor moze nie zostac uaktualniony
		if (SD_wybierz()) odpowiedz = RES_OK;
		SD_cofnij_wybor();
		break;
	
		case GET_SECTOR_COUNT :	// Odczytaj liczbe sektorow na dysku
		if ((SD_wyslij_komende(CMD9, 0) == 0) && SD_odbierz_blok_danych(csd, 16)) 
		{
			if ((csd[0] >> 6) == 1)	// SDC ver 2.00 
			{	
				csize = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
				*(LBA_t*)bufor = csize << 10;
			} else					// SDC ver 1.XX or MMC
			{					
				n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
				csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
				*(LBA_t*)bufor = csize << (n - 9);
			}
			odpowiedz = RES_OK;
		}
		SD_cofnij_wybor();
		break;
	
		case GET_BLOCK_SIZE :	// Get erase block size in unit of sector (DWORD)
		if (CardType & CT_SD2)	// SDv2? 
		{	
			if (SD_wyslij_komende(ACMD13, 0) == 0) // Read SD status
			{	
				SPI_wyslij_dane(0xFF);
				if (SD_odbierz_blok_danych(csd, 16)) {				// Read partial block
					for (n = 64 - 16; n; n--) SPI_wyslij_dane(0xFF);	// Purge trailing data
					*(DWORD*)bufor = 16UL << (csd[10] >> 4);
					odpowiedz = RES_OK;
				}
			}
		}
		else	// SDv1 or MMCv3 
		{					
			if ((SD_wyslij_komende(CMD9, 0) == 0) && SD_odbierz_blok_danych(csd, 16)) // Read CSD 
			{	
				if (CardType & CT_SD1) // SDv1
				{	
					*(DWORD*)bufor = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
					} else {					// MMCv3
					*(DWORD*)bufor = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
				}
				odpowiedz = RES_OK;
			}
		}
		SD_cofnij_wybor();
		break;
	
#if FF_USE_TRIM
		case CTRL_TRIM:		// Erase a block of sectors (used when _USE_TRIM in ffconf.h is 1)
		if (!(CardType & CT_SDC)) break;				// Check if the card is SDC 
		if (SD_disk_ioctl(MMC_GET_CSD, csd)) break;	// Get CSD
		if (!(csd[0] >> 6) && !(csd[10] & 0x40)) break;	// Check if sector erase can be applied to the card
		range = buff; st = (DWORD)range[0]; ed = (DWORD)range[1];	// Load sector block
		if (!(CardType & CT_BLOCK)) {
			st *= 512; ed *= 512;
		}
		if (SD_wyslij_komende(CMD32, st) == 0 && SD_wyslij_komende(CMD33, ed) == 0 && SD_wyslij_komende(CMD38, 0) == 0 && SD_czekaj_na_gotowosc(60000))
		{	// Erase sector block
			res = RES_OK;	// FatFs does not check result of this command
		}
		break;
#endif
	
	}
	
	return odpowiedz;
}

void SD_cofnij_wybor(void)
{
	PORTB |= (1<<CS);			// CS ustaw na wysoki stan aby zwolnic szyne danych 
	SPI_wyslij_dane(0xFF);		// Atrapa zegara + oprozneinie bufora danych
}

static int SD_wybierz(void)	// 1:Sukces, 0:Czas minal 
{
	PORTB &= ~(1<<CS);		// Ustaw CS na niski stan aby wybrac karte
	SPI_wyslij_dane(0xFF);	// Atrapa zegara

	if (SD_czekaj_na_gotowosc(500)) return 1;	// Zaczekaj az karta bd gotowa

	SD_cofnij_wybor();		// Czas minal
	return 0;

}

static int SD_czekaj_na_gotowosc(UINT czekaj)
{
	BYTE odpowiedz;


	czekaj /= 10;
	cli(); Timer2 = czekaj; sei();
	do 
	{
		odpowiedz = SPI_wyslij_dane(0xFF);
		cli(); czekaj = Timer2; sei();
	}while (odpowiedz != 0xFF && czekaj);

	return (odpowiedz == 0xFF) ? 1 : 0;		// 1:Gotowy, 0:Czas minal 
}
