#include "stdio.h"
#include "stdlib.h"
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_interface.h"
#include "espconn.h"

struct espconn udp_conn;
esp_udp udp;

LOCAL void servo_set_deg(short degrees)
{
	os_printf("degrees: %d\n", degrees);
}

LOCAL void ICACHE_FLASH_ATTR
servo_conn_recv_cb(void *arg, char *data, unsigned short len)
{
	if (len < 4)
	{
		char *end;
		servo_set_deg((short)strtol(data, &end, 10));
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
