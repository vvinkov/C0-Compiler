#include "Parser.hpp"
#include "../Greska/Greska.hpp"

namespace C0Compiler
{
	Parser::Parser(std::deque<std::shared_ptr<Token>>&& tokeni, std::shared_ptr<AST> korijen) :
		m_tokeni(tokeni), m_korijen(korijen)
	{
		m_vracanjeOk = false;
	}

	//void Parser::pocisti()
	//{
	//	while (!m_tokeni.empty())
	//	{
	//		delete m_tokeni.front();
	//		m_tokeni.front() = nullptr;
	//		m_tokeni.pop_front();
	//	}

	//	delete zadnji;
	//	zadnji = nullptr;
	//}

	Token& Parser::citaj()
	{
		//delete zadnji;
		//m_zadnji.reset();
		m_zadnji = m_tokeni.front();
		m_tokeni.pop_front();
		m_vracanjeOk = true;
		return *m_zadnji;
	}

	Token& Parser::procitaj(TokenTip tip)
	{
		if (m_tokeni.front()->OfType(tip))
			return citaj();

		else
			throw SintaksnaGreska(m_tokeni.front()->Redak(), m_tokeni.front()->Stupac(), *m_tokeni.front(), tip);
	}

	bool Parser::sljedeci(TokenTip tip)
	{
		return m_tokeni.front()->Tip() == tip;
	}

	void Parser::vrati()
	{
		if (m_vracanjeOk)
		{
			m_vracanjeOk = false;
			m_tokeni.push_front(m_zadnji);
		}
		else
		{
			throw Greska("Greška", m_tokeni.front()->Redak(), m_tokeni.front()->Stupac(), "buffer overflow");
		}
	}

	void Parser::parsiraj()
	{
		std::deque<std::shared_ptr<Token>> resultTokeni;
		
		// preskoči početak
		procitaj(POCETAK);

		// prvo parsiraj use direktive
		while (sljedeci(USE))
		{
			// spremi tokene iz druge datoteke u temp deque
			std::deque<std::shared_ptr<Token>> tempTokeni = std::move(parseUse());

			// i appendaj ih u result deque. želimo da zadrže isti poredak.
			for (std::deque<std::shared_ptr<Token>>::iterator it = tempTokeni.begin(); it != tempTokeni.end(); ++it)
				resultTokeni.push_back(*it);
		}

		// nakon svih use direktiva, njihove tokene prependamo u glavni deque obrnutim redom, opet, da zadrže poredak
		for (std::deque<std::shared_ptr<Token>>::reverse_iterator rit = resultTokeni.rbegin(); rit != resultTokeni.rend(); ++rit)
			m_tokeni.push_front(*rit);

		// nakon toga stižu globalne deklaracije i definicije. dodajemo ih
		// u korijen apstraktnog sintaksnog stabla (one se same granaju dalje)
		while(!citaj().OfType(KRAJ))
			m_korijen->dodajDijete(parseGlobal());	
	}

	std::deque<std::shared_ptr<Token>> Parser::parseUse()
	{
		citaj();			// USE
		procitaj(STRLIT);	// STRLIT, ako je sve u redu
		std::shared_ptr<Leaf> datoteka = std::make_shared<Leaf>(*m_zadnji);
		std::shared_ptr<UseDirektiva> useDir(new UseDirektiva(m_zadnji->Redak(), m_zadnji->Stupac(), { datoteka }));
		return useDir->izvrsi(*this);
	}

