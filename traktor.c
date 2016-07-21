#include "stdio.h"
#include "stdlib.h"
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_interface.h"
#include "espconn.h"
#include "pwm.h"

#define SERVO_CNT_DUTY_US 1500 // 1.5 milliseconds as per spec
// according to http://www.servodatabase.com/servo/hitec/hs-50
#define SERVO_MIN_DUTY_US 600
#define SERVO_MAX_DUTY_US 2400

#define SERVO_CNT_DUTY ((SERVO_CNT_DUTY_US * 1000) / 45)
#define SERVO_MIN_DUTY ((SERVO_MIN_DUTY_US * 1000) / 45)
#define SERVO_MAX_DUTY ((SERVO_MAX_DUTY_US * 1000) / 45)

#define SERVO_SLOPE (SERVO_MAX_DUTY - SERVO_MIN_DUTY) / 179

#define PWM_0_OUT_IO_MUX  PERIPHS_IO_MUX_GPIO2_U
#define PWM_0_OUT_IO_NUM  2
#define PWM_0_OUT_IO_FUNC FUNC_GPIO2
#define PWM_CHANNEL 1

struct espconn udp_conn;
esp_udp udp;
uint32 duty[PWM_CHANNEL] = {SERVO_CNT_DUTY};

LOCAL void servo_set_deg(uint32 degrees)
{
	uint32 dt;
	if (degrees < 361 && degrees >= 0) {
		dt = SERVO_MIN_DUTY + SERVO_SLOPE * degrees;
	} else {
		dt = degrees;
	}
	pwm_set_duty(dt, 0);
	duty[0] = pwm_get_duty(0);
	os_printf("duty: %d\n", duty[0]);
}

LOCAL void ICACHE_FLASH_ATTR
servo_conn_recv_cb(void *arg, char *data, unsigned short len)
{
	if (len < 32)
	{
		char *end;
		servo_set_deg((uint32)strtol(data, &end, 10));
	} else {
		os_printf("too long data: %s", data);
	}
}

void setup_pwm()
{
	//PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	//gpio_output_set(0,0,(1 << 2), 0);
	uint32 io_info[][3] = {{PWM_0_OUT_IO_MUX, PWM_0_OUT_IO_FUNC, PWM_0_OUT_IO_NUM}};
	//uint32 period = 21600;
	uint32 period = 1000000;
	pwm_init(period, duty, PWM_CHANNEL, io_info);
	pwm_start();
	os_printf("pwm initialized, period: %d\n", pwm_get_period());
	os_printf("duty 0: %d\n", pwm_get_duty(0));
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
			setup_pwm();
			break;
		default: break;
	}
}


void ICACHE_FLASH_ATTR user_init()
{
  // init gpio sussytem
  gpio_init();

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
