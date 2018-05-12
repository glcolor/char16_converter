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
#include "Token.h"
#include <map>
#include <vector>

namespace jx
{
	using namespace std;

	//文本编码
	enum Encoding
	{
		//未知编码，根据BOM来确定
		EncodingUnknown = 0,
		//ASCII编码
		EncodingASCII,
		//UTF-8编码
		EncodingUTF8,
		//Little-Endian字节顺序的UTF-16编码
		EncodingUTF16,
		//Big-Endian字节顺序的UTF-16编码
		EncodingUTF16BE,
	};

	//单字节字符串转双字节字符串
	std::wstring StringToWString(const char*str,size_t len, UINT cp);

	//双字节字符串转单字节字符串
	std::string WStringToString(const wchar_t*wstr, size_t len, UINT cp);

	//ansi字符串utf-8字符串
	std::string AnsiToUTF8(const char* str);

	//通过检查bom来检测字符编码集
	Encoding CheckBom(const unsigned char * data, int len, int* bomSize);

	//词法分析器
	class Lexer
	{
	public:
		Lexer();
		~Lexer();

		//获取代码文件的名字
		const std::string&	GetFileName() const { return m_FileName; }

		//对指定源代码进行词法分析
		bool		Tokenize(const std::string& fileName);

		//获取下一个Token
		Token		Next(bool skipWhiteSpace=true);

		//获取下一个Token
		void		Next(Token& t,bool skipWhiteSpace = true);
	protected:
		//获取下一个Token的数据
		TokenType	GetToken(const char *source, size_t sourceLength, size_t *tokenLength) const;
		void		ParseToken(const char *source, size_t sourceLength, size_t &tokenLength, TokenType &tokenType) const;
		bool		IsWhiteSpace(const char *source, size_t sourceLength, size_t &tokenLength, TokenType &tokenType) const;
		bool		IsComment(const char *source, size_t sourceLength, size_t &tokenLength, TokenType &tokenType) const;
		bool		IsConstant(const char *source, size_t sourceLength, size_t &tokenLength, TokenType &tokenType) const;
		bool		IsKeyWord(const char *source, size_t sourceLength, size_t &tokenLength, TokenType &tokenType) const;
		bool		IsIdentifier(const char *source, size_t sourceLength, size_t &tokenLength, TokenType &tokenType) const;
	protected:
		string		m_FileName;
		char*		m_Source;
		uint32_t	m_Length;
		uint32_t	m_Offset;
		uint32_t	m_Line = 0;
	};
};
