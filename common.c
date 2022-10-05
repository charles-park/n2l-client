//------------------------------------------------------------------------------
/**
 * @file common.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID-N2L JIG common used function.
 * @version 0.1
 * @date 2022-10-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//------------------------------------------------------------------------------
#include "common.h"
#include "typedefs.h"

//------------------------------------------------------------------------------
// function prototype define
//------------------------------------------------------------------------------
char 	*remove_space_str 	(char *str);
char	*toupperstr			(char *str);
char	*tolowerstr			(char *str);
int		fread_int       	(char *filename);
int		fread_line       	(char *filename, char *line);
int		fwrite_bool			(char *filename, char status);
int 	fwrite_str 			(char *filename, char *wstr);
int		find_appcfg_data	(char *fkey, char *fdata);

//------------------------------------------------------------------------------
char *remove_space_str (char *str)
{
	int len = strlen(str);

	if (*str == 0x20)
		while ((*str++ != 0x20) && len--)

	return	str;
}

//------------------------------------------------------------------------------
char *tolowerstr (char *str)
{
	int i, len = strlen(str);

	if (*str == 0x20)
		while ((*str++ != 0x20) && len--);

	for (i = 0; i < len; i++, str++)
		*str = tolower(*str);

	/* calculate input string(*str) start pointer */
	return	(str -len);
}

//------------------------------------------------------------------------------
char *toupperstr (char *str)
{
	int i, len = strlen(str);

	if (*str == 0x20)
		while ((*str++ != 0x20) && len--);

	for (i = 0; i < len; i++, str++)
		*str = toupper(*str);

	/* calculate input string(*str) start pointer */
	return	(str -len);
}

//------------------------------------------------------------------------------
int fread_int (char *filename)
{
	__s8	rdata[16];
	FILE 	*fp;

	if (access (filename, R_OK) != 0)
		return -1;

	// adc raw value get
	if ((fp = fopen(filename, "r")) != NULL) {
		fgets (rdata, sizeof(rdata), fp);
		fclose(fp);
	}
	
	return	(atoi(rdata));
}

//------------------------------------------------------------------------------
int fread_line (char *filename, char *line)
{
	FILE 	*fp;

	if (access (filename, R_OK) != 0)
		return -1;

	// adc raw value get
	if ((fp = fopen(filename, "r")) != NULL) {
		fgets (line, sizeof(line), fp);
		fclose(fp);
	}
	info ("filename = %s, rdata = %s\n", filename, line);
	
	return	((strlen (line) > 0) ? 0 : -1);
}

//------------------------------------------------------------------------------
int fwrite_bool (char *filename, char status)
{
	__s8	rdata;
	FILE 	*fp;

	// fan control set
	if ((fp = fopen(filename, "w")) != NULL) {
		fputc (status ? '1' : '0', fp);
		fclose(fp);
	}
	
	// fan control get
	if ((fp = fopen(filename, "r")) != NULL) {
		rdata = fgetc (fp);
		fclose(fp);
	}

	info ("rdata = %c, fan.enable = %c\n", rdata, status + '0');
	
	return	(rdata != (status + '0')) ? false : true;
}

//------------------------------------------------------------------------------
int fwrite_str (char *filename, char *wstr)
{
	__s8	rdata[128];
	FILE 	*fp;

	// fan control set
	if ((fp = fopen(filename, "w")) != NULL) {
		fputs (wstr, fp);
		fclose(fp);
	}
	
	// fan control get
	if ((fp = fopen(filename, "r")) != NULL) {
		fgets (rdata, sizeof(rdata), fp);
		fclose(fp);
	}

	info ("rdata = %s, wstr = %s\n", rdata, wstr);
	return	(strncmp (wstr, rdata, strlen(wstr)) != 0) ? true : false;
}

//------------------------------------------------------------------------------
int find_appcfg_data (char *fkey, char *fdata)
{
	FILE *fp;
	char read_line[256];
	bool appcfg = false;
	int count = 0;

	if (access (CONFIG_APP_FILE, R_OK) == 0) {
		if ((fp = fopen (CONFIG_APP_FILE, "r")) != NULL) {
			memset (read_line, 0x00, sizeof(read_line));
			while ((fgets(read_line, sizeof(read_line), fp) != NULL)) {
				if (!strncmp (read_line, "ODROID-APP-CONFIG", sizeof("ODROID-APP-CONFIG")-1)) {
					appcfg = true;
					break;
				}
				memset (read_line, 0x00, sizeof(read_line));
			}
			fclose (fp);
		}
		if (!appcfg)	return -1;

		if ((fp = fopen (CONFIG_APP_FILE, "r")) != NULL) {
			memset (read_line, 0x00, sizeof(read_line));
			while (fgets(read_line, sizeof(read_line), fp) != NULL) {
				char *ptr = strstr (read_line, fkey);
				if (ptr != NULL) {
					ptr = strstr (ptr +1, "=");
					ptr = ptr +1;
					while ((ptr != NULL) && (*ptr == ' '))	ptr++;

					strncpy (fdata, ptr, strlen (ptr) -1);
					//info ("%s : (%d) %s\n", __func__, (int)strlen(fdata), fdata);
					fclose (fp);
					return 0;
				}
				memset (read_line, 0x00, sizeof(read_line));
			}
			fclose (fp);
		}
	}
	return -1;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
