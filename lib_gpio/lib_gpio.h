//------------------------------------------------------------------------------
/**
 * @file lib_gpio.h
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID-N2L JIG gpio library.
 * @version 0.1
 * @date 2022-10-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//------------------------------------------------------------------------------
#ifndef	__LIB_GPIO_H__
#define	__LIB_GPIO_H__

//------------------------------------------------------------------------------
#define	GPIO_CONTROL_PATH	"/sys/class/gpio"
#define	GPIO_DIR_OUT		1
#define	GPIO_DIR_IN			0

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
extern	int gpio_export 	(int gpio);
extern	int gpio_direction 	(int gpio, int status);
extern	int gpio_set_value 	(int gpio, int s_value);
extern	int gpio_get_value 	(int gpio, int *g_value);
extern	int gpio_unexport 	(int gpio);

//------------------------------------------------------------------------------
#endif	// __LIB_GPIO_H__
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
