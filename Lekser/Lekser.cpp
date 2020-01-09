#include "Lekser.hpp"
#include "../Greska//Greska.hpp"

namespace C0Compiler
{
	Lekser::Lekser(std::shared_ptr<std::ifstream> const& code)
	{
		m_code = code;
		m_redak = 0;
		m_stupac = 0;
		carriageReturn();
	}

	std::set<char> Lekser::escapeZnakovi = { 'n', 't', 'v', 'b', 'r', 'f', 'a', '\'', '"', '\\' };
	std::set<char> Lekser::escapeSekvence = { '\n', '\t', '\v', '\b', '\r', '\f', '\a', '\'', '\"', '\\' };
	
	//void Lekser::pocisti()
	//{
	//	while (!m_tokeni.empty())
	//	{
	//		delete m_tokeni.front();
	//		m_tokeni.front() = nullptr;
	//		m_tokeni.pop_front();
	//	}
	//	m_code->close();
	//}

	char Lekser::citaj()
	{
		// ako glava ne može dalje...
		if (m_stupac + 1 >= m_linija.size())
		{
			if (std::getline(*m_code, m_linija))
			{
				m_linija += '\n';
				carriageReturn();
				++m_redak;
			}
		}
		++m_stupac;
		m_sadrzaj += m_linija[m_stupac];
		m_vracanjeOk = true;
		return m_linija[m_stupac];
	}

	char Lekser::procitaj(char znak)
	{
		char procitan = citaj();
		if (znak == procitan) 
			return znak;

		throw LeksickaGreska(m_redak, m_stupac, procitan, znak);
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
		if (m_vracanjeOk)
		{
			m_vracanjeOk = false;
			--m_stupac;
			m_sadrzaj.pop_back();
		}
		else
			throw Greska("Greška", m_redak, m_stupac, "buffer overflow");
	}

	void Lekser::tokeniziraj(TokenTip tip)
	{
		std::shared_ptr<Token> noviToken = std::make_shared<Token>(tip, m_sadrzaj, m_redak, m_stupac - m_sadrzaj.size() + 2);	// voodoo da dobijem pravi stupac gdje token počinje
		m_tokeni.push_back(noviToken);
		m_sadrzaj.clear();
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

	std::deque<std::shared_ptr<Token>> Lekser::leksiraj()
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
					// ako si pročitao '\', pogledaj je li sljedeći znak escape znak
					sljedeci = citaj();
					if (escapeZnakovi.find(sljedeci) == escapeZnakovi.end())
					{
						// ako nije, imamo problem
						throw LeksickaGreska(m_redak, m_stupac, "neispravan string");
					}
				}
				else if (isprint(znak) && znak != '"')
				{
					// ako si pročitao znak i taj znak nije '"', još si u stringu
					continue;
				}

