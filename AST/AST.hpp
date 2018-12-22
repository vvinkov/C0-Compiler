#ifndef _AST_HPP_
#define _AST_HPP_

#include "../Token/Token.hpp"

namespace C0Compiler
{
	// da, i AST-ovi sad mogu imati tipove
	enum ASTtip
	{
		USE_DIREKTIVA,
		FUNKCIJA
	};

	// opća klasa za AST, da ih mogu sve pobacati u jedan container
	// 21.12.2018. ideja je bila: privatni konstruktori ovdje i friendly factory u parseru,
	// ali nešto me zeza i trenutno nemam internet da provjerim šta mu je
	class AST
	{
		private:
			std::deque<Token*> m_tokeni;

		protected:
			// ovo bez problema može biti i public jer ionako 
			// nitko neće nikad konstruirati čisti AST, ali više mi se sviđa ovako
			AST() = delete;
			AST(std::deque<Token*> tokeni) : m_tokeni(tokeni){}

		public:
			virtual void evaluiraj() = 0;	// ovo će se možda drugačije zvati ubuduće, analogon "izvrši" s IP,
			virtual ~AST(){}				// osim što sad ne interpretiram, nego optimiziram i compiliram
	};

	class UseDirektiva : public AST
	{
		public:
			UseDirektiva(std::deque<Token*> tokeni) : AST(std::move(tokeni)){}
			
			virtual void evaluiraj() override {};
			virtual ~UseDirektiva(){}
	};

	class Funkcija : public AST
	{
		public:
			Funkcija(std::deque<Token*> tokeni) : AST(std::move(tokeni)){}
			
			virtual void evaluiraj() override {};
			virtual ~Funkcija(){}
	};
}

#endif