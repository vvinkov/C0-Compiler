﻿#include "Greska.hpp"

namespace C0Compiler
{
	// općenite greške

	Greska::Greska(std::string const& vrsta, int redak, int stupac)
		: poruka(vrsta + "! Redak: " + std::to_string(redak) +
			", stupac: " + std::to_string(stupac) + ". Opis: ") {}

	Greska::Greska(std::string const& vrsta, int redak, int stupac, std::string const& opis) 
		: poruka(vrsta + "! Redak: " + std::to_string(redak) +
		", stupac: " + std::to_string(stupac) + ". Opis: " + opis + ".") {}

	// leksičke greške
	LeksickaGreska::LeksickaGreska(int redak, int stupac, char dobio, char ocekujem) : Greska("Leksička greška", redak, stupac)
	{
		poruka << "neočekivan znak '"<< dobio <<"', očekujem '" << ocekujem << "'.";
	}

	LeksickaGreska::LeksickaGreska(int redak, int stupac, char znak) : Greska("Leksička greška", redak, stupac)
	{
		poruka << "nepoznat znak: '" << znak << "'";
	}

	// sintaksne greške
	SintaksnaGreska::SintaksnaGreska(int redak, int stupac, Token const& dobio, enum TokenTip ocekujem) :Greska("Sintaksna greška", redak, stupac)
	{
		poruka << "neočekivan token '" << dobio << "', očekujem '" << tokenString[ocekujem] << "'.";
	}

	SintaksnaGreska::SintaksnaGreska(int redak, int stupac, Token const& dobio, std::list<TokenTip>&& ocekujem) :Greska("Sintaksna greška", redak, stupac)
	{
		poruka << "neočekivan token '" << dobio << "', očekujem jedan od: ";
		for (std::list<TokenTip>::iterator it = ocekujem.begin(); it != ocekujem.end();)
		{
			poruka << "'" << tokenString[*it] << "'";
			if (++it != ocekujem.end())
				poruka << ", ";
		}
	}

}