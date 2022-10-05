//------------------------------------------------------------------------------
/**
 * @file audio_test.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID-N2L JIG audio test function.
 * @version 0.1
 * @date 2022-10-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#include "../common.h"
#include "../typedefs.h"

//------------------------------------------------------------------------------
#define HDMI_HPD_FILENAME	"/sys/class/amhdmitx/amhdmitx0/hpd_state"
#define	HDMI_EDID_FILENAME	"/sys/class/amhdmitx/amhdmitx0/rawedid"
#define	HDMI_EDID_PASS		"00ffffffffffff00"

#define	HDMI_HPD	0
#define	HDMI_EDID	1

struct hdmi_test_t {
	bool	status;
	char 	fname[128];
};

struct hdmi_test_t hdmi[2];

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void hdmi_test_init (void)
{
	FILE	*fp;
	char	filename[256];

	memset (hdmi, 0x00, sizeof(hdmi));

	info ("---------------------------------\n");
	info ("[ %s : %s ]\n", __FILE__, __func__);
	// APP config data read
	{
		if (find_appcfg_data ("HDMI_HPD_FILENAME",  hdmi[HDMI_HPD].fname))
			sprintf (hdmi[HDMI_HPD].fname, "%s", HDMI_HPD_FILENAME);
		if (find_appcfg_data ("HDMI_EDID_FILENAME",  hdmi[HDMI_EDID].fname))
			sprintf (hdmi[HDMI_EDID].fname, "%s", HDMI_EDID_FILENAME);
	}

	if (access (hdmi[HDMI_HPD].fname, R_OK) == 0) {
		hdmi[HDMI_HPD].status = fread_int (hdmi[HDMI_HPD].fname);
	}

	if (access (hdmi[HDMI_EDID].fname, R_OK) == 0) {
		char	rdata[16];
		if (!fread_line (hdmi[HDMI_EDID].fname, rdata)) {
			// HDMI_EDID_PASS string is edid pass.
			if (!strncmp(rdata, HDMI_EDID_PASS, sizeof(HDMI_EDID_PASS)))
				hdmi[HDMI_EDID].status = true;
		}
	}
	info ("HPD  filename = %s, access = %d\n",
		hdmi[HDMI_HPD].fname, access(hdmi[HDMI_HPD].fname, R_OK));
	info ("EDID filename = %s, access = %d\n",
		hdmi[HDMI_EDID].fname, access(hdmi[HDMI_EDID].fname, R_OK));
	info ("---------------------------------\n");
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int hdmi_test_func (char *msg, char *resp_msg)
{
	char *ptr, msg_clone[20], item;

	memset (msg_clone, 0x00, sizeof(msg_clone));
	sprintf (msg_clone, "%s", msg);

	if ((ptr = strtok (msg_clone, ",")) != NULL) {
		ptr = toupperstr(ptr);

		if		(!strncmp(ptr, "HPD", sizeof("HPD")))
			item = HDMI_HPD;
		else if (!strncmp(ptr, "EDID", sizeof("EDID")))
			item = HDMI_EDID;
		else {
			err ("Unknown msg! %s\n", ptr);
			return -1;
		}
		/* resp msg : [status,] */
		sprintf (resp_msg, "%d,%s", hdmi[item].status, hdmi[item].status ? "PASS" : "FAIL");
		info ("msg = %s, resp = %s\n", msg, resp_msg);
		return	0;
	}
	err ("msg null pointer err!\n");
	return -1;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void hdmi_test_exit (void)
{
	info ("%s\n", __func__);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
