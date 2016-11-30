//#include "stdio.h"
//#include "stdlib.h"
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_interface.h"
#include "espconn.h"
#include "hw_timer.c"

#define HW_TIMER_US 5

#define SERVO_CNT_DUTY_US 1500 // center at 1.5 milliseconds as per spec
#define SERVO_MIN_DUTY_US 600
#define SERVO_MAX_DUTY_US 2400
#define SERVO_FREQ 20000 / HW_TIMER_US
#define SERVO_PIN BIT0

#define MOTOR_FREQ 50 / HW_TIMER_US
#define MOTOR_MIN_DUTY 0
#define MOTOR_MAX_DUTY MOTOR_FREQ
#define MOTOR_PIN BIT2

struct espconn udp_conn;
esp_udp udp;
volatile uint32 servo_pulse = SERVO_CNT_DUTY_US / HW_TIMER_US;
volatile uint32 motor_duty = MOTOR_MAX_DUTY;
volatile bool s_pull_low_p = false; // pull servo low predicate
volatile bool m_pull_low_p = true; // pull motor low predicate
volatile uint32 low_mask = MOTOR_PIN;
volatile uint32 high_mask = SERVO_PIN;
volatile uint32 last_low = MOTOR_PIN;
volatile uint32 last_high = SERVO_PIN;

/**
 * callback function used by the hardware timer
 */
LOCAL void
hw_ctrl_cb(void)
{
	static uint32 servo_count_timer = 0;
	static uint32 motor_count_timer = 0;

	servo_count_timer++;
	motor_count_timer++;

	s_pull_low_p = servo_count_timer >= servo_pulse;
	m_pull_low_p = (motor_count_timer >= motor_duty) &&
		(motor_duty != MOTOR_MAX_DUTY);

	if (s_pull_low_p && m_pull_low_p) {
		high_mask = 0;
		low_mask = SERVO_PIN | MOTOR_PIN;
	} else if (s_pull_low_p && !m_pull_low_p) {
		high_mask = MOTOR_PIN;
		low_mask = SERVO_PIN;
	} else if (!s_pull_low_p && m_pull_low_p) {
		high_mask = SERVO_PIN;
		low_mask = MOTOR_PIN;
	} else if (!s_pull_low_p && !m_pull_low_p) {
		high_mask = SERVO_PIN | MOTOR_PIN;
		low_mask = 0;
	}

	if (servo_count_timer >= SERVO_FREQ) {
		servo_count_timer = 0;
	}

	if (motor_count_timer >= MOTOR_FREQ) {
		motor_count_timer = 0;
	}

	if ((high_mask != last_high) || (low_mask != last_low)) {
		gpio_output_set(high_mask, low_mask, 0, 0);
	}
	last_high = high_mask;
	last_low = low_mask;
}

// Servo control stuff.
LOCAL void
servo_set_pulse(uint32 microseconds) {
	if (microseconds >= SERVO_MIN_DUTY_US && microseconds <= SERVO_MAX_DUTY_US) {
		servo_pulse = microseconds / HW_TIMER_US;
		os_printf("us: %d, pulse: %d\n", microseconds, servo_pulse);
	} else {
		//os_printf("overflow/underflow: %d us\n", microseconds);
	}
}

LOCAL void
motor_set_duty(uint32 percent) {
	if (percent >= 0 && percent <= 100) {
		motor_duty = ((MOTOR_MAX_DUTY * percent) / 100);
		os_printf("%: %d, duty: %d\n", percent, motor_duty);
	}
}

LOCAL void ICACHE_FLASH_ATTR
servo_conn_recv_cb(void *arg, char *data, unsigned short len)
{
	if (len > 1) {
		char *end;
		if (data[0] == 'M') {
			motor_set_duty((uint32)strtol(data+1, &end, 10));
		} else if (len < 32) {
			servo_set_pulse((uint32)strtol(data, &end, 10));
		}
	}
}

LOCAL void ICACHE_FLASH_ATTR
user_set_station_config(void)
{
	char ssid[32] = "--";
	char password[64] = "";
	struct station_config stationConf;

	stationConf.bssid_set = 0;
	os_memcpy(&stationConf.ssid, ssid, 32);
	os_memcpy(&stationConf.password, password, 64);

	wifi_station_set_config(&stationConf);
}

LOCAL void ICACHE_FLASH_ATTR
setup_udp()
{
	sint8 err;
	udp_conn.type = ESPCONN_UDP;
	udp_conn.state = ESPCONN_NONE;
	udp.local_port = 53850;
	udp_conn.proto.udp = &udp;
	err = espconn_create(&udp_conn);
}

LOCAL void ICACHE_FLASH_ATTR
wifi_event_handler(System_Event_t *event)
{
	switch(event->event)
	{
		case EVENT_STAMODE_CONNECTED:
			os_printf("Connected to %s\n", event->event_info.connected.ssid);
			break;
		case EVENT_STAMODE_GOT_IP:
			os_printf("setting up udp...\n");
			setup_udp();
			os_printf("udp set up, registering callback... \n");
			espconn_regist_recvcb(&udp_conn, servo_conn_recv_cb);
			os_printf("callback registered\n");
			break;
		default: break;
	}
}

void ICACHE_FLASH_ATTR
user_init()
{
	hw_timer_init(FRC1_SOURCE, 1);
	hw_timer_set_func(hw_ctrl_cb);

	gpio_init();
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	gpio_output_set(SERVO_PIN, MOTOR_PIN, MOTOR_PIN | SERVO_PIN, 0);

	hw_timer_arm(HW_TIMER_US);

	// set UART baud rate
	// uart_init(115200, 115200); can't find the correct header.
	// thank you http://kacangbawang.com/esp8266-sdk-os_printf-prints-garbage/
	uart_div_modify(0, UART_CLK_FREQ / 115200);

	// set up wifi access
	wifi_set_event_handler_cb(wifi_event_handler);
	wifi_set_opmode(STATION_MODE);
	user_set_station_config();
}
