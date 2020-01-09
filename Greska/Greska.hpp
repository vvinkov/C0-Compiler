#ifndef GRESKA_HPP
#define GRESKA_HPP

#include <string>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <list>

#include "../Token/Token.hpp"

namespace C0Compiler
{
	class Greska : std::exception
	{
		protected:
			std::stringstream poruka;

		public:
			Greska(std::string const& vrsta, int redak, int stupac);
			Greska(std::string const& vrsta, int redak, int stupac, std::string const& opis);
			virtual const char* what() const override { return poruka.str().c_str(); };

			virtual ~Greska() {}
	};

	class LeksickaGreska : public Greska
	{
		public:
			LeksickaGreska(int redak, int stupac, char dobio, char ocekujem);
			LeksickaGreska(int redak, int stupac, std::string const& opis) : Greska("Leksička greška", redak, stupac, opis) {}
			LeksickaGreska(int redak, int stupac, char znak);

			virtual ~LeksickaGreska(){}
	};

	class SintaksnaGreska : public Greska
	{
		public:
			SintaksnaGreska(int redak, int stupac, std::string const& opis) : Greska("Sintaksna greška", redak, stupac, opis) {}
			SintaksnaGreska(int redak, int stupac, Token const& dobio, enum TokenTip ocekujem);
			SintaksnaGreska(int redak, int stupac, Token const& dobio, std::list<TokenTip>&& ocekujem);

			virtual ~SintaksnaGreska(){}
	};

	class SemantickaGreska : public Greska
	{
		public:
			SemantickaGreska(int redak, int stupac, std::string const& opis) : Greska("Greška!", redak, stupac, opis) {}
	};
}

#endif