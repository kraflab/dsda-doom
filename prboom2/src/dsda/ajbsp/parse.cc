//----------------------------------------------------------------------------
//  Lexer (tokenizer)
//----------------------------------------------------------------------------
//
//  Copyright (c) 2022  Andrew Apted
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//----------------------------------------------------------------------------

#include "system.h"
#include "parse.h"

#include <cstdlib>
#include <cctype>
#include <cassert>

namespace ajbsp
{

token_kind_e lexer_c::Next(std::string& s)
{
	s.clear();

	SkipToNext();

	if (pos >= data.size())
		return TOK_EOF;

	unsigned char ch = (unsigned char) data[pos];

	if (ch == '"')
		return ParseString(s);

	if (ch == '-' || ch == '+' || std::isdigit(ch))
		return ParseNumber(s);

	if (std::isalpha(ch) || ch == '_' || ch >= 128)
		return ParseIdentifier(s);

	// anything else is a single-character symbol
	s.push_back(data[pos++]);

	return TOK_Symbol;
}


bool lexer_c::Match(const char *s)
{
	assert(s);
	assert(s[0]);

	bool is_keyword = std::isalnum(s[0]);

	SkipToNext();

	size_t ofs = 0;

	for (; *s != 0 ; s++, ofs++)
	{
		if (pos + ofs >= data.size())
			return false;

		unsigned char A = (unsigned char) data[pos + ofs];
		unsigned char B = (unsigned char) s[0];

		// don't change a char when high-bit is set (for UTF-8)
		if (A < 128) A = std::tolower(A);
		if (B < 128) B = std::tolower(B);

		if (A != B)
			return false;
	}

	pos += ofs;

	// for a keyword, require a non-alphanumeric char after it.
	if (is_keyword && pos < data.size())
	{
		unsigned char ch = (unsigned char) data[pos];

		if (std::isalnum(ch) || ch >= 128)
			return false;
	}

	return true;
}


int lexer_c::LastLine()
{
	return line;
}


void lexer_c::Rewind()
{
	pos  = 0;
	line = 1;
}


int LEX_Int(const std::string& s)
{
	// strtol handles all the integer sequences of the UDMF spec
	return (int)std::strtol(s.c_str(), NULL, 0);
}


double LEX_Double(const std::string& s)
{
	// strtod handles all the floating-point sequences of the UDMF spec
	return std::strtod(s.c_str(), NULL);
}


bool LEX_Boolean(const std::string& s)
{
	if (s.empty())
		return false;

	return (s[0] == 't' || s[0] == 'T');
}

//----------------------------------------------------------------------------

void lexer_c::SkipToNext()
{
	while (pos < data.size())
	{
		unsigned char ch = (unsigned char) data[pos];

		// bump line number at end of a line
		if (ch == '\n')
			line += 1;

		// skip whitespace and control chars
		if (ch <= 32 || ch == 127)
		{
			pos++;
			continue;
		}

		if (ch == '/' && pos+1 < data.size())
		{
			// single line comment?
			if (data[pos+1] == '/')
			{
				pos += 2;

				while (pos < data.size() && data[pos] != '\n')
					pos++;

				continue;
			}

			// multi-line comment?
			if (data[pos+1] == '*')
			{
				pos += 2;

				while (pos < data.size())
				{
					if (pos+1 < data.size() && data[pos] == '*' && data[pos+1] == '/')
					{
						pos += 2;
						break;
					}

					if (data[pos] == '\n')
						line += 1;

					pos++;
				}

				continue;
			}
		}

		// reached a token!
		return;
	}
}


token_kind_e lexer_c::ParseIdentifier(std::string& s)
{
	// NOTE: we lowercase the identifier put into 's'.

	for (;;)
	{
		unsigned char ch = (unsigned char) data[pos];

		// don't change a char when high-bit is set (for UTF-8)
		if (ch < 128)
			ch = std::tolower(ch);

		if (! (std::isalnum(ch) || ch == '_' || ch >= 128))
			break;

		s.push_back((char) ch);
		pos++;
	}

	assert(s.size() > 0);

	return TOK_Ident;
}


token_kind_e lexer_c::ParseNumber(std::string& s)
{
	if (data[pos] == '-' || data[pos] == '+')
	{
		// no digits after the sign?
		if (pos+1 >= data.size() || ! std::isdigit(data[pos+1]))
		{
			s.push_back(data[pos++]);
			return TOK_Symbol;
		}
	}

	for (;;)
	{
		s.push_back(data[pos++]);

		if (pos >= data.size())
			break;

		unsigned char ch = (unsigned char) data[pos];

		// this is fairly lax, but adequate for our purposes
		if (! (std::isalnum(ch) || ch == '+' || ch == '-' || ch == '.'))
			break;
	}

	return TOK_Number;
}


token_kind_e lexer_c::ParseString(std::string& s)
{
	// NOTE: we allow newlines ('\n') in the string, rather than produce an
	//       an unterminated-string error.

	pos++;

	while (pos < data.size())
	{
		unsigned char ch = (unsigned char) data[pos++];

		if (ch == '"')
			break;

		if (ch == '\\')
		{
			ParseEscape(s);
			continue;
		}

		// bump line number at end of a line
		if (ch == '\n')
			line += 1;

		// skip all control characters except TAB and NEWLINE
		if (ch < 32 && ! (ch == '\t' || ch == '\n'))
			continue;

		if (ch == 127)  // DEL
			continue;

		s.push_back((char) ch);
	}

	return TOK_String;
}


void lexer_c::ParseEscape(std::string& s)
{
	if (pos >= data.size())
	{
		s.push_back('\\');
		return;
	}

	unsigned char ch = (unsigned char) data[pos];

	// avoid control chars, especially newline
	if (ch < 32 || ch == 127)
	{
		s.push_back('\\');
		return;
	}

	pos++;

	// octal sequence?  1 to 3 digits.
	if ('0' <= ch && ch <= '7')
	{
		int val = (int)(ch - '0');

		ch = (unsigned char) data[pos];
		if ('0' <= ch && ch <= '7')
		{
			val = val * 8 + (int)(ch - '0');
			pos++;
		}

		ch = (unsigned char) data[pos];
		if ('0' <= ch && ch <= '7')
		{
			val = val * 8 + (int)(ch - '0');
			pos++;
		}

		s.push_back((char) val);
		return;
	}

	// hexadecimal sequence?  followed by 1 to 2 hex digits.
	if (ch == 'x' || ch == 'X')
	{
		char buffer[16];
		char *p = buffer;

		*p++ = '0';
		*p++ = 'x';
		*p++ = '0';

		ch = (unsigned char) data[pos];
		if (std::isxdigit(ch)) { *p++ = ch; pos++; }

		ch = (unsigned char) data[pos];
		if (std::isxdigit(ch)) { *p++ = ch; pos++; }

		*p = 0;

		int val = (int)std::strtol(buffer, NULL, 0);
		s.push_back((char) val);
		return;
	}

	switch(ch)
	{
		case 'a': s.push_back('\a'); break;  // bell
		case 'b': s.push_back('\b'); break;  // backspace
		case 'f': s.push_back('\f'); break;  // form feed
		case 'n': s.push_back('\n'); break;  // newline
		case 't': s.push_back('\t'); break;  // tab
		case 'r': s.push_back('\r'); break;  // carriage return
		case 'v': s.push_back('\v'); break;  // vertical tab

		// the default is to reproduce the same character
		default: s.push_back(ch); break;
	}
}


} // namespace ajbsp

//--- editor settings ---
// vi:ts=4:sw=4:noexpandtab
