#include "stdio.h"
#include "stdlib.h"
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_interface.h"
#include "espconn.h"
#include "hw_timer.c"

#define HW_TIMER_US 5

#define SERVO_CNT_DUTY_US 1500 // 1.5 milliseconds as per spec
// according to http://www.servodatabase.com/servo/hitec/hs-50
#define SERVO_MIN_DUTY_US 600
#define SERVO_MAX_DUTY_US 2400
#define SERVO_FREQ 20000 / HW_TIMER_US

struct espconn udp_conn;
esp_udp udp;
volatile uint32 pulse = SERVO_CNT_DUTY_US / HW_TIMER_US;

void hw_servo_cb(void)
{
	static uint32 servo_count_timer = 0;
	servo_count_timer++;
	if (servo_count_timer >= pulse) {
		gpio_output_set(0, BIT2, 0, 0);
	}
	if (servo_count_timer >= SERVO_FREQ) {
		gpio_output_set(BIT2, 0, 0, 0);
		servo_count_timer = 0;
	}
}

// Servo control stuff.
void servo_set_pulse(uint32 microseconds) {
	pulse = microseconds / HW_TIMER_US;
	os_printf("us: %d, pulse: %d\n", microseconds, pulse);
}

LOCAL void
servo_conn_recv_cb(void *arg, char *data, unsigned short len)
{
	if (len < 32)
	{
		char *end;
		servo_set_pulse((uint32)strtol(data, &end, 10));
	} else {
		os_printf("too long data: %s", data);
	}
}

void ICACHE_FLASH_ATTR user_set_station_config(void)
{
	char ssid[32] = "++";
	char password[64] = "samsalabim";
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
			os_printf("Connected to %s\n",
					event->event_info.connected.ssid);
			break;
		case EVENT_STAMODE_GOT_IP:
			os_printf("setting up udp\n");
			setup_udp();
			os_printf("udp set up, registering callback\n");
			espconn_regist_recvcb(&udp_conn, servo_conn_recv_cb);
			os_printf("registered callback\n");
			break;
		default: break;
	}
}


void ICACHE_FLASH_ATTR user_init()
{
	hw_timer_init(FRC1_SOURCE,1);
	hw_timer_set_func(hw_servo_cb);

	gpio_init();
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	gpio_output_set(BIT2, 0, BIT2, 0);

	hw_timer_arm(HW_TIMER_US);

	// set UART baud rate
	// uart_init(115200, 115200); Can't find the correct header.
	// thank you http://kacangbawang.com/esp8266-sdk-os_printf-prints-garbage/
	uart_div_modify(0, UART_CLK_FREQ / 115200);

	// set up wifi access
	wifi_set_event_handler_cb(wifi_event_handler);
	wifi_set_opmode(STATION_MODE);
	user_set_station_config();
	//gpio_output_set(0, (1 << pin), 0, 0);
}
