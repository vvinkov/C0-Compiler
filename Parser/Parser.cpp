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

		delete zadnji;
		zadnji = nullptr;
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
		Program prog;

		// prvo parsiraj use direktive
		while (sljedeci(USE))
			prog.dodajDijete(parseUse());	

		// nakon toga stižu funkcije, koje su pretty much sve ostalo
		while(!citaj().isOfType(KRAJ))
			parseFunction();	
	}

	AST* Parser::parseUse()
	{
		citaj();	// USE
		citaj();	// STRLIT, ako je sve u redu
		if (zadnji->isOfType(STRLIT))
		{
			Leaf* datoteka = new Leaf(*zadnji);
			return new UseDirektiva({ datoteka });
		}
		else
		{
			sintaksnaGreska("Ocekujem " + tokenString[STRLIT] +
							", dobio " + tokenString[zadnji->getTip()] +
							": " + zadnji->getSadrzaj());
		}
	}

	AST* Parser::parseFunction()
	{
		// 21.12.2018. možda bi bilo dobro dodati pointere i arraye kao povratne vrijednosti
		if (zadnji->isOfType(INT)
			|| zadnji->isOfType(BOOL)
			|| zadnji->isOfType(STRING)
			|| zadnji->isOfType(CHAR)
			|| zadnji->isOfType(VOID))
		{
			// ako si pročitao INT, BOOL, STRING, CHAR ili VOID, 
			// imaš legalnu povratnu vrijednost funkcije
			ASTList* argumenti = new ASTList;

			Leaf* povratniTip = new Leaf(*zadnji);
			Leaf* ime = new Leaf(procitaj(IDENTIFIER));

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
					sintaksnaGreska("Nepoznat tip " + tokenString[zadnji->getTip()] +
									", ocekujem " + tokenString[INT] +
									", " + tokenString[BOOL] +
									", " + tokenString[STRING] +
									", ili " + tokenString[CHAR]);
				}
				// else zapamti tip i pročitaj ime varijable
				Leaf* tip = new Leaf(*zadnji);
				Leaf* ime = new Leaf(procitaj(IDENTIFIER));
				
				if (!sljedeci(OZATV))
					procitaj(ZAREZ);

				argumenti->push_back(new Varijabla({ tip, ime }));
			}

			if(sljedeci(TZAREZ))
			{
				// ako je poslije deklaracije funkcije točkazarez, onda još nema defniciju.
				// pročitaj ga i pusti deklaraciju funkcije da radi svoje
				procitaj(TZAREZ);
				return new DeklaracijaFunkcije({ povratniTip, ime, argumenti });
			}
			// else, funkcija ima i tijelo - idemo ga parsirati
			procitaj(VOTV);
			ASTList* tijelo = new ASTList;
			while (!sljedeci(VZATV))
				tijelo->push_back(parseStatement());

			return new DefinicijaFunkcije({ povratniTip, ime, argumenti, tijelo });
		}
		else
		{
			sintaksnaGreska("Nepoznat tip " + tokenString[zadnji->getTip()] +
							", ocekujem " + tokenString[INT] +
							", " + tokenString[BOOL] +
							", " + tokenString[STRING] +
							", " + tokenString[CHAR] +
							", ili " + tokenString[VOID]);
		}
	}

	AST* Parser::parseStatement()
	{

	}

}