	std::shared_ptr<AST> Parser::parseGlobal()
	{	
		// pročitali smo token prije ulaska u funkciju pa imamo
		// odakle izvući redak i stupac
		int redak = m_zadnji->Redak();
		int stupac = m_zadnji->Stupac();

		switch (m_zadnji->Tip())
		{
			case STRUCT:
			{
				std::shared_ptr<Leaf> ime(new Leaf(procitaj(IDENTIFIER)));	// ime strukture

				//ako je sljedeći točkazarez, još nemaš definiciju strukture, nego samo deklaraciju.
				if (sljedeci(TZAREZ))
				{
					return std::shared_ptr<DeklaracijaStrukture>(new DeklaracijaStrukture(redak, stupac, { ime }));
				}
				else
				{
					// ako ne slijedi točkazarez, onda mora slijediti otvorena vitičasta zagrada
					procitaj(VOTV);

					std::shared_ptr<ASTList> fields(new ASTList(redak, stupac));
					while (!sljedeci(VZATV))
					{
						// else zapamti tip i pročitaj ime varijable
						std::shared_ptr<Tip> tip = std::dynamic_pointer_cast<Tip>(parseType());
						std::shared_ptr<Leaf> ime(new Leaf(procitaj(IDENTIFIER)));

						if (!sljedeci(VZATV))
							procitaj(TZAREZ);

						// LLVM ne dozvoljava "prave" deklaracije varijabli u strukturama, nego ga samo zanimaju njihovi tipovi.
						// zato u fields spremam tip i ime umjesto da konstruiram AST DeklaracijaVarijable
						fields->push_back(tip);
						fields->push_back(ime);

						return std::shared_ptr<AST>(new DefinicijaStrukture(m_zadnji->Redak(), m_zadnji->Stupac(), { ime, fields }));
					}

					procitaj(VZATV);
					procitaj(TZAREZ);
				}
				break;
			}

			case TYPEDEF:
			{
				std::shared_ptr<Tip> tip = std::dynamic_pointer_cast<Tip>(parseType());
				std::shared_ptr<Leaf> alias(new Leaf(procitaj(IDENTIFIER)));
				return std::shared_ptr<AST>(new TypeDef(redak, stupac, { tip, alias }));
			}

			// ako u globalnom dosegu imamo nešto što nije ni deklaracija/definicija
			// strukture ni typedef, onda to mora biti deklaracija/definicija funkcije.
			// dakle, sljedeći token je tip (povratni tip funkcije).
			default:
			{
				// ovdje moramo vratiti zato što parseType čita token; 
				// ne želimo preskočiti token koji predstavlja povratni tip funkcije
				vrati(); 
						 
				std::shared_ptr<ASTList> argumentiFunkcije(new ASTList(redak, stupac));
				std::shared_ptr<Tip> povratniTip = std::dynamic_pointer_cast<Tip>(parseType());
				//std::shared_ptr<Leaf> imeFunkcije(new Leaf(procitaj(IDENTIFIER)));
				std::shared_ptr<Leaf> imeFunkcije = std::make_shared<Leaf>(procitaj(IDENTIFIER));

				// nakon imena funkcije slijede njeni argumenti
				procitaj(OOTV);
				while (!sljedeci(OZATV))
				{
					// parsiramo tip i ime argumenta
					std::shared_ptr<Tip> tipArgumenta = std::dynamic_pointer_cast<Tip>(parseType());
					std::shared_ptr<Leaf> imeArgumenta(new Leaf(procitaj(IDENTIFIER)));

					// ako nije zadnji, sljedeći znak je zarez
					if (!sljedeci(OZATV))
						procitaj(ZAREZ);

					// LLVM ne dozvoljava "prave" deklaracije varijabli među argumentima funkcije, nego ga samo zanimaju njihovi tipovi.
					// zato u argumente spremam tipove i imena umjesto AST-ova DeklaracijaVarijable
					argumentiFunkcije->push_back(tipArgumenta);
					argumentiFunkcije->push_back(imeArgumenta);
				}
				procitaj(OZATV);

				if (sljedeci(TZAREZ))
				{
					// ako je poslije deklaracije funkcije točkazarez, onda još nema defniciju.
					// pročitaj ga i pusti deklaraciju funkcije da radi svoje
					procitaj(TZAREZ);
					return std::shared_ptr<AST>(new DeklaracijaFunkcije(redak, stupac, { povratniTip, imeFunkcije, argumentiFunkcije }));
				}

				// else, funkcija ima i tijelo - idemo ga parsirati
				procitaj(VOTV);
				std::shared_ptr<ASTList> tijelo(new ASTList(m_zadnji->Redak(), m_zadnji->Stupac()));
				while (!sljedeci(VZATV))
					tijelo->push_back(parseStatement());

				procitaj(VZATV);
				return std::shared_ptr<AST>(new DefinicijaFunkcije(redak, stupac, { povratniTip, imeFunkcije, argumentiFunkcije, tijelo }));
			}
		}
	}

