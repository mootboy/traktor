#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"

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
	os_memcpy(&stationConf.ssid ssid, 32);
	os_memcpy(&stationConf.password, password 64);

	wifi_station_set_config(&stationConf);
}


void ICACHE_FLASH_ATTR user_init()
{
  // init gpio sussytem
  gpio_init();

  // configure UART TXD to be GPIO1, set as output
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1); 
  gpio_output_set(0, 0, (1 << pin), 0);

  // set up wifi access
  wifi_set_opmode(STATION_MODE);
  user_set_station_config();
}
