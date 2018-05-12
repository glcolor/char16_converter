/*
Copyright(c) 2018 glcolor

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include <string>
#include <algorithm>

namespace jx
{
	//标识符类型
	enum class TokenType:short
	{
		Unknown=0,
		WhiteSpace,
		Identifier,
		IntNumber,
		BitsNumber,
		RealNumber,
		Char,
		UnicodeChar,
		String,
		BlockString,
		UnicodeString,
		NonTerminatedString,
		LineComment,
		MultilineComment,

		Assign,

		EoF
	};

	struct TokenString
	{
		const char*			m_Data;
		int					m_Length;
		TokenString()
		{
			m_Data = nullptr;
			m_Length = 0;
		}

		TokenString(const char* str, int len=-1)
		{
			m_Data = str;
			m_Length = len;
			if (m_Length < 0)
			{
				m_Length = (int)strlen(m_Data);
			}
		}

		//转换为std::string
		operator std::string() const
		{
			return std::string(m_Data, m_Length);
		}

		std::string ToString() const
		{
			return std::string(m_Data, m_Length);
		}

		//比较操作
		bool operator ==(const TokenString& str) const
		{
			if (m_Length != str.m_Length) return false;
			return strncmp(m_Data, str.m_Data, m_Length)==0;
		}
		bool operator !=(const TokenString& str) const
		{
			if (m_Length == str.m_Length) return false;
			return strncmp(m_Data, str.m_Data, m_Length) != 0;
		}
		bool operator ==(const char* str) const
		{
			int len = (int)strlen(str);
			if (m_Length != len) return false;
			return strncmp(m_Data, str, m_Length) == 0;
		}
		bool operator !=(const char* str) const
		{
			int len = (int)strlen(str);
			if (m_Length == len) return false;
			return strncmp(m_Data, str, m_Length) != 0;
		}
		bool operator ==(const std::string& str) const
		{
			if (m_Length != str.length()) return false;
			return strncmp(m_Data, str.c_str(), m_Length) == 0;
		}
		bool operator !=(const std::string& str) const
		{
			if (m_Length == str.length()) return false;
			return strncmp(m_Data, str.c_str(), m_Length) != 0;
		}
		bool operator<(const TokenString& str) const
		{
			int r = strncmp(m_Data, str.m_Data, __min(m_Length, str.m_Length));
			if (r == 0)
			{
				return m_Length < str.m_Length;
			}
			return r;
		}
	};

	struct Token
	{
		TokenType		m_Type;
		TokenString		m_Text;
		uint32_t		m_Line;

		Token()
		{

		}

		Token( TokenType m_Type, const TokenString& txt )
		{
			this->m_Type = m_Type;
			this->m_Text = txt;
			this->m_Line = 0;
		}

		std::string GetText() const
		{
			return m_Text;
		}
	};
}