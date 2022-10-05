//------------------------------------------------------------------------------
/**
 * @file system_test.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID-N2L JIG system test function.
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

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#define	SYSTEM_DISPLAY_SIZE	"/sys/class/graphics/fb0/virtual_size"

// Client HDMI connect to VU5 (800x480)
#define	SYSTEM_LCD_SIZE_X	800
#define	SYSTEM_LCD_SIZE_Y	480

struct system_test_t {
	int		mem_size;	// GB size
	int		fb_x, fb_y;
	int		lcd_x, lcd_y;
	char	ipaddr[20], mac_addr[20];
};

struct system_test_t client_system;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void system_test_init (void)
{
	FILE	*fp;
	struct 	sysinfo sinfo;
	__u32	mem_size;

	info ("---------------------------------\n");
	info ("[ %s : %s ]\n", __FILE__, __func__);

	memset (&client_system, 0x00, sizeof(client_system));

	// APP config data read
	{
		char get_info[16];
		if (find_appcfg_data ("SYSTEM_LCD_X", get_info))
			client_system.lcd_x = SYSTEM_LCD_SIZE_X;
		else
			client_system.lcd_x = atoi(get_info);

		if (find_appcfg_data ("SYSTEM_LCD_Y", get_info))
			client_system.lcd_y = SYSTEM_LCD_SIZE_Y;
		else
			client_system.lcd_y = atoi(get_info);
	}

	if (access (SYSTEM_DISPLAY_SIZE, R_OK) == 0) {

		if ((fp = fopen(SYSTEM_DISPLAY_SIZE, "r")) != NULL) {
			char rdata[16], *ptr;
			memset (rdata, 0x00, sizeof(rdata));
			if (fgets (rdata, sizeof(rdata), fp) != NULL) {
				if ((ptr = strtok (rdata, ",")) != NULL)
					client_system.fb_x = atoi(ptr);
				if ((ptr = strtok (NULL, ",")) != NULL)
					client_system.fb_y = atoi(ptr) / 2;
			}
			fclose(fp);
		}
	}

	if (!sysinfo (&sinfo)) {
		if (sinfo.totalram) {
			mem_size = sinfo.totalram / 1024 / 1024;

			if		((mem_size > 4096) && (mem_size < 8192))
				client_system.mem_size = 8;
			else if	((mem_size > 2048) && (mem_size < 4096))
				client_system.mem_size = 4;
			else if ((mem_size > 1024) && (mem_size < 2048))
				client_system.mem_size = 2;
			else
				client_system.mem_size = 0;
		}
	}
	info ("fb_x = %d, fb_y = %d, lcd_x = %d, lcd_y = %d, mem = %d GB\n",
		client_system.fb_x , client_system.fb_y,
		client_system.lcd_x, client_system.lcd_y, client_system.mem_size);

	info ("---------------------------------\n");
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int system_test_func (char *msg, char *resp_msg)
{
	char *ptr, msg_clone[20];

	info ("msg = %s\n", msg);
	memset (msg_clone, 0x00, sizeof(msg_clone));
	sprintf (msg_clone, "%s", msg);

	if ((ptr = strtok (msg_clone, ",")) != NULL) {
		ptr = toupperstr(ptr);
		if			(!strncmp(ptr, "MEM", sizeof("MEM"))) {
			sprintf (resp_msg, "%d,%d GB", 1, client_system.mem_size); 
		} else if	(!strncmp(ptr, "LCD", sizeof("LCD"))) {
			bool status;
			if ((client_system.fb_x != client_system.lcd_x) ||
				(client_system.fb_y != client_system.lcd_y))
				status = 0;
			else
				status = 1;
			sprintf (resp_msg, "%d,%dx%d", status, client_system.fb_x, client_system.fb_y); 
		} else {
			info ("err : unknown msg! %s\n", ptr);
			return -1;
		}
		return 0;
	}
	info ("msg null pointer err! %s\n", ptr);
	return -1;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void system_test_exit (void)
{
	info ("%s\n", __func__);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
