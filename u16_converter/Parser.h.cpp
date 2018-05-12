#pragma once
#include "Lexer.h"

namespace Jx
{
	class Parser
	{
	public:
		Parser();
		~Parser();

		bool	Parse(const string& fileName,const string& prefix);
		//-------------------------------------------------------------------------
	private:
		string	m_Prefix;
		Lexer	m_Lexer;
	};
};
