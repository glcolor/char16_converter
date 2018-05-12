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

#include "stdafx.h"
#include "Lexer.h"
#include <fstream>

namespace jx
{
	//标识符定义的结构
	struct TokenDefine
	{
		const char* m_Text;
		size_t      m_Length;
		TokenType	m_TokenType;
	};

#define DefineToken(str, tok) {str, sizeof(str)-1, tok}

	//标识符定义集
	TokenDefine const g_TokenDefines[] =
	{
		DefineToken("=", TokenType::Assign),
	};

	//总标识符数量
	const unsigned int g_TokenCount = sizeof(g_TokenDefines) / sizeof(TokenDefine);

	//空格的定义
	const char * const g_WhiteSpace = " \t\r\n";

	//标识符的定义数据，全局只需要一个实例就可以了
	struct TokenData
	{
		const TokenDefine **m_KeywordTable[256];
		
		TokenData()
		{
			memset(m_KeywordTable, 0, sizeof(m_KeywordTable));

			// 初始化跳转表
			for (uint32_t n = 0; n < g_TokenCount; n++)
			{
				const TokenDefine& current = g_TokenDefines[n];
				unsigned char start = current.m_Text[0];

				// 如果跳转表项不存在，则创建新项
				if (!m_KeywordTable[start])
				{
					//假设有相同首字母的关键字最多不超过32个
					m_KeywordTable[start]= new const TokenDefine*[32];
					memset(m_KeywordTable[start], 0, sizeof(TokenDefine*) * 32);
				}

				// 按照从长到短的顺序添加token，以便更高效的匹配关键字
				const TokenDefine** tok = m_KeywordTable[start];
				unsigned insert = 0, index = 0;
				while (tok[index])
				{
					if (tok[index]->m_Length >= current.m_Length)
					{
						++insert;
					}
					++index;
				}

				while (index > insert)
				{
					tok[index]= tok[index - 1];
					--index;
				}

				tok[insert] = &current;
			}
		}

		~TokenData()
		{
			for (uint32_t n = 0; n < 256; n++)
			{
				if (m_KeywordTable[n])
				{
					delete [] m_KeywordTable[n];
				}
			}
		}
	};

	TokenData	g_Tokens;
	//================================================
	//从源代码中获取表示一个字符的UTF-8序列
	int DecodeUTF8(const char *encodedBuffer, unsigned int *outLength)
	{
		const unsigned char *buf = (const unsigned char*)encodedBuffer;

		int value = 0;
		int length = -1;
		unsigned char byte = buf[0];
		if ((byte & 0x80) == 0)
		{
			// 单字节
			if (outLength) { *outLength = 1; }
			return byte;
		}
		else if ((byte & 0xE0) == 0xC0)
		{
			//  两个或多个字节
			value = int(byte & 0x1F);
			length = 2;

			if (value < 2)
			{
				length = -1;
			}
		}
		else if ((byte & 0xF0) == 0xE0)
		{
			// 还有两个字节
			value = int(byte & 0x0F);
			length = 3;
		}
		else if ((byte & 0xF8) == 0xF0)
		{
			// 还有三个字节
			value = int(byte & 0x07);
			length = 4;
		}

		int n = 1;
		for (; n < length; n++)
		{
			byte = buf[n];
			if ((byte & 0xC0) == 0x80)
			{
				value = (value << 6) + int(byte & 0x3F);
			}
			else
			{
				break;
			}
		}

		if (n == length)
		{
			if (outLength) { *outLength = (unsigned)length; }
			return value;
		}

		//  不符合规范的UTF-8序列
		return -1;
	}

	//单字节字符串转双字节字符串
	std::wstring StringToWString(const char*str,size_t strLen, UINT cp)
	{
		int len = MultiByteToWideChar(cp, 0, str, strLen, 0, 0);
		if (!len)	return L"";

		std::vector<wchar_t> abuff(len + 1);
		if (!MultiByteToWideChar(cp, 0, str, strLen, &abuff[0], len))
		{
			return L"";
		}
		abuff[len] = 0;
		return &abuff[0];
	}

