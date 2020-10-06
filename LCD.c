/*Obsluga wyswietlacza alfanumerycznego 16x2 z ukladem sterujacym HD4470*/

#define RS PC3
#define EN PC2
#define DELAY 2
#define DELAY_DANE 40
#define LCD_PORT PORTC
#define LCD_DDRX DDRC

void LCD_komenda(unsigned char);
void LCD_napis(char *);
void LCD_dane(unsigned char);
void LCD_inicjalizacja_wyswietlacza(void);


unsigned char skonwertowany_bajt[2]={0, 0};


void LCD_inicjalizacja(void)
{
	LCD_DDRX |= 0xFC;
	_delay_ms(20);
	
	LCD_PORT = (LCD_PORT & 0x0F) | (0b00110000);
	LCD_PORT &= ~(1<<RS);
	LCD_PORT |= (1<<EN);
	_delay_ms(5);
	LCD_PORT &= ~(1<<EN);
	
	LCD_PORT = (LCD_PORT & 0x0F) | (0b00110000);
	LCD_PORT &= ~(1<<RS);
	LCD_PORT |= (1<<EN);
	_delay_us(200);
	LCD_PORT &= ~(1<<EN);
	
	LCD_PORT = (LCD_PORT & 0x0F) | (0b00110000);
	LCD_PORT &= ~(1<<RS);
	LCD_PORT |= (1<<EN);
	_delay_ms(2);
	LCD_PORT &= ~(1<<EN);
	
	LCD_PORT = (LCD_PORT & 0x0F) | (0b00100000);
	LCD_PORT &= ~(1<<RS);
	LCD_PORT |= (1<<EN);
	_delay_ms(2);
	LCD_PORT &= ~(1<<EN);
	
	LCD_komenda(0x28);		//ustawania wyswietlacz do pracy na 4 bitach, oraz liczbe wierszy na 2
	LCD_komenda(0x0C);		//Ustala pozycje 0 wyswietlacza na poczatku lini
	LCD_komenda(0x01);		//czysci ekran
	LCD_komenda(0x06);
	//Ponizszy kod obsluguje wyliczanie przerwan w celu synchronizacji obrazu na wyswietlaczu
	TIMSK  |=(1<<TOIE1);			//Odblokowanie przerwania od przepelnienia timera 1
	TCNT1=64686;
	sei();
	TCCR1B |=(1<<CS12);
}

void LCD_dane(unsigned char znak)	//Funkcja dziala analogicznie jak wysylanie komend, jedyna roznica jest ustalenie bitu RS(Register Select) na 1 aby dane byly interpretowane jako znaki a nie komendy
{
	LCD_PORT = (LCD_PORT & 0x0F) | (znak & 0xF0);
	LCD_PORT |= (1<<RS);
	LCD_PORT |= (1<<EN);
	_delay_us(DELAY_DANE);
	LCD_PORT &= ~(1<<EN);
	
	LCD_PORT = (LCD_PORT & 0x0F) | (znak << 4);
	LCD_PORT |= (1<<EN);
	_delay_us(DELAY_DANE);
	LCD_PORT &= ~(1<<EN);
}

void LCD_komenda(unsigned char komenda)
{
	LCD_PORT = (LCD_PORT & 0x0F) | (komenda & 0xF0);			//pomocnicza_1 przyjmuje pierwsze 4 bity komendy, a kolejne 4 sa zerowane i zostana wyslane potem
	LCD_PORT &= ~(1<<RS);				//wyslanie pierwszych 4 bitow komendy do wyswietlacza oraz za jednym zamachem ustawienie bitu ENABLE na wysoki aby potem go wyzerowac i uzyskac w ten sposob pik (wysoki niski) aby rejestr wyswietlacza zatrzasna dane znakodowane na bitach
	LCD_PORT |= (1<<EN);
	_delay_ms(DELAY);
	LCD_PORT &= ~(1<<EN);	//wyzerowanie bitu ENABLE
	
	LCD_PORT = (LCD_PORT & 0x0F) | (komenda << 4);			// przesuniecie bitowe pozostalych 4 bitow komendy i jednoczesne utracenie pierwszych 4 ktore juz zostaly wyslane
	LCD_PORT |= (1<<EN);			//analogicznie jak dla pierwszych 4 bitow
	_delay_ms(DELAY);
	LCD_PORT &= ~(1<<EN);							//analogicznie jak dla pierwszych 4 bitow
	_delay_ms(DELAY);
}

void LCD_napis(char * tresc)
{
	int licznik_1;
	
	for(licznik_1=0; tresc[licznik_1] != '\0'; licznik_1++)
	{
		if(tresc[licznik_1]=='\n')
		{
			LCD_komenda(0xC0);//przenies kursor do 2 lini wyswietlacza
			continue;
		}
		LCD_dane(tresc[licznik_1]);
	}
	LCD_komenda(0x03);	// Ustala miejsce pisania w 1 wierszu i 1 lini wyswietlacza
}

unsigned char * LCD_konwertuj_na_HEX(unsigned char do_konwersji)
{
	if((do_konwersji>>4)<0x0A)							//sprawdzenie czy 1 polowa bajtu < A (10)
	{
		skonwertowany_bajt[0]=(0x30|(do_konwersji>>4));		//ustalenie 1 polowy 1 znaku wyjsciowego aby byla to cyfra
	}														//2 polowa 1 znaku wyjsciowego to przepisanie 2 polowy bajtu wejsciwego
	else
	{															//jesli jestesmy tu znaczy to ze mamy litere wiec 1
		if((do_konwersji>>4)>0x0A)
		{
			skonwertowany_bajt[0]=(0x40|(0x07&((do_konwersji>>4)-0x01)));
		}
		else
		{
			skonwertowany_bajt[0]=(0x40|0x01);
		}
	}
	
	if((do_konwersji & 0x0F)<0x0A)
	{
		skonwertowany_bajt[1]=(0x30|(do_konwersji & 0x0F));
	}
	else
	{
		if((do_konwersji & 0x0F)>0x0A)
		{
			skonwertowany_bajt[1]=(0x40|(0x07&((do_konwersji & 0x0F)-0x01)));
		}
		else
		{
			skonwertowany_bajt[1]=(0x40|0x01);
		}
	}
	
	return skonwertowany_bajt;
}

void LCD_wyswietl_HEX(unsigned char dane)
{
	LCD_konwertuj_na_HEX(dane);
	LCD_dane(*skonwertowany_bajt);
	LCD_dane(*(skonwertowany_bajt+1));
}