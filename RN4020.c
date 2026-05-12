/****************************************************************************
* Title                 :   Complete RN4020 Library
* Filename              :   RN4020_comp.h
* Author                :   BS
* Origin Date           :   05/05/2026
* Notes                 :   None
*****************************************************************************/
/**************************CHANGE LIST **************************************
*
*    Date    Software Version    Initials       Description
*  05/05/26    1.0.0               BS       Interface Created.
*
*****************************************************************************/

/**
 * @mainpage
 * ### General Description ###
 * RN4020is a simple solution for adding Bluetooth 4.1 (also known as Bluetooth Low Energy, BLE, Bluetooth Smart) to your design.
 * It features the <a href="http://ww1.microchip.com/downloads/en/DeviceDoc/50002279A.pdf">RN4020</a> module from MicroChip that has the complete Bluetooth stack, and can act both as a client or a server.
 * Additionally, the RN4020 module supports MicroChip Low-energy Data Profile (MLDP).
 * RN4020 communicates with the target board microcontroller through RX, TX and CMD/MLDP, PWM (con.), and RST (wake) lines.
 * The board is designed to use a 3.3V power supply only.
 */ 

/******************************************************************************
* Includes
*******************************************************************************/
#include "RN4020.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>				//UART
#include <avr/interrupt.h>		//UART

/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/

#define maxrxindex 32

/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/

/******************************************************************************
* Module Typedefs
*******************************************************************************/

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/

//***********UART***********//

unsigned char ucRecBuf;
unsigned char ucRecFlag=0;

volatile char gotUART[maxrxindex];
volatile char rx_index = 0;
char string_received = 0;

unsigned char ucSendIndex=0;
char *datBuf;


//***********Function Variables***********//

char cmd[5] = "";				//string for command to the module
char rllyshortResponse[5] = ""; //string for really short responses of the module
char shortResponse[9] = "";		//string for short responses of the module
char longResponse[33] = "";		//string for long responses of the module

/******************************************************************************
* Function Definitions
*******************************************************************************/

//***********Format Functions***********//

void replaceAt(char* destinationString, const char* sourceString, uint8_t startIndex) {
	if (!destinationString || !sourceString) return;

	uint8_t i = 0;
	while (sourceString[i] != '\0') {
		destinationString[startIndex + i] = sourceString[i];
		i++;
	}
}


//***********UART Functions***********//

void uartInit(unsigned int baud)
{
	/* Set baud rate */
	UBRR0H = (unsigned char)(baud>>8);
	UBRR0L = (unsigned char)baud;
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)| (1<<RXCIE0)| (1<<TXCIE0);
	/* Set frame format: 8data, 1stop bit */
	UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);
	
	ucRecFlag=0;
}
void uartTransmitChar(char data)
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) )
	;
	/* Put data into buffer, sends the data */
	UDR0 = data;
}

unsigned char uartGetReceivedChar()
{
	return ucRecBuf;
}

void uartGetReceivedString(char* Response)
{
	while(uartCheckReceived())
	{
		if(rx_index < (maxrxindex - 1) && !string_received)
		{
			if(uartGetReceivedChar() == '\n' || uartGetReceivedChar() == '\r')
			{
				gotUART[rx_index] = '\0';
				string_received = 1;
			}
			else
			{
				gotUART[rx_index] = uartGetReceivedChar();
				rx_index++;
			}
		}
		
	}
	replaceAt(Response, gotUART,0);
}

unsigned char uartCheckReceived()
{
	if (ucRecFlag)
	{
		ucRecFlag=WAITING;
		return RECVD;
	}
	else return WAITING;
}


void uartSendData(char* data)
{
	datBuf=data;
	ucSendIndex=0;
	UDR0 = data[ucSendIndex++];
}

ISR(USART_TX_vect)
{
	if (datBuf[ucSendIndex]) 	UDR0 = datBuf[ucSendIndex++];
}

