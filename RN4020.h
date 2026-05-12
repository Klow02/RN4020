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

#ifndef RN4020_H_
#define RN4020_H_

/******************************************************************************
* Includes
*******************************************************************************/
#include <stdint.h>
#include <stdbool.h>

/******************************************************************************
* Preprocessor Constants
*******************************************************************************/

#define RECVD 1
#define WAITING 0

/******************************************************************************
* Configuration Constants
*******************************************************************************/

/******************************************************************************
* Macros
*******************************************************************************/

/******************************************************************************
* Typedefs
*******************************************************************************/

typedef enum
{
	BR_2400 = 0,
	BR_9600,
	BR_19200,
	BR_38400,
	BR_115200,
	BR_230400,
	BR_460800,
	BR_921600
}baud_rate_t;

typedef enum
{
	RESET_SOME = 1,
	RESET_ALL
}reset_parameter_t;

typedef enum
{
	Timer1 = 1,
	Timer2,
	Timer3,
}set_timer_t;

typedef enum
{
	central = 0,
	realtimeread,
	autoAdvertise,
	enable_MLDP,
	auto_MLDP,
	no_dir_adv,
	flow_control,
	run_scpt_afr_pwron,
	enable_auth,
	enable_rem_cmd,
	no_bond_save,
	io_cap,
	block_setcmd_in_remote,
	enable_OTA,
	iOS_mode,
	server_only,
	enable_UART_in_srpt,
	autoenter_MLDP,
	no_MLDP_echo
}feature_pick_t;

typedef enum
{
	device_info = 0,
	battery,
	heart_rate,
	health_thermo,
	glucose,
	blood_pressure,
	run_speed_cadence,
	cyc_speed_cadence,
	cur_time,
	nxt_DST_chnge,
	ref_time_upd,
	link_loss,
	immediate_alert,
	tx_power,
	alert_notf,
	phone_alert_stat,
	scan_param,
	user_def_priv_servc
}server_services_t;

typedef enum
{
	OFF,
	ON
}echo_state_t;

typedef enum
{
	APIN0,
	APIN1,
	APIN2
}pin_t;

typedef enum
{
	NO_SAVE = 0,
	SAVED,
}bond_saving_t;

typedef enum
{
	PUBLIC_ADDRESS = 0,
	RANDOM_ADDRESS,
}mac_address_t;

typedef enum
{
	ENTER = 1,
	EXIT = 0,
}observer_t;

typedef enum
{
	RECEIVE = 1,
	SEND = 2,
	WAIT = 0
}send_rec_t;

typedef enum
{
	NOTIFICATION_STOP = 0,
	NOTIFICATION_START = 1
}configuration_t;

typedef enum
{
	NO_SECURITY = 0,
	OTA_ENCRYPTED,
	OTA_AUTHENTICATED,
}security_t;

typedef enum
{
	PW_ON = 0,
	TIMER1,
	TIMER2,
	CONN,
	DISCON,
	PIOL,
	PIOH,
	ALERTH,
	ALERTL,
	ALERTO
}debug_event_t;

typedef enum
{
	FW_UART = 1,
	FW_OTA,
}fw_update_t;
/******************************************************************************
* Variables
*******************************************************************************/

/******************************************************************************
* Function Prototypes
*******************************************************************************/

//***********UART Functions***********//

void uartInit(unsigned int baud);
void uartTransmitChar(char data);
unsigned char uartGetReceivedChar();
void uartGetReceivedString(char* Response);
unsigned char uartCheckReceived();
void uartSendData(char* data);

//***********Format Functions***********//

/*Replaces the Destinationstring with the Sourcestring starting at the IndexPosition */
void replaceAt(char* destinationString, const char* sourceString, uint8_t startIndex);

//***********Setter Functions***********//

/**
 * @brief
 * Sets the serialized Bluetooth-friendly name of the device, where the string is up to 15 alphanumeric characters.
 * @param device_name
 * Desired bluetooth name
 */
void set_serialized_name(char* name);

/**
 * @brief
 * Sets the baud rate of the UART communication
 * @param baud_rate
 * desired baud rate, from 2400 to 921K
 * @return
 * 0 if succesful, -1 if invalid baud rate passed
 */
int8_t set_baudrate(baud_rate_t baud);

