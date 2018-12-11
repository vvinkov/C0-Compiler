#include "Token.hpp"

namespace C0Compiler
{
	std::ostream& operator<<(std::ostream& out, Token const& t)
	{
		return out << t.m_tip << "[(" << t.m_redak << ',' << t.m_stupac << ") " << t.m_sadrzaj << ']';
	}
}