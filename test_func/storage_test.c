//------------------------------------------------------------------------------
/**
 * @file storage_test.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID-N2L JIG storage test function.
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
#define	STORAGE_EMMC_FILENAME	"mmcblk0"
#define	STORAGE_SD_FILENAME		"mmcblk1"
// default check speed (MB/s)
#define	STORAGE_EMMC_CHECK_SPEED	140
#define	STORAGE_SD_CHECK_SPEED		40
#define STORAGE_TEST_CMD		"dd of=/dev/null bs=16M count=1 iflag=nocache,dsync oflag=nocache,dsync if=/dev/"

enum eITEM_STORAGE {
	ITEM_STORAGE_EMMC = 0,
	ITEM_STORAGE_SD,
	ITEM_STORAGE_END
};

// Storage Read (16 Mbytes, 1 block count)
static char STORAGE_TEST_FLAG[128];

struct storage_test_t {
	bool	is_file;
	bool	status;
	int		speed, check_speed;
	char	fname[32];
};

struct storage_test_t	storage[ITEM_STORAGE_END];
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void storage_test_init (void)
{
	FILE	*fp;
	char	cmd[256], rdata[256], *ptr, item;

	info ("---------------------------------\n");
	info ("[ %s : %s ]\n", __FILE__, __func__);

	memset (STORAGE_TEST_FLAG, 0x00, sizeof(STORAGE_TEST_FLAG));
	memset (storage, 0x00, sizeof(storage));
	// APP config data read
	{
		char	int_str[8];

		if (find_appcfg_data ("STORAGE_TEST_FLAG", STORAGE_TEST_FLAG))
			sprintf (STORAGE_TEST_FLAG, "%s", STORAGE_TEST_CMD);

		if (find_appcfg_data ("STORAGE_EMMC_FILENAME", storage[ITEM_STORAGE_EMMC].fname))
			sprintf (storage[ITEM_STORAGE_EMMC].fname, "%s", STORAGE_EMMC_FILENAME);
		if (find_appcfg_data ("STORAGE_uSD_FILENAME", storage[ITEM_STORAGE_SD].fname))
			sprintf (storage[ITEM_STORAGE_SD].fname, "%s", STORAGE_SD_FILENAME);
		if (find_appcfg_data ("STORAGE_TEST_CMD", STORAGE_TEST_FLAG))
			sprintf (STORAGE_TEST_FLAG, "%s", STORAGE_TEST_CMD);

		memset (int_str, 0x00, sizeof(int_str));
		if (find_appcfg_data ("STORAGE_EMMC_CHECK_SPEED", int_str))
			storage[ITEM_STORAGE_EMMC].check_speed = STORAGE_EMMC_CHECK_SPEED;
		else
			storage[ITEM_STORAGE_EMMC].check_speed = atoi(int_str);
			
		memset (int_str, 0x00, sizeof(int_str));
		if (find_appcfg_data ("STORAGE_uSD_CHECK_SPEED", int_str))
			storage[ITEM_STORAGE_SD].check_speed = STORAGE_SD_CHECK_SPEED;
		else
			storage[ITEM_STORAGE_SD].check_speed = atoi(int_str);
	}
	info ("TEST Flag = %s\n", STORAGE_TEST_FLAG);
	info ("emmc blk name = %s, sd blk name = %s\n",
			storage[ITEM_STORAGE_EMMC].fname,
			storage[ITEM_STORAGE_SD].fname);
	info ("emmc check speed = %d MB/s, sd check speed = %d MB/s\n",
			storage[ITEM_STORAGE_EMMC].check_speed,
			storage[ITEM_STORAGE_SD].check_speed);

	for (item = 0; item < ITEM_STORAGE_END; item++) {
		char dev_name[128];
		memset (dev_name, 0x00, sizeof(dev_name));
		sprintf (dev_name, "/dev/%s", storage[item].fname);
		if (access (dev_name, R_OK) == 0) {
			storage[item].is_file = true;

			memset (cmd, 0x00, sizeof(cmd));
			memset (rdata, 0x00, sizeof(rdata));
			sprintf (cmd, "%s%s 2<&1", STORAGE_TEST_FLAG, storage[item].fname);
			if ((fp = popen(cmd, "r")) != NULL) {
				while (fgets (rdata, sizeof(rdata), fp) != NULL) {
					if ((ptr = strstr (rdata, " MB/s")) != NULL) {
						while (*ptr != ',')		ptr--;

						storage[item].speed = atoi(ptr+1);
						info ("find %s, Read speed = %d MB/s\n",
							item ? "uSD" : "EMMC", storage[item].speed);
						break;
					}
				}
				pclose(fp);
			}
		}
	}
	info ("---------------------------------\n");
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int storage_test_func (char *msg, char *resp_msg)
{
	char *ptr, msg_clone[20], item;
	int	ret;

	info ("msg = %s\n", msg);
	memset (msg_clone, 0x00, sizeof(msg_clone));
	sprintf (msg_clone, "%s", msg);

	if ((ptr = strtok (msg_clone, ",")) != NULL) {
		ptr = toupperstr(ptr);
		if      (!strncmp(ptr, "EMMC", sizeof("EMMC")-1))	item = ITEM_STORAGE_EMMC;
		else if	(!strncmp(ptr, "SD",   sizeof("SD")  -1))	item = ITEM_STORAGE_SD;
		else {
			info ("%s : msg unknown %s\n", __func__,  ptr);
			return -1;
		}
		storage[item].status = storage[item].speed > storage[item].check_speed ? 1 : 0;
		sprintf (resp_msg, "%d,%d MB/s", storage[item].status, storage[item].speed); 
		info ("msg = %s, resp = %s\n", msg, resp_msg);
		return	0;
	} 
	info ("msg null pointer err!\n");
	return -1;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void storage_test_exit (void)
{
	info ("%s\n", __func__);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
