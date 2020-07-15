#ifndef _INPUT_H
#define _INPUT_H

#define SW_START    0x0080
#define SW_SERVICE  0x0040
#define SW_UP       0x0020
#define SW_DOWN     0x0010
#define SW_LEFT     0x0008
#define SW_RIGHT    0x0004
#define SW_1        0x0002
#define SW_2        0x0001
#define SW_3        0x8000
#define SW_4        0x4000
#define SW_ANY      (SW_1 | SW_2 | SW_3 | SW_4 | SW_START)

#include <SDL.h>

int input_init(void);
void input_poll(void);
void input_shutdown(void);

int input_test_button(void);
int input_1p_start_impulse(void);
int input_2p_start_impulse(void);
int input_1p_coin_impulse(void);
int input_2p_coin_impulse(void);
int input_service_coin_impulse(void);

const uint8_t *input_get_keys(void);

uint16_t input_get_impulse(int player);
uint16_t input_get_state(int player);
uint16_t input_get_state_prev(int player);

#endif // _INPUT_H