				else
				{
					if (znak == '"')
					{
						// ako si pročitao '"', došao si do kraja stringa
						citamString = false;
					}
					tokeniziraj(STRLIT);
				}
			}
			else if (citamKomentar)
			{
				if (znak == '*' && probajProcitati('/'))
				{
					// ako si u komentaru i pročitao si "*/", onda si došao dok kraja komentara
					citamKomentar = false;
					m_sadrzaj.clear();
				}
			}
			else if (isspace(znak))
				m_sadrzaj.clear();	// razmaci nisu dio sintakse

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
				if (m_sadrzaj == "true" || m_sadrzaj == "false") tokeniziraj(BOOLEAN);
				else if (m_sadrzaj == "NULL") tokeniziraj(NUL);
				else if (m_sadrzaj == "if") tokeniziraj(IF);
				else if (m_sadrzaj == "else") tokeniziraj(ELSE);
				else if (m_sadrzaj == "while") tokeniziraj(WHILE);
				else if (m_sadrzaj == "for") tokeniziraj(FOR);
				else if (m_sadrzaj == "assert") tokeniziraj(ASSERT);
				else if (m_sadrzaj == "break") tokeniziraj(BREAK);
				else if (m_sadrzaj == "continue") tokeniziraj(CONTINUE);
				else if (m_sadrzaj == "return") tokeniziraj(RETURN);
				else if (m_sadrzaj == "alloc") tokeniziraj(ALLOC);
				else if (m_sadrzaj == "alloc_array") tokeniziraj(ALLOCARRAY);
				else if (m_sadrzaj == "struct") tokeniziraj(STRUCT);
				else if (m_sadrzaj == "typedef") tokeniziraj(TYPEDEF);
				else if (m_sadrzaj == "int")
				{
					// ako si pročitao int, možda je poslije njega '*'
					if (probajProcitati('*'))
						tokeniziraj(INTPOINT);

					// ako nije, možda je "[]"
					else if (probajProcitati('['))
					{
						procitaj(']');
						tokeniziraj(INTARRAY);
					}
					else
					{
						// ako nije ništa od toga, imaš obični int
						tokeniziraj(INT);
					}
				}
				else if (m_sadrzaj == "bool")
				{
					// ako si pročitao bool, možda je poslije njega '*'
					if (probajProcitati('*'))
						tokeniziraj(BOOLPOINT);

					// ako nije, možda je "[]"
					else if (probajProcitati('['))
					{
						procitaj(']');
						tokeniziraj(BOOLARRAY);
					}
					else
					{
						// ako nije ništa od toga, imaš obični bool
						tokeniziraj(BOOL);
					}
				}
				else if (m_sadrzaj == "char")
				{
					// ako si pročitao char, možda je poslije njega '*'
					if (probajProcitati('*'))
						tokeniziraj(CHARPOINT);

					// ako nije, možda je "[]"
					else if (probajProcitati('['))
					{
						procitaj(']');
						tokeniziraj(CHARARRAY);
					}
					else
					{
						// ako nije ništa od toga, imaš obični char
						tokeniziraj(CHAR);
					}
				}
				else if (m_sadrzaj == "string")
				{
					// ako si pročitao string, možda je poslije njega '*'
					if (probajProcitati('*'))
						tokeniziraj(STRINGPOINT);

					// ako nije, možda je "[]"
					else if (probajProcitati('['))
					{
						procitaj(']');
						tokeniziraj(STRINGARRAY);
					}
					else
					{
						// ako nije ništa od toga, imaš obični string
						tokeniziraj(STRING);
					}
				}
				else if (m_sadrzaj == "void")
				{
					// ako si pročitao void, možda je poslije njega '*'
					if (probajProcitati('*'))
						tokeniziraj(STRINGPOINT);

					// ako nije, možda je "[]"
					else if (probajProcitati('['))
					{
						procitaj(']');
						tokeniziraj(STRINGARRAY);
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
							throw LeksickaGreska(m_redak, m_stupac, "nakon 0x očekujem heksadekadsku znamenku");
						}
					}
					else if (isdigit(sljedeci))
						throw LeksickaGreska(m_redak, m_stupac, "nisu dozvoljene vodeće nule u dekadskom zapisu");
					
					else
					{
						// ako je samo nula, odlično, to je legalan broj u dekadskom zapisu
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
					throw LeksickaGreska(m_redak, m_stupac, "neispravan char literal");
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
					m_sadrzaj.clear(); // ignoriramo komentare
				}
				else if (probajProcitati('*'))
				{
					// ako si pročitao "/*", onda si u blok komentaru, prebaci se u "komentar mode"
					// i sve što pročitaš dodaješ u taj komentar
					citamKomentar = true;
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

			// ako nemaš pojma što si pročitao...
			else
				throw LeksickaGreska(m_redak, m_stupac, znak);

		} // while
		if (citamKomentar)
			throw LeksickaGreska(m_redak, m_stupac, "nezatvoren komentar");

		tokeniziraj(KRAJ);
		return m_tokeni;
	} // leksiraj

} // namespace