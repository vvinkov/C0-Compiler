#ifndef PARSER_HPP
#define PARSER_HPP

#include <deque>
#include <iostream>
#include <memory>
#include "../Token/Token.hpp"
#include "../AST/AST.hpp"

namespace C0Compiler
{

	class Parser
	{
		private:
			std::deque<std::shared_ptr<Token>> m_tokeni;	// tokeni koje je lekser konstruirao
			std::shared_ptr<Token> m_zadnji;				// ovdje držim zadnji pročitani token 
			std::shared_ptr<AST> m_korijen;					// AST-ovi koje je parser konstruirao
			bool m_vracanjeOk;								// flag, dozvoljavamo li vraćanje glave za čitanje nazad
		
			//void pocisti();									// počisti memoriju alociranu za tokene, AST i "zadnji"
			
			std::deque<std::shared_ptr<Token>> parseUse();	// parsiraj #use direktivu
			std::shared_ptr<AST> parseGlobal();				// parsiraj typedef, funkciju ili strukturu (deklaraciju ili definiciju)
			std::shared_ptr<AST> parseTip();
			std::shared_ptr<AST> parseStatement();
			std::shared_ptr<AST> parseSimple();
			std::shared_ptr<AST> parseLValue();
			std::shared_ptr<AST> parseExpression();
			std::shared_ptr<AST> parseLogicki();
			std::shared_ptr<AST> parseBitwise();
			std::shared_ptr<AST> parseEquality();
			std::shared_ptr<AST> parseComparison();
			std::shared_ptr<AST> parseShifts();
			std::shared_ptr<AST> parseAdd();
			std::shared_ptr<AST> parseFactor();
			std::shared_ptr<AST> parseAssign();
			std::shared_ptr<AST> parseAllocate();
			std::shared_ptr<AST> parseAllocArray();
			std::shared_ptr<AST> parseStrelica();
			std::shared_ptr<AST> parseTocka();
			std::shared_ptr<AST> parseUnary();
			std::shared_ptr<AST> parseBase();

			friend std::deque<std::shared_ptr<Token>> UseDirektiva::izvrsi(Parser&); // use direktiva treba moći čačkati po privatnim dijelovima parsera

		protected:
			Token& citaj();									// čitaj sljedeći token
			Token& procitaj(TokenTip tip);					// čitaj sljedeći token ako je tipa 'tip', inače vrati grešku
			bool sljedeci(TokenTip tip);					// true ako je sljedeći token tipa 'tip', inače false
			void vrati();									// vrati se jedan token nazad
			//void sintaksnaGreska(std::string const& opis);	// prijavljuje sintaksnu grešku i zaustavlja izvršavanje programa

		public:
			Parser(std::deque<std::shared_ptr<Token>>&& tokeni);
			void parsiraj();	// konstruira AST-ove od tokena
	};
}
#endif