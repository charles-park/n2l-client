//------------------------------------------------------------------------------
/**
 * @file protocol.h
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID-N2L JIG UART protocol application.
 * @version 0.1
 * @date 2022-10-3
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//------------------------------------------------------------------------------
#ifndef	__PROTOCOL_H__
#define	__PROTOCOL_H__

#include "typedefs.h"

//------------------------------------------------------------------------------
/* Server to Client data size is 10 bytes */
//------------------------------------------------------------------------------
#pragma packet(1)
typedef struct server_protocol__t {
	/* @ : start protocol signal */
	__s8	head;

	/*
		command description:
			server to client : 'C'ommand, 'R'eady(boot)
			client to server : 'O'kay, 'A'ck, 'R'eady(boot), 'E'rror, 'B'usy
	*/
	__s8	cmd;
	__s8	uid[3];
	__s8	group[10];
	/* msg no, msg group, msg data1, msg data2, ... */
	__s8	data[10];

	__s8	reserved[6];
	/* # : end protocol signal */
	__s8	tail;
}	server_protocol_t;

#pragma packet(1)
typedef union server_protocol__u {
	server_protocol_t 	p;
	__u8				bytes[sizeof(server_protocol_t)];
}	server_protocol_u;

//------------------------------------------------------------------------------
/* Client to Server data size is 16 bytes */
#pragma packet(1)
typedef struct client_protocol__t {
	/* @ : start protocol signal */
	__s8	head;

	/*
		command description:
			server to client : 'C'ommand, 'R'eady(boot)
			client to server : 'O'kay, 'A'ck, 'R'eady(boot), 'E'rror, 'B'usy
	*/
	__s8	resp;
	__s8	uid[3];
	__s8	status;

	/* msg no, msg group, msg data1, msg data2, ... */
	__s8	data[16];

	__s8	reserved[9];
	/* # : end protocol signal */
	__s8	tail;
}	client_protocol_t;

#pragma packet(1)
typedef union client_protocol__u {
	client_protocol_t 	p;
	__u8				bytes[sizeof(client_protocol_t)];
}	client_protocol_u;

//------------------------------------------------------------------------------
// function prototype define
//------------------------------------------------------------------------------
extern	int 	protocol_catch		(ptc_var_t *var);
extern	int 	protocol_check		(ptc_var_t *var);
extern	int 	receive_msg_check 	(ptc_grp_t *puart, char *recv_msg);
extern	void 	send_msg 			(ptc_grp_t *puart, char *uid, char *resp_msg);

//------------------------------------------------------------------------------
#endif	// #define	__PROTOCOL_H__

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
