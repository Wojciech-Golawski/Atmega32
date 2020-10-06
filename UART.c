/*Gotowiec z noty katalogowej, plik napewno bd zmieniony.
Docelowo w tym pliku zamieszcze niskopoziomowe funkcje obslugujace interfejs UART*/




void UART_inicjalizacja(unsigned int predkosc)
{
	DDRD =0b11111111;
	//Ustaw predkosc
	UBRRH = (unsigned char)(predkosc>>8);
	UBRRL = (unsigned char)predkosc;
	// Odblokuj odbiornik i nadajnik
	UCSRB = (1<<RXEN)|(1<<TXEN);
	// Ustaw format ramki na 8 bitow danych i 1 bity stopu
	//|(1<<USBS)
	UCSRC = (1<<URSEL)|(1<<UCSZ0);//|(1<<UCSZ0);
}

void UART_wyslij( unsigned char dane )
{
	// Czekaj na oproznienie bufora transmisyjnego
	while ( !( UCSRA & (1<<UDRE)) )
	;
	// Umiesc dane w buforze, wyslij dane
	UDR = dane;
}
void USART_Transmit( unsigned int data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSRA & (1<<UDRE))) 
	;
	/* Copy 9th bit to TXB8 */
	UCSRB &= ~(1<<TXB8);
	if ( data & 0x0100 )
	UCSRB |= (1<<TXB8);
	/* Put data into buffer, sends the data */
	UDR = data;
}