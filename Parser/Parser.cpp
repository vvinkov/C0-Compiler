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
		Token zadnji = citaj();
		switch(zadnji.getTip())
		{
			case IF:
			{
				procitaj(OOTV);
				AST* uvjet = parseExpression();
				procitaj(OZATV);
				AST* tijeloIf = parseStatement();
			
				if (sljedeci(ELSE))
				{
					procitaj(ELSE);
					AST* tijeloElse = parseStatement();
					return new IfElse({ uvjet, tijeloIf, tijeloElse });
				}
				else
					return new If({ uvjet, tijeloIf });
			}

			case WHILE:
			{
				procitaj(OOTV);
				AST* uvjet = parseExpression();
				procitaj(OZATV);
				AST* tijelo = parseStatement();
				return new While({uvjet, tijelo});
			}

			case FOR:
			{
				procitaj(OOTV);
				AST* deklaracija;
				AST* uvjet;
				AST* inkrement;
				if(!sljedeci(TZAREZ))
					deklaracija = parseSimple();

				procitaj(TZAREZ);
				if(!sljedeci(TZAREZ))
					uvjet = parseExpression();

				procitaj(TZAREZ);
				if(!sljedeci(TZAREZ))
					uvjet = parseSimple();

				procitaj(OZATV);
				AST* tijelo = parseStatement();
				return new For({deklaracija, uvjet, inkrement, tijelo});
			}

			case RETURN:
			{
				AST* povratnaVrijednost;
				if(!sljedeci(TZAREZ))
					povratnaVrijednost = parseExpression();

				procitaj(TZAREZ);
				return new Return({povratnaVrijednost});
			}

			case BREAK:
			{
				procitaj(TZAREZ);
				return new Break;
			}

			case CONTINUE:
			{
				procitaj(TZAREZ);
				return new Continue;
			}

			case ASSERT:
			{
				procitaj(OOTV);
				AST* izraz = parseExpression();
				procitaj(OZATV);
				procitaj(TZAREZ);
				return new Assert({izraz});
			}

			case ERROR:
			{
				procitaj(OOTV);
				AST* izraz = parseExpression();
				procitaj(OZATV);
				procitaj(TZAREZ);
				return new Error({izraz});
			}

			case VOTV:
			{
				ASTList* blok = new ASTList;
				while(!sljedeci(VZATV))
					blok->push_back(parseStatement());

				procitaj(VZATV);
				return blok;
			}

			default:
				AST* simple = parseSimple();
				procitaj(TZAREZ);
				return simple;
		}
	}

	AST* Parser::parseSimple()
	{
		Token zadnji = citaj();
		switch (zadnji.getTip())
		{
			case INT:
			case BOOL:
			case STRING:
			case CHAR:
			case POINTER:
			case ARRAY:
			{
				Leaf* tip = new Leaf(zadnji);
				Leaf* ime = new Leaf(procitaj(IDENTIFIER));
				AST* varijabla = new Varijabla({tip, ime});
				if (sljedeci(ASSIGN))
				{
					procitaj(ASSIGN);
					AST* desno = parseExpression();
					return new DeklaracijaVarijable({varijabla, desno});
				}
				else
					return varijabla;
			}
			default:
				return parseExpression();
		}
	}

	AST* Parser::parseExpression()
	{
		AST* trenutni = parseLogicki();

		while (true)
		{
			if (citaj().getTip() == CONDQ)
			{
				AST* caseTrue = parseExpression();
				procitaj(DTOCKA);
				AST* caseFalse = parseExpression();
				trenutni = new TernarniOperator({trenutni, caseTrue, caseFalse});
			}
			else
			{
				vrati();
				return trenutni;
			}
		}
	}

	AST* Parser::parseLogicki()
	{
		AST* trenutni = parseBitwise();
		while (true)
		{
			Token& zadnji = citaj();
			switch (zadnji.getTip())
			{
				case LAND:
				case LOR:
					Leaf* operacija = new Leaf(zadnji);
					trenutni = new LogickiOperator({trenutni, parseBitwise(), operacija});
				
				default:
					vrati();
					return trenutni;
			}
		}
	}

	AST* Parser::parseBitwise()
	{
		AST* trenutni = parseEquality();
		while (true)
		{
			Token& zadnji = citaj();
			switch (zadnji.getTip())
			{
				case BITAND:
				case BITXOR:
				case BITOR:
					Leaf* operacija = new Leaf(zadnji);
					trenutni = new BitwiseOperator({ trenutni, parseEquality(), operacija });

				default:
					vrati();
					return trenutni;
			}
		}
	}

	AST* Parser::parseEquality()
	{
		AST* trenutni = parseComparison();
		while (true)
		{
			Token& zadnji = citaj();
			switch (zadnji.getTip())
			{
				case EQ:
				case NEQ:
					Leaf* operacija = new Leaf(zadnji);
					trenutni = new OperatorJednakost({ trenutni, parseComparison(), operacija });

				default:
					vrati();
					return trenutni;
			}
		}
	}

	AST* Parser::parseComparison()
	{
		AST* trenutni = parseShifts();
		while (true)
		{
			Token& zadnji = citaj();
			switch (zadnji.getTip())
			{
				case LESS:
				case LESSEQ:
				case GRT:
				case GRTEQ:
					Leaf* operacija = new Leaf(zadnji);
					trenutni = new OperatorUsporedbe({ trenutni, parseShifts(), operacija });

				default:
					vrati();
					return trenutni;
			}
		}
	}

	AST* Parser::parseShifts()
	{

	}
}