//menu.cpp

#include "menu.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <avr/pgmspace.h>
#include "lcd.h"
#include "Configuration.h"
#include "Marlin.h"
#include "ultralcd.h"
#include "language.h"
#include "static_assert.h"



extern int32_t lcd_encoder;

#define MENU_DEPTH_MAX       6

static menu_record_t menu_stack[MENU_DEPTH_MAX];

uint8_t menu_data[MENU_DATA_SIZE];
#ifndef __AVR__
#error "menu_data is non-portable to non 8bit processor"
#endif

uint8_t menu_depth = 0;

uint8_t menu_line = 0;
uint8_t menu_item = 0;
uint8_t menu_row = 0;
uint8_t menu_top = 0;

uint8_t menu_clicked = 0;

uint8_t menu_entering = 0;
uint8_t menu_leaving = 0;

menu_func_t menu_menu = 0;

static_assert(sizeof(menu_data)>= sizeof(menu_data_edit_t),"menu_data_edit_t doesn't fit into menu_data");


void menu_goto(menu_func_t menu, const uint32_t encoder, const bool feedback, bool reset_menu_state)
{
	asm("cli");
	if (menu_menu != menu)
	{
		menu_menu = menu;
		lcd_encoder = encoder;
		asm("sei");
		if (reset_menu_state)
		{
			// Resets the global shared C union.
			// This ensures, that the menu entered will find out, that it shall initialize itself.
			memset(&menu_data, 0, sizeof(menu_data));
		}
		if (feedback) lcd_quick_feedback();
	}
	else
		asm("sei");
}

void menu_start(void)
{
    if (lcd_encoder > 0x8000) lcd_encoder = 0;
    if (lcd_encoder < 0)
    {
        lcd_encoder = 0;
        _tone(BEEPER, 0);
        _delay(100);
        _noTone(BEEPER);
    }
    if (lcd_encoder < menu_top)
		menu_top = lcd_encoder;
    menu_line = menu_top;
    menu_clicked = LCD_CLICKED;
}

void menu_end(void)
{
	if (lcd_encoder >= menu_item)
	{
		lcd_encoder = menu_item - 1;
        _tone(BEEPER, 0);
        _delay(50);
        _noTone(BEEPER);
	}
	if (((uint8_t)lcd_encoder) >= menu_top + LCD_HEIGHT)
	{
		menu_top = lcd_encoder - LCD_HEIGHT + 1;
		lcd_draw_update = 1;
		menu_line = menu_top - 1;
		menu_row = -1;
	}
}

void menu_back(uint8_t nLevel)
{
     menu_depth = ((menu_depth > nLevel) ? (menu_depth - nLevel) : 0);
     menu_goto(menu_stack[menu_depth].menu, menu_stack[menu_depth].position, true, true);
}

void menu_back(void)
{
menu_back(1);
}

static void menu_back_no_reset(void)
{
	if (menu_depth > 0)
	{
		menu_depth--;		
		menu_goto(menu_stack[menu_depth].menu, menu_stack[menu_depth].position, true, false);
	}
}

void menu_back_if_clicked(void)
{
	if (lcd_clicked())
		menu_back();
}

void menu_back_if_clicked_fb(void)
{
	if (lcd_clicked())
	{
        lcd_quick_feedback();
		menu_back();
	}
}

void menu_submenu(menu_func_t submenu)
{
	if (menu_depth < MENU_DEPTH_MAX)
	{
		menu_stack[menu_depth].menu = menu_menu;
		menu_stack[menu_depth++].position = lcd_encoder;
		menu_goto(submenu, 0, true, true);
	}
}

static void menu_submenu_no_reset(menu_func_t submenu)
{
	if (menu_depth < MENU_DEPTH_MAX)
	{
		menu_stack[menu_depth].menu = menu_menu;
		menu_stack[menu_depth++].position = lcd_encoder;
		menu_goto(submenu, 0, true, false);
	}
}

uint8_t menu_item_ret(void)
{
	lcd_beeper_quick_feedback();
	lcd_draw_update = 2;
	lcd_button_pressed = false;
	return 1;
}

/*
int menu_draw_item_printf_P(char type_char, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	int ret = 0;
    lcd_set_cursor(0, menu_row);
	if (lcd_encoder == menu_item)
		lcd_print('>');
	else
		lcd_print(' ');
	int cnt = vfprintf_P(lcdout, format, args);
	for (int i = cnt; i < 18; i++)
		lcd_print(' ');
	lcd_print(type_char);
	va_end(args);
	return ret;
}
*/