/**
 * @brief
 * This command resets the configurations to the factory default at the next reboot.
 * @param reset
 * When the input parameter is ‘1’, a majority of the settings will be restored to the
 * factory default, but some settings, such as device name, device info, script and
 * private services, stay the same. When the input parameter is ‘2’, all parameters are
 * restored to factory default.
 * @return
 * 0 if succesful, -1 if failed
 */
int8_t factory_reset(reset_parameter_t reset);

/**
 * @brief
 * Starts the application timers or stops them 
 * @param timer, us
 * First parameter is the timerindicator, the second is the time in uS. Time can be set up to 7FFFFFFF
 */
void set_timer(set_timer_t timer, uint32_t us);

/**
 * @brief
 * Sets the devicename with up to 20 characters
 * @return
 * return 0 if succesfull and -1 if not
 */
int8_t set_name(char* name);

/**
 * @brief
 * Sets the device transmission power in a range from 1 to 7
 * @return
 * return 0 if succesfull and -1 if not
 */
int8_t set_transmission_power(char powerlevel);

/**
 * @brief
 * Sets the supported features of current RN4020 module.
 * @param features
 * 32-bit bitmap that indicates features to be supported. After changing the features, a reboot is necessary to make the changes effective.
 */
void set_features(feature_pick_t feature);

/**
 *@brief
 *Sets the supported services of the device in server role.
 *@param service
 *32-bit bitmap that indicates services to be supported. After changing the services, a reboot is necessary to make the changes effective.
 */
void set_server_services(server_services_t service);

/**
 * @brief
 * Sets the initial connection parameters for future connections
 * @param interval
 * Desired connection interval
 * @param latency
 * Desired connection latency
 * @param timeout
 * Desired connection timeout
 * @return
 */
int8_t set_connection_params(uint16_t interval, uint16_t latency, uint16_t timeout);


//***********Getter Functions***********//

uint8_t get_baud();
uint8_t get_sec_set_MLDP();
void get_firmware_v(char* gtstr10);
uint32_t get_timer_expi_set(set_timer_t timer);
void get_device_name(char* gtstr21);
uint8_t get_transmpwr_lvl();
void get_feature_settings(char* gtstr9);
void get_sup_server_serv(char* gtstr9);
void get_connection_settings(char* gtstr15);


//***********Action Functions***********//

void echo(echo_state_t state);
void set_analogpin_out(pin_t pin, uint16_t val);
uint32_t get_analogpin_in(pin_t pin);
void set_digitalpin_out(char pin1_15, char state1_15);
char get_digitalpin_in(char pin1_15);
void start_advertise(uint16_t interval, uint16_t tspan);
void bond(bond_saving_t bonding);
void dump_configuration();
char establish_connection(mac_address_t type, char* address);
void start_scan();
void helpup2fw1_20();
void enter_observer_role(observer_t state, send_rec_t rec);
void disconnect();
uint32_t get_connection_signal_strenght();
char set_broadcast_information(char* msgstringhex, char strlnth, send_rec_t startadvertisement);
void enter_dormant_state();
void retrieve_connection_status();
void reboot();
void chge_par_cur_con();
void unbond();
void show_firmware_v();
void stop_scan();
void stop_advertisement();
void stop_connecting();


//***********I2C Functions***********//

void enable_I2C();
void disable_I2C();
void read_EEPROM();
void write_EEPROM();
void gen_I2C_bus_event();
void I2C_read_data();
void I2C_write_data();
	

//***********PWM Functions***********//

void turn_PWM_on();
void turn_PWM_off();
	
	
//***********Services Functions***********//	
	
void list_client_services();
void list_server_services();
void read_val2client_handle();
void write_val2client_handle();
void read_conf_client_UUID();
void read_val_client_UUID();
void notify_client_ab_characteristic();
void write_val2client_UUID();
void read_val_server_handle();
void write_val2server_handle();
void read_val_server_UUID();
void write_val2server_UUID();


//***********Private Services Functions***********//

void set_priv_char_UUID();
void set_primary_service_UUID_filter();
void set_priv_service_UUID();
void clr_priv_service();


//***********MLDP Functions***********//

void set_MLDP_sec_mode();
void enter_MLDP_mode();


//***********Scripting Functions***********//

void show_script();
void clear_script();
void pause_script();
void run_script();
void write_script();


//***********Remote Functions***********//

void set_remote_mode();


//***********Firmware Update Functions***********//

void device_firmware_update();

#endif /* RN4020_H_ */

