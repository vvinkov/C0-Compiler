#include "Lekser.hpp"

namespace C0Compiler
{
	Lekser::Lekser(std::ifstream* code)
	{
		m_code = code;
		redak = 0;
		stupac = 0;
		carriageReturn();
	}

	std::set<char> Lekser::escapeZnakovi = { 'n', 't', 'v', 'b', 'r', 'f', 'a', '\'', '"', '\\' };
	std::set<char> Lekser::escapeSekvence = { '\n', '\t', '\v', '\b', '\r', '\f', '\a', '\'', '\"', '\\' };
	
	void Lekser::pocisti()
	{
		while (!m_tokeni.empty())
		{
			delete m_tokeni.front();
			m_tokeni.front() = nullptr;
			m_tokeni.pop_front();
		}
		m_code->close();
	}

	char Lekser::citaj()
	{
		// ako glava ne može dalje...
		if (stupac + 1 >= linija.size())
		{
			if (std::getline(*m_code, linija))
			{
				linija += '\n';
				carriageReturn();
				++redak;
			}
		}
		++stupac;
		sadrzaj += linija[stupac];
		vracanje_ok = true;
		return linija[stupac];
	}

	char Lekser::procitaj(char znak)
	{
		char procitan = citaj();
		if (znak == procitan) return znak;

		std::string opis = "neocekivan znak '";
		opis += procitan;
		opis += "', ocekujem '";
		opis += znak;
		opis += "'.";
		lexGreska(opis);
		exit(1);
	}

	bool Lekser::probajProcitati(char znak)
	{
		if (znak == citaj())
			return true;

		vrati();
		return false;
	}

	void Lekser::vrati()
	{
		if (vracanje_ok)
		{
			vracanje_ok = false;
			--stupac;
			sadrzaj.pop_back();
		}
		else
		{
			// 12.12.2018. možda bi dobro bilo imati apstraktnu klasu greška, derivirati sve greške iz nje
			// i ovaj gadni dio s printanjem i čišćenjem riješiti u nekoj virtualnoj metodi
			std::cerr << "Greska! Nije moguce vratiti glavu vise od jednog mjesta unazad!" << std::endl;
			pocisti();
			exit(1); 
		}
	}

	void Lekser::tokeniziraj(TokenTip tip)
	{
		Token* token = new Token(tip, sadrzaj, redak, stupac-sadrzaj.size()+2);	// voodoo da dobijem pravi stupac gdje token počinje
		m_tokeni.push_back(token);
		sadrzaj.clear();
	}

	void Lekser::lexGreska(std::string const& opis)
	{
		std::cerr << "Leksicka greska! Redak " << redak << ", stupac " << stupac << ". Opis: " << opis << std::endl;
		pocisti();
		std::cin.get();
		exit(1);
	}

	int Lekser::kleeneZvijezda(std::function<bool(char)> && uvjet)
	{
		char znak;
		int procitani = 0;
		do
		{
			znak = citaj();
			++procitani;
		} 
		while (uvjet(znak));
		vrati();

		return procitani - 1;
	}