	//双字节字符串转单字节字符串
	std::string WStringToString(const wchar_t*wstr,size_t strLen, UINT cp)
	{
		int len = WideCharToMultiByte(cp, 0, wstr, strLen, 0, 0, 0, 0);
		if (!len)	return "";

		std::vector<char> abuff(len + 1);
		if (!WideCharToMultiByte(cp, 0, wstr, strLen, &abuff[0], len, 0, 0))
		{
			return "";
		}
		abuff[len] = 0;
		return &abuff[0];
	}

	std::string AnsiToUTF8(const char* str)
	{
		size_t len = strlen(str);
		std::wstring utf16 = StringToWString(str,len, CP_ACP);
		if (utf16.empty()) return "";
		std::string utf8 = WStringToString(utf16.c_str(),utf16.length(), CP_UTF8);
		return utf8;
	}

	Encoding CheckBom(const unsigned char * data, int len, int* bomSize)
	{
		if (bomSize) *bomSize = 0;
		if (len < 2) return Encoding::EncodingUnknown;
		if (data[0] == 0xFF && data[1] == 0xFE)
		{
			if (bomSize) *bomSize = 2;
			return Encoding::EncodingUTF16;
		}
		else if (data[0] == 0xFE && data[1] == 0xFF)
		{
			if (bomSize) *bomSize = 2;
			return Encoding::EncodingUTF16BE;
		}
		else
		{
			if (len >= 3)
			{
				if (data[0] == 0xEF && data[1] == 0xBB && data[2] == 0xBF)
				{
					if (bomSize) *bomSize = 3;
					return Encoding::EncodingUTF8;
				}
			}
		}
		return Encoding::EncodingUnknown;
	}
	//==============================================================================================================================
	Lexer::Lexer()
	{
		m_Source = nullptr;
		m_Offset = 0;
		m_Length = 0;
		m_Line = 1;
	}

	Lexer::~Lexer()
	{
	}

	bool Lexer::Tokenize(const std::string& fileName)
	{
		m_FileName = fileName;
		ifstream fs;
		fs.open(fileName, ios::binary | ios::in);
		if (!fs.is_open()) return false;
		fs.seekg(0, ios::end);
		size_t len = (size_t)fs.tellg();
		fs.seekg(0, ios::beg);
		m_Source = new char[len + 1];
		fs.read(m_Source, len);
		fs.close();
		m_Source[len] = 0;
		m_Length = len;

		m_Offset = 0;
		//检查是否有bom，如果没有bom，则作为ansi处理，需要转换为utf-8
		int bomSize = 0;
		Encoding enc = CheckBom((unsigned char*)m_Source, len, &bomSize);
		if (enc == Encoding::EncodingASCII || enc == Encoding::EncodingUnknown)
		{
			string str = AnsiToUTF8(m_Source + bomSize);
			delete[] m_Source;
			m_Source = _strdup(str.c_str());
			m_Length = (uint32_t)str.length();
		}
		else if (enc == Encoding::EncodingUTF16 || enc == Encoding::EncodingUTF16BE)
		{
			string str = WStringToString((wchar_t*)(m_Source + bomSize),(len-bomSize)/2,CP_UTF8);
			delete[] m_Source;
			m_Source = _strdup(str.c_str());
			m_Length = (uint32_t)str.length();
		}
		else
		{
			m_Offset = bomSize;
		}
		m_Line = 1;

		return true;
	}

	Token Lexer::Next(bool skipWhiteSpace)
	{
		Token t;
		Next(t,skipWhiteSpace);
		return t;
	}

