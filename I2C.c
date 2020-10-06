/*Niskopoziomowe funkcje obslugujace magistrale I2C*/

void I2C_inicjalizacja(void)
{
	PORTC |= 0x03;
	
	TWSR=0x00;	//preskaler na 0
	TWBR=0x20;	//f=100kHz
}

void I2C_start(void)
{
	cli();
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while((TWCR & (1<<TWINT)) ==0);		// Czekamy az hardware wysle bit startu po poprzednim sprawdzeniu zajetosci szyny
	sei();
}

void I2C_stop(void)
{
	cli();
	TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
	sei();
}

void I2C_zapisz(uint8_t dane)
{
	cli();
	TWDR = dane;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while((TWCR & (1<<TWINT)) == 0);
	sei();
}

uint8_t I2C_odczytaj_z_ACK(void)
{
	cli();
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	while((TWCR & (1<<TWINT)) == 0);
	sei();
	
	return TWDR;
}

uint8_t I2C_odczytaj_bez_ACK(void)
{
	cli();
	TWCR = (1<<TWINT)|(1<<TWEN);
	while((TWCR & (1<<TWINT)) == 0);
	sei();
	
	return TWDR;
}

uint8_t I2C_pobierz_status(void)
{
	uint8_t status;
	
	cli();
	status = TWSR & 0xF8;
	sei();
	
	return status;
}