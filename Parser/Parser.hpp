#ifndef PARSER_HPP
#define PARSER_HPP

// premišljam se da ovdje napišem BKG za C0 ili da uputim ljude na dokumentaciju

#include <deque>
#include <iostream>
#include "../Token/Token.hpp"
#include "../AST/AST.hpp"

namespace C0Compiler
{

	class Parser
	{
		private:
			std::deque<Token*> m_tokeni;					// tokeni koje je lekser konstruirao
			Token* zadnji;									// ovdje držim zadnji pročitani token 
			//AST* m_korijen;									// AST-ovi koje je parser konstruirao
			bool vracanje_ok;								// flag, dozvoljavamo li vraćanje glave za čitanje nazad
		
			void pocisti();									// počisti memoriju alociranu za tokene, AST i "zadnji"
			
			AST* parseUse();								// parsiraj #use direktivu
			AST* parseFunction();								// parsiraj funkciju (deklaraciju ili definiciju)
			AST* parseStatement();

		protected:
			Token& citaj();									// čitaj sljedeći token
			Token& procitaj(TokenTip tip);					// čitaj sljedeći token ako je tipa 'tip', inače vrati grešku
			bool sljedeci(TokenTip tip);					// true ako je sljedeći token tipa 'tip', inače false
			void vrati();									// vrati se jedan token nazad
			void sintaksnaGreska(std::string const& opis);	// prijavljuje sintaksnu grešku i zaustavlja izvršavanje programa

		public:
			Parser() = delete;
			Parser(std::deque<Token*>&& tokeni);
			void parsiraj();								// konstruira AST-ove od tokena
	};
}
#endif