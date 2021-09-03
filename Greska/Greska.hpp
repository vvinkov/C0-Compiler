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
			Greska(Greska const& druga);
			Greska(Greska&& druga);

			virtual const char* what() const override { return poruka.str().c_str(); };

			virtual ~Greska() {}
	};

	class LeksickaGreska : public Greska
	{
		public:
			LeksickaGreska(int redak, int stupac, char dobio, char ocekujem);
			LeksickaGreska(int redak, int stupac, std::string const& opis) : Greska("Leksička greška", redak, stupac, opis) {}
			LeksickaGreska(int redak, int stupac, char znak);
			LeksickaGreska(LeksickaGreska const& druga) : Greska(druga) {};
			LeksickaGreska(LeksickaGreska&& druga) : Greska(druga) {};

			virtual ~LeksickaGreska(){}
	};

	class SintaksnaGreska : public Greska
	{
		public:
			SintaksnaGreska(int redak, int stupac, std::string const& opis) : Greska("Sintaksna greška", redak, stupac, opis) {}
			SintaksnaGreska(int redak, int stupac, Token const& dobio, enum TokenTip ocekujem);
			SintaksnaGreska(int redak, int stupac, Token const& dobio, std::list<TokenTip>&& ocekujem);
			SintaksnaGreska(SintaksnaGreska const& druga) : Greska(druga) {};
			SintaksnaGreska(SintaksnaGreska&& druga) : Greska(druga) {};

			virtual ~SintaksnaGreska(){}
	};

	class SemantickaGreska : public Greska
	{
		public:
			SemantickaGreska(int redak, int stupac, std::string const& opis) : Greska("Greška", redak, stupac, opis) {}
			SemantickaGreska(SemantickaGreska const& druga) : Greska(druga) {};
			SemantickaGreska(SemantickaGreska&& druga) : Greska(druga) {};

			virtual ~SemantickaGreska(){}
	};

	class Iznimka : public Greska
	{
		public:
			Iznimka(int redak, int stupac, std::string const& vrsta, std::string const& opis) : Greska(vrsta, redak, stupac, opis){}
			Iznimka(Iznimka const& druga) : Greska(druga) {};
			Iznimka(Iznimka&& druga) : Greska(druga) {};

			void Prijavi();
			virtual ~Iznimka(){}
	};
}

#endif