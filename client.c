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

//------------------------------------------------------------------------------
// Default global value
//------------------------------------------------------------------------------
#define	CLIENT_FB_DEVICE		"/dev/fb0"
#define	CLIENT_UART_DEVICE		"/dev/ttyS0"
#define	CLIENT_UI_CONFIG		"ui.cfg"
#define	ALIVE_DISPLAY_R_ITEM	2
#define	ALIVE_DISPLAY_IMTERVAL	2000	/* 2000 ms */
#define	APP_LOOP_DELAY			500		/* 1000 us */

#define	RECEIVE_MSG_SIZE		sizeof(recv_protocol_u)
#define	RESPONSE_MSG_SIZE		sizeof(send_protocol_u)

//#define	UART_MSG_TEST
//------------------------------------------------------------------------------
struct client_t {
	// connect to server ???
	bool		is_connect;

	/* FB name */
	char		fb_dev[128];
	char		uart_dev[128];
	char		ui_config[128];

	fb_info_t	*pfb;
	ui_grp_t	*pui;
	ptc_grp_t	*puart;
	char		recv_msg[RECEIVE_MSG_SIZE];

	int			alive_r_item;
};

//------------------------------------------------------------------------------
// Function prototype
//------------------------------------------------------------------------------
void 	test_func_init 		(void);
void 	test_func_exit 		(void);
int 	test_func_run 		(struct client_t *pclient, char *group, char *msg, char *resp_msg);
int 	app_init 			(struct client_t *pclient) ;
void 	app_exit 			(struct client_t *pclient);
void 	ui_item_update 		(struct client_t *pclient, int uid, bool is_info, char *resp_msg);
void 	ui_item_init 		(struct client_t *pclient);
void 	recv_msg_parse 		(struct client_t *pclient, char *resp_msg);
bool 	run_interval_check 	(struct timeval *t, double interval_ms);
void 	client_alive 		(struct client_t *pclient);
int 	main				(int argc, char **argv);

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
	if 		(!strncmp(group, "ADC"    , sizeof("ADC")    -1))	return	adc_test_func 		(msg, resp_msg);
	else if	(!strncmp(group, "AUDIO"  , sizeof("AUDIO")  -1))	return	audio_test_func 	(msg, resp_msg);
	else if	(!strncmp(group, "FAN"    , sizeof("FAN")    -1))	return	fan_test_func 		(msg, resp_msg);
	else if	(!strncmp(group, "HDMI"   , sizeof("HDMI")   -1))	return	hdmi_test_func 		(msg, resp_msg);
	else if	(!strncmp(group, "HEADER" , sizeof("HEADER") -1))	return	header40_test_func 	(msg, resp_msg);
	else if	(!strncmp(group, "LED"    , sizeof("LED")    -1))	return	led_test_func 		(msg, resp_msg);
	else if	(!strncmp(group, "STORAGE", sizeof("STORAGE")-1))	return	storage_test_func 	(msg, resp_msg);
	else if	(!strncmp(group, "SYSTEM" , sizeof("SYSTEM") -1))	return	system_test_func 	(msg, resp_msg);
	else if	(!strncmp(group, "USB"    , sizeof("USB")    -1))	return	usb_test_func 		(msg, resp_msg);

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
		char int_str[8];
		if (find_appcfg_data ("CLIENT_FB_DEVICE",   pclient->fb_dev))
			sprintf (pclient->fb_dev,    "%s", CLIENT_FB_DEVICE);
		if (find_appcfg_data ("CLIENT_UART_DEVICE", pclient->uart_dev))
			sprintf (pclient->uart_dev,  "%s", CLIENT_UART_DEVICE);
		if (find_appcfg_data ("CLIENT_UI_CONFIG",   pclient->ui_config))
			sprintf (pclient->ui_config, "%s", CLIENT_UI_CONFIG);

		memset (int_str, 0x00, sizeof(int_str));
		if (find_appcfg_data ("ALIVE_DISPLAY_R_ITEM", int_str))
			pclient->alive_r_item = ALIVE_DISPLAY_R_ITEM;
		else
			pclient->alive_r_item = atoi(int_str);
	}
	info ("CLIENT_FB_DEVICE     = %s\n", pclient->fb_dev);
	info ("CLIENT_UART_DEVICE   = %s\n", pclient->uart_dev);
	info ("CLIENT_UI_CONFIG     = %s\n", pclient->ui_config);
	info ("ALIVE_DISPLAY_R_ITEM = %d\n", pclient->alive_r_item);

	pclient->pfb	= fb_init 	(pclient->fb_dev);
	pclient->pui	= ui_init	(pclient->pfb, pclient->ui_config) ;
	pclient->puart	= uart_init (pclient->uart_dev, B115200);

	if ((pclient->pfb == NULL) || (pclient->pui == NULL) || (pclient->puart == NULL)) {
		err ("SYSTEM Initialize fail. Program Exit!\n");
		exit(0);
	}

	/* UART recv protocol install */
	if (pclient->puart) {
		if (ptc_grp_init (pclient->puart, 1)) {
			if (!ptc_func_init (pclient->puart, 0, sizeof(recv_protocol_u),
									protocol_check, protocol_catch)) {
				err ("uart0 protocol install fail");
				exit(0);
			}
		}
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
	char resp_msg[RESPONSE_MSG_SIZE];
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
	// for parse msg
	char uid[3+1], group[10+1], msg[10+1], status, resp_cmd;

	memset (   uid, 0x00, sizeof(uid));	memset ( group, 0x00, sizeof(group));
	memset (   msg, 0x00, sizeof(msg));

	memcpy (   uid, &pclient->recv_msg[0] , sizeof(uid)   -1);
	memcpy ( group, &pclient->recv_msg[3] , sizeof(group) -1);
	memcpy (   msg, &pclient->recv_msg[13], sizeof(msg)   -1);

	// connect to server 'A' CMD & 'BOOT' Group
	if (!pclient->is_connect) {
		if (!strncmp(group, "BOOT", sizeof("BOOT") -1))
			pclient->is_connect = true;

		info ("Server and Client connection yet... %d, %s\n", pclient->is_connect, group);
		return;
	} else {
		if (!strncmp(group, "REBOOT", sizeof("REBOOT") -1) && pclient->is_connect) {
			pclient->is_connect = false;
			info ("Server Reset... Server to Client disconnect...\n");
			return;
		}
	}

	// test func run
	status = test_func_run (pclient, group, msg, resp_msg);

	switch (status)
	{
		case	1:	resp_cmd = 'F';		break;	/* Fail */
		case	0:	resp_cmd = 'O';		break;	/* Okay */
		default	:	resp_cmd = 'B';		break;	/* Busy */
	}

	/* response to server */
	protocol_msg_send (pclient->puart, resp_cmd, uid, resp_msg);
	memset (pclient->recv_msg, 0x00, sizeof(pclient->recv_msg));
}

