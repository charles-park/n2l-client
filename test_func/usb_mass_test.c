//------------------------------------------------------------------------------
/**
 * @file usb_mass_test.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID-N2L JIG usb_mass test function.
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
// USB Connector & Position name
//------------------------------------------------------------------------------
/*
	+---------------+
	| USB 3 | USB 4 |	USB3 : LEFT_UP,	USB4 : RIGHT_UP
	+-------+-------+
	| USB 1 | USB 2 |	USB1 : LEFT_DN,	USB2 : RIGHT_DN
	+-------+-------+ 

	ODROID-N2L (BACK View)
	                 +----------------+
	                 | USB 3 (USB3.0) |	USB3 : LEFT_UP (USB30/USB20)
	+-----+ +------+ +----------------+
	| PWR | | HDMI | | USB 1 (USB2.0) |	USB1 : LEFT_DN (USB20)
	+-----+ +------+ +----------------+
*/
//------------------------------------------------------------------------------
/* USB spec, CON pos, Detect Filename : 5000, 480, 12 */
//------------------------------------------------------------------------------
// Storage Read (16 Mbytes, 1 block count)
//------------------------------------------------------------------------------
#define	USB_MASS_TEST_CMD	"dd of=/dev/null bs=16M count=1 iflag=nocache,dsync oflag=nocache,dsync if=/dev/"

#define	USB_L_DN_FILENAME	"/sys/bus/usb/devices/1-2"
#define	USB_L_DN_SPEED		480
#define	USB_R_DN_FILENAME	""
#define	USB_R_DN_SPEED		0
#define	USB_L_UP_FILENAME	"/sys/bus/usb/devices/2-1"
#define	USB_L_UP_SPEED		5000
#define	USB_R_UP_FILENAME	""
#define	USB_R_UP_SPEED		0

//------------------------------------------------------------------------------
enum ITEM_USB_PORT {
	ITEM_USB_L_DN = 0,
	ITEM_USB_R_DN,
	ITEM_USB_L_UP,
	ITEM_USB_R_UP,
	ITEM_USB_END
};

static char USB_MASS_TEST_FLAG[128];

struct usb_test_t {
	bool	status, is_mass;
	int		speed, check_speed, read_speed;
	char	fname[128], mass_node[8];
};

static struct usb_test_t usb[ITEM_USB_END];

//------------------------------------------------------------------------------
bool get_usb_speed (struct usb_test_t *usb)
{
	if (access(usb->fname, R_OK) == 0) {
		FILE *fp;
		char rdata[256], *ptr;

		memset  (rdata, 0x00, sizeof(rdata));
		sprintf (rdata, "%s/speed", usb->fname);

		usb->speed  = fread_int (rdata);
		usb->status	= (usb->check_speed != usb->speed) ? false : true;

		memset  (rdata, 0x00, sizeof(rdata));
		sprintf (rdata, "find %s/ -name sd* 2<&1", usb->fname);
		// mass-storage check
		if ((fp = popen(rdata, "r")) != NULL) {
			memset (rdata, 0x00, sizeof(rdata));
			fgets (rdata, sizeof(rdata), fp);
			pclose(fp);
			if ((ptr = strstr (rdata, "sd")) != NULL) {
				usb->is_mass = true;
				strncpy (usb->mass_node, ptr, strlen(ptr)-1);
				memset  (rdata, 0x00, sizeof(rdata));
				sprintf (rdata, "%s%s 2<&1", USB_MASS_TEST_FLAG, usb->mass_node);
				if ((fp = popen(rdata, "r")) != NULL) {
					memset (rdata, 0x00, sizeof(rdata));
					while (fgets (rdata, sizeof(rdata), fp) != NULL) {
						if ((ptr = strstr (rdata, " MB/s")) != NULL) {
							while (*ptr != ',')		ptr--;

							usb->read_speed = atoi(ptr+1);
							info ("find mass-storage node = /dev/%s, read speed = %d MB/s\n",
										usb->mass_node, usb->read_speed);
						}
					}
					pclose(fp);
				}
			}
		}
		info ("status = %d, usb speed = %d, fname = %s, is_mass = %d\n",
							usb->status, usb->speed, usb->fname, usb->is_mass);
	}
	return	usb->status;
}

