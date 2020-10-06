/*Obsluga klawiatury zrobiona na zewnetrznym ukladzie PCF8574A(Zajmuje tylko 2 piny procesora do I2C)*/

#include <stdlib.h>
#include <ctype.h>
#include "I2C.c"

#define BLAD -10

int sprawdz_kolumny(void);
int sprawdz_wiersze(void);

//extern char bufor[33];

int klawiatura(void)
{
	int klawisz = -1; //-1 oznacza brak wsicnietego klawisza
	int kolumna = 0;   // 0 Oznacza brak wykrytej kolumny
	int wiersz = 0;	   // 0 Oznacza brak wykrytego wiersza	
	
	kolumna=sprawdz_kolumny();
	switch (kolumna)
	{
	case 0: //LCD_napis("Nic");			//tutaj juz nic nie robi sie bo poczatkowa wartosc -1 oznacza brak wcisnietego klawisza
			klawisz= -1;
			break;
	case 1: wiersz=sprawdz_wiersze();
			switch(wiersz)
			{
				case 1: //LCD_dane('1');
						klawisz='1';
				break;
				case 2: //LCD_dane('4');
						klawisz='4';
				break;
				case 3: //LCD_dane('7');
						klawisz='7';
				break;
				case 4: //LCD_dane('*');
						klawisz='*';
				break;
				default://LCD_napis("Wiele");
						klawisz=-2;		//wartosc -2 oznacza wcisniecie przynajmniej 2 klawiszy w jednym czasie
				break;
			}			
			break;
	case 2:	 wiersz=sprawdz_wiersze();
			 switch(wiersz)
			 {
				 case 1: //LCD_dane('2');
						 klawisz='2';
				 break;
				 case 2: //LCD_dane('5');
						 klawisz='5';
				 break;
				 case 3: //LCD_dane('8');
						 klawisz='8';
				 break;
				 case 4: //LCD_dane('0');
						 klawisz='0';
				 break;
				 default://LCD_napis("Wiele");
						 klawisz=-2;		//wartosc -2 oznacza wcisniecie przynajmniej 2 klawiszy w jednym czasie
				 break;
			 }
			break;
	case 3:	wiersz=sprawdz_wiersze();
			switch(wiersz)
			{
				case 1: //LCD_dane('3');
						klawisz='3';
				break;
				case 2: //LCD_dane('6');
						klawisz='6';
				break;
				case 3: //LCD_dane('9');
						klawisz='9';
				break;
				case 4: //LCD_dane('#');
						klawisz='#';
				break;
				default://LCD_napis("Wiele");
						klawisz=-2;		//wartosc -2 oznacza wcisniecie przynajmniej 2 klawiszy w jednym czasie
				break;
			}
			break;
	case 4: wiersz=sprawdz_wiersze();
			switch(wiersz)
			{
				case 1: //LCD_dane('A');
						klawisz='A';
				break;
				case 2: //LCD_dane('B');
						klawisz='B';
				break;
				case 3: //LCD_dane('C');
						klawisz='C';
				break;
				case 4: //LCD_dane('D');
						klawisz='D';
				break;
				default://LCD_napis("Wiele");
						klawisz=-2;		//wartosc -2 oznacza wcisniecie przynajmniej 2 klawiszy w jednym czasie
				break;
			}
			break;
	default://LCD_napis("Wiele");
			klawisz=-2;		//wartosc -2 oznacza wcisniecie przynajmniej 2 klawiszy w jednym czasie
			break;
	}
		
	return klawisz;
}

int sprawdz_kolumny(void)
{
	int kolumna = 0;	// 0 oznacza ze nie wykryto zadnej aktywnej kolumny
	
	I2C_start();									// Rozpoczyna komunikacje na magistrali I2C, jesli szyna danych jest wolna
	if(I2C_pobierz_status() != 0x08)	return BLAD;// Wartosc BLAD oznacza jakis blad w funkcji, 0x08(bit startu zostal poprawnie wyslany na magistrali I2C)
	
	I2C_zapisz(0x70);								// Wywoluje urzadzenie o adresie 0x70(uklad PCF8574AN z ZAMIAREM ZAPISU)
	if(I2C_pobierz_status() != 0x18)	return BLAD;// Wartosc BLAD oznacza jakis blad w funkcji, 0x18(bajt danych bedacy adresem w trybie PISANIA do ukladu przeslano oraz
													// otrzymano potwierdzenie
	
	I2C_zapisz(0x0F);								// Wysyla dane do ukladu opisujace stany pinow
	if(I2C_pobierz_status() != 0x28)	return BLAD;// Wartosc BLAD oznacza jakis blad w funkcji, 0x28((bajt danych przeslano oraz otrzymano potwierdzenie)
	
	I2C_stop();										// Zakonczenie komunikacji na magistrali I2C
	_delay_us(10);
	
	I2C_start();									// Rozpoczyna komunikacje na magistrali I2C, jesli szyna danych jest wolna
	if(I2C_pobierz_status() != 0x08)	return BLAD;// Wartosc BLAD oznacza jakis blad w funkcji, 0x08(bit startu zostal poprawnie wyslany na magistrali I2C)
	
	I2C_zapisz(0x71);								// Wywoluje urzadzenie o adresie 0x71(uklad PCF8574AN z ZAMIAREM ODCZYTU)
	if(I2C_pobierz_status() != 0x40)	return BLAD;// Wartosc BLAD oznacza jakis blad w funkcji, 0x40(bajt danych bedacy adresem w trybie CZYTANIA z ukladu przeslano oraz
													// otrzymano potwierdzenie)

	switch(kolumna = I2C_odczytaj_bez_ACK())
	{
		case 0x0F:	kolumna=0;	//Brak wykrytej kolumny(czyli nic nie wcisnieto)
					break;
		
		case 0x0E:	kolumna=1;
					break;
					
		case 0x0D:	kolumna=2;
					break;
		
		case 0x0B:	kolumna=3;
					break;
		
		case 0x07:	kolumna=4;
					break;
		
		default:	kolumna=-1;	//Wykryto wiecej niz jedna kolumne
					break;
	}

	if(I2C_pobierz_status() != 0x58)	return BLAD;// Wartosc BLAD oznacza jakis blad w funkcji, 0x58(Odebrano bajt danych, nie wyslano potwierdzenia co jest sygnalem o
													// wkrotce nadchodzacym koncu transmisji)
	I2C_stop();										// Zakonczenie komunikacji na magistrali I2C
	_delay_us(10);

	return kolumna;
}