	std::deque<Token*> Lekser::leksiraj()
	{
		char znak, sljedeci;
		bool citamString = false;
		bool citamKomentar = false;
		
		tokeniziraj(POCETAK);	// označi početak datoteke. kad ga ne bi bilo, prvi citaj() u parseru bi preskočio prvi token.
								// ovako preskoči POCETAK, koji ionako ne služi ničemu osim da ga se preskoči
		while (znak = citaj())
		{
			if (citamString)
			{
				if (znak == '\\')
				{
					// ako si pročitao '\', pogledaj je li sljedeći escape znak
					sljedeci = citaj();
					if (escapeZnakovi.find(sljedeci) == escapeZnakovi.end())
						// ako nije, imamo problem
						lexGreska("neispravan string");
				}
				else if (isprint(znak) && znak != '"')
					// ako si pročitao znak i taj znak nije '"', još si u stringu
					continue;

				else
				{
					if(znak == '"')
						// ako si pročitao '"', došao si do kraja stringa
						citamString = false;
					tokeniziraj(STRLIT);
				}
			}
			else if (citamKomentar)
			{
				if (znak == '*' && probajProcitati('/'))
				{
					// ako si u komentaru i pročitao si "*/", onda si došao dok kraja komentara
					citamKomentar = false;
					sadrzaj.clear();
				}
			}
			else if (isspace(znak))
				sadrzaj.clear();	// ne leksiramo Python, razmaci nisu dio sintakse

			else if (znak == '"')
			{
				// ako si pročitao '"', našao si početak stringa
				citamString = true;
				continue;
			}

			else if (znak == '#') 
			{
				// poslije '#' ide samo "use", inače imamo grešku
				procitaj('u');
				procitaj('s');
				procitaj('e');
				tokeniziraj(USE);
			}

			else if (isalpha(znak) || znak == '_')
			{
				// ako si pročitao slovo ili '_', onda si našao identifier
				// svako sljedeće slovo ili broj pripada istom tom identifieru
				kleeneZvijezda([](char znak){ return isalnum(znak) || znak == '_'; });

				// ovisno o tome što si pročitao, tokeniziraj
				if (sadrzaj == "true" || sadrzaj == "false") tokeniziraj(BOOLEAN);
				else if (sadrzaj == "NULL") tokeniziraj(NUL);
				else if (sadrzaj == "if") tokeniziraj(IF);
				else if (sadrzaj == "else") tokeniziraj(ELSE);
				else if (sadrzaj == "while") tokeniziraj(WHILE);
				else if (sadrzaj == "for") tokeniziraj(FOR);
				else if (sadrzaj == "assert") tokeniziraj(ASSERT);
				else if (sadrzaj == "error") tokeniziraj(ERROR);
				else if (sadrzaj == "break") tokeniziraj(BREAK);
				else if (sadrzaj == "continue") tokeniziraj(CONTINUE);
				else if (sadrzaj == "return") tokeniziraj(RETURN);
				else if (sadrzaj == "alloc") tokeniziraj(ALLOC);
				else if (sadrzaj == "alloc_array") tokeniziraj(ALLOCA);
				else if (sadrzaj == "int")
				{
					// ako si pročitao int, možda je poslije njega '*'
					if (probajProcitati('*'))
						tokeniziraj(POINTER);

					// ako nije, možda je "[]"
					else if (probajProcitati('['))
					{
						procitaj(']');
						tokeniziraj(ARRAY);
					}
					else
						// ako nije ništa od toga, imaš obični int
						tokeniziraj(INT);
				}
				else if (sadrzaj == "bool")
				{
					// vidi komentare za int, sasvim je analogno
					if (probajProcitati('*'))
						tokeniziraj(POINTER);

					else if (probajProcitati('['))
					{
						procitaj(']');
						tokeniziraj(ARRAY);
					}
					else
						tokeniziraj(BOOL);
				}
				else if (sadrzaj == "char")
				{
					// vidi komentare za int, sasvim je analogno
					if (probajProcitati('*'))
						tokeniziraj(POINTER);

					else if (probajProcitati('['))
					{
						procitaj(']');
						tokeniziraj(ARRAY);
					}
					else
						tokeniziraj(CHAR);
				}
				else if (sadrzaj == "string")
				{
					// vidi komentare za int, sasvim je analogno
					if (probajProcitati('*'))
						tokeniziraj(POINTER);

					else if (probajProcitati('['))
					{
						procitaj(']');
						tokeniziraj(ARRAY);
					}
					else tokeniziraj(STRING);
				}
				else if (sadrzaj == "void")
				{
					// vidi komentare za int, sasvim je analogno
					if (probajProcitati('*'))
						tokeniziraj(POINTER);

					else if (probajProcitati('['))
					{
						procitaj(']');
						tokeniziraj(ARRAY);
					}
				}
				else
					tokeniziraj(IDENTIFIER);
			}
			else if (isdigit(znak))
			{
				// ako si pročitao znamenku, onda si našao broj
				if (znak == '0')
				{
					sljedeci = citaj();
					// ako je poslije nule x, onda je broj u heksadekadskom zapisu
					if (sljedeci == 'x' || sljedeci == 'X')
					{
						sljedeci = citaj();
						if (isxdigit(sljedeci))
						{
							kleeneZvijezda(isxdigit);
							tokeniziraj(HEKSADEKADSKI);
						}
						else
						{
							lexGreska("nakon 0x ocekujem heksadekadsku znamenku");
						}
					}
					else if (isdigit(sljedeci))
					{
						lexGreska("nisu dozvoljene vodece nule u dekadskom zapisu");
					}
					else
					{
						// ako je samo nula, odlično, to je broj u dekadskom zapisu
						vrati();
						tokeniziraj(DEKADSKI);
					}
				}
				else
				{
					// ako je znamenka, a nije nula, imaš broj u dekadskom zapisu
					kleeneZvijezda(isdigit);
					tokeniziraj(DEKADSKI);
				}
			}
			else if (znak == '\'')
			{
				sljedeci = citaj();
				if (!isprint(sljedeci) && escapeSekvence.find(sljedeci) != escapeSekvence.end() && sljedeci != '\0')
				{
					// ako si pročitao ', a u njemu nije znak ili escape sekvenca, imaš bezvezni char literal
					lexGreska("neispravan char literal");
				}
				else
				{
					procitaj('\'');
					tokeniziraj(CHRLIT);
				}
			}
			// jesi li našao neki od separatora?
			else if (znak == '(')
				tokeniziraj(OOTV);
			else if (znak == ')')
				tokeniziraj(OZATV);
			else if (znak == '[')
				tokeniziraj(UOTV);
			else if (znak == ']')
				tokeniziraj(UZATV);
			else if (znak == '{')
				tokeniziraj(VOTV);
			else if (znak == '}')
				tokeniziraj(VZATV);
			else if (znak == ',')
				tokeniziraj(ZAREZ);
			else if (znak == ';')
				tokeniziraj(TZAREZ);

			// jesi li našao operator?
			else if (znak == '!')
			{
				if (probajProcitati('='))
					tokeniziraj(NEQ);
				else
					tokeniziraj(USKL);
			}
			else if (znak == '~')
			{
				tokeniziraj(TILDA);
			}
			else if (znak == '*')
			{
				if (probajProcitati('='))
					tokeniziraj(ZVJEQ);
				else
					tokeniziraj(ZVJ);
			}
			else if (znak == '.')
				tokeniziraj(TOCKA);

			else if (znak == '-')
			{
				if (probajProcitati('>'))
					tokeniziraj(STRELICA);
				else if (probajProcitati('='))
					tokeniziraj(MINUSEQ);
				else if (probajProcitati('-'))
					tokeniziraj(DECR);
				else tokeniziraj(MINUS);
			}

			else if (znak == '/')
			{
				if (probajProcitati('='))
					tokeniziraj(SLASHEQ);
				else if (probajProcitati('/'))
				{
					// ako si pročitao "//", sve do kraja reda je komentar
					kleeneZvijezda([](char znak) {return znak != '\n'; }); 
					procitaj('\n');
					sadrzaj.clear(); // sjetimo se PROG1: "compiler ignorira komentare". kad bih ih stvarno tokenizirao, samo bih otvorio vrata gomili false positive sintaksnih grešaka
				}
				else if (probajProcitati('*'))
					// ako si pročitao "/*", onda si u blok komentaru, prebaci se u "komentar mod"
					// i sve što pročitaš šibaj u taj komentar
					citamKomentar = true;
					
				else
					tokeniziraj(SLASH);
			}

			else if (znak == '%')
			{
				if (probajProcitati('='))
					tokeniziraj(MODEQ);
				else
					tokeniziraj(MOD);
			}

			else if (znak == '+')
			{
				if (probajProcitati('='))
					tokeniziraj(PLUSEQ);
				else if (probajProcitati('+'))
					tokeniziraj(INCR);
				else
					tokeniziraj(PLUS);
			}	

			else if (znak == '<')
			{
				if (probajProcitati('<'))
				{
					if (probajProcitati('='))
						tokeniziraj(LSHIFTEQ);
					else
						tokeniziraj(LSHIFT);
				}
				else if (probajProcitati('='))
					tokeniziraj(LESSEQ);

				else
					tokeniziraj(LESS);
			}

			else if (znak == '>')
			{
			if (probajProcitati('>'))
			{
				if (probajProcitati('='))
					tokeniziraj(RSHIFTEQ);
				else
					tokeniziraj(RSHIFT);
				}
				else if (probajProcitati('='))
					tokeniziraj(GRTEQ);
				else 
					tokeniziraj(GRT);
			}

			else if (znak == '=')
			{
				if (probajProcitati('='))
					tokeniziraj(EQ);
				else
					tokeniziraj(ASSIGN);
			}

			else if (znak == '&')
			{
				if (probajProcitati('='))
					tokeniziraj(BANDEQ);
				else if (probajProcitati('&'))
					tokeniziraj(LAND);
				else 
					tokeniziraj(BITAND);
			}

			else if (znak == '|')
			{
				if (probajProcitati('='))
					tokeniziraj(BOREQ);
				else if (probajProcitati('|'))
					tokeniziraj(LOR);
				else tokeniziraj(BITOR);
			}

			else if (znak == '^')
			{
				if (probajProcitati('='))
					tokeniziraj(BXOREQ);
				else
					tokeniziraj(BITXOR);
			}

			else if (znak == '?')
				tokeniziraj(CONDQ);

			else if (znak == ':')
				tokeniziraj(DTOCKA);

			// ako nemaš pojma što si pročitao...
			else
			{
				lexGreska("Nepoznat znak " + znak);
			}
		} // while
		if (citamKomentar)
			lexGreska("Nezatvoren komentar");

		tokeniziraj(KRAJ);
		return m_tokeni;
	} // leksiraj

} // namespace