	std::shared_ptr<AST> Parser::parseType()
	{
		citaj();
		int redak = m_zadnji->Redak();
		int stupac = m_zadnji->Stupac();
		switch (m_zadnji->Tip())
		{
			case STRUCT:
			{
				std::shared_ptr<Leaf> doslovnoStruct(new Leaf(*m_zadnji));	// jer doslovno piše "struct"
				std::shared_ptr<Leaf> imeStrukture(new Leaf(procitaj(IDENTIFIER)));
				return std::shared_ptr<AST>(new Tip(redak, stupac, { doslovnoStruct, imeStrukture }));
			}
			case INT:
			case BOOL:
			case STRING:
			case CHAR:
			case VOID:
			case INTPOINT:
			case BOOLPOINT:
			case STRINGPOINT:
			case CHARPOINT:
			case INTARRAY:
			case BOOLARRAY:
			case STRINGARRAY:
			case CHARARRAY:
			{
				std::shared_ptr<Leaf> doslovnoTajTip(new Leaf(*m_zadnji));	// jer u tokenu doslovno piše taj tip (int, bool, string...)
				return std::shared_ptr<Tip>(new Tip(redak, stupac, { doslovnoTajTip }));
			}
			default:
				throw SintaksnaGreska(m_zadnji->Redak(), m_zadnji->Stupac(), *m_zadnji, { INT, BOOL, STRING, CHAR, INTPOINT, BOOLPOINT, STRINGPOINT, CHARPOINT, INTARRAY, BOOLARRAY, STRINGARRAY, CHARARRAY, STRUCT });
		}
	}

	std::shared_ptr<AST> Parser::parseStatement()
	{
		citaj();
		int redak = m_zadnji->Redak();
		int stupac = m_zadnji->Stupac();
		switch(m_zadnji->Tip())
		{
			case IF:
			{
				procitaj(OOTV);
				std::shared_ptr<AST> uvjet = parseExpression();
				procitaj(OZATV);
				std::shared_ptr<AST> tijeloIf = parseStatement();
			
				if (sljedeci(ELSE))
				{
					procitaj(ELSE);
					std::shared_ptr<AST> tijeloElse = parseStatement();
					return std::shared_ptr<AST>(new IfElse(redak, stupac, { uvjet, tijeloIf, tijeloElse }));
				}
				else
					return std::shared_ptr<AST>(new IfElse(redak, stupac, { uvjet, tijeloIf, nullptr }));
			}

			case WHILE:
			{
				procitaj(OOTV);
				std::shared_ptr<AST> uvjet = parseExpression();
				procitaj(OZATV);
				std::shared_ptr<AST> tijelo = parseStatement();
				return std::shared_ptr<AST>(new While(redak, stupac, {uvjet, tijelo}));
			}

			case FOR:
			{
				procitaj(OOTV);
				std::shared_ptr<AST> deklaracija;
				std::shared_ptr<AST> uvjet;
				std::shared_ptr<AST> inkrement;
				if(!sljedeci(TZAREZ))
					deklaracija = parseSimple();

				procitaj(TZAREZ);
				if(!sljedeci(TZAREZ))
					uvjet = parseExpression();

				procitaj(TZAREZ);
				if(!sljedeci(TZAREZ))
					inkrement = parseSimple();

				procitaj(OZATV);
				std::shared_ptr<AST> tijelo = parseStatement();
				return std::shared_ptr<AST>(new For(redak, stupac, {deklaracija, uvjet, tijelo, inkrement}));
			}

			case RETURN:
			{
				std::shared_ptr<AST> povratnaVrijednost;
				if(!sljedeci(TZAREZ))
					povratnaVrijednost = parseExpression();

				procitaj(TZAREZ);
				return std::shared_ptr<AST>(new Return(redak, stupac, {povratnaVrijednost}));
			}

			case BREAK:
			{
				procitaj(TZAREZ);
				return std::shared_ptr<AST>(new Break(redak, stupac));
			}

			case CONTINUE:
			{
				procitaj(TZAREZ);
				return std::shared_ptr<AST>(new Continue(redak, stupac));
			}

			case ASSERT:
			{
				procitaj(OOTV);
				std::shared_ptr<AST> izraz = parseExpression();
				procitaj(OZATV);
				procitaj(TZAREZ);
				return std::shared_ptr<AST>(new Assert(redak, stupac, {izraz}));
			}

			case VOTV:
			{
				std::shared_ptr<ASTList> blok(new ASTList(redak, stupac));
				while(!sljedeci(VZATV))
					blok->push_back(parseStatement());

				procitaj(VZATV);
				return blok;
			}

			default:
				vrati();
				std::shared_ptr<AST> simple = parseSimple();
				procitaj(TZAREZ);
				return simple;
		}
	}

