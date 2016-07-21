#include "pwm.h"

#define SERVO_CNT_DUTY ((1500 * 1000) / 45)
#define SERVO_MIN_DUTY ((600 * 1000) / 45)
#define SERVO_MAX_DUTY ((2400 * 1000) / 45)

#define SERVO_SLOPE (SERVO_MAX_DUTY - SERVO_MIN_DUTY) / 179

#define PWM_0_OUT_IO_MUX  PERIPHS_IO_MUX_GPIO2_U
#define PWM_0_OUT_IO_NUM  2
#define PWM_0_OUT_IO_FUNC FUNC_GPIO2
#define PWM_CHANNEL 1

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
