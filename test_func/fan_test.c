//------------------------------------------------------------------------------
/**
 * @file common.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID-N2L JIG common used function.
 * @version 0.1
 * @date 2022-10-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//------------------------------------------------------------------------------
#include "../common.h"
#include "../typedefs.h"

//------------------------------------------------------------------------------
// FAN Control File & Value
//------------------------------------------------------------------------------
#define FAN_CONTROL_FILENAME	"/sys/devices/platform/pwm-fan/hwmon/hwmon0/pwm1_enable"
#define	FAN_ENABLE		1
#define	FAN_DISABLE		0

//------------------------------------------------------------------------------
struct fan_test_t {
	bool	is_file;
	bool	status;
	bool	enable;
	char	fname[128];
};

static struct fan_test_t fan;

//------------------------------------------------------------------------------
void fan_test_init (void)
{
	memset (&fan, 0x00, sizeof(fan));

	info ("---------------------------------\n");
	info ("[ %s : %s ]\n", __FILE__, __func__);
	// APP config data read
	{
		if (find_appcfg_data ("FAN_CONTROL_FILENAME",  fan.fname))
			sprintf (fan.fname, "%s", FAN_CONTROL_FILENAME);
	}
	if (access(fan.fname, R_OK) == 0)	fan.is_file = true;
	info ("finename = %s\n", fan.fname);
	info ("---------------------------------\n");
}

//------------------------------------------------------------------------------
int fan_test_func (char *msg, char *resp_msg)
{
	char 	*ptr, msg_clone[20];
	int		ret;

	memset (msg_clone, 0x00, sizeof(msg_clone));
	sprintf (msg_clone, "%s", msg);

	if (!fan.is_file) {
		err ("%s not found!\n", fan.fname);
		return -1;
	}

	if ((ptr = strtok (msg_clone, ",")) != NULL) {
		ptr = toupperstr(ptr);

		if      	(!strncmp(ptr, "ON", sizeof("ON"))) {
			fan.status = true;
			ret = fwrite_bool (fan.fname, fan.status);
		} else if 	(!strncmp(ptr, "OFF", sizeof("OFF"))) {
			fan.status = false;
			ret = fwrite_bool (fan.fname, fan.status);
		} else {
			err ("unknown msg! %s\n", ptr);
			return -1;
		}
		/* resp msg : [status, control status] */
		sprintf (resp_msg, "%d,%d", ret, fan.status);
		return	0;
	}
	err ("msg null pointer err!\n");
	return -1;
}

//------------------------------------------------------------------------------
void fan_test_exit (void)
{
	info ("%s\n", __func__);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
