/*Podpiecie funkcji niskopoziomowych pod biblioteke FatFs*/
#include "SD.c"


DSTATUS disk_initialize (BYTE pdrv)
{
	return SD_inicjalizacja();
}

DSTATUS disk_status (BYTE pdrv)
{
	return SD_disk_status();
}

DRESULT disk_read (BYTE pdrv, BYTE* buff, LBA_t sector, UINT count)
{
	return SD_odczytaj_dane(buff, sector, count);
}

DRESULT disk_write (BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count)
{
	return SD_zapisz_dane(buff, sector, count);
}

DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff)
{
	return SD_disk_ioctl(cmd, buff);
}