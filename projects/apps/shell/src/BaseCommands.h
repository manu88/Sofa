/*
 * This file is part of the Sofa project
 * Copyright (c) 2018 Manuel Deneu.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once




// from EGA driver
/* Hardware text mode color constants. */
enum vga_color {
        VGA_COLOR_BLACK = 0,
        VGA_COLOR_BLUE = 1,
        VGA_COLOR_GREEN = 2,
        VGA_COLOR_CYAN = 3,
        VGA_COLOR_RED = 4,
        VGA_COLOR_MAGENTA = 5,
        VGA_COLOR_BROWN = 6,
        VGA_COLOR_LIGHT_GREY = 7,
        VGA_COLOR_DARK_GREY = 8,
        VGA_COLOR_LIGHT_BLUE = 9,
        VGA_COLOR_LIGHT_GREEN = 10,
        VGA_COLOR_LIGHT_CYAN = 11,
        VGA_COLOR_LIGHT_RED = 12,
        VGA_COLOR_LIGHT_MAGENTA = 13,
        VGA_COLOR_LIGHT_BROWN = 14,
        VGA_COLOR_WHITE = 15,
};

int InitConsoleFDs(int fdRead , int fdWrite);

ssize_t writeConsole( const void* b , size_t len);
ssize_t readConsole( void*b , size_t len);

void setTermColor( int color);
void clearTerm(void);
void setTermCoords(uint8_t x , uint8_t y);

void PrintHelp(void);

int exec_ls( const char* args);

int exec_cat( const char* args);

int exec_touch( const char* args);

int exec_exec( const char* args);

int exec_ps( const char* args);
int exec_kill( const char* args);

int exec_mkdir( const char* args);

int exec_sleep(const char* args);
int exec_stat(const char* args);

int exec_renice( const char* args);