//------------------------------------------------------------------------------
void usb_test_init (void)
{
	info ("---------------------------------\n");
	info ("[ %s : %s ]\n", __FILE__, __func__);

	memset (usb, 0x00, sizeof(usb));

	// APP config data read
	{
		char	int_str[8];

		if (find_appcfg_data ("STORAGE_TEST_FLAG", USB_MASS_TEST_FLAG))
			sprintf (USB_MASS_TEST_FLAG, "%s", USB_MASS_TEST_CMD);

		// USB PORT LEFT-DN
		if (find_appcfg_data ("USB_L_DN_FILENAME", usb[ITEM_USB_L_DN].fname))
			sprintf (usb[ITEM_USB_L_DN].fname, "%s", USB_L_DN_FILENAME);

		memset (int_str, 0x00, sizeof(int_str));
		if (find_appcfg_data ("USB_L_DN_SPEED", int_str))
			usb[ITEM_USB_L_DN].check_speed = USB_L_DN_SPEED;
		else
			usb[ITEM_USB_L_DN].check_speed = atoi(int_str);

		// USB PORT RIGHT-DN
		if (find_appcfg_data ("USB_R_DN_FILENAME", usb[ITEM_USB_R_DN].fname))
			sprintf (usb[ITEM_USB_R_DN].fname, "%s", USB_R_DN_FILENAME);

		memset (int_str, 0x00, sizeof(int_str));
		if (find_appcfg_data ("USB_R_DN_SPEED", int_str))
			usb[ITEM_USB_R_DN].check_speed = USB_R_DN_SPEED;
		else
			usb[ITEM_USB_R_DN].check_speed = atoi(int_str);

		// USB PORT LEFT-UP
		if (find_appcfg_data ("USB_L_UP_FILENAME", usb[ITEM_USB_L_UP].fname))
			sprintf (usb[ITEM_USB_L_UP].fname, "%s", USB_L_UP_FILENAME);

		memset (int_str, 0x00, sizeof(int_str));
		if (find_appcfg_data ("USB_L_UP_SPEED", int_str))
			usb[ITEM_USB_L_UP].check_speed = USB_L_UP_SPEED;
		else
			usb[ITEM_USB_L_UP].check_speed = atoi(int_str);

		// USB PORT RIGHT-UP
		if (find_appcfg_data ("USB_R_UP_FILENAME", usb[ITEM_USB_R_UP].fname))
			sprintf (usb[ITEM_USB_R_UP].fname, "%s", USB_R_UP_FILENAME);

		memset (int_str, 0x00, sizeof(int_str));
		if (find_appcfg_data ("USB_R_UP_SPEED", int_str))
			usb[ITEM_USB_R_UP].check_speed = USB_R_UP_SPEED;
		else
			usb[ITEM_USB_R_UP].check_speed = atoi(int_str);
	}

	{
		int item;

		for (item = 0; item < ITEM_USB_END; item++) {
			if (usb[item].check_speed)	get_usb_speed (&usb[item]);
		}
	}

	info ("---------------------------------\n");
}

//------------------------------------------------------------------------------
// msg desc : CON hpos(up, dn), CON vpos(left, right)
// return : 0 -> sucess, -1 -> msg error
//------------------------------------------------------------------------------
int usb_test_func (char *msg, char *resp_msg)
{
	char	*ptr, msg_clone[20], item, item_is_port;

	memset (msg_clone, 0x00, sizeof(msg_clone));
	sprintf (msg_clone, "%s", msg);

	if ((ptr = strtok (msg_clone, ",")) != NULL) {
		ptr = toupperstr(ptr);

		if 			(!strncmp(ptr, "L_DN_PORT", sizeof("L_DN_PORT")-1)) {
			item = ITEM_USB_L_DN;	item_is_port = true;
		} else  if	(!strncmp(ptr, "L_UP_PORT", sizeof("L_UP_PORT")-1)) {
			item = ITEM_USB_L_UP;	item_is_port = true;
		} else  if	(!strncmp(ptr, "R_DN_PORT", sizeof("R_DN_PORT")-1)) {
			item = ITEM_USB_R_DN;	item_is_port = true;
		} else  if	(!strncmp(ptr, "R_UP_PORT", sizeof("R_UP_PORT")-1)) {
			item = ITEM_USB_R_UP;	item_is_port = true;
		} else  if	(!strncmp(ptr, "L_DN_READ", sizeof("L_DN_READ")-1)) {
			item = ITEM_USB_L_DN;	item_is_port = false;
		} else  if	(!strncmp(ptr, "L_UP_READ", sizeof("L_UP_READ")-1)) {
			item = ITEM_USB_L_UP;	item_is_port = false;
		} else  if	(!strncmp(ptr, "R_DN_READ", sizeof("R_DN_READ")-1)) {
			item = ITEM_USB_R_DN;	item_is_port = false;
		} else  if	(!strncmp(ptr, "R_UP_READ", sizeof("R_UP_READ")-1)) {
			item = ITEM_USB_R_UP;	item_is_port = false;
		} else {
			info ("%s : msg unknown %s\n", __func__,  ptr);
			return -1;
		}
		/* resp msg : [status, get_speed-read mb/s */
		if (item_is_port)
			sprintf (resp_msg, "%d,%d",	usb[item].status, usb[item].speed);
		else
			sprintf (resp_msg, "%d,%d MB/s",
				usb[item].status, usb[item].is_mass ? usb[item].read_speed : 0);

		info ("msg = %s, resp = %s\n", msg, resp_msg);
		return	0;
	}
	info ("msg null pointer err!\n");
	return -1;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void usb_test_exit (void)
{
	info ("%s\n", __func__);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