ISR(USART_RX_vect)
{
	ucRecBuf = UDR0;					//Writing received data to variable ucRXBuffer.
	ucRecFlag = RECVD;
}



//***********Setter Functions***********//

void set_serialized_name(char* name)
{
	char tmp[19] = "S-,";
	replaceAt(tmp, name,3);
	uartSendData(tmp);
}

int8_t set_baudrate(baud_rate_t baud)
{
	char tmp[5] = "SB,0";
	switch(baud)
	{
		case BR_2400:
		break;
		case BR_9600:
		tmp[3] ='1';
		break;
		case BR_19200:
		tmp[3] = '2';
		break;
		case BR_38400:
		tmp[3] = '3';
		break;
		case BR_115200:
		tmp[3] = '4';
		break;
		case BR_230400:
		tmp[3] = '5';
		break;
		case BR_460800:
		tmp[3] = '6';
		break;
		case BR_921600:
		tmp[3] = '7';
		break;
		default:
		return -1;
	}

	uartSendData(tmp);

	return 0;
}

int8_t factory_reset(reset_parameter_t reset)
{
	char tmp[5] = "SF,1";

	switch(reset)
	{
		case RESET_SOME:
		break;
		case RESET_ALL:
		tmp[3] = '2';
		break;
		default:
		return -1;
	}
	
	uartSendData(tmp);
	return 0;
}

void set_timer(set_timer_t timer, uint32_t us)
{
	char tmp[14] = "SM,0,00000000";
	char tmp8[10] = "";

	switch (timer)
	{
		case Timer1:
		tmp[3] = '1';
		break;
		case Timer2:
		tmp[3] = '2';
		break;
		case Timer3:
		tmp[3] = '3';
		break;
	}

	if (us == 0)replaceAt(tmp, ",FFFFFFFF",4);
	else
	{
		sprintf(tmp8, ",%08x", us);
		replaceAt(tmp, tmp8,4);
	}
	
	uartSendData(tmp);
}

int8_t set_name(char* name)
{
	char tmp[21] = "SN,";
	
	if ( sizeof(name) > 20)
	return -1;

	replaceAt(tmp, name,3);

	uartSendData(tmp);
	return 0;
}

int8_t set_transmission_power(char powerlevel)
{
	char tmp[5] = "SP,0";
	
	switch (powerlevel)
	{
	case 0:
		break;
	case 1:
		tmp[3] = '1';
		break;
	case 2:
		tmp[3] = '2';
		break;
	case 3:
		tmp[3] = '3';
		break;
	case 4:
		tmp[3] = '4';
		break;
	case 5:
		tmp[3] = '5';
		break;
	case 6:
		tmp[3] = '6';
		break;
	case 7:
		tmp[3] = '7';
		break;
	default:
		return -1;
	}

	uartSendData(tmp);

	return 0;
}

void set_features(feature_pick_t feature)
{
	char tmp[12] = "SR,00000000";
	switch (feature)
	{
	case central:
		tmp[3] = '8';
		break;
	case realtimeread:
		tmp[3] = '4';
		break;
	case autoAdvertise:
		tmp[3] = '2';
		break;
	case enable_MLDP:
		tmp[3] = '1';
		break;
	case auto_MLDP:
		tmp[4] = '8';
		break;
	case no_dir_adv:
		tmp[4] = '4';
		break;
	case flow_control:
		tmp[4] = '2';
		break;
	case run_scpt_afr_pwron:
		tmp[4] = '1';
		break;
	case enable_auth:
		tmp[5] = '4';
		break;
	case enable_rem_cmd:
		tmp[5] = '2';
		break;
	case no_bond_save:
		tmp[5] = '1';
		break;
	case io_cap:
		tmp[6] = 'E';
		break;
	case block_setcmd_in_remote:
		tmp[6] = '1';
		break;
	case enable_OTA:
		tmp[7] = '8';
		break;
	case iOS_mode:
		tmp[7] = '4';
		break;
	case server_only:
		tmp[7] = '2';
		break;
	case enable_UART_in_srpt:
		tmp[7] = '1';
		break;
	case autoenter_MLDP:
		tmp[8] = '8';
		break;
	case no_MLDP_echo:
		tmp[8] = '4';
		break;
	}
	uartSendData(tmp);
	
}

