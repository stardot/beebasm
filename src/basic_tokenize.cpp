/*************************************************************************************************

	basic_tokenize.cpp - tokenize BBC BASIC

	Copyright (C) Charles Reilly 2022

	This file is licensed under the GNU General Public License version 3

*************************************************************************************************/

#include <assert.h>
#include <cerrno>
#include <cstring>
#include "basic_keywords.h"
#include "basic_tokenize.h"

// Read a text file keeping track of the line number and
// normalising line ends to carriage return (0x0D).
class Reader
{
public:
	Reader(FILE* file) : m_file(file)
	{
		m_line = 1;
		m_current = 0;
		m_end = false;
		m_errno = 0;
		m_lastcr = false;
		Next();
	}

	int LineNumber() const { return m_line; }
	char Current() const { return m_current; }
	bool End() const { return m_end; }

	void Next()
	{
		if (m_current == 0x0D)
		{
			if (m_end)
			{
				return;
			}
			m_line++;
		}
		int next = fgetc(m_file);
		if (m_lastcr && (next == 0x0A))
		{
			next = fgetc(m_file);
		}
		if (next == EOF)
		{
			if (ferror(m_file))
			{
				m_errno = errno;
			}
			// m_lastcr no longer matters
			m_end = true;
			m_current = 0x0D;
		}
		else if (next == 0x0A)
		{
			// Convert LF to CR
			m_lastcr = false;
			m_current = 0x0D;
		}
		else
		{
			m_lastcr = (next == 0x0D);
			m_current = next;
		}
	}

private:
	FILE* m_file;
	bool m_end;
	bool m_lastcr;
	char m_current;
	int m_errno;
	int m_line;
};

// A buffer for a single tokenized line of BASIC.
class Writer
{
public:
	void Init(int line_number)
	{
		m_fail = false;
		m_buffer[0] = 0x0D;
		m_buffer[1] = (line_number >> 8) & 0xFF;
		m_buffer[2] = line_number & 0xFF;
		m_length = 4;
	}
	bool Finish()
	{
		if (!m_fail)
		{
			assert(m_length < 0x100);
			m_buffer[3] = m_length;
		}
		return !m_fail;
	}
	void write(char c)
	{
		if (m_length < static_cast<int>(sizeof(m_buffer)))
		{
			m_buffer[m_length++] = c;
		}
		else
		{
			m_fail = true;
		}
	}
	int Length() const { return m_length; }
	const char* Data() const { return m_buffer; }

private:
	// Buffer cannot be any bigger because writes must fail when size exceeds byte
	char m_buffer[255];
	int m_length;
	bool m_fail;
};

// Type of a function that tests a property of a character
typedef bool (CHAR_TEST)(char c);

// Copy characters from the source to a tokenized line as long as a property holds.
void skip_write(CHAR_TEST f, Reader &reader, Writer &writer)
{
	while (f(reader.Current()))
	{
		writer.write(reader.Current());
		reader.Next();
	}
}

// Functions to test various character properties.  Built-in CRT functions are not
// used because they are locale dependent.
static bool is_not_cr(char c)
{
	return c != 0x0D;
}

static bool is_alpha(char c)
{
	return ('A' <= c) && (c <= 'Z');
}

static bool is_digit(char c)
{
	return ('0' <= c) && (c <= '9');
}

static bool is_alpha_digit(char c)
{
	// Note that this includes backtick aka pounds sterling
	return (('_' <= c) && (c <= 'z')) || (('A' <= c) && (c <= 'Z')) || is_digit(c);
}

static bool is_dot_digit(char c)
{
	return c == '.' || is_digit(c);
}

static bool is_hex_digit(char c)
{
	return (('A' <= c) && (c <= 'F')) || is_digit(c);
}

