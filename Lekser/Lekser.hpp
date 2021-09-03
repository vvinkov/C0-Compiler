#ifndef LEKSER_HPP
#define LEKSER_HPP

#include <fstream>
#include <deque>
#include <string>
#include <functional>
#include <set>
#include <iostream>
#include <memory>
#include "../Token/Token.hpp"
#include "../Greska/Greska.hpp"

namespace C0Compiler
{
	class Lekser
	{
		public:
			Lekser(std::shared_ptr<std::ifstream> code);		// uzmi kod napisan u C0 i napravi tokene iz njega
			std::deque<std::shared_ptr<Token>> leksiraj();			// konstruiraj tokene od source-a

		private:
			std::shared_ptr<std::ifstream> m_code;			// kod programa napisanog u C0
			std::deque<std::shared_ptr<Token>> m_tokeni;	// tokeni koje je lekser prepoznao
			std::string m_linija;							// jedna linija iz datoteke s kodom
			std::string m_sadrzaj;							// sadržaj sljedećeg tokena
			bool m_vracanjeOk;								// flag, dozvoljavamo li vraćanje glave za čitanje nazad
			int  m_redak;									// redni broj retka u kojem se glava za čitanje nalazi
			int  m_stupac;									// redni broj stupca u kojem se glava za čitanje nalazi

			static std::set<char> escapeZnakovi;
			static std::set<char> escapeSekvence;

			void carriageReturn() { m_stupac = -1; }	// vrati glavu za čitanje na početak linije

		protected:
			char citaj();												// čitaj sljedeći znak iz input datoteke
			char procitaj(char znak);									// čitaj sljedeći znak iz input datoteke ako je jednak 'znak'-u, inače vrati grešku
			bool probajProcitati(char znak);							// čitaj sljedeći znak iz input datoteke ako je jednak 'znak'-u i vrati true, inače vrati false
			int	 KleeneZvijezda(std::function<bool(char)> && uvjet);	// čitaj 0 ili više znakova koji zadovoljavaju uvjet, vrati broj pročitanih
			void vrati();												// vrati glavu za čitanje jedno mjesto nazad
			void tokeniziraj(TokenTip tip);								// spremi token sadržaja sadrzaj i tipa tip
	};
}

#endif
