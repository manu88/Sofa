#ifndef CANSID_H
#define CANSID_H

#define MAX_VALUES 8

struct cansid_state {
    
	enum
    {
		CANSID_ESC,
		CANSID_BRACKET,
		CANSID_PARSE,
        
		CANSID_BGCOLOR,
		CANSID_FGCOLOR,
		CANSID_EQUALS,
        
		CANSID_ENDVAL,
	} state;
    
    enum
    {
        CANSID_NOOP,
        CANSID_CLEAR,
        CANSID_MOVECURSOR,
        CANSID_COLOR,
        
    } operation;
    
	unsigned char style;
	unsigned char next_style;
    
    unsigned char values[MAX_VALUES];
    int currentIndex;
    
    
};

struct color_char
{
	unsigned char style;
	unsigned char ascii;
};

struct cansid_state cansid_init(void);
struct color_char cansid_process(struct cansid_state *state, char x);

#endif
