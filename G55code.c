/**************************************************************************/
/**************************************************************************/

/*Software License Agreement (BSD License)

Copyright (c) 2012, Adafruit Industries
All rights reserved.*/


/**************************************************************************/

#include <asf.h>
#include <string.h>
#include <board.h>
#include <delay.h>
#include "conf_example.h"



static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		#ifdef CONF_UART_CHAR_LENGTH
		.charlength = CONF_UART_CHAR_LENGTH,
		#endif
		.paritytype = CONF_UART_PARITY,
		#ifdef CONF_UART_STOP_BITS
		.stopbits = CONF_UART_STOP_BITS,
		#endif
	};
	
	/* Configure console UART. */
	//sysclk_enable_peripheral_clock(USART0);
	stdio_serial_init(USART0, &uart_serial_options);
}


void Initialize_PN532(void);
void begin(void);
bool SAMConfig(void);
uint32_t getFirmwareVersion(void);
bool sendCommandCheckAck(uint8_t *, uint8_t, uint16_t);  //timeout = 1000
bool readPassiveTargetID(uint8_t, uint8_t *, uint8_t *, uint16_t); //timeout 0 means no timeout - will block forever.
uint8_t AuthenticateBlock (uint8_t *, uint8_t, uint32_t, uint8_t, uint8_t *);
uint8_t ReadDataBlock (uint8_t, uint8_t *);
void readdata(uint8_t*, uint8_t);
void writecommand(uint8_t*, uint8_t);
bool isready(void);
bool waitready(uint16_t);
bool readack(void);
void spi_write(uint8_t);
uint8_t spi_read(void);
void digitalWrite(ioport_pin_t, bool);
bool digitalRead(ioport_pin_t);
uint8_t datavalidate(uint8_t*, uint8_t);
void sendExample(void);



#ifndef _BV
#define _BV(bit) (1<<(bit))
#endif

#define PN532_SCK  12
#define PN532_MOSI 5
#define PN532_SS   1
#define PN532_MISO 8
#define LED 3
#define ACK 13

#define SerialNumber_LEN 6
#define ZoneNumber_LEN 1
#define ZoneType_LEN 1
#define ZoneNumber_MAX 64
#define ZoneNumber_MIN 1

#define ExtraZonesLen 11
uint8_t ExtraZoneTypes[] = {29, 30, 31, 32, 35, 36, 37, 41, 81, 87, 88};

