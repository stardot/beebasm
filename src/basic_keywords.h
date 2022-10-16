/*************************************************************************************************

	basic_keywords.h - a table of BBC BASIC keywords

*************************************************************************************************/

#ifndef BASIC_KEYWORDS_H_
#define BASIC_KEYWORDS_H_

#define ARRAY_LENGTH(a) (sizeof(a)/sizeof(a[0]))

typedef unsigned char byte;

enum keyword_flags
{
	kw_C_flag = 0x01,
	kw_M_flag = 0x02,
	kw_S_flag = 0x04,
	kw_F_flag = 0x08,
	kw_L_flag = 0x10,
	kw_R_flag = 0x20,
	kw_P_flag = 0x40
};

struct keyword
{
	const char* name;
	byte length;
	byte token;
	byte flags;
};

extern const keyword keyword_list[];
extern int keyword_list_length;

#endif // BASIC_KEYWORDS_H_
