#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_interface.h"

static const int pin = 1;
static volatile os_timer_t some_timer;

void some_timerfunc(void *arg)
{
  //Do blinky stuff
  if (GPIO_REG_READ(GPIO_OUT_ADDRESS) & (1 << pin))
  {
    // set gpio low
    gpio_output_set(0, (1 << pin), 0, 0);
  }
  else
  {
    // set gpio high
    gpio_output_set((1 << pin), 0, 0, 0);
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

void wifi_event_handler(System_Event_t *event)
{
	switch(event->event)
	{
		case EVENT_STAMODE_CONNECTED:
			os_printf("Connected to %s\n",
					event->event_info.connected.ssid);
			break;
		case EVENT_STAMODE_GOT_IP:
			os_printf("Got IP: " IPSTR, IP2STR(&event->event_info.got_ip.ip));
			os_printf("\n");
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