// Tokenize a line number.  If the number is too big
// simply copy it and return false.
static bool tokenize_linenum(Reader &reader, Writer &writer)
{
	char buffer[6];
	int zero_count = 0;
	while (reader.Current() == '0')
	{
		++zero_count;
		reader.Next();
	}
	int index = 0;
	int acc = 0;
	while (is_digit(reader.Current()))
	{
		char c = reader.Current();
		acc = 10 * acc + (c - '0');
		if (acc >= 0x8000)
		{
			while (zero_count--)
				writer.write('0');
			for (int i = 0; i != index; ++i)
			{
				writer.write(buffer[i]);
			}
			skip_write(is_digit, reader, writer);
			return false;
		}
		buffer[index++] = c;
		reader.Next();
	}
	writer.write(static_cast<unsigned char>(0x8D));
	writer.write((((acc & 0xC000) >> 12) | ((acc & 0xC0) >> 2)) ^ 0x54);
	writer.write((acc & 0x3F) | 0x40);
	writer.write((acc >> 8) | 0x40);
	return true;
}

static char read_next_char(Reader &reader)
{
	reader.Next();
	return reader.Current();
}

// Parse the characters from reader starting with the current character.
// If they form a keyword (and they are not C flagged with a digit or letter following) then:
//     Consume all the characters.
//     Return the keyword.
// If they form the prefix of a keyword then:
//     Consume all the matching prefix characters.
//     Write those same characters to the output.
//     If the last matching character was alphabetic (i.e. not '$') then:
//         Consume and write all the remaining is_alpha_digit characters.
//     return 0
// If there were no prefix characters (e.g. just 'Q' or '_') then
//     Consume and write all the remaining is_alpha_digit characters.
//     return 0
static const keyword* parse_keyword(Reader &reader, Writer &writer)
{
	// The number of characters consumed and matched from ReadStream
	int match_count = 0;
	// The name of the keyword whose first match_count characters are those read from ReadStream
	const char* match_name;

	for (int keyword_index = 0; keyword_index != keyword_list_length; ++keyword_index)
	{
		const keyword kw = keyword_list[keyword_index];
		if (!match_count || ((match_count <= kw.length) && !memcmp(match_name, kw.name, match_count)))
		{
			// Prefix seen so far matches this token - consume more matching characters
			while ((match_count < kw.length) && (reader.Current() == kw.name[match_count]))
			{
				reader.Next();
				++match_count;
			}
			if (match_count)
			{
				if (match_count == kw.length)
				{
					// Complete match
					if (kw.flags & kw_C_flag)
					{
						// Has C flag, so cannot be followed by letters or digits
						if (is_alpha_digit(reader.Current()))
						{
							for (int i = 0; i != match_count; ++i)
							{
								writer.write(kw.name[i]);
							}
							skip_write(is_alpha_digit, reader, writer);
							return 0;
						}
					}
					return &keyword_list[keyword_index];
				}
				if (reader.Current() == '.')
				{
					// Abbreviation so consume dot and return
					reader.Next();
					return &keyword_list[keyword_index];
				}
				match_name = kw.name;
			}
		}
	}
	if (match_count)
	{
		// Found a prefix but it wasn't a match so output it
		for (int i = 0; i != match_count; ++i)
		{
			writer.write(match_name[i]);
		}
		if (is_alpha(match_name[match_count-1]))
		{
			skip_write(is_alpha_digit, reader, writer);
		}
	}
	else
	{
		skip_write(is_alpha_digit, reader, writer);
	}
	return 0;
}

