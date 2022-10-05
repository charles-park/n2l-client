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
#define	CLIENT_UI_CONFIG	"ui.cfg"

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
	ui_grp_t	*pui;
	ptc_grp_t	*puart;
	char		recv_msg[128];
};

//------------------------------------------------------------------------------
// Function prototype
//------------------------------------------------------------------------------
void	test_func_init	(void);
void	test_func_exit	(void);
int		test_func_run	(struct client_t *pclient, char *group, char *msg, char *resp_msg);
int		app_init		(struct client_t *pclient);
void	app_exit		(struct client_t *pclient);
void	ui_item_update	(struct client_t *pclient, int uid, bool is_info, char *resp_msg);
void	ui_item_init	(struct client_t *pclient);
void	recv_msg_parse	(struct client_t *pclient, char *resp_msg);
int		main			(int argc, char **argv);

//------------------------------------------------------------------------------
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
void test_func_exit (void)
{
	adc_test_exit ();		audio_test_exit ();
	fan_test_exit ();		hdmi_test_exit ();
	header40_test_exit ();	led_test_exit ();
	storage_test_exit ();	system_test_exit ();
	usb_test_exit ();
}

//------------------------------------------------------------------------------
int test_func_run (struct client_t *pclient, char *group, char *msg, char *resp_msg)
{
	if 		(!strncmp(group, "ADC"    , sizeof("ADC")))		return	adc_test_func 		(msg, resp_msg);
	else if	(!strncmp(group, "AUDIO"  , sizeof("AUDIO")))	return	audio_test_func 	(msg, resp_msg);
	else if	(!strncmp(group, "FAN"    , sizeof("FAN")))		return	fan_test_func 		(msg, resp_msg);
	else if	(!strncmp(group, "HDMI"   , sizeof("HDMI")))	return	hdmi_test_func 		(msg, resp_msg);
	else if	(!strncmp(group, "HEADER" , sizeof("HEADER")))	return	header40_test_func 	(msg, resp_msg);
	else if	(!strncmp(group, "LED"    , sizeof("LED")))		return	led_test_func 		(msg, resp_msg);
	else if	(!strncmp(group, "STORAGE", sizeof("STORAGE")))	return	storage_test_func 	(msg, resp_msg);
	else if	(!strncmp(group, "SYSTEM" , sizeof("SYSTEM")))	return	system_test_func 	(msg, resp_msg);
	else if	(!strncmp(group, "USB"    , sizeof("USB")))		return	usb_test_func 		(msg, resp_msg);

	err ("Unknown msg : %s\n", group);
	return	false;
}

//------------------------------------------------------------------------------
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

	pclient->pfb	= fb_init 	(pclient->fb_dev);
	pclient->pui	= ui_init	(pclient->pfb, pclient->ui_config) ;
	pclient->puart	= uart_init (pclient->uart_dev, B1152000);

	if ((pclient->pfb == NULL) || (pclient->pui == NULL) || (pclient->puart == NULL)) {
		err ("SYSTEM Initialize fail. Program Exit!\n");
		exit(0);
	}
	info ("---------------------------------\n");
	return 0;
}

//------------------------------------------------------------------------------
void app_exit (struct client_t *pclient)
{
	uart_close(pclient->puart);
	ui_close  (pclient->pui);
	fb_clear  (pclient->pfb);
	fb_close  (pclient->pfb);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void ui_item_update (struct client_t *pclient, int uid, bool is_info, char *resp_msg)
{
	char *ptr = strtok(resp_msg, ",");

	if (ptr != NULL) {
		if (!is_info)
			ui_set_ritem (pclient->pfb, pclient->pui, uid,
							(atoi(ptr) == 1) ? -1 : COLOR_RED, -1);
		if ((ptr = strtok (NULL, ",")) != NULL)
			ui_set_sitem (pclient->pfb, pclient->pui, uid, -1, -1, ptr);
		ui_update    (pclient->pfb, pclient->pui, uid);
	}
}

//------------------------------------------------------------------------------
void ui_item_init (struct client_t *pclient)
{
	int i;
	char resp_msg[20];
	ui_grp_t *pui = pclient->pui;

	for (i = 0; i < pui->i_item_cnt; i++) {
		memset (resp_msg, 0x00, sizeof(resp_msg));

		test_func_run (pclient,	pui->i_item[i].group,
						pui->i_item[i].item, resp_msg);
		ui_item_update (pclient, pui->i_item[i].uid,
						pui->i_item[i].is_info, resp_msg);
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void recv_msg_parse (struct client_t *pclient, char *resp_msg)
{
	// parse msg
	char uid[3], group[10], msg[10];

	memset (   uid, 0x00, sizeof(uid));	memset ( group, 0x00, sizeof(group));
	memset (   msg, 0x00, sizeof(msg));

	memcpy (   uid, &pclient->recv_msg[0] , sizeof(uid));
	memcpy ( group, &pclient->recv_msg[3] , sizeof(group));
	memcpy (   msg, &pclient->recv_msg[13], sizeof(msg));

	// test func run
	test_func_run (pclient, group, msg, resp_msg);

	/* response to server */
	send_msg (pclient->puart, uid, resp_msg);
	memset (pclient->recv_msg, 0x00, sizeof(pclient->recv_msg));
}

//------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	struct client_t	client;

	test_func_init ();
	app_init (&client);
	ui_item_init (&client);

	while (true)
		sleep(1);

	test_func_exit ();
	app_exit(&client);
	return 0;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