uint8_t pn532ack[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
uint8_t pn532response_firmwarevers[] = {0x00, 0xFF, 0x06, 0xFA, 0xD5, 0x03};
uint8_t pn532_packetbuffer[64];

bool _usingSPI;
uint8_t _reset;
ioport_pin_t _ss, _clk, _mosi, _miso, _LED, _ACK;
uint8_t _uid[7];
uint8_t _uidLen;
uint8_t _key[6];


int main (void) {

	sysclk_init();
	board_init();
	configure_console();
	Initialize_PN532();
	
	/*while(1){
		sendExample();
		delay_ms(500);
		uint32_t timeout = 0;
		while(digitalRead(_ACK)){
			timeout++;
			if(timeout>10000000){
				timeout = 0;
				sendExample();
			}
		}

		digitalWrite(_LED, HIGH);
		delay_s(8);
		digitalWrite(_LED, LOW);
		delay_s(10);
	}*/
	
	begin();
	uint32_t versiondata = getFirmwareVersion();

	if (! versiondata) {
		while (!versiondata){
			versiondata = getFirmwareVersion();
		}
	}

	SAMConfig();
	
	while(1){
		
		uint8_t success;
		uint8_t id[] = {0, 0, 0, 0, 0, 0, 0};
		uint8_t id_len;
		
		// Wait for an NFC tag
		success = readPassiveTargetID(0x00, id, &id_len, 0);
		
		if (success) {
			
			if (id_len == 4)
			{

				//uint8_t keya[6] = { 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB };
				uint8_t password[6] = {0, 0, 0, 0, 0xAC, 0x08};
				uint8_t key = 0xBC;
				uint8_t u;
				for(u=0; u<4; u++){
					password[u] = id[u];
				}
				for(u=0;u<6;u++){
					password[u] = password[u] ^ key;
				}

				success = AuthenticateBlock(id, id_len, 4, 0, password);
				
				if (success)
				{
					uint8_t data_SN[16];
					uint8_t data_ZN[16];
					uint8_t data_ZT[16];


					if(success){

						success = ReadDataBlock(4, data_SN);
						success = ReadDataBlock(5, data_ZN);
						success = ReadDataBlock(6, data_ZT);

					}

					if(success){
						success = datavalidate(data_SN, 0);
					}

					if(success){
						success = datavalidate(data_ZN, 1);
					}

					if(success){
						success = datavalidate(data_ZT, 2);
					}
					
					if (success)
					{

						//create UART packet for cell module
						/*uint8_t checksum = 0;
						uint8_t sum = 0;
						char message[10] = {0xAA, 0, 0, 0, 0, 0, 0, 0, 0, 0};
						message[1] = '0' + data_ZN[0];
						message[2] = '0' + data_ZT[0];
						message[3] = data_SN[0];
						message[4] = data_SN[1];
						message[5] = data_SN[2];
						message[6] = data_SN[3];
						message[7] = data_SN[4];
						message[8] = data_SN[5];
						sum += 0xAA + 0x30 + 0x30 + data_ZN[0] + data_ZT[0];
						uint8_t t;
						for(t=0; t<9; t++){
							sum += data_SN[t];
						}
						checksum = sum & 0xFF;
						message[9] = checksum;

						//put data into TX buffer
						//for(u=0; u<MessageLen; u++){
						usart_putchar(USART0, message[0]);
						usart_putchar(USART0, message[1]);
						usart_putchar(USART0, message[2]);
						usart_putchar(USART0, message[3]);
						usart_putchar(USART0, message[4]);
						usart_putchar(USART0, message[5]);
						usart_putchar(USART0, message[6]);
						usart_putchar(USART0, message[7]);
						usart_putchar(USART0, message[8]);
						usart_putchar(USART0, message[9]);
						//}
						*/
						sendExample();
						//delay_ms(500);
						uint32_t timeout = 0;
						while(digitalRead(_ACK)){
							timeout++;
							if(timeout>10000000){
								timeout = 0;
								sendExample();
							}
						}
						digitalWrite(_LED, HIGH);
						delay_ms(3000);
						digitalWrite(_LED, LOW);
						delay_ms(10000);
						

					}
					else
					{
						delay_ms(1);
						char message[5] = {0xAA, 0xFF, 0xFF, 0xFF, 0xA7};
						
						//put data into TX buffer
						//for(u=0; u<ErrMessageLen; u++){
						usart_putchar(USART0, message[0]);
						usart_putchar(USART0, message[1]);
						usart_putchar(USART0, message[2]);
						usart_putchar(USART0, message[3]);
						usart_putchar(USART0, message[4]);
						//}
					}
				}
				else
				{
					delay_ms(1);
					char message[5] = {0xAA, 0xFF, 0xFF, 0xFF, 0xA7};
					//put data into TX buffer
					//for(u=0; u<ErrMessageLen; u++){
					usart_putchar(USART0, message[0]);
					usart_putchar(USART0, message[1]);
					usart_putchar(USART0, message[2]);
					usart_putchar(USART0, message[3]);
					usart_putchar(USART0, message[4]);
					//}
				}
			}
			
		}
	}
}



void sendExample(){
		uint8_t buf[10] = {0xAA, 0x32, 0x33, 0x31, 0x32, 0x33, 0x41, 0x42, 0x43, 0x6B};
		usart_putchar(USART0, buf[0]);
		usart_putchar(USART0, buf[1]);
		usart_putchar(USART0, buf[2]);
		usart_putchar(USART0, buf[3]);
		usart_putchar(USART0, buf[4]);
		usart_putchar(USART0, buf[5]);
		usart_putchar(USART0, buf[6]);
		usart_putchar(USART0, buf[7]);
		usart_putchar(USART0, buf[8]);
		usart_putchar(USART0, buf[9]);
}


void Initialize_PN532(){

	_reset=0;
	_usingSPI=true;

	_ss = IOPORT_CREATE_PIN(PIOB, PN532_SS);
	_clk = IOPORT_CREATE_PIN(PIOA, PN532_SCK);
	_mosi = IOPORT_CREATE_PIN(PIOA, PN532_MOSI);
	_miso = IOPORT_CREATE_PIN(PIOB, PN532_MISO);
	_LED = IOPORT_CREATE_PIN(PIOB, LED);
	_ACK = IOPORT_CREATE_PIN(PIOA, ACK);
	ioport_set_pin_dir(_ss, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(_clk, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(_mosi, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(_miso, IOPORT_DIR_INPUT);
	ioport_set_pin_dir(_LED, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(_ACK, IOPORT_DIR_INPUT);

}



void begin() {

	if (_usingSPI) {

		digitalWrite(_ss, LOW);
		delay_ms(1000);
		pn532_packetbuffer[0] = 0x02;
		sendCommandCheckAck(pn532_packetbuffer, 1, 1000);
		digitalWrite(_ss, HIGH);
	}

}



uint32_t getFirmwareVersion(void) {

	uint32_t response;

	pn532_packetbuffer[0] = 0x02;

	if (! sendCommandCheckAck(pn532_packetbuffer, 1, 1000)) {
		return 0;
	}

	readdata(pn532_packetbuffer, 12);
	if (0 != strncmp((char *)pn532_packetbuffer, (char *)pn532response_firmwarevers, 6)) {
		return 0;
	}

	int offset = _usingSPI ? 6 : 7;
	response = pn532_packetbuffer[offset++];
	response <<= 8;
	response |= pn532_packetbuffer[offset++];
	response <<= 8;
	response |= pn532_packetbuffer[offset++];
	response <<= 8;
	response |= pn532_packetbuffer[offset++];
	return response;

}



//Sends a command and waits a specified period for the ACK
bool sendCommandCheckAck(uint8_t *cmd, uint8_t cmdlen, uint16_t timeout) {

	writecommand(cmd, cmdlen);
	if (!waitready(timeout)) {
		return false;
	}

	if (!readack()) {
		return false;
	}

	if (!waitready(timeout)) {
		return false;
	}
	
	return true;

}


bool SAMConfig(void) {

	pn532_packetbuffer[0] = 0x14;
	pn532_packetbuffer[1] = 0x01;
	pn532_packetbuffer[2] = 0x14;
	pn532_packetbuffer[3] = 0x01;

	if (! sendCommandCheckAck(pn532_packetbuffer, 4, 1000))
	return false;


	readdata(pn532_packetbuffer, 8);
	int offset = _usingSPI ? 5 : 6;
	return  (pn532_packetbuffer[offset] == 0x15);

}


//Waits for an ISO14443A target to enter the field
bool readPassiveTargetID(uint8_t cardbaudrate, uint8_t * uid, uint8_t * uidLength, uint16_t timeout) {

	pn532_packetbuffer[0] = 0x4A;
	pn532_packetbuffer[1] = 1;
	pn532_packetbuffer[2] = cardbaudrate;

	if (!sendCommandCheckAck(pn532_packetbuffer, 3, timeout))
	{
		return 0x0;
	}

	readdata(pn532_packetbuffer, 20);
	if (pn532_packetbuffer[7] != 1)
	return 0;

	uint16_t sens_res = pn532_packetbuffer[9];
	sens_res <<= 8;
	sens_res |= pn532_packetbuffer[10];
	*uidLength = pn532_packetbuffer[12];
	for (uint8_t i=0; i < pn532_packetbuffer[12]; i++)
	{
		uid[i] = pn532_packetbuffer[13+i];
	}

	return 1;

}


uint8_t AuthenticateBlock (uint8_t * uid, uint8_t uidLen, uint32_t blockNumber, uint8_t keyNumber, uint8_t * keyData){

	uint8_t i;
	memcpy (_key, keyData, 6);
	memcpy (_uid, uid, uidLen);
	_uidLen = uidLen;

	pn532_packetbuffer[0] = 0x40;
	pn532_packetbuffer[1] = 1;
	pn532_packetbuffer[2] = (keyNumber) ? 0x61 : 0x60;
	pn532_packetbuffer[3] = blockNumber;
	memcpy (pn532_packetbuffer+4, _key, 6);
	for (i = 0; i < _uidLen; i++)
	{
		pn532_packetbuffer[10+i] = _uid[i];
	}

	if (! sendCommandCheckAck(pn532_packetbuffer, 10+_uidLen, 1000))
	return 0;

	readdata(pn532_packetbuffer, 12);
	if (pn532_packetbuffer[7] != 0x00)
	{
		return 0;
	}

	return 1;

}



uint8_t ReadDataBlock (uint8_t blockNumber, uint8_t * data){

	pn532_packetbuffer[0] = 0x40;
	pn532_packetbuffer[1] = 1;
	pn532_packetbuffer[2] = 0x30;
	pn532_packetbuffer[3] = blockNumber;

	if (! sendCommandCheckAck(pn532_packetbuffer, 4, 1000))
	{
		return 0;
	}

	readdata(pn532_packetbuffer, 26);
	if (pn532_packetbuffer[7] != 0x00)
	{
		return 0;
	}

	memcpy (data, pn532_packetbuffer+8, 16);

	return 1;

}



bool readack() {

	uint8_t ackbuff[6];
	readdata(ackbuff, 6);
	return (0 == strncmp((char *)ackbuff, (char *)pn532ack, 6));

}



bool isready() {
	
	digitalWrite(_ss, LOW);
	delay_ms(2);
	spi_write(0x02);
	uint8_t x = spi_read();
	digitalWrite(_ss, HIGH);
	return x == 0x01;

}


//Waits until the PN532 is ready.
bool waitready(uint16_t timeout) {

	uint16_t timer = 0;
	while(!isready()) {
		if (timeout != 0) {
			timer += 10;
			if (timer > timeout) {
				return false;
			}
		}
		delay_ms(10);
	}

	return true;

}


void readdata(uint8_t* buff, uint8_t n) {

	digitalWrite(_ss, LOW);
	delay_ms(2);
	spi_write(0x03);

	for (uint8_t i=0; i<n; i++) {
		delay_ms(1);
		buff[i] = spi_read();
	}

	digitalWrite(_ss, HIGH);

}



void writecommand(uint8_t* cmd, uint8_t cmdlen) {

	uint8_t checksum;
	cmdlen++;

	digitalWrite(_ss, LOW);
	delay_ms(2);
	spi_write(0x01);
	checksum = 0x00 + 0x00 + 0xFF;
	spi_write(0x00);
	spi_write(0x00);
	spi_write(0xFF);
	spi_write(cmdlen);
	spi_write(~cmdlen + 1);
	spi_write(0xD4);
	checksum += 0xD4;

	for (uint8_t i=0; i<cmdlen-1; i++) {
		spi_write(cmd[i]);
		checksum += cmd[i];
	}

	spi_write(~checksum);
	spi_write(0x00);
	digitalWrite(_ss, HIGH);
	
}


void spi_write(uint8_t c) {

	int8_t i;
	digitalWrite(_clk, HIGH);

	for (i=0; i<8; i++) {
		digitalWrite(_clk, LOW);
		if (c & _BV(i)) {
			digitalWrite(_mosi, HIGH);
			} else {
			digitalWrite(_mosi, LOW);
		}
		digitalWrite(_clk, HIGH);
	}
	
}


uint8_t spi_read(void) {
	int8_t i, x;
	x = 0;
	digitalWrite(_clk, HIGH);

	for (i=0; i<8; i++) {
		if (digitalRead(_miso)) {
			x |= _BV(i);
		}
		digitalWrite(_clk, LOW);
		digitalWrite(_clk, HIGH);
	}

	return x;

}


void digitalWrite(ioport_pin_t x, bool pinlevel) {
	
	ioport_set_pin_level(x, pinlevel);
}



bool digitalRead(ioport_pin_t x) {

	if (ioport_get_pin_level(x) == IOPORT_PIN_LEVEL_LOW) {
		return false;
	}
	else {
		return true;
	}

}



uint8_t datavalidate(uint8_t* data, uint8_t sel){
	
	if(sel == 0)  //serial number
	{
		int i;
		for(i=0; i<SerialNumber_LEN; i++){
			if ((data[i] < 48)||((data[i] > 57) && (data[i] < 65))||(data[i]>90)){  //if data not a number or letter
				return 0;
			}
		}

		for(i=SerialNumber_LEN; i<16; i++){
			if(data[i] > 0){
				return 0;
			}
		}

		return 1;

	}

	else if (sel == 1)  //zone number
	{

		int i;
		for(i=0; i<ZoneNumber_LEN; i++){
			if ((data[i] < ZoneNumber_MIN)||(data[i] > ZoneNumber_MAX)){  //if data is not between 1 and 64
				return 0;
			}
		}

		for(i=ZoneNumber_LEN; i<16; i++){
			if(data[i] > 0){
				return 0;
			}
		}

		return 1;
	}



	else if (sel == 2)  //zone type
	{

		int i;
		for(i=0; i<ZoneType_LEN; i++){
			if ((data[i] < 0)||(data[i] > 26)){  //if data not 0-26 or in extrazonetypes array
				int secondchance = 0;
				int j;
				for(j=0; j<ExtraZonesLen; j++){
					if (data[i] == ExtraZoneTypes[j]){
						secondchance = 1;
					}
				}

				if(secondchance == 0){
					return 0;
				}
			}
		}

		for(i=ZoneType_LEN; i<16; i++){
			if(data[i] > 0){
				return 0;
			}
		}

		return 1;
	}

	else
	{
		return 0;
	}

}
