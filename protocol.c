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
int protocol_check(ptc_var_t *var)
{
	/* head & tail check with protocol size */
	if(var->buf[(var->p_sp + var->size -1) % var->size] != '#')	return 0;
	if(var->buf[(var->p_sp               ) % var->size] != '@')	return 0;
	return 1;
}

//------------------------------------------------------------------------------
int protocol_catch(ptc_var_t *var)
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
void send_msg (ptc_grp_t *puart, char *uid, char *resp_msg)
{
	int len = sizeof(client_protocol_u), i;
	char *ptr;
	client_protocol_u	client;

	memset(&client, ' ', sizeof(client_protocol_u));

	if ((ptr = strtok (resp_msg, ",")) != NULL) {
		client.p.status = *ptr;
		if ((ptr = strtok (NULL, ",")) != NULL)
			sprintf (client.p.data, "%s", ptr);
	}
	strncpy(client.p.uid, uid, sizeof(client.p.uid));

	client.p.head = '@';	client.p.tail = '#';
	client.p.resp = 'O';	// Okay

	info ("%s : %s\n", __func__, client.bytes);
	for (i = 0; i < sizeof(client_protocol_u); i++)
		queue_put(&puart->tx_q, &client.bytes[i]);
}

//------------------------------------------------------------------------------
int receive_msg_check (ptc_grp_t *puart, char *recv_msg)
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
