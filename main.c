/*
 *
 * Created: 2019-04-10 19:20:47
 * Author : Wojtek
 */ 

#ifndef F_CPU					// if F_CPU was not defined in Project -> Properties
#define F_CPU 8000000UL			// define it now as 8 MHz unsigned long
#endif

#define ILOSC_PROBEK_FDP 2

#include <avr/io.h>				// this is always included in AVR programs
#include <util/delay.h>			// add this to use the delay function
#include <stdlib.h>
#include <string.h>				//funkcja strncpy
#include <avr/sleep.h>			//umozliwia uspanie procesora, i uruchomienie trybu pracy ADC Noice Reduction
#include <avr/interrupt.h>		//umozliwia obsluge przerwan
#include <math.h>

#include "ADC.c"
#include "ff.c"

 
long long srednia_FDP=0;

extern char bufor[33];

FATFS Fat; 

int main(void) 
{	
	FIL fil;				/* File object */
	FRESULT fr;
	UINT d;
	long long a=95, b=119;
	
	LCD_inicjalizacja();
	I2C_inicjalizacja();
	ADC_inicjalizacja();

	if(klawiatura() == '1')
	{
		ADC_kalibracja(&a, &b);
	}
	
	f_mount(&Fat, "", 0);
	fr = f_open(&fil, "pomiary.txt", FA_WRITE);
	
	strcpy(bufor, "OK              \n              ");
	_delay_ms(1000);
	
	int wartosc_pomiaru = 0;
	struct napiecie n1={0};
	
	while(1)
	{		
		bufor[0]='C';
		bufor[1]='1';
		bufor[2]=':';
		dtostrf(wartosc_pomiaru=ADC_pomiar(PA1), 4, 0, bufor+3);			// pomiar na kanale 1
		ADC_filtr_dolnoprzepustowy(wartosc_pomiaru, ILOSC_PROBEK_FDP, &srednia_FDP);
		ADC_usrednij_wartosc(srednia_FDP, &n1);
		ADC_oblicz_napiecie(&n1, a, b);
		dtostrf(n1.srednia_ciagnieta_przetwornik, 4, 0, bufor+12);
		bufor[7]='S';
		bufor[8]='R';
		bufor[9]='E';
		bufor[10]='D';
		bufor[11]=':';
		bufor[16]='\n';
		bufor[17]='U';
		bufor[18]=':';
		strncpy(&bufor[19], n1.calkowite, 1);
		strncpy(&bufor[20], ".", 1);
		strncpy(&bufor[21], n1.ulamki, 2);
		strncpy(&bufor[23], "         ", 9);

		f_write(&fil, bufor, 3, &d);
		f_write(&fil, "\t", 1,  &d);
		f_write(&fil, &bufor[4], 3, &d);
		f_write(&fil, "\t", 1, &d);
		f_write(&fil, &bufor[7], 5, &d);
		f_write(&fil, "\t", 1, &d);
		f_write(&fil, &bufor[12], 4, &d);
		f_write(&fil, "\t", 1, &d);
		f_write(&fil, &bufor[17], 2, &d);
		f_write(&fil, "\t", 1, &d);
		f_write(&fil, &bufor[19], 1, &d);
		f_write(&fil, ",", 1, &d);
		f_write(&fil, &bufor[21], 2, &d);
		f_write(&fil, "\n", 1, &d);
		
		if(klawiatura()=='#')
		{
			break;
		}
		_delay_ms(100);
	}
	
	f_close(&fil);
	cli();
	LCD_komenda(0x01);
	LCD_napis("Koniec");
	_delay_ms(1000);
	
	return(0);
}

ISR(ADC_vect)
{
	//tutaj mozna wpisac instrukcje wykonywane w ramach przerwania, funkcja ta jest wywolywana automatycznie
}

ISR(TIMER1_OVF_vect)
{
	LCD_napis(bufor);
			
	TCNT1=64686;
}