static void menu_draw_item_puts_P(char type_char, const char* str)
{
    lcd_set_cursor(0, menu_row);
    lcd_printf_P(PSTR("%c%-18.18S%c"), (lcd_encoder == menu_item)?'>':' ', str, type_char);
}

static int menu_draw_item_puts_P(char type_char, const char* str, char num)
{
    lcd_set_cursor(0, menu_row);
    lcd_printf_P(PSTR("%c%-.16S "), (lcd_encoder == menu_item)?'>':' ', str);
    lcd_putc(num);
    lcd_set_cursor(19, menu_row);
    lcd_putc(type_char);
}

/*
int menu_draw_item_puts_P_int16(char type_char, const char* str, int16_t val, )
{
    lcd_set_cursor(0, menu_row);
	int cnt = lcd_printf_P(PSTR("%c%-18S%c"), (lcd_encoder == menu_item)?'>':' ', str, type_char);
	return cnt;
}
*/

void menu_item_dummy(void)
{
	menu_item++;
}

uint8_t menu_item_text_P(const char* str)
{
	if (menu_item == menu_line)
	{
		if (lcd_draw_update) menu_draw_item_puts_P(' ', str);
		if (menu_clicked && (lcd_encoder == menu_item))
			return menu_item_ret();
	}
	menu_item++;
	return 0;
}

uint8_t menu_item_submenu_P(const char* str, menu_func_t submenu)
{
	if (menu_item == menu_line)
	{
		if (lcd_draw_update) menu_draw_item_puts_P(LCD_STR_ARROW_RIGHT[0], str);
		if (menu_clicked && (lcd_encoder == menu_item))
		{
			menu_submenu(submenu);
			return menu_item_ret();
		}
	}
	menu_item++;
	return 0;
}

uint8_t menu_item_back_P(const char* str)
{
	if (menu_item == menu_line)
	{
		if (lcd_draw_update) menu_draw_item_puts_P(LCD_STR_UPLEVEL[0], str);
		if (menu_clicked && (lcd_encoder == menu_item))
		{
			menu_back();
			return menu_item_ret();
		}
	}
	menu_item++;
	return 0;
}

uint8_t menu_item_function_P(const char* str, menu_func_t func)
{
	if (menu_item == menu_line)
	{
		if (lcd_draw_update) menu_draw_item_puts_P(' ', str);
		if (menu_clicked && (lcd_encoder == menu_item))
		{
			menu_clicked = false;
			lcd_consume_click();
			lcd_update_enabled = 0;
			if (func) func();
			lcd_update_enabled = 1;
			return menu_item_ret();
		}
	}
	menu_item++;
	return 0;
}

//! @brief Menu item function taking single parameter
//!
//! Ideal for numbered lists calling functions with number parameter.
//! @param str Item caption
//! @param number aditional character to be added after str, e.g. number
//! @param func pointer to function taking uint8_t with no return value
//! @param fn_par value to be passed to function
//! @retval 0
//! @retval 1 Item was clicked
uint8_t menu_item_function_P(const char* str, char number, void (*func)(uint8_t), uint8_t fn_par)
{
    if (menu_item == menu_line)
    {
        if (lcd_draw_update) menu_draw_item_puts_P(' ', str, number);
        if (menu_clicked && (lcd_encoder == menu_item))
        {
            menu_clicked = false;
            lcd_consume_click();
            lcd_update_enabled = 0;
            if (func) func(fn_par);
            lcd_update_enabled = 1;
            return menu_item_ret();
        }
    }
    menu_item++;
    return 0;
}

uint8_t menu_item_gcode_P(const char* str, const char* str_gcode)
{
	if (menu_item == menu_line)
	{
		if (lcd_draw_update) menu_draw_item_puts_P(' ', str);
		if (menu_clicked && (lcd_encoder == menu_item))
		{
			if (str_gcode) enquecommand_P(str_gcode);
			return menu_item_ret();
		}
	}
	menu_item++;
	return 0;
}


const char menu_20x_space[] PROGMEM = "                    ";

const char menu_fmt_int3[] PROGMEM = "%c%.15S:%s%3d";

const char menu_fmt_float31[] PROGMEM = "%-12.12s%+8.1f";

const char menu_fmt_float13[] PROGMEM = "%c%.12S:%s%+06.3f";

const char menu_fmt_float13off[] PROGMEM = "%c%.12S:%s%";

template<typename T>
static void menu_draw_P(char chr, const char* str, int16_t val);

