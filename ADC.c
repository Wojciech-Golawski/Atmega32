/*Obsluga przetwornika analogowo-cyfrowego znajdujacego sie na pokladzie procesora*/
#include "klawiatura.c"
//#include "LCD.c"

#define WIELKOSC_BUFORA_ADC 8


struct napiecie
{
	long srednia_ciagnieta_przetwornik;
	char calkowite[4];
	char ulamki[4];
};

int adc_bufor[WIELKOSC_BUFORA_ADC]={0,0};
int licznik=0;

extern char bufor[33];

void ADC_inicjalizacja(void)
{
	// Inicjalizacja przetwornika analogowo cyfrowego:
	ADCSRA |= (1<<ADEN);		//		1		wlaczenie przetwornika analogowo cyfrowego
	ADCSRA |= (1<<ADPS2);		//		2		ustawienie preskalera, preskaler=16
	ADCSRA |= (1<<ADPS1);		//		2		ustawienie preskalera, preskaler=64
	ADMUX  |= (1<<REFS0);		//		3		ustawineie napiecia odniesienia na wewnetrzne
	//ADMUX  |= (1<<REFS1);		//		3		napiecie wbudowane ~2,56V
}

int ADC_pomiar(unsigned char kanal)
{
	ADMUX = (ADMUX & 0xF8) | kanal;
	ADCSRA |= (1<<ADSC);		//start konwersji
	while(ADCSRA & (1<<ADSC));	// czekanie a¿ kontroler zrobi pomiar i da sygnal ¿e jest gotowy
	
	return ADCW;
}

int ADC_NR_pomiar(unsigned char kanal)
{
	ADCSRA |= (1<<ADIE);			//odblokowywyje przerwanie od ADC po ukonczonej konwersji
	ADMUX = (ADMUX & 0xF8) | kanal;	//wybor kanalu
	set_sleep_mode(SLEEP_MODE_ADC);	//wybranie specjalnego trybu zasilania
	sei();							//odblokowanie globalnych przerwan
	sleep_mode();					//wprowadzenie procesora w wybrany specjalny tryb zasilania
	ISR(ADC_vect);					//wybudzenie procesora w skotek zakonczenia konwersji
	cli();							//zablokowanie globalnych przerwan
	return ADCW;					//zwrocenie dannych uzyskanych w konwersji ADC
}

int ADC_pomiar_oversampling(unsigned char kanal, unsigned dodatkowe_bity)
{
	long long int wynik=0;
	int licznik=0;
	for(licznik=0; licznik<pow(4.0, dodatkowe_bity); licznik++)
	{
		wynik+=ADC_NR_pomiar(kanal);
	}
	return wynik>>dodatkowe_bity;
}

void ADC_oblicz_napiecie(struct napiecie* n1, long long a, long long b)
{
	//long long a=95;
	//long long b=119;
	unsigned long wynik = a*n1->srednia_ciagnieta_przetwornik+b;

	wynik /=100;
	div_t volty=div(wynik, 100);
	itoa(volty.quot, n1->calkowite, 10);
	itoa(volty.rem, n1->ulamki, 10);
	if(volty.rem < 10)
	{
		n1->ulamki[0] = '0';
		n1->ulamki[1] = volty.rem + '0';
	}
	n1->calkowite[3]=0;
	n1->ulamki[3]=0;
}

void ADC_usrednij_wartosc(int wartosc_adc, struct napiecie * struktura)
{
	int licznik_1=0;
	int suma=0;
	
	adc_bufor[licznik++]=wartosc_adc;
	if(licznik>WIELKOSC_BUFORA_ADC-1)
	{
		licznik=0;
	}
	for(licznik_1=0; licznik_1<WIELKOSC_BUFORA_ADC; licznik_1++)
	{
		suma+=adc_bufor[licznik_1];
	}
	(struktura->srednia_ciagnieta_przetwornik)=suma/WIELKOSC_BUFORA_ADC;
}

 void ADC_filtr_dolnoprzepustowy(int pomiar, int liczba_probek, long long int* srednia)
 {
	 *srednia*=liczba_probek;
	 *srednia+=pomiar;
	 *srednia/=(liczba_probek+1);
 }
 
 void ADC_kalibracja(long long * a, long long * b)
 {
	int klawisz=-1;
		 
	strncpy(bufor, "Kalibracja ADC  \nA par a B par b", 33);
	do									//Oczekiwanie na zwolnienie klawisza 1
	{
		klawisz=klawiatura();
	} while (!(-1 == klawisz));
	
	do									//Oczekiwanie na wcisniecie klawisza 
	{
		klawisz=klawiatura();
	}while(-1 == klawisz);

	do
	{
		if(klawisz=='A')
		{
			*a = pobierz_napis(bufor, "Podaj parametr a\n               ", 4);
		}
		else if(klawisz=='B')
		{
			*b = pobierz_napis(bufor, "Podaj parametr b\n               ", 6);
		}
		else
		{
			strncpy(bufor, "Wcisnij A lub B \nD aby zakonczyc", 33);
		}
	}
	while('D' != (klawisz=klawiatura()));
	
 }