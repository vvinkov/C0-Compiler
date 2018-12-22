#include "Parser.hpp"

namespace C0Compiler
{
	Parser::Parser(std::deque<Token*>&& tokeni) : m_tokeni(tokeni) 
	{
		vracanje_ok = false;
	}

	void Parser::pocisti()
	{
		while (!m_tokeni.empty())
		{
			delete m_tokeni.front();
			m_tokeni.front() = nullptr;
			m_tokeni.pop_front();
		}

		while (!m_ASTs.empty())
		{
			delete m_ASTs.front();
			m_ASTs.front() = nullptr;
			m_ASTs.pop_front();
		}

		while (!buffer.empty())
		{
			delete buffer.front();
			buffer.front() = nullptr;
			buffer.pop_front();
		}

		delete zadnji;
		zadnji = nullptr;
	}

	void Parser::dodajGranu(Token* token) 
	{
		buffer.push_back(token);
	}

	void Parser::dodajGranu(Token& token)
	{
		buffer.push_back(&token);
	}

	AST const& Parser::sASTavi(ASTtip tip)
	{
		switch (tip)
		{
			case USE_DIREKTIVA:
				UseDirektiva* novi = new UseDirektiva(buffer);
				m_ASTs.push_back(novi);
				return *novi;

			case FUNKCIJA:
				Funkcija* novi = new Funkcija(buffer);
				m_ASTs.push_back(novi);
				break;

			default:
				std::cerr << "Greska! Nepoznat tip AST-a!" << std::endl;
		}
		buffer.clear();
	}

	Token& Parser::citaj()
	{
		zadnji = m_tokeni.front();
		m_tokeni.pop_front();
		return *zadnji;
	}

	Token& Parser::procitaj(TokenTip tip)
	{
		if (m_tokeni.front()->isOfType(tip))
			return citaj();

		else
			sintaksnaGreska("Ocekujem " + tokenString[tip] + ", dobio " + tokenString[m_tokeni.front()->getTip()] + 
				": " + m_tokeni.front()->getSadrzaj());
	}

	bool Parser::sljedeci(TokenTip tip)
	{
		return m_tokeni.front()->getTip() == tip;
	}

	void Parser::vrati()
	{
		if (vracanje_ok)
		{
			vracanje_ok = false;
			m_tokeni.push_front(zadnji);
			zadnji = nullptr;
		}
		else
		{
			std::cerr << "Greska! Nije moguce vratiti se vise od jednog tokena unazad! " << std::endl;
			exit(1);
		}
	}

	void Parser::sintaksnaGreska(std::string const& opis)
	{
		std::cerr << "Sintaksna greska! Redak " << zadnji->getRedak() << ", stupac " << zadnji->getStupac() << ". Opis: " << opis << std::endl;
		pocisti();
		std::cin.get();
		exit(1);
	}

	void Parser::parsiraj()
	{
		// prvo parsiraj use direktive
		while (sljedeci(USE))
			parsirajUse();	

		// nakon toga stižu funkcije, koje su pretty much sve ostalo
		while(!citaj().isOfType(KRAJ))
			parsirajFunkcija();	
	}

	AST const& Parser::parsirajUse()
	{
		citaj();
		if (zadnji->isOfType(STRLIT))
		{
			return sASTavi(USE_DIREKTIVA);
		}
		else
		{
			sintaksnaGreska("Ocekujem " + tokenString[STRLIT] +
							", dobio " + tokenString[zadnji->getTip()] +
							": " + zadnji->getSadrzaj());
		}
	}

	AST const& Parser::parsirajFunkcija()
	{
		// 21.12.2018. možda bi bilo dobro dodati pointere i arraye kao povratne vrijednosti
		if (zadnji->isOfType(INT)
			|| zadnji->isOfType(BOOL)
			|| zadnji->isOfType(STRING)
			|| zadnji->isOfType(CHAR)
			|| zadnji->isOfType(VOID))
		{
			dodajGranu(zadnji);
			// ako si pročitao INT, BOOL, STRING, CHAR ili VOID, 
			// imaš legalnu povratnu vrijednost funkcije
			
			dodajGranu(procitaj(IDENTIFIER));
			procitaj(OOTV);
			while (!sljedeci(OZATV))
			{
				citaj();
				// 21.12.2018. možda bi bilo dobro dodati pointere i arraye kao parametre
				if (!(zadnji->isOfType(INT)
					|| zadnji->isOfType(BOOL)
					|| zadnji->isOfType(STRING)
					|| zadnji->isOfType(CHAR)))
				{
					sintaksnaGreska("Nepoznat tip " + zadnji->getTip +
									", ocekujem " + tokenString[INT] +
									", " + tokenString[BOOL] +
									", " + tokenString[STRING] +
									", ili " + tokenString[CHAR]);
				}
				// else...
				// zapamti tip i pročitaj ime varijable
				dodajGranu(zadnji);
				dodajGranu(procitaj(IDENTIFIER));
				if (!sljedeci(OZATV))
					procitaj(ZAREZ);// OVDJE SMO STALI
			}
		}
	}

}