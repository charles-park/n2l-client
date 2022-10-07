//------------------------------------------------------------------------------
/**
 * @file adc_test.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID-N2L JIG adc test function.
 * @version 0.1
 * @date 2022-10-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#include "../common.h"
#include "../typedefs.h"

//------------------------------------------------------------------------------
/* adc voltage cal = 1.8V / 4096 * raw value */
//------------------------------------------------------------------------------
// HEADER 20x2 (40), 441mV(FIX)
#define ADC_40_FILENAME		"/sys/bus/iio/devices/iio:device0/in_voltage2_raw"
#define	ADC_40_IN_MAX		500
#define	ADC_40_IN_MIN		400

// HEADER 20x2 (37), 1350mV(FIX)
#define ADC_37_FILENAME		"/sys/bus/iio/devices/iio:device0/in_voltage3_raw"
#define	ADC_37_IN_MAX		1400
#define	ADC_37_IN_MIN		1300

#define	ADC_37		0
#define	ADC_40		1

#define	RETRY_COUNT		5

//------------------------------------------------------------------------------
struct adc_test_t {
	bool	status;
	int		value;
	int		voltage;
	int		max, min;
	char	fname[128];
};

struct adc_test_t adc[2];
//------------------------------------------------------------------------------
void adc_test_init (void)
{
	memset (adc, 0x00, sizeof(adc));

	// APP config data read
	{
		char int_str[8];

		if (find_appcfg_data ("ADC_37_FILENAME",  adc[ADC_37].fname))
			sprintf(adc[ADC_37].fname, "%s", ADC_37_FILENAME);

		memset (int_str, 0x00, sizeof(int_str));
		if (find_appcfg_data ("ADC_37_MAX",  int_str))
			adc[ADC_37].max = ADC_37_IN_MAX;
		else
			adc[ADC_37].max = atoi (int_str);

		memset (int_str, 0x00, sizeof(int_str));
		if (find_appcfg_data ("ADC_37_MIN",  int_str))
			adc[ADC_37].min = ADC_37_IN_MIN;
		else
			adc[ADC_37].min = atoi (int_str);

		if (find_appcfg_data ("ADC_40_FILENAME",  adc[ADC_40].fname))
			sprintf(adc[ADC_40].fname, "%s", ADC_40_FILENAME);

		memset (int_str, 0x00, sizeof(int_str));
		if (find_appcfg_data ("ADC_40_MAX",  int_str))
			adc[ADC_40].max = ADC_40_IN_MAX;
		else
			adc[ADC_40].max = atoi (int_str);

		memset (int_str, 0x00, sizeof(int_str));
		if (find_appcfg_data ("ADC_40_MIN",  int_str))
			adc[ADC_40].min = ADC_40_IN_MIN;
		else
			adc[ADC_40].min = atoi (int_str);
	}

	{
		int retry, i, adc_ch;

		info ("---------------------------------\n");
		info ("[ %s : %s ]\n", __FILE__, __func__);
		for (adc_ch = 0; adc_ch < 2; adc_ch++) {
		info ("adc[adc_ch].fname = %s(%d)\n", adc[adc_ch].fname, (int)strlen(adc[adc_ch].fname));
			if (access (adc[adc_ch].fname, R_OK) == 0) {
				for (i = 0, retry = RETRY_COUNT; i < retry; i++) {
					adc[adc_ch].value = fread_int (adc[adc_ch].fname);
					adc[adc_ch].voltage = 1800 * adc[adc_ch].value / 4096;
					if ((adc[adc_ch].voltage > adc[adc_ch].min) && 
						(adc[adc_ch].voltage < adc[adc_ch].max) ) {
						adc[adc_ch].status = true;
						break;
					}
				} 
			}
			info ("filename = %s\n", adc[adc_ch].fname);
			info ("adc = %d, volt = %d mV, max = %d, min = %d, status = %d\n",
											adc[adc_ch].value,
											adc[adc_ch].voltage,
											adc[adc_ch].max,
											adc[adc_ch].min,
											adc[adc_ch].status);
		}
		info ("---------------------------------\n");
	}
}

//------------------------------------------------------------------------------
int adc_test_func (char *msg, char *resp_msg)
{
	char *ptr, msg_clone[20], item, item_is_mv;

	info ("msg = %s\n", msg);
	memset (msg_clone, 0x00, sizeof(msg_clone));
	sprintf (msg_clone, "%s", msg);

	if ((ptr = strtok (msg_clone, ",")) != NULL) {
		ptr = toupperstr(ptr);

		if      (!strncmp(ptr, "ADC_37", sizeof("ADC_37")-1)) {
			item = ADC_37;	item_is_mv = false;
		}
		else if (!strncmp(ptr, "ADC_40", sizeof("ADC_40")-1)) {
			item = ADC_40;	item_is_mv = false;
		}
		else if (!strncmp(ptr, "ADC_37_MV", sizeof("ADC_37_MV")-1)) {
			item = ADC_37;	item_is_mv = true;
		}
		else if (!strncmp(ptr, "ADC_40_MV", sizeof("ADC_40_MV")-1)) {
			item = ADC_40;	item_is_mv = true;
		} else {
			info ("%s : msg unknown %s\n", __func__,  ptr);
			return -1;
		}

		/* resp msg : [status, get_speed-read mb/s */
		if (item_is_mv)
			sprintf (resp_msg, "%d,%04d mV",
				adc[item].status, adc[item].voltage);
		else
			sprintf (resp_msg, "%d,%s",
				adc[item].status, adc[item].status ? "PASS" : "FAIL");

		info ("msg = %s, resp = %s\n", msg, resp_msg);
		return	0;
	}
	info ("msg null pointer err!\n");
	return -1;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void adc_test_exit (void)
{
	info ("%s\n", __func__);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
