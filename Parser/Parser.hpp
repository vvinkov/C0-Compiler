#ifndef _PARSER_HPP_
#define _PARSER_HPP_

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
			std::deque<AST*> m_ASTs;						// AST-ovi koje je parser konstruirao
			std::deque<Token*> buffer;						// mjesto gdje držim tokene koji još nisu složeni u AST 
			Token* zadnji;									// ovdje držim zadnji pročitani token 
			bool vracanje_ok;								// flag, dozvoljavamo li vraćanje glave za čitanje nazad
		
			void pocisti();									// počisti memoriju alociranu za tokene, AST i "zadnji"
			void dodajGranu(Token* token);					// dodaje granu u AST (čitaj: push_back zadnji u buffer)
			void dodajGranu(Token& token);
			AST const& sASTavi(ASTtip tip);					// sastavlja AST tipa "tip" od tokena koji su u bufferu i vraća isti AST
			
			AST const& parsirajUse();						// parsiraj #use direktivu
			AST const& parsirajFunkcija();					// parsiraj funkciju
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