void set_server_services(server_services_t service)
{
	char tmp[12] = "SS,00000000";
	switch (service)
	{
		case device_info:
			tmp[3] = '8';
			break;
		case battery:
			tmp[3] = '4';
			break;
		case heart_rate:
			tmp[3] = '2';
			break;
		case health_thermo:
			tmp[3] = '1';
			break;
		case glucose:
			tmp[4] = '8';
			break;
		case blood_pressure:
			tmp[4] = '4';
			break;
		case run_speed_cadence:
			tmp[4] = '2';
			break;
		case cyc_speed_cadence:
			tmp[4] = '1';
			break;
		case cur_time:
			tmp[5] = '8';
			break;
		case nxt_DST_chnge:
			tmp[5] = '4';
			break;
		case ref_time_upd:
			tmp[5] = '2';
			break;
		case link_loss:
			tmp[5] = '1';
			break;
		case immediate_alert:
			tmp[6] = '8';
			break;
		case tx_power:
			tmp[6] = '4';
			break;
		case alert_notf:
			tmp[6] = '2';
			break;
		case phone_alert_stat:
			tmp[6] = '1';
			break;
		case scan_param:
			tmp[7] = '4';
			break;
		case user_def_priv_servc:
			tmp[10] = '1';
			break;
	}
	uartSendData(tmp);
	
}

int8_t set_connection_params(uint16_t interval, uint16_t latency, uint16_t timeout)
{
	char tmp[17] = "ST,0000,0000,0000";
	char tmp_interval[5];
	char tmp_lat[5];
	char tmp_timeout[5];

	if ((interval < 0x0006) || (interval > 0x0C80))
	return -1;
	if ((latency > 0x01F3) || (latency > (timeout * 10 / (interval * 1.25 - 1))))
	return -1;
	if ((timeout < 0x000A) || (timeout > 0x0C80))
	return -1;

	sprintf(tmp_interval,"%04x", interval);
	sprintf(tmp_lat, "%04x", latency);
	sprintf(tmp_timeout, "%04x", timeout);

	replaceAt(tmp, tmp_interval, 3);
	replaceAt(tmp, tmp_lat, 8);
	replaceAt(tmp, tmp_timeout, 13);

	uartSendData(tmp);

	return 0;
}


//***********Getter Functions***********//

uint8_t get_baud()
{
	replaceAt(cmd, "GB",0);
	uartSendData(cmd);
	if(uartCheckReceived())
	{
		return uartGetReceivedChar();
	}
}

uint8_t get_sec_set_MLDP()
{
	replaceAt(cmd, "GE",0);
	uartSendData(cmd);
	if(uartCheckReceived())
	{
		return uartGetReceivedChar();
	}
}

void get_firmware_v(char* gtstr10)
{
	replaceAt(cmd, "GDF",0);
	uartSendData(cmd);
	uartGetReceivedString(gtstr10);
}

uint32_t get_timer_expi_set(set_timer_t timer)
{
	switch (timer)
	{
	case Timer1:
		replaceAt(cmd,"GM,1",0);
		uartSendData(cmd);
		break;
	case Timer2:
		replaceAt(cmd, "GM,2",0);
		uartSendData(cmd);
		break;
	case Timer3:
		replaceAt(cmd, "GM,3",0);
		uartSendData(cmd);
		break;
	}
	uartGetReceivedString(shortResponse);
	uint32_t t = 0;
	sscanf(shortResponse, "%x", &t);
	return t;
}

void get_device_name(char* gtstr21)
{
	replaceAt(cmd, "GN",0);
	uartSendData(cmd);
	uartGetReceivedString(gtstr21);
}

