/*Niskopoziomowe funkcje obslugujace komunikacje przez magistrale SPI*/
#define SCK PB7
#define MISO PB6
#define MOSI PB5
#define CS PB4

void SPI_inicjalizacja(void)
{
	DDRB |= (1<<MOSI);	//ustalenie pinu MOSI na wyjscie
	DDRB |= (1<<SCK);	//ustalenie pinu SCK na wyjscie									//pozostale piny sa ustawiane na wejscie
	DDRB |= (1<<CS);	//ustalenie pinu SS na wyjscie
	
	SPCR |= (1<<MSTR);	//ustalenie procesora jako MASTER w komunikacji po magistrali SPI
	SPCR |= (1<<SPR1);	//ustawienie zegara SPI na 1/128 glownego zegara procesora
	SPCR |= (1<<SPR0);	//jak wyzej(bo trzeba 2 bity zmienic)
	SPCR |= (1<<SPE);	//uruchomienie modulu SPI
	PORTB |= (1<<CS);	//ustawienie urzadzenia SPI jako nieaktywne, stan 1 na pinie SS
}

char SPI_wyslij_dane(unsigned char dane)
{
	SPDR=dane;
	while(!(SPSR & (1<<SPIF)));
	
	return SPDR;
}