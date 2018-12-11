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
		if (glava + 1 >= linija.size())
		{
			if (std::getline(*m_code, linija))
			{
				linija += '\n';
				carriageReturn();
				++redak;
			}
		}
		++stupac;
		++glava;
		sadrzaj += linija[glava];
		vracanje_ok = true;
		return linija[glava];
	}

	char Lekser::procitaj(char znak)
	{
		char procitan = citaj();
		if (znak == procitan) return znak;

		std::string opis = "neocekivan znak ";
		opis += procitan;
		opis += ", ocekujem ";
		opis += znak;
		opis += '.';
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
			--glava;
			sadrzaj.pop_back();
		}
		else
		{
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

	int Lekser::kleeneZvijezda(std::function<bool(char)> const& uvjet)
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

		while (znak = citaj())
		{
			if (citamString)
			{
				if (znak == '\\')
				{
					sljedeci = citaj();
					if (escapeZnakovi.find(sljedeci) == escapeZnakovi.end())
					{
						lexGreska("neispravan string");
					}


				}
				else if (isprint(znak) && znak != '"')
					continue;

				else
				{
					procitaj('"');
					citamString = false;
					tokeniziraj(STRLIT);
				}
			}
			else if (isspace(znak))
				sadrzaj.clear();	// ne leksiramo Python, razmaci nisu dio sintakse

			else if (znak == '"')
			{
				citamString = true;
				continue;
			}

			else if (isalpha(znak) || znak == '_')
			{
				// onda je jedan od identifiera
				kleeneZvijezda([](char znak){ return isalnum(znak) || znak == '_'; });
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
					if (probajProcitati('*'))
						tokeniziraj(POINTER);

					else if (probajProcitati('['))
					{
						procitaj(']');
						tokeniziraj(ARRAY);
					}
					else
						tokeniziraj(INT);
				}
				else if (sadrzaj == "bool")
				{
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
				// onda je broj u decimalnom ili heksadekadskom zapisu
				if (znak == '0')
				{
					sljedeci = citaj();
					if (sljedeci == 'x' || sljedeci == 'X')
					{
						// onda je heksadekadski
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
						vrati();
						tokeniziraj(DECIMALNI);
					}
				}
				else
				{
					kleeneZvijezda(isdigit);
					tokeniziraj(DECIMALNI);
				}
			}
			else if (znak == '\'')
			{
				sljedeci = citaj();
				if (!isprint(sljedeci) && escapeSekvence.find(sljedeci) != escapeSekvence.end() && sljedeci != '\0')
				{
					lexGreska("neispravan char literal");
				}
				else
				{
					procitaj('\'');
					tokeniziraj(CHRLIT);
				}
			}
			// je li jedan od separatora?
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

			// je li operator?
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
					kleeneZvijezda([](char znak) {return znak != '\n'; }); // može ovo i urednije, samo trebam smisliti lijepo ime za funkciju
					procitaj('\n');
					sadrzaj.clear();
					//tokeniziraj(COMMENT);
				}
				else if (probajProcitati('*'))
				{
					do
					{
						do
						{
							sljedeci = citaj();
						} 
						while (sljedeci != '*');
					}
					while(!probajProcitati('/'));
					//tokeniziraj(COMMENT);
					sadrzaj.clear(); // sjetimo se PROG1: "compiler ignorira komentare". kad bih ih stvarno leksirao, samo bih otvorio vrata gomili false positive sintaksnih grešaka
				}
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
		} // while

		return m_tokeni;
	} // leksiraj

} // namespace