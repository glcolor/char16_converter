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

	//�ı�����
	enum Encoding
	{
		//δ֪���룬����BOM��ȷ��
		EncodingUnknown = 0,
		//ASCII����
		EncodingASCII,
		//UTF-8����
		EncodingUTF8,
		//Little-Endian�ֽ�˳���UTF-16����
		EncodingUTF16,
		//Big-Endian�ֽ�˳���UTF-16����
		EncodingUTF16BE,
	};

	//���ֽ��ַ���ת˫�ֽ��ַ���
	std::wstring StringToWString(const char*str,size_t len, UINT cp);

	//˫�ֽ��ַ���ת���ֽ��ַ���
	std::string WStringToString(const wchar_t*wstr, size_t len, UINT cp);

	//ansi�ַ���utf-8�ַ���
	std::string AnsiToUTF8(const char* str);

	//ͨ�����bom������ַ����뼯
	Encoding CheckBom(const unsigned char * data, int len, int* bomSize);

	//�ʷ�������
	class Lexer
	{
	public:
		Lexer();
		~Lexer();

		//��ȡ�����ļ�������
		const std::string&	GetFileName() const { return m_FileName; }

		//��ָ��Դ������дʷ�����
		bool		Tokenize(const std::string& fileName);

		//��ȡ��һ��Token
		Token		Next(bool skipWhiteSpace=true);

		//��ȡ��һ��Token
		void		Next(Token& t,bool skipWhiteSpace = true);
	protected:
		//��ȡ��һ��Token������
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
