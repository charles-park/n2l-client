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
#define AUDIO_LEFT_FILENAME			"/root/audio_test/1Khz_Left.wav"
#define AUDIO_RIGHT_FILENAME		"/root/audio_test/1Khz_Right.wav"
#define	AUDIO_PLAY_TIME				2

//------------------------------------------------------------------------------
struct audio_test_t {
	bool			enable;
	bool			is_busy;
	__s8			filename[128];
	__s8			lch_fname[128];
	__s8			rch_fname[128];
	__s16			play_time;
	pthread_t		pthread;
	pthread_mutex_t	mutex;
};

static struct audio_test_t audio;
//------------------------------------------------------------------------------
void *audio_test_thread (void *arg)
{
	FILE *fp;
	struct audio_test_t *paudio = (struct audio_test_t *)arg;
	__s8 cmd[256];

	while (true) {
		if (!paudio->is_busy && paudio->enable) {
			pthread_mutex_lock(&paudio->mutex);
			paudio->is_busy = true;
			pthread_mutex_unlock(&paudio->mutex);

			if (!strncmp("sleep", paudio->filename, strlen("sleep")-1))
				sleep(paudio->play_time);
			else {
				memset (cmd, 0x00, sizeof(cmd));
				sprintf (cmd, "aplay -Dhw:0,1 %s -d %d && sync", paudio->filename, paudio->play_time);
				if (NULL == (fp = popen(cmd, "r")))	{
					err("popen() error!\n");
					exit(1);
				}
				pclose(fp);
			}

			pthread_mutex_lock(&paudio->mutex);
			paudio->enable = false;		paudio->is_busy = false;
			pthread_mutex_unlock(&paudio->mutex);
		}
		usleep(50);
	}
}

//------------------------------------------------------------------------------
// msg desc : channel(left, right, stop), play time (sec)
// test cmd = "aplay -Dhw:0,1 {audio_filename} -d {play_time}"
// return : status, 0 -> success, 1 -> device busy or error
//------------------------------------------------------------------------------
int audio_test_func (char *msg, char *resp_msg)
{
	if (!audio.is_busy) {
		char *ptr, msg_clone[20];

		info ("msg = %s\n", msg);
		memset (msg_clone, 0x00, sizeof(msg_clone));
		sprintf (msg_clone, "%s", msg);
		
		if ((ptr = strtok (msg_clone, ",")) != NULL) {
			ptr = toupperstr(ptr);

			memset (audio.filename, 0x00, sizeof (audio.filename));
			if			(!strncmp(ptr, "L_CH_ON", sizeof("L_CH_ON")-1)) {
				if (access (audio.lch_fname, R_OK)) {
					err ("%s not found.\n", audio.lch_fname);
					return -1;
				}
				sprintf (audio.filename, "%s", audio.lch_fname);
			} else if 	(!strncmp(ptr, "R_CH_ON", sizeof("R_CH_ON")-1)) {
				if (access (audio.rch_fname, R_OK)) {
					err ("%s not found.\n", audio.rch_fname);
					return -1;
				}
				sprintf (audio.filename, "%s", audio.rch_fname);
			} else if 	(!strncmp(ptr, "L_CH_OFF", sizeof("L_CH_OFF")-1)) {
				sprintf (audio.filename, "%s", "sleep");
			} else if 	(!strncmp(ptr, "R_CH_OFF", sizeof("R_CH_OFF")-1)) {
				sprintf (audio.filename, "%s", "sleep");
			}	else {
				info ("%s : msg unknown %s\n", __func__,  ptr);
				return -1;
			}

			pthread_mutex_lock(&audio.mutex);
			if (!audio.enable)
				audio.enable = true;
			pthread_mutex_unlock(&audio.mutex);
			usleep(100);
			sprintf(resp_msg, "%d,%d", audio.enable, audio.play_time);
			info ("msg = %s, resp = %s\n", msg, resp_msg);
			return 0;
		}
		err ("null pointer msg\n");
		return -1;
	}
	// device busy now
	return	1;
}

//------------------------------------------------------------------------------
int audio_test_init (void)
{
	info ("---------------------------------\n");
	info ("[ %s : %s ]\n", __FILE__, __func__);

	memset (&audio, 0x00, sizeof(audio));

	// APP config data read
	{
		char int_str[8];
		memset (int_str, 0x00, sizeof(int_str));
		if (find_appcfg_data ("AUDIO_LEFT_FILENAME",  audio.lch_fname))
			sprintf (audio.lch_fname, "%s", AUDIO_LEFT_FILENAME);

		if (find_appcfg_data ("AUDIO_RIGHT_FILENAME", audio.rch_fname))
			sprintf (audio.rch_fname, "%s", AUDIO_RIGHT_FILENAME);

		if (find_appcfg_data ("AUDIO_PLAY_TIME", int_str))
			audio.play_time = AUDIO_PLAY_TIME;
		else
			audio.play_time = atoi(int_str);

		if (audio.play_time <= 0)
			audio.play_time = AUDIO_PLAY_TIME;
	}

	pthread_create (&audio.pthread, NULL, audio_test_thread, &audio);

	info ("audio left  channel filename = %s, access = %d\n",
			audio.lch_fname, access(audio.lch_fname, R_OK));
	info ("audio right channel filename = %s, access = %d\n",
			audio.rch_fname, access(audio.rch_fname, R_OK));
	info ("audio play time = %d\n",	audio.play_time);
	info ("---------------------------------\n");
}

//------------------------------------------------------------------------------
void audio_test_exit (void)
{
	info ("rx thread detach = %d\n", pthread_detach(audio.pthread));
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
