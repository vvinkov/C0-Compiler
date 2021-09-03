#include "Token.hpp"
#include "../AST/AST.hpp"

namespace C0Compiler
{
	// iako je one-liner, ovo je prebačeno u .cpp da izbjegnem cirkularni dependency Token.hpp->AST.hpp->Token.hpp->...
	Token::operator Leaf() const { return Leaf(*this); }

	std::ostream& operator<<(std::ostream& out, Token const& t)
	{
		out << tokenString[t.m_tip];
		if(t.m_tip != POCETAK &&
			t.m_tip != KRAJ &&
			t.m_tip != PRAZNO)
			out << "('" << t.m_sadrzaj << "')";

		return out;
	}
}