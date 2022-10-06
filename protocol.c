//------------------------------------------------------------------------------
/**
 * @file protocol.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID-N2L JIG UART protocol application.
 * @version 0.1
 * @date 2022-10-3
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <getopt.h>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#include "lib_uart/lib_uart.h"
/* protocol control 함수 */
#include "protocol.h"

//------------------------------------------------------------------------------
//
// https://docs.google.com/spreadsheets/d/18J4B4bqgUbMBA8XeoDkMcKPVEAeNCP2jQm5TOlgKUAo/edit#gid=744952288
//
//------------------------------------------------------------------------------
int protocol_check (ptc_var_t *var)
{
	/* head & tail check with protocol size */
	if(var->buf[(var->p_sp + var->size -1) % var->size] != '#')	return 0;
	if(var->buf[(var->p_sp               ) % var->size] != '@')	return 0;
	return 1;
}

//------------------------------------------------------------------------------
int protocol_catch (ptc_var_t *var)
{
	char cmd = var->buf[(var->p_sp + 1) % var->size];

	switch (cmd) {
		case 'C':	case 'A':	case 'P':
		return 1;
		default :
		break;
	}
	err ("Unknown command = %c\n", cmd);
	return	0;
}

//------------------------------------------------------------------------------
void protocol_msg_send (ptc_grp_t *puart, char ack, char *uid, char *resp_m)
{
	int len = sizeof(send_protocol_u), i;
	char *ptr, resp_msg[20];
	send_protocol_u	send;

	memset(&send, ' ', sizeof(send_protocol_u));
	memset(resp_msg, 0x00, sizeof(resp_msg));
	strncpy (resp_msg, resp_m, sizeof(resp_msg));

	if ((ptr = strtok (resp_msg, ",")) != NULL) {
		send.p.status = *ptr;
		if ((ptr = strtok (NULL, ",")) != NULL) {
			char data[sizeof(send.p.data) +1];
			sprintf (data, "%16s", ptr);
			strncpy (send.p.data, data, sizeof(send.p.data));
		}
	}
	strncpy(send.p.uid, uid, sizeof(send.p.uid));

	send.p.head = '@';	send.p.tail = '#';
	send.p.resp = ack;	// Resp cmd

	for (i = 0; i < sizeof(send_protocol_u); i++)
		queue_put(&puart->tx_q, &send.bytes[i]);

	{
		char LF = '\n', CR = '\r';
		queue_put(&puart->tx_q, &LF);
		queue_put(&puart->tx_q, &CR);
	}
}

//------------------------------------------------------------------------------
int protocol_msg_check (ptc_grp_t *puart, char *recv_msg)
{
	__u8 idata, p_cnt;

	/* uart data processing */
	if (queue_get (&puart->rx_q, &idata))
		ptc_event (puart, idata);

	for (p_cnt = 0; p_cnt < puart->pcnt; p_cnt++) {
		if (puart->p[p_cnt].var.pass) {
			ptc_var_t *var = &puart->p[p_cnt].var;
			int i;

			puart->p[p_cnt].var.pass = false;
			puart->p[p_cnt].var.open = true;

			/* uuid(3) + group(10) + item(10) char size = 23 */
			for (i = 0; i < 23; i++)
				// uuid start position is 2
				recv_msg [i] = var->buf[(var->p_sp + 2 + i) % var->size];
			return	true;
		}
	}
	return	false;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