	std::shared_ptr<AST> Parser::parseSimple()
	{
		try
		{
			std::shared_ptr<Tip> tip = std::dynamic_pointer_cast<Tip>(parseType());
			std::shared_ptr<Leaf> ime(new Leaf(procitaj(IDENTIFIER)));
			if (sljedeci(ASSIGN))
			{
				procitaj(ASSIGN);
				std::shared_ptr<AST> desno = parseExpression();
				return std::shared_ptr<AST>(new DeklaracijaVarijable(tip->Redak(), tip->Stupac(), { tip, ime, desno }));
			}
			else
				return std::shared_ptr<AST>(new DeklaracijaVarijable(tip->Redak(), tip->Stupac(), { tip, ime }));
		}
		catch(SintaksnaGreska const& e) 
		{
			// ako ovdje parseType ne uspije, ne želimo da baci sintaksnu grešku, jer možda ovdje uopće
			// ne treba biti tip. ako ne uspijemo parsirati tip, probamo expression
			vrati();
			return parseExpression();
		}
	}

	std::shared_ptr<AST> Parser::parseExpression()
	{
		std::shared_ptr<AST> trenutni = parseLogical();

		while (true)
		{
			if (sljedeci(CONDQ))
			{
				procitaj(CONDQ);
				int redak = trenutni->Redak();
				int stupac = trenutni->Stupac();

				std::shared_ptr<AST> caseTrue = parseExpression();
				procitaj(DTOCKA);
				std::shared_ptr<AST> caseFalse = parseExpression();
				trenutni = std::shared_ptr<AST>(new TernarniOperator(redak, stupac, { trenutni, caseTrue, caseFalse }));
			}
			else
			{
				//vrati();
				return trenutni;
			}
		}
	}

	std::shared_ptr<AST> Parser::parseLogical()
	{
		std::shared_ptr<AST> trenutni = parseBitwise();
		while (true)
		{
			citaj();
			switch (m_zadnji->Tip())
			{
				case LAND:
				case LOR:
				{
					std::shared_ptr<Leaf> operacija(new Leaf(*m_zadnji));
					trenutni = std::shared_ptr<AST>(new LogickiOperator(operacija->Redak(), operacija->Stupac(), { trenutni, parseBitwise(), operacija }));
					break;
				}
				default:
				{
					vrati();
					return trenutni;
				}
			}
		}
	}

	std::shared_ptr<AST> Parser::parseBitwise()
	{
		std::shared_ptr<AST> trenutni = parseEquality();
		while (true)
		{
			citaj();
			switch (m_zadnji->Tip())
			{
				case BITAND:
				case BITXOR:
				case BITOR:
				{
					std::shared_ptr<Leaf> operacija(new Leaf(*m_zadnji));
					trenutni = std::shared_ptr<AST>(new BinarniOperator(operacija->Redak(), operacija->Stupac(), { trenutni, parseEquality(), operacija }));
					break;
				}
				default:
				{
					vrati();
					return trenutni;
				}
			}
		}
	}

	std::shared_ptr<AST> Parser::parseEquality()
	{
		std::shared_ptr<AST> trenutni = parseComparison();
		while (true)
		{
			citaj();
			switch (m_zadnji->Tip())
			{
				case EQ:
				case NEQ:
				{
					std::shared_ptr<Leaf> operacija(new Leaf(*m_zadnji));
					trenutni = std::shared_ptr<AST>(new OperatorJednakost(operacija->Redak(), operacija->Stupac(), { trenutni, parseComparison(), operacija }));
					break;
				}
				default:
				{
					vrati();
					return trenutni;
				}
			}
		}
	}

	std::shared_ptr<AST> Parser::parseComparison()
	{
		std::shared_ptr<AST> trenutni = parseShifts();
		while (true)
		{
			citaj();
			switch (m_zadnji->Tip())
			{
				case LESS:
				case LESSEQ:
				case GRT:
				case GRTEQ:
				{
					std::shared_ptr<Leaf> operacija(new Leaf(*m_zadnji));
					trenutni = std::shared_ptr<AST>(new OperatorUsporedbe(operacija->Redak(), operacija->Stupac(), { trenutni, parseShifts(), operacija }));
					break;
				}
				default:
				{
					vrati();
					return trenutni;
				}
			}
		}
	}