	void Lexer::Next(Token & t, bool skipWhiteSpace)
	{
		// 解析一个新的Token
		if (skipWhiteSpace)
		{
			do
			{
				if (m_Offset >= m_Length)
				{
					t.m_Type = TokenType::EoF;
					t.m_Text = TokenString();
				}
				else
				{
					size_t len = 0;
					t.m_Type = GetToken(m_Source + m_Offset, m_Length - m_Offset, &len);
					t.m_Text = TokenString(m_Source + m_Offset, (int)len);
				}

				t.m_Line = m_Line;
				for (int i = 0; i < t.m_Text.m_Length; i++)
				{
					if (t.m_Text.m_Data[i] == '\n') ++m_Line;
				}
				m_Offset += t.m_Text.m_Length;
			} while (t.m_Type == TokenType::WhiteSpace || t.m_Type == TokenType::LineComment || t.m_Type == TokenType::MultilineComment);
		}
		else
		{
			if (m_Offset >= m_Length)
			{
				t.m_Type = TokenType::EoF;
				t.m_Text = TokenString();
			}
			else
			{
				size_t len = 0;
				t.m_Type = GetToken(m_Source + m_Offset, m_Length - m_Offset, &len);
				t.m_Text = TokenString(m_Source + m_Offset, (int)len);
				m_Offset += t.m_Text.m_Length;
			}

			t.m_Line = m_Line;
			for (int i = 0; i < t.m_Text.m_Length; i++)
			{
				if (t.m_Text.m_Data[i] == '\n') ++m_Line;
			}
		}
	}

	//检测一个字符是否是指定进制下的一个合法的值
	bool IsDigitInRadix(char ch, int radix)
	{
		if (ch >= '0' && ch <= '9') { return (ch - '0') < radix; }
		if (ch >= 'A' && ch <= 'Z') { return (ch - 'A' + 10) < radix; }
		if (ch >= 'a' && ch <= 'z') { return (ch - 'a' + 10) < radix; }
		return false;
	}

	TokenType Lexer::GetToken(const char *source, size_t sourceLength, size_t *tokenLength) const
	{
		TokenType tokenType;
		size_t     tlen;
		ParseToken(source, sourceLength, tlen, tokenType);
		if (tokenLength) { *tokenLength = tlen; }

		return tokenType;
	}

	void Lexer::ParseToken(const char *source, size_t sourceLength, size_t &tokenLength, TokenType &tokenType) const
	{
		if (IsWhiteSpace(source, sourceLength, tokenLength, tokenType))		{ return; }
		if (IsComment(source, sourceLength, tokenLength, tokenType))		{ return; }
		if (IsConstant(source, sourceLength, tokenLength, tokenType))		{ return; }
		if (IsIdentifier(source, sourceLength, tokenLength, tokenType))		{ return; }
		if (IsKeyWord(source, sourceLength, tokenLength, tokenType))		{ return; }

		//未知的标识符
		tokenType = TokenType::Unknown;
		tokenLength = 1;
	}

	bool Lexer::IsWhiteSpace(const char *source, size_t sourceLength, size_t &tokenLength, TokenType &tokenType) const
	{
		// 将UTF-8 BOM(EF BB BF) 视为空格
		if (sourceLength >= 3 &&
			uint8_t(source[0]) == 0xEFu && uint8_t(source[1]) == 0xBBu && uint8_t(source[2]) == 0xBFu)
		{
			tokenType = TokenType::WhiteSpace;
			tokenLength = 3;
			return true;
		}

		size_t n;
		int numWsChars = (int)strlen(g_WhiteSpace);
		for (n = 0; n < sourceLength; n++)
		{
			bool isWhiteSpace = false;
			for (int w = 0; w < numWsChars; w++)
			{
				if (source[n] == g_WhiteSpace[w])
				{
					isWhiteSpace = true;
					break;
				}
			}
			if (!isWhiteSpace) { break; }
		}

		if (n > 0)
		{
			tokenType = TokenType::WhiteSpace;
			tokenLength = n;
			return true;
		}

		return false;
	}

