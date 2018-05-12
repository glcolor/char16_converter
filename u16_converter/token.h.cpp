#pragma once
#include <string>
#include <algorithm>

namespace Jx
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

		Assign,		// =

		Dot,		// .

		Minus,		// -
		Plus,		// +
		Asterisk,	// *
		Slash,		// /
		Modulo,		// %
		Or,			// ||
		And,		// &&
		BOr,		// |
		BAnd,		// &
		Xor,		// ^
		BNot,		// ~
		LeftShift,	// <<
		RightShift,	// >>


		AddAssign,			// +=
		SubtractAssign,		// -=
		MultiplyAssign,		// *=
		DivideAssign,		// /=
		ModAssign,			// %=
		AndAssign,			// &=
		OrAssign,			// |=
		XorAssign,			// ^=
		LeftShiftAssign,	// <<=
		RightShiftAssign,	// >>=

		NotEqual,			// !=
		Equals,				// ==

		LessThan,			// <
		GreaterThan,		// >
		LessThanEqual,		// <=
		GreaterThanEqual,	// >=

		RightParen,			// )
		LeftParen,			// (

		LeftBrace,			// {
		RightBrace,			// }

		LeftBracket,		// [
		RightBracket,		// ]

		While,		// while
		If,			// if
		ElseIf,		// elseif
		Else,		// else

		Colon,		//:
		Semicolon,	//;
		Comma,		//,
		Ellipses,	//...

		Null,		// null

		Function,	// function
		For,		// for
		ForEach,	// foreach
		In,			// in
		Local,		// var 或 local
		Global,		// global
		Break,		// break
		Continue,	// continue
		Yield,		// yield
		Resume,		// resume

		Enum,		// enum
		Class,		// class
		New,		// new
		Base,		// base

		Const,		// const

		Compare,	// <>

		Ret,		// return

		Increment,	// ++
		Decrement,	// --

		Operator,	// operator
		Import,		// import

		LineComment,
		MultilineComment,

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