#include <stdint.h>
#include <stdbool.h>
#include QMK_KEYBOARD_H

// We rely on QMK’s built-in flags from rgb_matrix_types.h:
#include "rgb_matrix_types.h"

/******************************************************************************
 * 1) Numeric -> QMK's built-in LED flags
 *
 * Per rgb_matrix_types.h, we have:
 *   LED_FLAG_KEYLIGHT   = 0x01
 *   LED_FLAG_UNDERGLOW  = 0x02
 *   LED_FLAG_MODIFIER   = 0x10
 *   LED_FLAG_NONE       = 0x00
 *
 * Your JSON said:
 *   1 => "key"
 *   2 => "mod"
 *   4 => "underglow"
 * We'll convert them at runtime to avoid static init collisions.
 *****************************************************************************/

static uint8_t convert_lulu_json_flag(uint8_t json_flag) {
    // Map your JSON flags to QMK’s built-ins:
    switch (json_flag) {
        case 1: return LED_FLAG_KEYLIGHT;  // 0x01
        case 2: return LED_FLAG_MODIFIER;  // 0x10
        case 4: return LED_FLAG_UNDERGLOW; // 0x02
        default: return LED_FLAG_NONE;     // 0x00
    }
}

/******************************************************************************
 * 2) 70 LED entries: coordinate + numericFlag
 *    We'll do a single array, then populate g_led_config at startup.
 *
 * If your `led_config_t` is:
 *     typedef struct {
 *         int16_t x;
 *         int16_t y;
 *     } led_point_t;
 *
 *     typedef struct {
 *         led_point_t point[DRIVER_LED_TOTAL];
 *         uint8_t flags[DRIVER_LED_TOTAL];
 *     } led_config_t;
 *****************************************************************************/

typedef struct {
    int16_t x;
    int16_t y;
    uint8_t rawFlag; // 1,2,4 from the JSON
} lulu_led_entry_t;

// Your 70 items from the JSON (positional, no named fields .x= or .y=).
// We'll store the 'rawFlag' as the third element.
static const lulu_led_entry_t luluLayout[70] = {
    // index:  x,   y,   rawFlag
    [0]  = {  86,  55,  2 },
    [1]  = {  51,  55,  2 },
    [2]  = {  17,  40,  2 },
    [3]  = {  17,  10,  2 },
    [4]  = {  51,  10,  2 },
    [5]  = {  86,  10,  2 },
    [6]  = {  86,   0,  4 },
    [7]  = {  68,   0,  4 },
    [8]  = {  51,   0,  4 },
    [9]  = {  34,   0,  4 },
    [10] = {  17,   0,  4 },
    [11] = {   0,   0,  1 },
    [12] = {   0,  16,  1 },
    [13] = {  17,  16,  4 },
    [14] = {  34,  16,  4 },
    [15] = {  51,  16,  4 },
    [16] = {  68,  16,  4 },
    [17] = {  86,  16,  4 },
    [18] = {  86,  32,  4 },
    [19] = {  68,  32,  4 },
    [20] = {  51,  32,  4 },
    [21] = {  34,  32,  4 },
    [22] = {  17,  32,  4 },
    [23] = {   0,  32,  1 },
    [24] = {   0,  48,  1 },
    [25] = {  17,  48,  1 },
    [26] = {  34,  48,  4 },
    [27] = {  51,  48,  4 },
    [28] = {  68,  48,  4 },
    [29] = {  86,  48,  4 },
    [30] = { 103,  40,  4 },
    [31] = {  96,  64,  1 },
    [32] = {  77,  64,  1 },
    [33] = {  60,  64,  1 },
    [34] = {  43,  64,  1 },
    [35] = { 137,  55,  2 },
    [36] = { 172,  55,  2 },
    [37] = { 206,  40,  2 },
    [38] = { 206,  10,  2 },
    [39] = { 172,  10,  2 },
    [40] = { 137,  10,  2 },
    [41] = { 137,   0,  4 },
    [42] = { 155,   0,  4 },
    [43] = { 172,   0,  4 },
    [44] = { 189,   0,  4 },
    [45] = { 206,   0,  4 },
    [46] = { 224,   0,  1 },
    [47] = { 224,  16,  1 },
    [48] = { 206,  16,  4 },
    [49] = { 189,  16,  4 },
    [50] = { 172,  16,  4 },
    [51] = { 155,  16,  4 },
    [52] = { 137,  16,  4 },
    [53] = { 137,  32,  4 },
    [54] = { 155,  32,  4 },
    [55] = { 172,  32,  4 },
    [56] = { 189,  32,  4 },
    [57] = { 206,  32,  4 },
    [58] = { 224,  32,  1 },
    [59] = { 224,  48,  1 },
    [60] = { 206,  48,  4 },
    [61] = { 189,  48,  4 },
    [62] = { 172,  48,  4 },
    [63] = { 155,  48,  4 },
    [64] = { 137,  48,  4 },
    [65] = { 120,  40,  4 },
    [66] = { 127,  64,  1 },
    [67] = { 146,  64,  1 },
    [68] = { 163,  64,  1 },
    [69] = { 180,  64,  1 },
};