	bool Lexer::IsComment(const char *source, size_t sourceLength, size_t &tokenLength, TokenType &tokenType) const
	{
		int stackCount = 0;
		if (sourceLength < 2)
		{
			return false;
		}

		if (source[0] != '/')
		{
			return false;
		}

		if (source[1] == '/')
		{
			// 单行注释

			// 计算长度
			size_t n;
			for (n = 2; n < sourceLength; n++)
			{
				if (source[n] == '\n')
				{
					break;
				}
			}

			tokenType = TokenType::LineComment;
			tokenLength = n < sourceLength ? n + 1 : n;

			return true;
		}

		if (source[1] == '*')
		{
			// 多行注释
			++stackCount;
			// 计算长度
			size_t n;
			for (n = 2; n < sourceLength - 1; )
			{
				if (source[n] == '*' && source[n + 1] == '/')
				{
					--stackCount;
					if (stackCount == 0)
					{
						++n;
						break;
					}
				}
				else if (source[n] == '/' && source[n + 1] == '*')
				{
					++stackCount;
				}
				++n;
			}

			tokenType = TokenType::MultilineComment;
			tokenLength = n + 1;

			return true;
		}

		return false;
	}

	bool Lexer::IsConstant(const char *source, size_t sourceLength, size_t &tokenLength, TokenType &tokenType) const
	{
		if ((sourceLength > 1) && (source[0] == 'L' || source[0] == 'u') && (source[1] == '"' || source[1]=='\''))
		{
			//unicode字符或unicode字符串
			tokenType = (source[1] == '\'') ? TokenType::UnicodeChar : TokenType::UnicodeString;
			char quote = source[1];
			bool evenSlashes = true;
			size_t n;
			for (n = 2; n < sourceLength; n++)
			{
				if (source[n] == quote && evenSlashes)
				{
					tokenLength = n + 1;
					return true;
				}
				if (source[n] == '\\') { evenSlashes = !evenSlashes; }
				else { evenSlashes = true; }
			}

			tokenType = TokenType::NonTerminatedString;
			tokenLength = n;
		}

		// 以数字开头
		if ((source[0] >= '0' && source[0] <= '9') || (source[0] == '.' && sourceLength > 1 && source[1] >= '0' && source[1] <= '9'))
		{
			// 是否是特殊进制的数据？
			if (source[0] == '0' && sourceLength > 1)
			{
				// 确定该数值的进制，当前支持二进制、八进制、十进制和十六进制
				int radix = 0;
				switch (source[1])
				{
				case 'b': case 'B': radix = 2; break;
				case 'o': case 'O': radix = 8; break;
				case 'd': case 'D': radix = 10; break;
				case 'x': case 'X': radix = 16; break;
				}

				if (radix)
				{
					size_t n;
					for (n = 2; n < sourceLength; n++)
					{
						if (!IsDigitInRadix(source[n], radix))
						{
							break;
						}
					}
					tokenType = TokenType::BitsNumber;
					tokenLength = n;
					return true;
				}
			}

			size_t n;
			for (n = 0; n < sourceLength; n++)
			{
				if (source[n] < '0' || source[n] > '9')
				{
					break;
				}
			}

			if (n < sourceLength)
			{
				if ((source[n] == '.' || source[n] == 'e' || source[n] == 'E'))
				{
					if (source[n] == '.')
					{
						n++;
						for (; n < sourceLength; n++)
						{
							if (source[n] < '0' || source[n] > '9')
							{
								break;
							}
						}
					}

					if (n < sourceLength && (source[n] == 'e' || source[n] == 'E'))
					{
						n++;
						if (n < sourceLength && (source[n] == '-' || source[n] == '+'))
						{
							n++;
						}

						for (; n < sourceLength; n++)
						{
							if (source[n] < '0' || source[n] > '9')
							{
								break;
							}
						}
					}

					if (n < sourceLength && (source[n] == 'f' || source[n] == 'F'))
					{
						tokenType = TokenType::RealNumber;
						tokenLength = n + 1;
					}
					else
					{
						tokenType = TokenType::RealNumber;
						tokenLength = n;
					}
					return true;
				}

				if (source[n] == 'L' || source[n] == 'l')
				{
					tokenType = TokenType::IntNumber;
					tokenLength = n + 1;
					return true;
				}
			}

			tokenType = TokenType::IntNumber;
			tokenLength = n;
			return true;
		}

		// 字符串常量由双引号或单引号包围
		if (source[0] == '"' || source[0] == '\'')
		{
			//如果是双单引号，表示块字符串
			if (sourceLength >= 4 && source[0] == '\'' && source[1] == '\'')
			{
				// 块字符串，计算其长度
				size_t n;
				for (n = 2; n < sourceLength - 1; n++)
				{
					if (source[n] == '\'' && source[n + 1] == '\'')
					{
						break;
					}
				}

				tokenType = TokenType::BlockString;
				tokenLength = n + 2;
			}
			else
			{
				// 普通字符串
				tokenType = (source[0] == '\'')?TokenType::Char:TokenType::String;
				char quote = source[0];
				bool evenSlashes = true;
				size_t n;
				for (n = 1; n < sourceLength; n++)
				{
					if (source[n] == quote && evenSlashes)
					{
						tokenLength = n + 1;
						return true;
					}
					if (source[n] == '\\') { evenSlashes = !evenSlashes; }
					else { evenSlashes = true; }
				}

				tokenType = TokenType::NonTerminatedString;
				tokenLength = n;
			}

			return true;
		}

		return false;
	}