	std::shared_ptr<AST> Parser::parseShifts()
	{
		std::shared_ptr<AST> trenutni = parseAdd();
		while (true)
		{
			citaj();
			switch (m_zadnji->Tip())
			{
				case LSHIFT:
				case RSHIFT:
				{
					std::shared_ptr<Leaf> operacija(new Leaf(*m_zadnji));
					trenutni = std::shared_ptr<AST>(new BinarniOperator(operacija->Redak(), operacija->Stupac(), { trenutni, parseAdd(), operacija }));
					break;
				}
				default:
				{
					vrati();
					return trenutni;
				}
			}
		}
	}

	std::shared_ptr<AST> Parser::parseAdd()
	{
		std::shared_ptr<AST> trenutni = parseFactor();
		while (true)
		{
			citaj();
			switch (m_zadnji->Tip())
			{
				case PLUS:
				case MINUS:
				{
					std::shared_ptr<Leaf> operacija(new Leaf(*m_zadnji));
					trenutni = std::shared_ptr<AST>(new BinarniOperator(operacija->Redak(), operacija->Stupac(), { trenutni, parseFactor(), operacija }));
					break;
				}
				default:
				{
					vrati();
					return trenutni;
				}
			}
		}
	}

	std::shared_ptr<AST> Parser::parseFactor()
	{
		std::shared_ptr<AST> trenutni = parseAssign();

		while (true)
		{
			citaj();
			switch (m_zadnji->Tip())
			{
				case ZVJ:
				case SLASH:
				case MOD:
				{
					std::shared_ptr<Leaf> operacija(new Leaf(*m_zadnji));
					trenutni = std::shared_ptr<AST>(new BinarniOperator(operacija->Redak(), operacija->Stupac(), { trenutni, parseAssign(), operacija }));
					break;
				}
				default:
				{
					vrati();
					return trenutni;
				}
			}
		}
	}

	std::shared_ptr<AST> Parser::parseAssign()
	{
		std::shared_ptr<AST> trenutni = parseAllocate();

		while (true)
		{
			citaj();
			switch (m_zadnji->Tip())
			{
				case PLUSEQ:
				case MINUSEQ:
				case ZVJEQ:
				case SLASHEQ:
				case MODEQ:
				case LSHIFTEQ:
				case RSHIFTEQ:
				case ASSIGN:
				case BANDEQ:
				case BXOREQ:
				case BOREQ:
				{
					std::shared_ptr<Leaf> operacija(new Leaf(*m_zadnji));
					trenutni = std::shared_ptr<AST>(new OperatorPridruzivanja(operacija->Redak(), operacija->Stupac(), { trenutni, parseExpression(), operacija }));
					break;
				}
				default:
				{
					vrati();
					return trenutni;
				}
			}
		}
	}

	std::shared_ptr<AST> Parser::parseAllocate()
	{
		std::shared_ptr<AST> trenutni = parseAllocArray();
		int redak = trenutni->Redak();
		int stupac = trenutni->Stupac();
		if (sljedeci(ALLOC))
		{
			procitaj(ALLOC);
			procitaj(OOTV);
			std::shared_ptr<Tip> tip = std::dynamic_pointer_cast<Tip>(parseType());
			procitaj(OZATV);
			trenutni = std::shared_ptr<AST>(new Alokacija(redak, stupac, { tip }));
		}
		return trenutni;
	}

	std::shared_ptr<AST> Parser::parseAllocArray()
	{
		std::shared_ptr<AST> trenutni;
		if (sljedeci(ALLOCARRAY))
		{
			Token& temp = procitaj(ALLOCARRAY);
			int redak = temp.Redak();
			int stupac = temp.Stupac();
			procitaj(OOTV);
			std::shared_ptr<Tip> tip = std::dynamic_pointer_cast<Tip>(parseType());
			procitaj(ZAREZ);
			std::shared_ptr<AST> koliko = parseExpression();
			procitaj(OZATV);
			trenutni = std::shared_ptr<AST>(new AlokacijaArray(redak, stupac, { tip, koliko }));
		}
		else
			trenutni = parseArrow();

		return trenutni;
	}

	std::shared_ptr<AST> Parser::parseArrow()
	{
		std::shared_ptr<AST> trenutni = parseDot();
		while (true)
		{
			if (sljedeci(STRELICA))
			{
				procitaj(STRELICA);
				trenutni = std::shared_ptr<AST>(new Strelica(m_zadnji->Redak(), m_zadnji->Stupac(), { trenutni, parseDot() }));
			}
			else
				return trenutni;
		}
	}