/******************************************************************************
 * 3) The actual led_config_t
 *
 * QMK expects:
 *   typedef struct {
 *       led_point_t point[DRIVER_LED_TOTAL];
 *       uint8_t flags[DRIVER_LED_TOTAL];
 *   } led_config_t;
 *****************************************************************************/
led_config_t g_led_config = {
    // We'll fill these arrays at runtime to avoid static calls
    .point = { {0,0} }, // placeholders
    .flags = { 0 }
};

/******************************************************************************
 * 4) Fill g_led_config at startup
 *****************************************************************************/
void keyboard_post_init_user(void) {
    // We'll copy the 70 items from luluLayout into g_led_config.
    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
        g_led_config.point[i].x = luluLayout[i].x;
        g_led_config.point[i].y = luluLayout[i].y;
        g_led_config.flags[i]   = convert_lulu_json_flag(luluLayout[i].rawFlag);
    }
}

/******************************************************************************
 * 5) Layer-based coloring: use QMK's built-in flags in 'flags[]'
 *
 *   LED_FLAG_KEYLIGHT   = 0x01
 *   LED_FLAG_UNDERGLOW  = 0x02
 *   LED_FLAG_MODIFIER   = 0x10
 *****************************************************************************/
bool rgb_matrix_indicators_user(void) {
    // Turn all off
    for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
        rgb_matrix_set_color(i, 0, 0, 0);
    }

    uint8_t layer = get_highest_layer(layer_state);
    switch (layer) {
        case 0:
            // Example: Keylights => #E3E8EA, Underglow => white
            for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
                uint8_t f = g_led_config.flags[i];
                if (f & LED_FLAG_KEYLIGHT) {
                    // #E3E8EA => (227,232,234)
                    rgb_matrix_set_color(i, 227, 232, 234);
                } else if (f & LED_FLAG_UNDERGLOW) {
                    rgb_matrix_set_color(i, 255, 255, 255);
                }
            }
            break;

        case 1:
            // Example: LED_FLAG_MODIFIER => white
            for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
                if (g_led_config.flags[i] & LED_FLAG_MODIFIER) {
                    rgb_matrix_set_color(i, 255, 255, 255);
                }
            }
            break;

        case 2:
            // Example: LED_FLAG_KEYLIGHT => red
            for (uint8_t i = 0; i < DRIVER_LED_TOTAL; i++) {
                if (g_led_config.flags[i] & LED_FLAG_KEYLIGHT) {
                    rgb_matrix_set_color(i, 255, 0, 0);
                }
            }
            break;

        default:
            break;
    }

    // Return false so QMK can run other default animations if enabled
    return false;
}

/******************************************************************************
 * 6) The Keymap Layers
 *    from your earlier JSON export
 *****************************************************************************/
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [0] = LAYOUT(
        KC_GRV, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_BSPC,
        KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_MINS,
        KC_LCTL,KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN,KC_QUOT,
        KC_LSFT,KC_Z, KC_X, KC_C, KC_V, KC_B, KC_LBRC,KC_RBRC,KC_N, KC_M, KC_COMM,KC_DOT,
        KC_SLSH,KC_RSFT,KC_LALT,MO(1), KC_LGUI,KC_ENT, KC_SPC, MO(2), KC_BSPC, KC_RGUI
    ),

    [1] = LAYOUT(
        KC_ESC,  KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS, KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
        KC_F1,   KC_F2,  KC_F3,  KC_UP,  KC_F5,  KC_F6,   KC_7,   KC_8,   KC_9,   KC_F10, KC_F11, KC_F12,
        KC_GRV,  KC_EXLM,KC_LEFT,KC_DOWN,KC_RGHT,KC_PERC, KC_4,   KC_5,   KC_6,   KC_LPRN,KC_RPRN,KC_TILD,
        KC_TRNS, KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS, KC_TRNS,KC_TRNS,KC_1,   KC_2,   KC_3,   KC_LCBR,
        KC_RCBR, KC_PIPE,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS, KC_TRNS,KC_0,   KC_BSPC,KC_DOT
    ),

    [2] = LAYOUT(
        RM_NEXT,RM_PREV,RM_SATU,RM_SATD,RM_VALU,RM_VALD, KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,
        KC_GRV, KC_1,   KC_2,   KC_3,   KC_4,   KC_5,    KC_6,   KC_7,   KC_8,   KC_9,   KC_0,   KC_TRNS,
        KC_F1,  KC_F2,  KC_F3,  KC_F4,  KC_F5,  KC_F6,   KC_NO,  KC_LEFT,KC_DOWN,KC_UP,   KC_RGHT,KC_NO,
        KC_F7,  KC_F8,  KC_F9,  KC_F10, KC_F11, KC_F12,  RM_TOGG,KC_TRNS,KC_PLUS,KC_MINS,KC_EQL, KC_LBRC,
        KC_RBRC,KC_BSLS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS, KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS
    ),

    [3] = LAYOUT(
        KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
        KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
        KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
        KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
        KC_NO, KC_NO, KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS,KC_TRNS
    )
};
