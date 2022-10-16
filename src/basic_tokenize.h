/*************************************************************************************************

	basic_tokenize.h - tokenize BBC BASIC

	Copyright (C) Charles Reilly 2022

	This file is licensed under the GNU General Public License version 3

*************************************************************************************************/

#ifndef TOKENIZE_H_
#define TOKENIZE_H_

#include <stdio.h>
#include <vector>

struct TokenizeError
{
	TokenizeError()
	{
		messageText = 0;
		lineNumber = 0;
	}
	TokenizeError(int line, const char* message)
	{
		lineNumber = line;
		messageText = message;
	}
	bool IsError() const { return messageText != 0; };
	const char* messageText;
	int lineNumber;
};

// Read a plain text BBC BASIC program from `file` and write it tokenized to `tokenized`
TokenizeError tokenize_file(FILE* file, std::vector<unsigned char>& tokenized);

#endif // TOKENIZE_H_