	std::shared_ptr<AST> Parser::parseDot()
	{
		std::shared_ptr<AST> trenutni = parseUnary();
		while (true)
		{
			if (sljedeci(TOCKA))
			{
				procitaj(TOCKA);
				trenutni = std::shared_ptr<AST>(new Tocka(m_zadnji->Redak(), m_zadnji->Stupac(), { trenutni, parseBase() }));
			}
			else
				return trenutni;
		}
	}

	std::shared_ptr<AST> Parser::parseUnary()
	{
		citaj();
		int redak = m_zadnji->Redak();
		int stupac = m_zadnji->Stupac();
		std::shared_ptr<AST> iza;	// iza operatora, naravno
		switch (m_zadnji->Tip())
		{
			case USKL:
			{
				iza = parseExpression();
				return std::shared_ptr<AST>(new Negacija(redak, stupac, { iza }));
			}
			case TILDA:
			{
				iza = parseExpression();
				return std::shared_ptr<AST>(new Tilda(redak, stupac, { iza }));
			}
			case MINUS:
			{
				iza = parseExpression();
				return std::shared_ptr<AST>(new Minus(redak, stupac, { iza }));
			}
			case ZVJ:
			{
				iza = parseBase();
				return std::shared_ptr<AST>(new Dereferenciranje(redak, stupac, { iza }));
			}
		}
		// ako nismo pročitali nijedan unarni operator, trebamo vratiti glavu
		vrati();
		return parseBase();
	}

	std::shared_ptr<AST> Parser::parseBase()
	{
		citaj();
		int redak = m_zadnji->Redak();
		int stupac = m_zadnji->Stupac();
		switch (m_zadnji->Tip())
		{
			case OOTV:
			{
				std::shared_ptr<AST> uZagradi = parseExpression();
				procitaj(OZATV);
				return uZagradi;
			}
			case IDENTIFIER:
			{
				// može biti identifier varijable ili funkcije
				// ako se radi o funkciji, mora imati zagrade nakon sebe
				std::shared_ptr<Leaf> ime(new Leaf(*m_zadnji));
				std::shared_ptr<Varijabla> varijabla(new Varijabla(redak, stupac, { ime }));
				citaj();
				switch (m_zadnji->Tip())
				{
					case OOTV:
					{
						std::shared_ptr<ASTList> argumenti(new ASTList(m_zadnji->Redak(), m_zadnji->Stupac()));
						while (!sljedeci(OZATV))
						{
							std::shared_ptr<AST> imeVarijable = parseExpression();
							if (!sljedeci(OZATV))
								procitaj(ZAREZ);

							argumenti->push_back(imeVarijable);
						}
						procitaj(OZATV);
						return std::shared_ptr<AST>(new PozivFunkcije(redak, stupac, { ime, argumenti }));
					}

					case INCR:
					case DECR:
					{
						vrati();
						std::shared_ptr<AST> trenutni = varijabla;
						while (true)
						{
							citaj();
							if (m_zadnji->Tip() == INCR)
								trenutni = std::shared_ptr<AST>(new Inkrement(redak, stupac, { trenutni }));
							
							else if(m_zadnji->Tip() == DECR)
								trenutni = std::shared_ptr<AST>(new Dekrement(redak, stupac, { trenutni }));

							else
							{
								vrati();
								break;
							}
						}
						return trenutni;
					}

					case UOTV:
					{
						std::shared_ptr<AST> uZagradi = parseExpression();
						procitaj(UZATV);
						return std::shared_ptr<AST>(new UglateZagrade(redak, stupac, { varijabla, uZagradi }));
					}

					default:
					{
						vrati();
						return varijabla;
					}
				}
			}

			case DEKADSKI:
			case HEKSADEKADSKI:
				return std::shared_ptr<AST>(new IntLiteral(*m_zadnji));
			case STRLIT:
				return std::shared_ptr<AST>(new StringLiteral(*m_zadnji));
			case CHRLIT:
				return std::shared_ptr<AST>(new CharLiteral(*m_zadnji));
			case BOOLEAN:
				return std::shared_ptr<AST>(new BoolLiteral(*m_zadnji));
			case NUL:
				return std::shared_ptr<AST>(new Leaf(*m_zadnji));
		}
		throw SintaksnaGreska(m_zadnji->Redak(), m_zadnji->Stupac(), "neparsiran token");
	}
}
