#ifndef _LEKSER_HPP_
#define _LEKSER_HPP_

#include <fstream>
#include <deque>
#include <string>
#include <functional>
#include <set>
#include <iostream>
#include "../Token/Token.hpp"

namespace C0Compiler
{
	class Lekser
	{
		private:
			std::ifstream* m_code;										// kod programa napisanog u C0
			std::deque<Token*> m_tokeni;								// tokeni koje je lekser prepoznao
			std::string linija;											// jedna linija iz datoteke s kodom
			std::string sadrzaj;										// sadržaj sljedećeg tokena
			bool vracanje_ok;											// flag, dozvoljavamo li vraćanje glave za čitanje nazad
			int redak;													// redni broj retka u kojem se glava za čitanje nalazi
			int stupac;													// redni broj stupca u kojem se glava za čitanje nalazi
			int glava;													// mjesto u liniji na kojem se nalazi glava za čitanje

			static std::set<char> escapeZnakovi;
			static std::set<char> escapeSekvence;

			void carriageReturn() { glava = -1; stupac = -1; }			// vrati glavu za čitanje na početak linije

		protected:
			
			char citaj();												// čitaj sljedeći znak iz input datoteke
			char procitaj(char znak);									// čitaj sljedeći znak iz input datoteke ako je jednak 'znak'-u, inače vrati grešku
			bool probajProcitati(char znak);							// čitaj sljedeći znak iz input datoteke ako je jednak 'znak'-u i vrati true, inače vrati false
			int	 kleeneZvijezda(std::function<bool(char)> && uvjet);	// čitaj 0 ili više znakova koji zadovoljavaju uvjet, vrati broj pročitanih
			void vrati();												// vrati glavu za čitanje jedno mjesto nazad
			void tokeniziraj(TokenTip tip);								// spremi token sadržaja sadrzaj i tipa tip
			void lexGreska(std::string const& opis);					// ispisuje leksičku grešku i zaustavlja izvršavanje programa

		public:
			void pocisti();												// počisti alociranu memoriju

			Lekser(std::ifstream* code);
			std::deque<Token*> leksiraj();
	};
}

#endif