void tokenize_line(Reader &reader, Writer &writer)
{
	// From the prompt BASIC starts in tokenize-numbers mode.
	// This function works like the tokenizer used by AUTO.

	bool start_of_line = true;
	bool tokenize_numbers = false;

	while (true)
	{
		char c = reader.Current();

		if (c == 0x0D)
			return;

		if (c == ' ')
		{
			writer.write(c);
			reader.Next();
			continue;
		}

		if (c == '&')
		{
			writer.write(c);
			reader.Next();
			skip_write(is_hex_digit, reader, writer);
			continue;
		}

		if (c == '\"')
		{
			writer.write(c);
			do
			{
				c = read_next_char(reader);
				if (c == 0x0D)
					return;
				writer.write(c);
			}
			while (c != '\"');
			reader.Next();
			continue;
		}

		if (c == ':')
		{
			writer.write(c);
			reader.Next();
			start_of_line = true;
			tokenize_numbers = false;
			continue;
		}

		if (c == ',')
		{
			writer.write(c);
			reader.Next();
			continue;
		}

		if (c == '*')
		{
			if (start_of_line)
			{
				skip_write(is_not_cr, reader, writer);
				return;
			}
			writer.write(c);
			reader.Next();
			start_of_line = false;
			tokenize_numbers = false;
			continue;
		}

		if (is_dot_digit(c))
		{
			if ((c != '.') && tokenize_numbers)
			{
				tokenize_linenum(reader, writer);
				continue;
			}
			skip_write(is_dot_digit, reader, writer);
			start_of_line = false;
			tokenize_numbers = false;
			continue;
		}

		if (!is_alpha_digit(reader.Current()))
		{
			start_of_line = false;
			tokenize_numbers = false;
			writer.write(reader.Current());
			reader.Next();
			continue;
		}

		const keyword* keyword = parse_keyword(reader, writer);
		if (!keyword)
		{
			start_of_line = false;
			tokenize_numbers = false;
			continue;
		}
		else
		{
			byte token = keyword->token;
			byte flags = keyword->flags;

			if ((flags & kw_C_flag) && is_alpha_digit(reader.Current()))
			{
				// This should never happen because parse_keyword handles the C flag
				assert(false);
				start_of_line = false;
				tokenize_numbers = false;
				continue;
			}

			if ((flags & kw_P_flag) && start_of_line)
			{
				token += 0x40;
			}

			writer.write(token);

			if (flags & kw_M_flag)
			{
				start_of_line = false;
				tokenize_numbers = false;
			}

			if (flags & kw_S_flag)
			{
				start_of_line = true;
				tokenize_numbers = false;
			}

			if (flags & kw_F_flag)
			{
				skip_write(is_alpha_digit, reader, writer);
			}

			if (flags & kw_L_flag)
			{
				tokenize_numbers = true;
			}

			if (flags & kw_R_flag)
			{
				skip_write(is_not_cr, reader, writer);
				return;
			}
		}
	}
}

// Read a plain text BBC BASIC program from `file` and write it tokenized to `tokenized`
TokenizeError tokenize_file(FILE* file, std::vector<unsigned char>& tokenized)
{
	Reader reader(file);

	int last_line = -1;

	Writer writer;

	while (!reader.End())
	{
		while (reader.Current() == ' ')
		{
			reader.Next();
		}

		int line = 0;
		bool saw_digit = false;
		while (is_digit(reader.Current()))
		{
			saw_digit = true;
			line = 10 * line + (reader.Current() - '0');
			if (line > 0x7FFF)
			{
				break;
			}
			reader.Next();
		}

		if (saw_digit)
		{
			if (line <= last_line)
			{
				return TokenizeError(reader.LineNumber(), "Line numbers must increase");
			}
		}
		else
		{
			// Start auto-numbering at 1 but permit explicit line 0
			line = last_line < 0 ? 1 : last_line + 1;
		}
		last_line = line;

		if (line > 0x7FFF)
		{
			return TokenizeError(reader.LineNumber(), "Line number too big");
		}

		writer.Init(line);
		tokenize_line(reader, writer);
		if (!writer.Finish())
		{
			return TokenizeError(reader.LineNumber(), "Line too long after tokenizing");
		}

		// If the line is non-empty write it to the buffer
		if (writer.Length() > 4)
		{
			tokenized.insert(tokenized.end(), writer.Data(), writer.Data() + writer.Length());
		}

		reader.Next();
	}
	tokenized.push_back(0x0D);
	tokenized.push_back(0xFF);

	return TokenizeError();
}
