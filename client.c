//------------------------------------------------------------------------------
/**
 * @file client.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID-N2L JIG Clint application.
 * @version 0.1
 * @date 2022-10-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//------------------------------------------------------------------------------
// for my lib
//------------------------------------------------------------------------------
/* 많이 사용되는 define 정의 모음 */
#include "typedefs.h"
#include "common.h"
#include "./lib_fb/lib_fb.h"
#include "./lib_ui/lib_ui.h"
#include "./lib_uart/lib_uart.h"

#include "./test_func/test_func.h"
#include "protocol.h"
//#include "client.h"

//------------------------------------------------------------------------------
// Default global value
//------------------------------------------------------------------------------
#define	CLIENT_FB_DEVICE	"/dev/fb0"
#define	CLIENT_UART_DEVICE	"/dev/ttyS0"
#define	CLIENT_UI_CONFIG	"/root/client/ui.cfg"

//------------------------------------------------------------------------------
struct client_t {
	/* build info */
	char		bdate[32], btime[32];
	/* JIG model name */
	char		model[32];
	/* FB name */
	char		fb_dev[128];
	char		uart_dev[128];
	char		ui_config[128];

	fb_info_t	*pfb;
	ui_grp_t	*pui_grp;
	ptc_grp_t	*puart;
	char		recv_msg[128];
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void test_func_exit (void)
{
	adc_test_exit ();		audio_test_exit ();
	fan_test_exit ();		hdmi_test_exit ();
	header40_test_exit ();	led_test_exit ();
	storage_test_exit ();	system_test_exit ();
	usb_test_exit ();
}

//------------------------------------------------------------------------------
void test_func_run (struct client_t *pclient, char *resp_msg)
{
	// parse msg
	char uid[3], group[10], msg[10];

	memset (   uid, 0x00, sizeof(uid));	memset ( group, 0x00, sizeof(group));
	memset (   msg, 0x00, sizeof(msg));

	memcpy (   uid, &pclient->recv_msg[0] , sizeof(uid));
	memcpy ( group, &pclient->recv_msg[3] , sizeof(group));
	memcpy (   msg, &pclient->recv_msg[13], sizeof(msg));

	if 		(!strncmp(group, "ADC"    , sizeof("ADC")))		adc_test_func 		(msg, resp_msg);
	else if	(!strncmp(group, "AUDIO"  , sizeof("AUDIO")))	audio_test_func 	(msg, resp_msg);
	else if	(!strncmp(group, "FAN"    , sizeof("FAN")))		fan_test_func 		(msg, resp_msg);
	else if	(!strncmp(group, "HDMI"   , sizeof("HDMI")))	hdmi_test_func 		(msg, resp_msg);
	else if	(!strncmp(group, "HEADER" , sizeof("HEADER")))	header40_test_func 	(msg, resp_msg);
	else if	(!strncmp(group, "LED"    , sizeof("LED")))		led_test_func 		(msg, resp_msg);
	else if	(!strncmp(group, "STORAGE", sizeof("STORAGE")))	storage_test_func 	(msg, resp_msg);
	else if	(!strncmp(group, "SYSTEM" , sizeof("SYSTEM")))	system_test_func 	(msg, resp_msg);
	else if	(!strncmp(group, "USB"    , sizeof("USB")))		usb_test_func 		(msg, resp_msg);
	else {
		err ("Unknown msg : %s\n", group);
		return;
	}
	/* response to server */
	send_msg (pclient->puart, uid, resp_msg);
	memset (pclient->recv_msg, 0x00, sizeof(pclient->recv_msg));
}

//------------------------------------------------------------------------------
void test_func_init (void)
{
	adc_test_init ();		audio_test_init ();
	fan_test_init ();		hdmi_test_init ();
	header40_test_init ();	led_test_init ();
	storage_test_init ();	system_test_init ();
	usb_test_init ();
}

//------------------------------------------------------------------------------
int app_init (struct client_t *pclient) 
{
	info ("---------------------------------\n");
	info ("[ %s : %s ]\n", __FILE__, __func__);

	memset (pclient, 0x00, sizeof(struct client_t));
	// APP config data read
	{
		if (find_appcfg_data ("CLIENT_FB_DEVICE",   pclient->fb_dev))
			sprintf (pclient->fb_dev,    "%s", CLIENT_FB_DEVICE);
		if (find_appcfg_data ("CLIENT_UART_DEVICE", pclient->uart_dev))
			sprintf (pclient->uart_dev,  "%s", CLIENT_UART_DEVICE);
		if (find_appcfg_data ("CLIENT_UI_CONFIG",   pclient->ui_config))
			sprintf (pclient->ui_config, "%s", CLIENT_UI_CONFIG);
	}
	info ("CLIENT_FB_DEVICE   = %s\n", pclient->fb_dev);
	info ("CLIENT_UART_DEVICE = %s\n", pclient->uart_dev);
	info ("CLIENT_UI_CONFIG   = %s\n", pclient->ui_config);

	pclient->pfb		= fb_init 	(pclient->fb_dev);
	pclient->pui_grp	= ui_init	(pclient->pfb, pclient->ui_config) ;
	pclient->puart		= uart_init (pclient->uart_dev, B1152000);

	if ((pclient->pfb == NULL) || (pclient->pui_grp == NULL) || (pclient->puart == NULL)) {
		err ("SYSTEM Initialize fail. Program Exit!\n");
		exit(0);
	}
	info ("---------------------------------\n");
	return 0;
}

//------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	struct client_t	client;

	app_init (&client);
	test_func_init ();
	test_func_exit ();
	return 0;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
