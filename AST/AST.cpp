#include "AST.hpp"

namespace C0Compiler
{
	////////////////////////////////////////////////////////////////////////////////////
	// AST

	AST::AST(std::list<AST*>&& djeca) : m_djeca(std::move(djeca)) 
	{
		for (std::list<AST*>::iterator it = djeca.begin(); it != djeca.end(); ++it)
			(*it)->setRoditelj(this);
	}

	void AST::dodajDijete(AST* dijete)
	{
		m_djeca.push_back(dijete);
		m_djeca.back()->setRoditelj(this);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Program - placeholder

	////////////////////////////////////////////////////////////////////////////////////
	// Use direktiva - placeholder

	////////////////////////////////////////////////////////////////////////////////////
	// Varijabla

	Varijabla::Varijabla(std::initializer_list<AST*> parametri)
	{
		std::initializer_list<AST*>::iterator it = parametri.begin();
		
		// užasno, znam.
		m_tip = static_cast<Token>(*dynamic_cast<Leaf const*>(*it++)).getSadrzaj();
		m_ime = static_cast<Token>(*dynamic_cast<Leaf const*>(*it++)).getSadrzaj();

		for (; it != parametri.end(); ++it)
		{

		}
	}
}