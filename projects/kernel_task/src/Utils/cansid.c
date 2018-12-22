#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "cansid.h"

#define ESC '\x1B'


static void _clear(struct cansid_state *state)
{
    memset(state->values, 0, sizeof(state->values));
    
    state->currentIndex = 0;
}

static int _setValue( struct cansid_state *state , unsigned char val)
{
    if (state->currentIndex < MAX_VALUES)
    {
        
        state->values[ state->currentIndex ] = state->values[ state->currentIndex ]*10 +val;
        //printf("[Set value %u at index %i -> %u]" , val , state->currentIndex , state->values[ state->currentIndex ]);
        return 1;
    }
    return 0;
}

/*
static void _addDigitToCurrentValue(struct cansid_state *state , unsigned char val)
{
    state->values[ state->currentIndex] = state->values[ state->currentIndex]* 10 + val;
    
    
}
*/

struct cansid_state cansid_init(void)
{
	struct cansid_state rv = {
		.state = CANSID_ESC,
		.style = 0x0F,
		.next_style = 0x0F
	};
	return rv;
}

static inline unsigned char cansid_convert_color(unsigned char color)
{
	const unsigned char lookup_table[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };
	return lookup_table[(int)color];
}

struct color_char cansid_process(struct cansid_state *state, char x)
{
    
	struct color_char rv =
    {
		.style = state->style,
		.ascii = '\0'
	};

    
    state->operation = CANSID_NOOP;
	switch (state->state)
    {
		case CANSID_ESC:
            _clear(state);
            
			if (x == ESC)
            {
				state->state = CANSID_BRACKET;
            }
			else
            {
				rv.ascii = x;
			}
			break;
            
		case CANSID_BRACKET:
			if (x == '[')
            {
				state->state = CANSID_PARSE;
            }
			else
            {
				state->state = CANSID_ESC;
				rv.ascii = x;
			}
			break;
            
		case CANSID_PARSE:
            if (isdigit(x))
            {
                _setValue(state, strtol(&x, NULL , 10));
                state->state = CANSID_ENDVAL;
            }
            else if (x == '=')
            {
                state->state = CANSID_EQUALS;
            }
            else
            {
                state->state = CANSID_ESC;
                state->next_style = state->style;
                rv.ascii = x;
            }
            /*
            
			if (x == '3')
            {
				state->state = CANSID_FGCOLOR;
			}
            else if (x == '4')
            {
				state->state = CANSID_BGCOLOR;
			}
            else if (x == '0')
            {
				state->state = CANSID_ENDVAL;
				state->next_style = 0x0F;
			}
            else if (x == '1')
            {
				state->state = CANSID_ENDVAL;
				state->next_style |= (1 << 3);
			}
            
            else if (x == '2' )
            {
                state->state = CANSID_CLEAR;
			}
            */
			break;
            /*
		case CANSID_BGCOLOR:
			if (x >= '0' && x <= '7') {
				state->state = CANSID_ENDVAL;
				state->next_style &= 0x1F;
				state->next_style |= cansid_convert_color(x - '0') << 4;
			} else {
				state->state = CANSID_ESC;
				state->next_style = state->style;
				rv.ascii = x;
			}
			break;
		case CANSID_FGCOLOR:
			if (x >= '0' && x <= '7') {
				state->state = CANSID_ENDVAL;
				state->next_style &= 0xF8;
				state->next_style |= cansid_convert_color(x - '0');
			} else {
				state->state = CANSID_ESC;
				state->next_style = state->style;
				rv.ascii = x;
			}
			break;
             */
		case CANSID_EQUALS:
			if (x == '1') {
				state->state = CANSID_ENDVAL;
				state->next_style &= ~(1 << 3);
			} else {
				state->state = CANSID_ESC;
				state->next_style = state->style;
				rv.ascii = x;
			}
			break;
		case CANSID_ENDVAL:
            
            if (isdigit(x))
            {
                _setValue(state, strtol(&x, NULL , 10));
                
            }
            else
            {
                state->currentIndex++;
                if (x == ';')
                {
                    state->state = CANSID_PARSE;
                }
                else if (x == 'H' )
                {
                    
                    state->operation = CANSID_MOVECURSOR;
                    
                    state->state = CANSID_ESC;
                }
                else if (x == 'm')
                {
                    // Finish and apply styles
                    state->state = CANSID_ESC;
                    
                    state->operation = CANSID_COLOR;
                    state->style = state->next_style;
                }
                else if (x == 'J')
                {
                    
                    state->state = CANSID_ESC;
                    
                    state->operation = CANSID_CLEAR;
                }
                else
                {
                    state->state = CANSID_ESC;
                    state->next_style = state->style;
                    rv.ascii = x;
                }
            }
			break;
            /*
        case CANSID_CLEAR:
            
            break;
             */
		default:
			break;
	}
	return rv;
}
