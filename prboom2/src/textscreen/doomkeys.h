//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//       Key definitions
//

#ifndef __DOOMKEYS__
#define __DOOMKEYS__

//
// DOOM keyboard definition.
// This is the stuff configured by Setup.Exe.
// Most key data are simple ascii (uppercased).
//
#define NUMKEYS 256
#define KEY_RIGHTARROW 0xae
#define KEY_LEFTARROW  0xac
#define KEY_UPARROW    0xad
#define KEY_DOWNARROW  0xaf
#define KEY_ESCAPE     27
#define KEY_ENTER      13
#define KEY_TAB        9

#define KEY_BACKSPACE  0x7f
#define KEY_PAUSE      0xff

#define KEY_EQUALS     0x3d
#define KEY_MINUS      0x2d

enum
{

    // Keys without character representations

    KEY_F1 = 0x80,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,

    KEY_RSHIFT,
    KEY_RCTRL,
    KEY_RALT,
    KEY_LALT = KEY_RALT,
    KEY_CAPSLOCK,
    KEY_NUMLOCK,
    KEY_SCRLCK,
    KEY_PRTSCR,
    KEY_HOME,
    KEY_END,
    KEY_PGUP,
    KEY_PGDN,
    KEY_INS,
    KEY_DEL,

    // Keys on the numerics keypad

    KEYP_0,
    KEYP_1,
    KEYP_2,
    KEYP_3,
    KEYP_4,
    KEYP_5,
    KEYP_6,
    KEYP_7,
    KEYP_8,
    KEYP_9,
    KEYP_DIVIDE,
    KEYP_PLUS,
    KEYP_MINUS,
    KEYP_MULTIPLY,
    KEYP_PERIOD,
    KEYP_EQUALS = KEY_EQUALS,
    KEYP_ENTER = KEY_ENTER,
};

#define SCANCODE_TO_KEYS_ARRAY {                                            \
    0,   0,   0,   0,   'a',                                  /* 0-9 */     \
    'b', 'c', 'd', 'e', 'f',                                                \
    'g', 'h', 'i', 'j', 'k',                                  /* 10-19 */   \
    'l', 'm', 'n', 'o', 'p',                                                \
    'q', 'r', 's', 't', 'u',                                  /* 20-29 */   \
    'v', 'w', 'x', 'y', 'z',                                                \
    '1', '2', '3', '4', '5',                                  /* 30-39 */   \
    '6', '7', '8', '9', '0',                                                \
    KEY_ENTER, KEY_ESCAPE, KEY_BACKSPACE, KEY_TAB, ' ',       /* 40-49 */   \
    KEY_MINUS, KEY_EQUALS, '[', ']', '\\',                                  \
    0,   ';', '\'', '`', ',',                                 /* 50-59 */   \
    '.', '/', KEY_CAPSLOCK, KEY_F1, KEY_F2,                                 \
    KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7,                   /* 60-69 */   \
    KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,                              \
    KEY_PRTSCR, KEY_SCRLCK, KEY_PAUSE, KEY_INS, KEY_HOME,     /* 70-79 */   \
    KEY_PGUP, KEY_DEL, KEY_END, KEY_PGDN, KEY_RIGHTARROW,                   \
    KEY_LEFTARROW, KEY_DOWNARROW, KEY_UPARROW,                /* 80-89 */   \
    KEY_NUMLOCK, KEYP_DIVIDE,                                               \
    KEYP_MULTIPLY, KEYP_MINUS, KEYP_PLUS, KEYP_ENTER, KEYP_1,               \
    KEYP_2, KEYP_3, KEYP_4, KEYP_5, KEYP_6,                   /* 90-99 */   \
    KEYP_7, KEYP_8, KEYP_9, KEYP_0, KEYP_PERIOD,                            \
    0, 0, 0, KEYP_EQUALS,                                     /* 100-103 */ \
}