int sprawdz_wiersze(void)
{
	int wiersz= 0;	// 0 oznacza ze nie wykryto zadnego aktywnego wiersza
	
	I2C_start();									// Rozpoczyna komunikacje na magistrali I2C, jesli szyna danych jest wolna
	if(I2C_pobierz_status() != 0x08)	return BLAD;// Wartosc BLAD oznacza jakis blad w funkcji, 0x08(bit startu zostal poprawnie wyslany na magistrali I2C)
	
	I2C_zapisz(0x70);								// Wywoluje urzadzenie o adresie 0x70(uklad PCF8574AN z ZAMIAREM ZAPISU)
	if(I2C_pobierz_status() != 0x18)	return BLAD;// Wartosc BLAD oznacza jakis blad w funkcji, 0x18(bajt danych bedacy adresem w trybie PISANIA do ukladu przeslano oraz
													// otrzymano potwierdzenie
	
	I2C_zapisz(0xF0);								// Wysyla dane do ukladu opisujace stany pinow
	if(I2C_pobierz_status() != 0x28)	return BLAD;// Wartosc BLAD oznacza jakis blad w funkcji, 0x28((bajt danych przeslano oraz otrzymano potwierdzenie)
	
	I2C_stop();										// Zakonczenie komunikacji na magistrali I2C
	_delay_us(10);
	
	I2C_start();									// Rozpoczyna komunikacje na magistrali I2C, jesli szyna danych jest wolna
	if(I2C_pobierz_status() != 0x08)	return BLAD;// Wartosc BLAD oznacza jakis blad w funkcji, 0x08(bit startu zostal poprawnie wyslany na magistrali I2C)
	
	I2C_zapisz(0x71);								// Wywoluje urzadzenie o adresie 0x71(uklad PCF8574AN z ZAMIAREM ODCZYTU)
	if(I2C_pobierz_status() != 0x40)	return BLAD;// Wartosc BLAD oznacza jakis blad w funkcji, 0x40(bajt danych bedacy adresem w trybie CZYTANIA z ukladu przeslano oraz
													// otrzymano potwierdzenie)
	
	switch(wiersz = I2C_odczytaj_bez_ACK())
	{
		case 0xF0:	wiersz=0;	//Brak wykrytego wiersza(czyli nic nie wcisnieto)
					break;
		
		case 0x70:	wiersz=1;
					break;
		
		case 0xB0:	wiersz=2;
					break;
		
		case 0xD0:	wiersz=3;
					break;
		
		case 0xE0:	wiersz=4;
					break;
		
		default:	wiersz=-1;	//Wykryto wiecej niz jeden wiersz
					break;
	}
	
	if(I2C_pobierz_status() != 0x58)	return BLAD;// Wartosc BLAD oznacza jakis blad w funkcji, 0x58(Odebrano bajt danych, nie wyslano potwierdzenia co jest sygnalem o
													// wkrotce nadchodzacym koncu transmisji)
	I2C_stop();										// Zakonczenie komunikacji na magistrali I2C
	_delay_us(10);
	
	return wiersz;
}

long int pobierz_napis(char * miejsce_na_napis, const char * komunikat, int dlugosc)
{
	int licznik=0, znak=0, klawisz=0;
	
	char napis[6]={0};
	strncpy(miejsce_na_napis, komunikat, 33);
	do									//Oczekiwanie na zwolnienie klawisza A
	{
		znak=klawiatura();
	} while (!(-1 == znak));
	
	do 
	{
		do
		{								//Oczekiwanie na wcisniecie jakiegos klawisza
			klawisz=znak=klawiatura();
		}while(-1 == znak);
		
		do 
		{								//Oczekiwanie na zwolnienie wcisnietego klawisza
			znak=klawiatura();
		} while (!(-1 == znak));
		
		if(klawisz>=0 && isdigit(klawisz))
		{	
			napis[licznik]=klawisz;
			licznik++;
			strncpy(&miejsce_na_napis[17], napis, licznik);
		}
	} while (licznik<dlugosc);
	
	_delay_ms(1000);
	
	
	return atol(napis);
}