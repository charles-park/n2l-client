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
int 	fread_line          (char *filename, char *line, int l_size);
int		fwrite_bool			(char *filename, char status);
int 	fwrite_str 			(char *filename, char *wstr);
int		find_appcfg_data	(char *fkey, char *fdata);

//------------------------------------------------------------------------------
char *remove_space_str (char *str)
{
	int len = strlen(str);

	while ((*str++ == 0x20) && len--)

	return	str -1;
}

//------------------------------------------------------------------------------
char *tolowerstr (char *str)
{
	int i, len = strlen(str);

	while ((*str++ == 0x20) && len--);

	for (i = 0; i < len; i++, str++)
		*str = tolower(*str);

	/* calculate input string(*str) start pointer */
	return	(str -len -1);
}

//------------------------------------------------------------------------------
char *toupperstr (char *str)
{
	int i, len = strlen(str);

	while ((*str++ == 0x20) && len--);

	for (i = 0; i < len; i++, str++)
		*str = toupper(*str);

	/* calculate input string(*str) start pointer */
	return	(str -len -1);
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
int fread_line (char *filename, char *line, int l_size)
{
	FILE 	*fp;

	if (access (filename, R_OK) != 0)
		return -1;

	// adc raw value get
	if ((fp = fopen(filename, "r")) != NULL) {
		fgets (line, l_size, fp);
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

	info ("rdata = %c, status = %c\n", rdata, status + '0');
	
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
#define CONFIG_APP_FILE_1     "/media/boot/app.cfg"
#define CONFIG_APP_FILE_2     "app.cfg"

int find_appcfg_data (char *fkey, char *fdata)
{
	FILE *fp;
	char read_line[128], *ptr, fname[64];
	bool appcfg = false, multiline = false;
	int cmd_cnt = 0, pos = 0;;

	memset (fname, 0x00, sizeof(fname));
	if (access(CONFIG_APP_FILE_1, R_OK) == 0)
		sprintf(fname, "%s", CONFIG_APP_FILE_1);
	else
		sprintf(fname, "%s", CONFIG_APP_FILE_2);

	if (access (fname, R_OK) == 0) {
		if ((fp = fopen (fname, "r")) != NULL) {
			memset (read_line, 0x00, sizeof(read_line));
			if (!strncmp ("R_DELAY", fkey, sizeof("R_DELAY")-1))
				multiline = true;

			while (fgets(read_line, sizeof(read_line), fp) != NULL) {

				if (!appcfg) {
					appcfg = strncmp ("ODROID-APP-CONFIG", read_line,
									strlen(read_line)-1) == 0 ? true : false;
					memset (read_line, 0x00, sizeof(read_line));
					continue;
				}
				if (read_line[0] != '#') {
					if ((ptr = strstr (read_line, fkey)) != NULL) {
						ptr = strstr (ptr +1, "=");
						ptr = ptr +1;
						while ((ptr != NULL) && (*ptr == ' '))	ptr++;

						pos = cmd_cnt * sizeof(read_line);
						strncpy(&fdata[pos], ptr, strlen (ptr) -1);

						if (multiline)	cmd_cnt++;
						else {
							fclose (fp);
							return 0;
						}
						//info ("%s : (%d) %s\n", __func__, (int)strlen(fdata), fdata);
					}
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