uint8_t get_transmpwr_lvl()
{
	replaceAt(cmd, "GP",0);
	uartSendData(cmd);
	if(uartCheckReceived())return uartGetReceivedChar();
}

void get_feature_settings(char* gtstr9)
{
	replaceAt(cmd, "GR",0);
	uartSendData(cmd);
	if(uartCheckReceived())uartGetReceivedString(gtstr9);
}

void get_sup_server_serv(char* gtstr9)
{
	replaceAt(cmd, "GS",0);
	uartSendData(cmd);
	if(uartCheckReceived())uartGetReceivedString(gtstr9);
}

void get_connection_settings(char* gtstr15)
{
	replaceAt(cmd, "GT",0);
	uartSendData(cmd);
	if(uartCheckReceived())uartGetReceivedString(gtstr15);
}


//***********Action Functions***********//

void echo(echo_state_t state)
{
	char On[8] = "Echo On";
	char Off[9] = "Echo Off";
	uartTransmitChar('+');
	uartGetReceivedString(shortResponse);
	switch (state)
	{
		case OFF:
		if (strcmp(Off, shortResponse))echo(OFF);
		else break;
		case ON:
		if (strcmp(On, shortResponse))echo(ON);
		else break;
	}
}

void set_analogpin_out(pin_t pin, uint16_t val)
{
	if ((val >= 0)&&(val <= 1300))
	{
		char tmp[10] = "@O,0,0000";
		char tmp2[5] = "";
		switch (pin)
		{
			case APIN0:
				break;
			case APIN1:
				tmp[3] = '1';
				break;
			case APIN2:
				tmp[3] = '2';
				break;
		}
		sprintf(tmp2, "%04x", val);
		replaceAt(tmp, tmp2,5);
		uartSendData(tmp);
	}
	
}

uint32_t get_analogpin_in(pin_t pin)
{
	char tmp[5] = "@I,0";
	switch (pin)
	{
		case APIN0:
			break;
		case APIN1:
			replaceAt(tmp, '1',3);
			break;
		case APIN2:
			replaceAt(tmp, '2',3);
			break;
	}
	uartSendData(tmp);
	uartGetReceivedString(shortResponse);
	uint32_t v = 0;
	sscanf(shortResponse, "%x", &v);
	return v;
}

void set_digitalpin_out(char pin1_15, char state1_15)
{
	char tmp[9] = "|O,00,00";
	char tmp2[3] = "";
	sprintf(tmp2, "%02x", pin1_15);
	replaceAt(tmp, tmp2,3);
	sprintf(tmp2, "%02x", state1_15);
	replaceAt(tmp, tmp2,6);
	uartSendData(tmp);
}

char get_digitalpin_in(char pin1_15)
{
	char tmp[9] = "|I,00";
	char tmp2[3] = "";
	sprintf(tmp2, "%02x", pin1_15);
	replaceAt(tmp, tmp2,3);
	uartSendData(tmp);
	uartGetReceivedString(rllyshortResponse);
	uint32_t p = 0;
	sscanf(rllyshortResponse, "%x", &p);
	return p;
}

void start_advertise(uint16_t interval, uint16_t tspan)
{
	char tmp[12] = "A,0000,0000";
	char tmp2[5] = "";
	if(((interval == 0)&&(tspan == 0))||(interval >= tspan))uartTransmitChar('A');
	else 
	{
		sprintf(tmp2, "%04x", interval);
		replaceAt(tmp, tmp2,2);
		sprintf(tmp2, "%04x", tspan);
		replaceAt(tmp, tmp2,7);
		uartSendData(tmp);
	}
}

void bond(bond_saving_t bonding)
{
	char tmp[4] = "B,0";
	if (bonding == 0)
	{
		uartSendData(tmp);
	}
	else
	uartTransmitChar('B');
}


void dump_configuration()
{
	uartTransmitChar('D');
}

