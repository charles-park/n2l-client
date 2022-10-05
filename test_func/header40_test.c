//------------------------------------------------------------------------------
/**
 * @file header40_test.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID-N2L JIG header40 test function.
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
#include "../lib_gpio/lib_gpio.h"

//------------------------------------------------------------------------------
// ODROID-N2/N2L Header to GPIO Number
// ref : https://docs.google.com/spreadsheets/d/18cRWfgj9xmlr1JQb91fNN7SQxrBZxkHoxOEJN6Yy4SI/edit#gid=1883790439
//------------------------------------------------------------------------------
static __u16	HeaderToGPIO[] = {
	  0,		// Not used
	  0,   0,	// |  1 : 3.3V      ||  2 : 5.0V      |
	493,   0,	// |  3 : I2C-2 SDA ||  4 : 5.0V      |
	494,   0,	// |  5 : I2C-2 SCL ||  6 : GND       |
	473, 488,	// |  7 : GPIOA.13  ||  8 : UART_TX_A |
	  0, 489,	// |  9 : GND       || 10 : UART_RX_A |
	479, 492,	// | 11 : GPIOX.3   || 12 : PWM_E     |
	480,   0,	// | 13 : GPIOX.4   || 14 : GND       |
	483, 476,	// | 15 : GPIOX.7   || 16 : GPIOX.0   |
	  0, 477,	// | 17 : 3.3V      || 18 : GPIOX.1   |
	484,   0,	// | 19 : SPI0_MOSI || 20 : GND       |
	485, 478,	// | 21 : SPI0_MISO || 22 : GPIOX.2   |
	487, 486,	// | 23 : SPI0_CLK  || 24 : SPI0_CS0  |
	  0, 464,	// | 25 : GND       || 26 : GPIOA.4   |
	474, 475,	// | 27 : I2C-3 SDA || 28 : I2C-3 SCL |
	490,   0,	// | 29 : GPIOX.14  || 30 : GND       |
	491, 472,	// | 31 : GPIOX.15  || 32 : GPIOA.12  |
	481,   0,	// | 33 : GPIOX.5   || 34 : GND       |
	482, 495,	// | 35 : GPIOX.6   || 36 : GPIOX.19  |
	  0,   0,	// | 37 : ADC.AIN3  || 38 : REF 1.8V  |
	  0,   0	// | 39 : GND       || 40 : ADC.AIN2  |
};

#define	PATTERNS_COUNT	4
#define	HEADER40_PINS	41
const __u16	Patterns[PATTERNS_COUNT][HEADER40_PINS] = {
	{
		// ALL High Pattern
		0,		// Not used
		0,   0,	// |  1 : 3.3V      ||  2 : 5.0V      |
		1,   0,	// |  3 : I2C-2 SDA ||  4 : 5.0V      |
		1,   0,	// |  5 : I2C-2 SCL ||  6 : GND       |
		1,   1,	// |  7 : GPIOA.13  ||  8 : UART_TX_A |
		0,   1,	// |  9 : GND       || 10 : UART_RX_A |
		1,   1,	// | 11 : GPIOX.3   || 12 : PWM_E     |
		1,   0,	// | 13 : GPIOX.4   || 14 : GND       |
		1,   1,	// | 15 : GPIOX.7   || 16 : GPIOX.0   |
		0,   1,	// | 17 : 3.3V      || 18 : GPIOX.1   |
		1,   0,	// | 19 : SPI0_MOSI || 20 : GND       |
		1,   1,	// | 21 : SPI0_MISO || 22 : GPIOX.2   |
		1,   1,	// | 23 : SPI0_CLK  || 24 : SPI0_CS0  |
		0,   1,	// | 25 : GND       || 26 : GPIOA.4   |
		1,   1,	// | 27 : I2C-3 SDA || 28 : I2C-3 SCL |
		1,   0,	// | 29 : GPIOX.14  || 30 : GND       |
		1,   1,	// | 31 : GPIOX.15  || 32 : GPIOA.12  |
		1,   0,	// | 33 : GPIOX.5   || 34 : GND       |
		1,   1,	// | 35 : GPIOX.6   || 36 : GPIOX.19  |
		0,   0,	// | 37 : ADC.AIN3  || 38 : REF 1.8V  |
		0,   0	// | 39 : GND       || 40 : ADC.AIN2  |
	},	{
		// ALL Clear Pattern
		0,		// Not used
		0,   0,	// |  1 : 3.3V      ||  2 : 5.0V      |
		0,   0,	// |  3 : I2C-2 SDA ||  4 : 5.0V      |
		0,   0,	// |  5 : I2C-2 SCL ||  6 : GND       |
		0,   0,	// |  7 : GPIOA.13  ||  8 : UART_TX_A |
		0,   0,	// |  9 : GND       || 10 : UART_RX_A |
		0,   0,	// | 11 : GPIOX.3   || 12 : PWM_E     |
		0,   0,	// | 13 : GPIOX.4   || 14 : GND       |
		0,   0,	// | 15 : GPIOX.7   || 16 : GPIOX.0   |
		0,   0,	// | 17 : 3.3V      || 18 : GPIOX.1   |
		0,   0,	// | 19 : SPI0_MOSI || 20 : GND       |
		0,   0,	// | 21 : SPI0_MISO || 22 : GPIOX.2   |
		0,   0,	// | 23 : SPI0_CLK  || 24 : SPI0_CS0  |
		0,   0,	// | 25 : GND       || 26 : GPIOA.4   |
		0,   0,	// | 27 : I2C-3 SDA || 28 : I2C-3 SCL |
		0,   0,	// | 29 : GPIOX.14  || 30 : GND       |
		0,   0,	// | 31 : GPIOX.15  || 32 : GPIOA.12  |
		0,   0,	// | 33 : GPIOX.5   || 34 : GND       |
		0,   0,	// | 35 : GPIOX.6   || 36 : GPIOX.19  |
		0,   0,	// | 37 : ADC.AIN3  || 38 : REF 1.8V  |
		0,   0	// | 39 : GND       || 40 : ADC.AIN2  |
	},	{
		// Cross Logic Pattern 1
		0,		// Not used
		0,   0,	// |  1 : 3.3V      ||  2 : 5.0V      |
		1,   0,	// |  3 : I2C-2 SDA ||  4 : 5.0V      |
		0,   0,	// |  5 : I2C-2 SCL ||  6 : GND       |
		1,   0,	// |  7 : GPIOA.13  ||  8 : UART_TX_A |
		0,   1,	// |  9 : GND       || 10 : UART_RX_A |
		1,   0,	// | 11 : GPIOX.3   || 12 : PWM_E     |
		0,   0,	// | 13 : GPIOX.4   || 14 : GND       |
		1,   0,	// | 15 : GPIOX.7   || 16 : GPIOX.0   |
		0,   1,	// | 17 : 3.3V      || 18 : GPIOX.1   |
		1,   0,	// | 19 : SPI0_MOSI || 20 : GND       |
		0,   1,	// | 21 : SPI0_MISO || 22 : GPIOX.2   |
		1,   0,	// | 23 : SPI0_CLK  || 24 : SPI0_CS0  |
		0,   1,	// | 25 : GND       || 26 : GPIOA.4   |
		1,   0,	// | 27 : I2C-3 SDA || 28 : I2C-3 SCL |
		0,   0,	// | 29 : GPIOX.14  || 30 : GND       |
		1,   0,	// | 31 : GPIOX.15  || 32 : GPIOA.12  |
		0,   0,	// | 33 : GPIOX.5   || 34 : GND       |
		1,   0,	// | 35 : GPIOX.6   || 36 : GPIOX.19  |
		0,   0,	// | 37 : ADC.AIN3  || 38 : REF 1.8V  |
		0,   0	// | 39 : GND       || 40 : ADC.AIN2  |
	},	{
		// Cross Logic Pattern 2
		0,		// Not used
		0,   0,	// |  1 : 3.3V      ||  2 : 5.0V      |
		0,   0,	// |  3 : I2C-2 SDA ||  4 : 5.0V      |
		1,   0,	// |  5 : I2C-2 SCL ||  6 : GND       |
		0,   1,	// |  7 : GPIOA.13  ||  8 : UART_TX_A |
		0,   0,	// |  9 : GND       || 10 : UART_RX_A |
		0,   1,	// | 11 : GPIOX.3   || 12 : PWM_E     |
		0,   0,	// | 13 : GPIOX.4   || 14 : GND       |
		0,   1,	// | 15 : GPIOX.7   || 16 : GPIOX.0   |
		0,   0,	// | 17 : 3.3V      || 18 : GPIOX.1   |
		0,   0,	// | 19 : SPI0_MOSI || 20 : GND       |
		1,   0,	// | 21 : SPI0_MISO || 22 : GPIOX.2   |
		0,   1,	// | 23 : SPI0_CLK  || 24 : SPI0_CS0  |
		0,   0,	// | 25 : GND       || 26 : GPIOA.4   |
		0,   1,	// | 27 : I2C-3 SDA || 28 : I2C-3 SCL |
		1,   0,	// | 29 : GPIOX.14  || 30 : GND       |
		0,   1,	// | 31 : GPIOX.15  || 32 : GPIOA.12  |
		1,   0,	// | 33 : GPIOX.5   || 34 : GND       |
		0,   1,	// | 35 : GPIOX.6   || 36 : GPIOX.19  |
		0,   0,	// | 37 : ADC.AIN3  || 38 : REF 1.8V  |
		0,   0	// | 39 : GND       || 40 : ADC.AIN2  |
	},
};

//------------------------------------------------------------------------------
struct header40_test_t {
	bool	is_file;
	bool	status;
};

static struct header40_test_t header40[HEADER40_PINS];

//------------------------------------------------------------------------------
enum eITEM_PATTERNS {
	ITEM_PATTERN_0 = 0,
	ITEM_PATTERN_1,
	ITEM_PATTERN_2,
	ITEM_PATTERN_3,
};

//------------------------------------------------------------------------------
static int header40_set_pattern (__u16 pattern)
{
	int i;
	for (i = 0; i < HEADER40_PINS; i++) {
		if (!HeaderToGPIO[i])
			continue;

		if (header40[i].is_file)
			header40[i].status =
				!gpio_set_value (HeaderToGPIO[i], Patterns[pattern][i]);

		if (!header40[i].status) {
			info ("Error Set Value GPIO%d, %d\n",
					HeaderToGPIO[i], Patterns[pattern][i]);
			return	0;
		}
	}
	return	1;
}

//------------------------------------------------------------------------------
void header40_test_init(void)
{
	int i;

	info ("---------------------------------\n");
	info ("[ %s : %s ]\n", __FILE__, __func__);
	memset (header40, 0x00, sizeof(header40));
	// APP config data read
	{
		char pins[256], *ptr, h_pin = 0;

		for (i = 0; i < 4; i++) {
			memset (pins, 0x00, sizeof(pins));
			switch (i) {
				case 0:
					if (find_appcfg_data ("HEADER40_01_10", pins)) {
						continue;
					}
				break;
				case 1:
					if (find_appcfg_data ("HEADER40_11_20", pins))
						continue;
				break;
				case 2:
					if (find_appcfg_data ("HEADER40_21_30", pins))
						continue;
				break;
				case 3:
					if (find_appcfg_data ("HEADER40_31_40", pins))
						continue;
				break;
				default :
				break;
			}
			info ("read pins (%02d - %02d) : %s\n", i*10+1, i*10+10, pins);

			h_pin = 1 + i * 10;
			ptr   = strtok (pins, ",");
			while (ptr != NULL) {
				HeaderToGPIO[h_pin++] = atoi(ptr);
				ptr = strtok (NULL, ",");
			}
		}
	}
	for (i = 0; i < HEADER40_PINS; i++) {
		if (HeaderToGPIO[i]) {
			header40[i].is_file = !gpio_export (HeaderToGPIO[i]);
			if (header40[i].is_file)
				gpio_direction (HeaderToGPIO[i], GPIO_DIR_OUT);
			info ("pin : %02d, gpio : %d\n", i, HeaderToGPIO[i]);
		}
	}
	info ("---------------------------------\n");
}

//------------------------------------------------------------------------------
int header40_test_func (char *msg, char *resp_msg)
{
	char	*ptr, msg_clone[20], item, item_is_port;

	memset (msg_clone, 0x00, sizeof(msg_clone));
	sprintf (msg_clone, "%s", msg);

	if ((ptr = strtok (msg_clone, ",")) != NULL) {
		ptr = toupperstr(ptr);

		if 		(!strncmp(ptr, "PATTERN_0", sizeof("PATTERN_0")))
			item = ITEM_PATTERN_0;
		else if	(!strncmp(ptr, "PATTERN_1", sizeof("PATTERN_1")))
			item = ITEM_PATTERN_1;
		else if	(!strncmp(ptr, "PATTERN_2", sizeof("PATTERN_2")))
			item = ITEM_PATTERN_2;
		else if	(!strncmp(ptr, "PATTERN_3", sizeof("PATTERN_3")))
			item = ITEM_PATTERN_3;
		else {
			info ("err : unknown msg! %s\n", ptr);
			return -1;
		}

		/* resp msg : [status, get_speed-read mb/s */
		sprintf (resp_msg, "%d,%d",	header40_set_pattern(item), item);
		info ("msg = %s, resp = %s\n", msg, resp_msg);
		return	0;
	}
	info ("msg null pointer err!\n");
	return -1;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void header40_test_exit (void)
{
	int i;
	info ("%s\n", __func__);

	memset (header40, 0x00, sizeof(header40));
	for (i = 0; i < HEADER40_PINS; i++) {
		if (HeaderToGPIO[i]) {
			header40[i].is_file = !gpio_unexport (HeaderToGPIO[i]);
		}
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