template<>
void menu_draw_P<int16_t*>(char chr, const char* str, int16_t val)
{
	int text_len = strlen_P(str);
	if (text_len > 15) text_len = 15;
	char spaces[21];
	strcpy_P(spaces, menu_20x_space);
	if (val <= -100) spaces[15 - text_len - 1] = 0;
	else spaces[15 - text_len] = 0;
	lcd_printf_P(menu_fmt_int3, chr, str, spaces, val);
}

template<>
void menu_draw_P<uint8_t*>(char chr, const char* str, int16_t val)
{
    menu_data_edit_t* _md = (menu_data_edit_t*)&(menu_data[0]);
    int text_len = strlen_P(str);
    if (text_len > 15) text_len = 15;
    char spaces[21];
    strcpy_P(spaces, menu_20x_space);
    spaces[12 - text_len] = 0;
    float factor = 1.0 + static_cast<float>(val) / 1000.0;
    if (val <= _md->minEditValue)
    {
        lcd_printf_P(menu_fmt_float13off, chr, str, spaces);
        lcd_puts_P(_i(" [off]"));
    }
    else
    {
        lcd_printf_P(menu_fmt_float13, chr, str, spaces, factor);
    }
}

//! Draw up to 10 chars of text, ':' and float number in format from +0.0 to +12345.0. The increased range is necessary
//! for displaying large values of extruder positions, which caused text overflow in the previous implementation.
//! @param chr first character to print on the line
//! @param str string label to print, will be appended with ':' automatically inside the function
//! @param val value to print aligned to the right side of the display  
//! 
//! Implementation comments:
//! The text needs to be prerendered into the prerendered[] to enable left alignment of text str including the colon behind it.
//! If we didn't want the colon behind it, the whole operation would have been solved with a single vsprintf call,
//! but such line would look different compared to every other similar menu item
//! So it is almost the same amount of code like before, but with added string prerendering 
void menu_draw_float31(char chr, const char* str, float val)
{
	uint8_t txtlen = strlen_P(str);
	if( txtlen > 10 )txtlen = 10;
	char prerendered[21];
	strcpy_P(prerendered, menu_20x_space);
	prerendered[0] = chr;        // start with the initial byte/space for menu navigation
	strncpy_P(prerendered+1, str, 10); // render the text and limit it to max 10 characters
	prerendered[txtlen+1] = ':'; // put the colon behind it
	prerendered[txtlen+2] = 0;   // terminate the string to be used inside the printf
	lcd_printf_P(menu_fmt_float31, prerendered, val);	
}

//draw up to 12 chars of text, ':' and float number in format +1.234
void menu_draw_float13(char chr, const char* str, float val)
{
	int text_len = strlen_P(str);
	if (text_len > 12) text_len = 12;
	char spaces[21];
	strcpy_P(spaces, menu_20x_space);
	spaces[12 - text_len] = 0;
	lcd_printf_P(menu_fmt_float13, chr, str, spaces, val);
}

template <typename T>
static void _menu_edit_P(void)
{
	menu_data_edit_t* _md = (menu_data_edit_t*)&(menu_data[0]);
	if (lcd_draw_update)
	{
		if (lcd_encoder < _md->minEditValue) lcd_encoder = _md->minEditValue;
		if (lcd_encoder > _md->maxEditValue) lcd_encoder = _md->maxEditValue;
		lcd_set_cursor(0, 1);
		menu_draw_P<T>(' ', _md->editLabel, (int)lcd_encoder);
	}
	if (LCD_CLICKED)
	{
		*((T)(_md->editValue)) = lcd_encoder;
		menu_back_no_reset();
	}
}

template <typename T>
uint8_t menu_item_edit_P(const char* str, T pval, int16_t min_val, int16_t max_val)
{
	menu_data_edit_t* _md = (menu_data_edit_t*)&(menu_data[0]);
	if (menu_item == menu_line)
	{
		if (lcd_draw_update) 
		{
			lcd_set_cursor(0, menu_row);
			menu_draw_P<T>((lcd_encoder == menu_item)?'>':' ', str, *pval);
		}
		if (menu_clicked && (lcd_encoder == menu_item))
		{
			menu_submenu_no_reset(_menu_edit_P<T>);
			_md->editLabel = str;
			_md->editValue = pval;
			_md->minEditValue = min_val;
			_md->maxEditValue = max_val;
			lcd_encoder = *pval;
			return menu_item_ret();
		}
	}
	menu_item++;
	return 0;
}

template uint8_t menu_item_edit_P<int16_t*>(const char* str, int16_t *pval, int16_t min_val, int16_t max_val);
template uint8_t menu_item_edit_P<uint8_t*>(const char* str, uint8_t *pval, int16_t min_val, int16_t max_val);

#undef _menu_data
