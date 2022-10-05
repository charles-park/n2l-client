//------------------------------------------------------------------------------
/**
 * @file header40_test.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID-N2L JIG header40 test function.
 * @version 0.1
 * @date 2022-10-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#include "../common.h"
#include "../typedefs.h"
#include "../lib_gpio/lib_gpio.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#define LED_POWER_GPIO		502
#define	LED_POWER_ON		1
#define	LED_POWER_OFF		0

#define	LED_ALIVE_PATH		"/sys/class/leds/blue:heartbeat"
#define LED_ALIVE_TRIGGER	"trigger"
#define	LED_ALIVE_CTRL		"brightness"
#define	LED_ALIVE_ON		255
#define	LED_ALIVE_OFF		0

#define	LED_POWER		0
#define	LED_ALIVE		1

struct led_test_t {
	bool	is_file;
	bool	status;
	char	filename[256];
};

struct led_test_t led[2];

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void led_test_init (void)
{
	memset (led, 0x00, sizeof(led));

	info ("---------------------------------\n");
	info ("[ %s : %s ]\n", __FILE__, __func__);

	fwrite_str		("/sys/class/leds/blue:heartbeat/trigger", "none");
	gpio_export    	(LED_POWER_GPIO);
	gpio_direction 	(LED_POWER_GPIO, GPIO_DIR_OUT);

	// confirm /sys/class/leds/blue:heartbeat/brightness
	sprintf (led[LED_ALIVE].filename, "%s/%s",
				LED_ALIVE_PATH, LED_ALIVE_CTRL);

	if (access (led[LED_ALIVE].filename, R_OK) == 0)
		led[LED_ALIVE].is_file = true;

	// confirm /sys/class/gpio502/value
	sprintf (led[LED_POWER].filename, "/sys/class/gpio/gpio%d/value",
				LED_POWER_GPIO);
	if (access (led[LED_POWER].filename, R_OK) == 0)
		led[LED_POWER].is_file = true;

	info ("LED ALIVE filename = %s, is_file = %s\n",
		led[LED_ALIVE].filename, led[LED_ALIVE].is_file ? "true" : "false");
	info ("LED POWER filename = %s, is_file = %s\n",
		led[LED_POWER].filename, led[LED_POWER].is_file ? "true" : "false");
	info ("---------------------------------\n");
}

//------------------------------------------------------------------------------
int led_test_func (char *msg, char *resp_msg)
{
	char *ptr, msg_clone[20], fname[128];
	int	ret, ctrl_led;

	info ("msg = %s\n", msg);
	memset (msg_clone, 0x00, sizeof(msg_clone));
	sprintf (msg_clone, "%s", msg);

	memset (fname, 0x00, sizeof(fname));
	if ((ptr = strtok (msg_clone, ",")) != NULL) {
		ptr = toupperstr(ptr);

		if			(!strncmp(ptr, "ALIVE_ON", sizeof("ALIVE_ON"))) {
			ctrl_led = LED_ALIVE;	led[ctrl_led].status = true;
		} else if	(!strncmp(ptr, "ALIVE_OFF", sizeof("ALIVE_OFF"))) {
			ctrl_led = LED_ALIVE;	led[ctrl_led].status = false;
		} else if	(!strncmp(ptr, "POWER_ON", sizeof("POWER_ON"))) {
			ctrl_led = LED_POWER;	led[ctrl_led].status = true;
		} else if	(!strncmp(ptr, "POWER_OFF", sizeof("POWER_OFF"))) {
			ctrl_led = LED_POWER;	led[ctrl_led].status = false;
		} else {
			info ("msg unknown");
			return -1;
		}
		if (!led[ctrl_led].is_file) {
			info ("Err file : %s not found.", led[ctrl_led].filename);
			return -1;
		}

		ret = fwrite_bool (led[ctrl_led].filename, led[ctrl_led].status);
		/* resp msg : [status, led status, adc2, ] */
		sprintf (resp_msg, "%d,%d", ret, led[ctrl_led].status);
		return	0;

	}
	info ("msg null pointer err!\n");
	return -1;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void led_test_exit (void)
{
	info ("%s\n", __func__);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
