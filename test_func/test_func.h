//------------------------------------------------------------------------------
/**
 * @file test_func.h
 * @author charles-park (charles.park@hardkernel.com)
 * @brief ODROID-N2L JIG device test function.
 * @version 0.1
 * @date 2022-10-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//------------------------------------------------------------------------------
#ifndef	__TEST_FUNC_H__
#define	__TEST_FUNC_H__

//------------------------------------------------------------------------------
// adc_test.c
//------------------------------------------------------------------------------
extern  void    adc_test_init (void);
extern  int     adc_test_func (char *msg, char *resp_msg);
extern  void    adc_test_exit (void);

//------------------------------------------------------------------------------
// audio_test.c
//------------------------------------------------------------------------------
extern  void    audio_test_init (void);
extern  int     audio_test_func (char *msg, char *resp_msg);
extern  void    audio_test_exit (void);

//------------------------------------------------------------------------------
// fan_test.c
//------------------------------------------------------------------------------
extern  void    fan_test_init (void);
extern  int     fan_test_func (char *msg, char *resp_msg);
extern  void    fan_test_exit (void);

//------------------------------------------------------------------------------
// hdmi_test.c
//------------------------------------------------------------------------------
extern  void    hdmi_test_init (void);
extern  int     hdmi_test_func (char *msg, char *resp_msg);
extern  void    hdmi_test_exit (void);

//------------------------------------------------------------------------------
// header40_test.c
//------------------------------------------------------------------------------
extern  void    header40_test_init (void);
extern  int     header40_test_func (char *msg, char *resp_msg);
extern  void    header40_test_exit (void);

//------------------------------------------------------------------------------
// led_test.c
//------------------------------------------------------------------------------
extern  void    led_test_init (void);
extern  int     led_test_func (char *msg, char *resp_msg);
extern  void    led_test_exit (void);

//------------------------------------------------------------------------------
// storage_test.c
//------------------------------------------------------------------------------
extern  void    storage_test_init (void);
extern  int     storage_test_func (char *msg, char *resp_msg);
extern  void    storage_test_exit (void);

//------------------------------------------------------------------------------
// system_test.c
//------------------------------------------------------------------------------
extern  void    system_test_init (void);
extern  int     system_test_func (char *msg, char *resp_msg);
extern  void    system_test_exit (void);

//------------------------------------------------------------------------------
// usb_mass_test.c
//------------------------------------------------------------------------------
extern  void    usb_test_init (void);
extern  int     usb_test_func (char *msg, char *resp_msg);
extern  void    usb_test_exit (void);

//------------------------------------------------------------------------------
#endif	// __TEST_FUNC_H__
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