char establish_connection(mac_address_t type, char* address)
{
	char tmp[17] = "E,0,000000000000";

	if (type == 1)
	tmp[2] = '1';

	else if (type != 0)
	return -1;

	if (strlen (address) > 12)
	return -1;

	replaceAt(tmp, address,4);

	uartSendData(tmp);

	return 0;
}

void start_scan(uint16_t interval, uint16_t tspan)
{
	char tmp[12] = "F,0000,0000";
	char tmp2[5] = "";
	if((interval == 0)&&(tspan == 0))uartTransmitChar('F');
	else
	{
		sprintf(tmp2, "%04x", interval);
		replaceAt(tmp, tmp2,2);
		sprintf(tmp2, "%04x", tspan);
		replaceAt(tmp, tmp2,7);
		uartSendData(tmp);
	}
}

void helpup2fw1_20()
{
	uartTransmitChar('H');
}

void enter_observer_role(observer_t state, send_rec_t rec)
{
	char tmp[4] = "J,0";
	if(state)tmp[2] = '1';
	uartSendData(tmp);
	if(rec)uartTransmitChar('F');
}

void disconnect()
{
	uartTransmitChar('K');
}

uint32_t get_connection_signal_strenght()
{
	uartTransmitChar('M');
	uartGetReceivedString(shortResponse);
	uint32_t p = 0;
	sscanf(shortResponse, "%x", &p);
	return p;
}

char set_broadcast_information(char* msgstringhex, char strlnth, send_rec_t startadvertisement)
{
	if(strlnth < 28)
	{
		msgstringhex[0] = "N";
		msgstringhex[1] = ',';
		uartSendData(msgstringhex);
		if(startadvertisement == 2)uartTransmitChar('A');
		return 0;
	}
	else return -1;
}

void enter_dormant_state()
{
	
}

void retrieve_connection_status()
{
	
}

void reboot()
{
	
}

void chge_par_cur_con()
{
	
}

void unbond()
{
	
}

void show_firmware_v()
{
	
}

void stop_scan()
{
	
}

void stop_advertisement()
{
	
}

void stop_connecting()
{
	
}


//***********I2C Functions***********//

void enable_I2C()
{
	
}

void disable_I2C()
{
	
}

void read_EEPROM()
{
	
}

void write_EEPROM()
{
	
}

void gen_I2C_bus_event()
{
	
}

void I2C_read_data()
{
	
}

void I2C_write_data()
{
	
}
	

//***********PWM Functions***********//

void turn_PWM_on()
{
	
}

void turn_PWM_off()
{
	
}
	
	
//***********Services Functions***********//	
	
void list_client_services()
{
	
}

void list_server_services()
{
	
}

void read_val2client_handle()
{
	
}

void write_val2client_handle()
{
	
}

void read_conf_client_UUID()
{
	
}

void read_val_client_UUID()
{
	
}

void notify_client_ab_characteristic()
{
	
}

void write_val2client_UUID()
{
	
}

void read_val_server_handle()
{
	
}

void write_val2server_handle()
{
	
}

void read_val_server_UUID()
{
	
}

void write_val2server_UUID()
{
	
}


//***********Private Services Functions***********//

void set_priv_char_UUID()
{
	
}

void set_primary_service_UUID_filter()
{
	
}

void set_priv_service_UUID()
{
	
}

void clr_priv_service()
{
	
}


//***********MLDP Functions***********//

void set_MLDP_sec_mode()
{
	
}

void enter_MLDP_mode()
{
	
}


//***********Scripting Functions***********//

void show_script()
{
	
}

void clear_script()
{
	
}

void pause_script()
{
	
}

void run_script()
{
	
}

void write_script()
{
	
}


//***********Remote Functions***********//

void set_remote_mode()
{
	
}


//***********Firmware Update Functions***********//

void device_firmware_update()
{
	
}

/*************** END OF FUNCTIONS ***************************************************************************/