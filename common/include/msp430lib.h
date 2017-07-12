/**
 * @file    msp430lib.h
 * @brief   
 * @version 00.10
 *
 * Put the file comments here.
 *
 * @verbatim
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2005
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 * @endverbatim
 */

#ifndef _MSP430LIB_H
#define _MSP430LIB_H

#define MSP430LIB_SUCCESS 1     //!< Indicates success of an API call.
#define MSP430LIB_FAILURE 0     //!< Indicates failure of an API call.

/**
 * @brief The IR remote keycodes returned by the MSP430. Requires the DVD
 * function on Philips PM4S set to code 020.
 */
enum msp430lib_keycode {
    MSP430LIB_KEYCODE_POWER           = 0x300c,
    MSP430LIB_KEYCODE_CHANINC         = 0x3020,
    MSP430LIB_KEYCODE_CHANDEC         = 0x3021,
    MSP430LIB_KEYCODE_VOLINC          = 0x3010,
    MSP430LIB_KEYCODE_VOLDEC          = 0x3011,
    MSP430LIB_KEYCODE_OK              = 0x300d,
    MSP430LIB_KEYCODE_MENU            = 0x302e,
    MSP430LIB_KEYCODE_MENUDONE        = 0x300f,
    MSP430LIB_KEYCODE_INFOSELECT      = 0x300f, // !!!!
    MSP430LIB_KEYCODE_SLEEP           = 0x3026,
    MSP430LIB_KEYCODE_SUBTITLE        = 0x30cb,
    MSP430LIB_KEYCODE_REPEAT          = 0x3022,
    MSP430LIB_KEYCODE_1               = 0x3001,
    MSP430LIB_KEYCODE_2               = 0x3002,
    MSP430LIB_KEYCODE_3               = 0x3003,
    MSP430LIB_KEYCODE_4               = 0x3004,
    MSP430LIB_KEYCODE_5               = 0x3005,
    MSP430LIB_KEYCODE_6               = 0x3006,
    MSP430LIB_KEYCODE_7               = 0x3007,
    MSP430LIB_KEYCODE_8               = 0x3008,
    MSP430LIB_KEYCODE_9               = 0x3009,
    MSP430LIB_KEYCODE_0               = 0x3000,
    MSP430LIB_KEYCODE_ENTER           = 0x2039,
    MSP430LIB_KEYCODE_INPUT           = 0x20ff,
    MSP430LIB_KEYCODE_REWIND          = 0x3172,
    MSP430LIB_KEYCODE_PLAY            = 0x3175,
    MSP430LIB_KEYCODE_FASTFORWARD     = 0x3174,
    MSP430LIB_KEYCODE_RECORD          = 0x3177,
    MSP430LIB_KEYCODE_STOP            = 0x3176,
    MSP430LIB_KEYCODE_PAUSE           = 0x3169,
};

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * @brief Initializes the MSP430 library. Must be called before any API calls.
 * @return MSP430LIB_SUCCESS on success and MSP430LIB_FAILURE on failure.
 */
extern int msp430lib_init(void);

/**
 * @brief Get the current Real Time Clock value from the MSP430.
 * @param year The current year returned.
 * @param month The current month returned.
 * @param day The current day returned.
 * @param hour The current hour returned.
 * @param minute The current minute returned.
 * @param second The current second returned.
 * @return MSP430LIB_SUCCESS on success and MSP430LIB_FAILURE on failure.
 */
extern int msp430lib_get_rtc(int *year, int *month, int *day, int *hour,
                             int *minute, int *second);

/**
 * @brief Set the current Real Time Clock value on the MSP430.
 * @param year The year to set.
 * @param month The month to set.
 * @param day The day to set.
 * @param hour The hour to set.
 * @param minute The minute to set.
 * @param second The second to set.
 * @return MSP430LIB_SUCCESS on success and MSP430LIB_FAILURE on failure.
 */
extern int msp430lib_set_rtc(int year, int month, int day, int hour,
                             int minute, int second);

/**
 * @brief Get a new IR key from the msp430 (if any).
 * @param key The key pressed returned or 0 if no new key pressed.
 * @return MSP430LIB_SUCCESS on success and MSP430LIB_FAILURE on failure.
 */
extern int msp430lib_get_ir_key(enum msp430lib_keycode *key);

/**
 * @brief Deinitalize the MSP430 library. No API calls can be made after this
 * function has been called.
 * @return MSP430LIB_SUCCESS on success and MSP430LIB_FAILURE on failure.
 */
extern int msp430lib_exit(void);

#if defined (__cplusplus)
}
#endif

#endif // _SIMPLEWIDGET_H