//------------------------------------------------------------------------------
bool run_interval_check (struct timeval *t, double interval_ms)
{
	struct timeval base_time;
	double difftime;

	gettimeofday(&base_time, NULL);

	if (interval_ms) {
		/* 현재 시간이 interval시간보다 크면 양수가 나옴 */
		difftime = (base_time.tv_sec - t->tv_sec) +
					((base_time.tv_usec - (t->tv_usec + interval_ms * 1000)) / 1000000);

		if (difftime > 0) {
			t->tv_sec  = base_time.tv_sec;
			t->tv_usec = base_time.tv_usec;
			return true;
		}
		return false;
	}
	/* 현재 시간 저장 */
	t->tv_sec  = base_time.tv_sec;
	t->tv_usec = base_time.tv_usec;
	return true;
}

//------------------------------------------------------------------------------
void client_alive (struct client_t *pclient)
{
	static struct timeval i_time;
	static bool onoff = false;

	if (run_interval_check(&i_time, ALIVE_DISPLAY_IMTERVAL)) {
		ui_set_ritem (pclient->pfb, pclient->pui, pclient->alive_r_item,
					onoff ? COLOR_GREEN : pclient->pui->bc.uint, -1);
		onoff = !onoff;
		if (!pclient->is_connect && onoff)
			protocol_msg_send (pclient->puart, 'R', "001", "1,BOOT");
	}
}

//------------------------------------------------------------------------------
#if defined(UART_MSG_TEST)
void uart_msg_test (struct client_t *pclient)
{
	char resp_msg[20], uid[4];

	memset (uid, 0x00, sizeof(uid));	sprintf (uid, "%03d", rand() % 1000);
	test_func_run (pclient, "ADC", "ADC_37", resp_msg);
	protocol_msg_send (pclient->puart, 'O', uid, resp_msg);
	test_func_run (pclient, "ADC", "ADC_40", resp_msg);
	protocol_msg_send (pclient->puart, 'F', uid, resp_msg);
	test_func_run (pclient, "ADC", "ADC_37_MV", resp_msg);
	protocol_msg_send (pclient->puart, 'B', uid, resp_msg);
	test_func_run (pclient, "ADC", "ADC_40_MV", resp_msg);
	protocol_msg_send (pclient->puart, '0', uid, resp_msg);

	memset (uid, 0x00, sizeof(uid));	sprintf (uid, "%03d", rand() % 1000);
	test_func_run (pclient, "AUDIO", "L_CH", resp_msg);
	protocol_msg_send (pclient->puart, 'O', uid, resp_msg);
	sleep(2);
	test_func_run (pclient, "AUDIO", "R_CH", resp_msg);
	protocol_msg_send (pclient->puart, 'F', uid, resp_msg);
	sleep(2);

	memset (uid, 0x00, sizeof(uid));	sprintf (uid, "%03d", rand() % 1000);
	test_func_run (pclient, "FAN", "ON", resp_msg);
	protocol_msg_send (pclient->puart, 'B', uid, resp_msg);
	test_func_run (pclient, "FAN", "OFF", resp_msg);
	protocol_msg_send (pclient->puart, 'O', uid, resp_msg);

	memset (uid, 0x00, sizeof(uid));	sprintf (uid, "%03d", rand() % 1000);
	test_func_run (pclient, "HDMI", "EDID", resp_msg);
	protocol_msg_send (pclient->puart, 'F', uid, resp_msg);
	test_func_run (pclient, "HDMI", "HPD", resp_msg);
	protocol_msg_send (pclient->puart, 'B', uid, resp_msg);
}
#endif

//------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	struct client_t	client;
	char resp_msg[RESPONSE_MSG_SIZE];

	test_func_init ();	app_init (&client);		ui_item_init (&client);

	// Send msg test func
#if defined(UART_MSG_TEST)
	uart_msg_test (&client);
#endif
	while (true) {
		client_alive (&client);

		if (protocol_msg_check (client.puart, client.recv_msg))
			recv_msg_parse (&client, resp_msg);

		usleep(APP_LOOP_DELAY);
	}

	test_func_exit ();	app_exit(&client);
	return 0;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
