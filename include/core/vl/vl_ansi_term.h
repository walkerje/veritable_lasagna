/**
 * ██    ██ ██       █████  ███████  █████   ██████  ███    ██  █████
 * ██    ██ ██      ██   ██ ██      ██   ██ ██       ████   ██ ██   ██
 * ██    ██ ██      ███████ ███████ ███████ ██   ███ ██ ██  ██ ███████
 *  ██  ██  ██      ██   ██      ██ ██   ██ ██    ██ ██  ██ ██ ██   ██
 *   ████   ███████ ██   ██ ███████ ██   ██  ██████  ██   ████ ██   ██
 * ====---: A Data Structure and Algorithms library for C11.  :---====
 *
 * Copyright 2026 Jesse Walker, released under the MIT license.
 * Git Repository:  https://github.com/walkerje/veritable_lasagna
 * \private
 */

#ifndef VL_ANSI_TERM_H
#define VL_ANSI_TERM_H

// ANSI escape introducer (Control Sequence Introducer)
#define VL_ANSI_CSI "\x1b["

// ---- Text attributes ----

#define VL_ANSI_RESET "\x1b[0m"
#define VL_ANSI_BOLD "\x1b[1m"
#define VL_ANSI_DIM "\x1b[2m"
#define VL_ANSI_ITALIC "\x1b[3m"
#define VL_ANSI_UNDERLINE "\x1b[4m"
#define VL_ANSI_BLINK "\x1b[5m"
#define VL_ANSI_REVERSE "\x1b[7m"
#define VL_ANSI_HIDDEN "\x1b[8m"
#define VL_ANSI_STRIKETHROUGH "\x1b[9m"

#define VL_ANSI_NO_BOLD_DIM "\x1b[22m"
#define VL_ANSI_NO_ITALIC "\x1b[23m"
#define VL_ANSI_NO_UNDERLINE "\x1b[24m"
#define VL_ANSI_NO_BLINK "\x1b[25m"
#define VL_ANSI_NO_REVERSE "\x1b[27m"
#define VL_ANSI_NO_HIDDEN "\x1b[28m"
#define VL_ANSI_NO_STRIKETHROUGH "\x1b[29m"

// ---- Standard colors (foreground) ----

#define VL_ANSI_FG_BLACK "\x1b[30m"
#define VL_ANSI_FG_RED "\x1b[31m"
#define VL_ANSI_FG_GREEN "\x1b[32m"
#define VL_ANSI_FG_YELLOW "\x1b[33m"
#define VL_ANSI_FG_BLUE "\x1b[34m"
#define VL_ANSI_FG_MAGENTA "\x1b[35m"
#define VL_ANSI_FG_CYAN "\x1b[36m"
#define VL_ANSI_FG_WHITE "\x1b[37m"

// ---- Standard colors (background) ----

#define VL_ANSI_BG_BLACK "\x1b[40m"
#define VL_ANSI_BG_RED "\x1b[41m"
#define VL_ANSI_BG_GREEN "\x1b[42m"
#define VL_ANSI_BG_YELLOW "\x1b[43m"
#define VL_ANSI_BG_BLUE "\x1b[44m"
#define VL_ANSI_BG_MAGENTA "\x1b[45m"
#define VL_ANSI_BG_CYAN "\x1b[46m"
#define VL_ANSI_BG_WHITE "\x1b[47m"

// ---- Bright colors (foreground) ----

#define VL_ANSI_FG_BRIGHT_BLACK "\x1b[90m"
#define VL_ANSI_FG_BRIGHT_RED "\x1b[91m"
#define VL_ANSI_FG_BRIGHT_GREEN "\x1b[92m"
#define VL_ANSI_FG_BRIGHT_YELLOW "\x1b[93m"
#define VL_ANSI_FG_BRIGHT_BLUE "\x1b[94m"
#define VL_ANSI_FG_BRIGHT_MAGENTA "\x1b[95m"
#define VL_ANSI_FG_BRIGHT_CYAN "\x1b[96m"
#define VL_ANSI_FG_BRIGHT_WHITE "\x1b[97m"

// ---- Bright colors (background) ----

#define VL_ANSI_BG_BRIGHT_BLACK "\x1b[100m"
#define VL_ANSI_BG_BRIGHT_RED "\x1b[101m"
#define VL_ANSI_BG_BRIGHT_GREEN "\x1b[102m"
#define VL_ANSI_BG_BRIGHT_YELLOW "\x1b[103m"
#define VL_ANSI_BG_BRIGHT_BLUE "\x1b[104m"
#define VL_ANSI_BG_BRIGHT_MAGENTA "\x1b[105m"
#define VL_ANSI_BG_BRIGHT_CYAN "\x1b[106m"
#define VL_ANSI_BG_BRIGHT_WHITE "\x1b[107m"

// ---- Default colors ----
//
#define VL_ANSI_FG_DEFAULT "\x1b[39m"
#define VL_ANSI_BG_DEFAULT "\x1b[49m"

// ---- Cursor + screen controls----

#define VL_ANSI_CLEAR_SCREEN "\x1b[2J"
#define VL_ANSI_CLEAR_SCROLLBACK "\x1b[3J"
#define VL_ANSI_CLEAR_LINE "\x1b[2K"

#define VL_ANSI_CURSOR_HOME "\x1b[H"
#define VL_ANSI_CURSOR_SAVE "\x1b[s"
#define VL_ANSI_CURSOR_RESTORE "\x1b[u"
#define VL_ANSI_CURSOR_HIDE "\x1b[?25l"
#define VL_ANSI_CURSOR_SHOW "\x1b[?25h"

#define VL_ANSI_WRAP_ENABLE "\x1b[?7h"
#define VL_ANSI_WRAP_DISABLE "\x1b[?7l"

#endif // VL_ANSI_TERM_H