// Default names for keys, to use in English or as fallback.
#define KEY_NAMES_ARRAY {                                            \
    { KEY_BACKSPACE,  "BACKSP" },   { KEY_TAB,        "TAB" },       \
    { KEY_INS,        "INS" },      { KEY_DEL,        "DEL" },       \
    { KEY_PGUP,       "PGUP" },     { KEY_PGDN,       "PGDN" },      \
    { KEY_ENTER,      "ENTER" },    { KEY_ESCAPE,     "ESC" },       \
    { KEY_F1,         "F1" },       { KEY_F2,         "F2" },        \
    { KEY_F3,         "F3" },       { KEY_F4,         "F4" },        \
    { KEY_F5,         "F5" },       { KEY_F6,         "F6" },        \
    { KEY_F7,         "F7" },       { KEY_F8,         "F8" },        \
    { KEY_F9,         "F9" },       { KEY_F10,        "F10" },       \
    { KEY_F11,        "F11" },      { KEY_F12,        "F12" },       \
    { KEY_HOME,       "HOME" },     { KEY_END,        "END" },       \
    { KEY_MINUS,      "-" },        { KEY_EQUALS,     "=" },         \
    { KEY_NUMLOCK,    "NUMLCK" },   { KEY_SCRLCK,     "SCRLCK" },    \
    { KEY_PAUSE,      "PAUSE" },    { KEY_PRTSCR,     "PRTSC" },     \
    { KEY_UPARROW,    "UP" },       { KEY_DOWNARROW,  "DOWN" },      \
    { KEY_LEFTARROW,  "LEFT" },     { KEY_RIGHTARROW, "RIGHT" },     \
    { KEY_RALT,       "ALT" },      { KEY_LALT,       "ALT" },       \
    { KEY_RSHIFT,     "SHIFT" },    { KEY_CAPSLOCK,   "CAPS" },      \
    { KEY_RCTRL,      "CTRL" },     { KEYP_5,         "NUM5" },      \
    { ' ',            "SPACE" },                                     \
    { 'a', "A" },   { 'b', "B" },   { 'c', "C" },   { 'd', "D" },    \
    { 'e', "E" },   { 'f', "F" },   { 'g', "G" },   { 'h', "H" },    \
    { 'i', "I" },   { 'j', "J" },   { 'k', "K" },   { 'l', "L" },    \
    { 'm', "M" },   { 'n', "N" },   { 'o', "O" },   { 'p', "P" },    \
    { 'q', "Q" },   { 'r', "R" },   { 's', "S" },   { 't', "T" },    \
    { 'u', "U" },   { 'v', "V" },   { 'w', "W" },   { 'x', "X" },    \
    { 'y', "Y" },   { 'z', "Z" },   { '0', "0" },   { '1', "1" },    \
    { '2', "2" },   { '3', "3" },   { '4', "4" },   { '5', "5" },    \
    { '6', "6" },   { '7', "7" },   { '8', "8" },   { '9', "9" },    \
    { '[', "[" },   { ']', "]" },   { ';', ";" },   { '`', "`" },    \
    { ',', "," },   { '.', "." },   { '/', "/" },   { '\\', "\\" },  \
    { '\'', "\'" },                                                  \
}

enum
{
    GAMEPAD_A,
    GAMEPAD_B,
    GAMEPAD_X,
    GAMEPAD_Y,
    GAMEPAD_BACK,
    GAMEPAD_GUIDE,
    GAMEPAD_START,
    GAMEPAD_LEFT_STICK,
    GAMEPAD_RIGHT_STICK,
    GAMEPAD_LEFT_SHOULDER,
    GAMEPAD_RIGHT_SHOULDER,
    GAMEPAD_DPAD_UP,
    GAMEPAD_DPAD_DOWN,
    GAMEPAD_DPAD_LEFT,
    GAMEPAD_DPAD_RIGHT,
    GAMEPAD_MISC1,
    GAMEPAD_PADDLE1,
    GAMEPAD_PADDLE2,
    GAMEPAD_PADDLE3,
    GAMEPAD_PADDLE4,
    GAMEPAD_TOUCHPAD_PRESS,
    GAMEPAD_TOUCHPAD_TOUCH,
    GAMEPAD_LEFT_TRIGGER,
    GAMEPAD_RIGHT_TRIGGER,
    GAMEPAD_LEFT_STICK_UP,
    GAMEPAD_LEFT_STICK_DOWN,
    GAMEPAD_LEFT_STICK_LEFT,
    GAMEPAD_LEFT_STICK_RIGHT,
    GAMEPAD_RIGHT_STICK_UP,
    GAMEPAD_RIGHT_STICK_DOWN,
    GAMEPAD_RIGHT_STICK_LEFT,
    GAMEPAD_RIGHT_STICK_RIGHT,

    NUM_GAMEPAD_BUTTONS
};

enum
{
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_X1,
    MOUSE_BUTTON_X2,
    MOUSE_BUTTON_WHEELUP,
    MOUSE_BUTTON_WHEELDOWN,
    MOUSE_BUTTON_WHEELLEFT,
    MOUSE_BUTTON_WHEELRIGHT,

    NUM_MOUSE_BUTTONS
};

enum
{
    AXIS_LEFTX,
    AXIS_LEFTY,
    AXIS_RIGHTX,
    AXIS_RIGHTY,

    NUM_AXES
};

enum
{
    AXIS_STRAFE,
    AXIS_FORWARD,
    AXIS_TURN,
    AXIS_LOOK,
};

enum
{
    GYRO_TURN,
    GYRO_LOOK,

    NUM_GYRO_AXES
};

#endif // __DOOMKEYS__
