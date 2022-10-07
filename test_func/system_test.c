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
#define DUPLEX_HALF 0x00
#define DUPLEX_FULL 0x01

#define ETHTOOL_GSET 0x00000001 /* Get settings command for ethtool */

struct ethtool_cmd {
	unsigned int cmd;
	unsigned int supported; /* Features this interface supports */
	unsigned int advertising; /* Features this interface advertises */
	unsigned short speed; /* The forced speed, 10Mb, 100Mb, gigabit */
	unsigned char duplex; /* Duplex, half or full */
	unsigned char port; /* Which connector port */
	unsigned char phy_address;
	unsigned char transceiver; /* Which tranceiver to use */
	unsigned char autoneg; /* Enable or disable autonegotiation */
	unsigned int maxtxpkt; /* Tx pkts before generating tx int */
	unsigned int maxrxpkt; /* Rx pkts before generating rx int */
	unsigned int reserved[4];
};

//------------------------------------------------------------------------------
#define	SYSTEM_DISPLAY_SIZE	"/sys/class/graphics/fb0/virtual_size"

// Client HDMI connect to VU5 (800x480)
#define	SYSTEM_LCD_SIZE_X	800
#define	SYSTEM_LCD_SIZE_Y	480

struct eth_info_t {
	char	name[20];
	char	ip[20];
	char	mac[20];
	int		link_speed;
};

struct system_test_t {
	int		mem_size;	// GB size
	int		fb_x, fb_y;
	int		lcd_x, lcd_y;

	// Ethernet info
	struct eth_info_t eth_info;
};

struct system_test_t client_system;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static int get_ip_addr (const char *eth_name, char *ip, int *link_speed)
{
	int fd;
	struct ifreq ifr;
	struct ethtool_cmd ecmd;

	/* this entire function is almost copied from ethtool source code */
	/* Open control socket. */
	*link_speed = 0;
	sprintf (ip, "---.---.---.---");

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return -1;

	strncpy(ifr.ifr_name, eth_name, IFNAMSIZ);
	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0)
		return -1;

	inet_ntop(AF_INET, ifr.ifr_addr.sa_data+2, ip, sizeof(struct sockaddr));

	/* Pass the "get info" command to eth tool driver */
	ecmd.cmd = ETHTOOL_GSET;
	ifr.ifr_data = (caddr_t)&ecmd;

	/* ioctl failed: */
	if (ioctl(fd, SIOCETHTOOL, &ifr))
	{
		close(fd);
		return -1;
	}
	close(fd);

	*link_speed = ecmd.speed;
	return 0;
}

//------------------------------------------------------------------------------
static int get_mac_addr (char *mac_str)
{
	int sock, if_count, i;
	struct ifconf ifc;
	struct ifreq ifr[10];
	unsigned char mac[6];

	memset(&ifc, 0, sizeof(struct ifconf));

	sprintf(mac_str, "xx:xx:xx:xx:xx:xx");
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return	-1;

	// 검색 최대 개수는 10개
	ifc.ifc_len = 10 * sizeof(struct ifreq);
	ifc.ifc_buf = (char *)ifr;

	// 네트웨크 카드 장치의 정보 얻어옴.
	ioctl(sock, SIOCGIFCONF, (char *)&ifc);

	// 읽어온 장치의 개수 파악
	if_count = ifc.ifc_len / (sizeof(struct ifreq));
	for (i = 0; i < if_count; i++) {
		if (ioctl(sock, SIOCGIFHWADDR, &ifr[i]) == 0) {
			memcpy(mac, ifr[i].ifr_hwaddr.sa_data, 6);
			info ("find device (%s), mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
				ifr[i].ifr_name, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

			if (!strncmp(ifr[i].ifr_name, "eth", sizeof("eth")-1)) {
				memset (mac_str, 0, 20);
				sprintf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x",
							mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
				close (sock);
				return 0;
			}
		}
	}
	close (sock);
	return -1;
}

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

	if (!get_mac_addr(client_system.eth_info.mac)) {
		info ("MAC ADDR : %s\n", client_system.eth_info.mac);
		if (!get_ip_addr("eth0", client_system.eth_info.ip,
							&client_system.eth_info.link_speed))
			info ("IP ADDR : %s, link speed = %d\n",
					client_system.eth_info.ip,
					client_system.eth_info.link_speed);
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
		if			(!strncmp(ptr, "MEM", sizeof("MEM")-1)) {
			sprintf (resp_msg, "%d,%d GB", 1, client_system.mem_size); 
		} else if	(!strncmp(ptr, "LCD", sizeof("LCD")-1)) {
			bool status;
			if ((client_system.fb_x != client_system.lcd_x) ||
				(client_system.fb_y != client_system.lcd_y))
				status = 0;
			else
				status = 1;
			sprintf (resp_msg, "%d,%dx%d", status, client_system.fb_x, client_system.fb_y); 
		} else if	(!strncmp(ptr, "ETH_IP"  , sizeof("ETH_IP")-1)) {
			sprintf (resp_msg, "%d,%s",
						client_system.eth_info.link_speed ? 1 : 0,
						client_system.eth_info.ip);
		} else if	(!strncmp(ptr, "ETH_MAC" , sizeof("ETH_MAC")-1)) {
			sprintf (resp_msg, "%d,%s",
						client_system.eth_info.link_speed ? 1 : 0,
						client_system.eth_info.mac);
		} else if	(!strncmp(ptr, "ETH_LINK", sizeof("ETH_LINK")-1)) {
			sprintf (resp_msg, "%d,%d Mb/s",
						client_system.eth_info.link_speed ? 1 : 0,
						client_system.eth_info.link_speed);
		} else {
			info ("%s : msg unknown %s\n", __func__,  ptr);
			return -1;
		}
		info ("msg = %s, resp = %s\n", msg, resp_msg);
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