	bool Lexer::IsIdentifier(const char *source, size_t sourceLength, size_t &tokenLength, TokenType &tokenType) const
	{
		tokenLength = 0;
		size_t offset = 0;
		unsigned int outLength = 0;
		int ch = DecodeUTF8(source, &outLength);
		// 标识符以字母或下划线开始
		if (iswalpha(ch) || ch == '_')
		{
			tokenType = TokenType::Identifier;
			tokenLength = outLength;
			offset += outLength;

			while (offset < sourceLength)
			{
				ch = DecodeUTF8(source + offset, &outLength);
				if (iswalnum(ch) || ch == '_')
				{
					offset += outLength;
					tokenLength += outLength;
				}
				else
				{
					break;
				}
			}

			// 检查是否是保留的关键字
			if (IsKeyWord(source, tokenLength, tokenLength, tokenType))
			{
				return false;
			}

			return true;
		}

		return false;
	}

	bool Lexer::IsKeyWord(const char *source, size_t sourceLength, size_t &tokenLength, TokenType &tokenType) const
	{
		unsigned char start = source[0];
		const TokenDefine **ptr = g_Tokens.m_KeywordTable[start];

		if (!ptr)
		{
			return false;
		}

		for (; *ptr; ++ptr)
		{
			size_t wlen = (*ptr)->m_Length;
			if (sourceLength >= wlen && strncmp(source, (*ptr)->m_Text, wlen) == 0)
			{
				if (wlen < sourceLength
					&&
					((source[wlen - 1] >= 'a' && source[wlen - 1] <= 'z') ||
					(source[wlen - 1] >= 'A' && source[wlen - 1] <= 'Z') ||
						(source[wlen - 1] >= '0' && source[wlen - 1] <= '9')
						)
					&&
					((source[wlen] >= 'a' && source[wlen] <= 'z') ||
					(source[wlen] >= 'A' && source[wlen] <= 'Z') ||
						(source[wlen] >= '0' && source[wlen] <= '9') ||
						(source[wlen] == '_')
						)
					)
				{
					continue;
				}

				tokenType = (*ptr)->m_TokenType;
				tokenLength = wlen;
				return true;
			}
		}

		return false;
	}
	//==============================================================================================================================
};

