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
#define CONFIG_APP_FILE     "app.cfg"

int find_appcfg_data (char *fkey, char *fdata)
{
	FILE *fp;
	char read_line[256], *ptr;
	bool appcfg = false;
	int count = 0;
	if (access (CONFIG_APP_FILE, R_OK) == 0) {
		if ((fp = fopen (CONFIG_APP_FILE, "r")) != NULL) {
			memset (read_line, 0x00, sizeof(read_line));
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

						strncpy (fdata, ptr, strlen (ptr) -1);
						//info ("%s : (%d) %s\n", __func__, (int)strlen(fdata), fdata);
						fclose (fp);
						return 0;
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
