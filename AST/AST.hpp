﻿#ifndef AST_HPP
#define AST_HPP

#include <list>
#include <string>
#include <initializer_list>

#include "../Token/Token.hpp"

namespace C0Compiler
{

	// opća klasa za AST, da ih mogu sve pobacati u jedan container
	// 21.12.2018. ideja je bila: privatni konstruktori ovdje i friendly factory u parseru,
	// ali nešto me zeza i trenutno nemam internet da provjerim šta mu je

	// 26.12.2018. deque ne prikazuje stablastu strukturu dovoljno dobro, sad je svaki AST čvor u 
	// jednom ogromnom globalnom AST i zna tko mu je roditelj, tko djeca i tko je veliki korijen.
	// zasad koristim list jer mi ne treba ništa jače

	// obavezno dodati CCtor, MCtor, OP= i destruktor! zasad memorija curi na sve strane!

	class AST
	{
		private:
			static AST* m_korijen;				// veliki korijen
			AST* m_roditelj;					// pointer na roditelja, nullptr ako se radi o korijenu (program)

		protected:
			std::list<AST*> m_djeca;			// lista djece, pointeri su zato da izbjegnem slicing

			AST() : m_djeca() {}
			AST(std::list<AST*>&& djeca);

		public:
			void dodajDijete(AST* dijete);		// dodaje granu u AST (čitaj: push_back u listu djece)
			void setKorijen(AST* korijen) { m_korijen = korijen; }
			void setRoditelj(AST* roditelj) { m_roditelj = roditelj; }

			AST* getRoditelj() { return m_roditelj; }
			bool isRoot() { return m_roditelj == nullptr; }

			virtual void compiliraj() = 0;		// ovo će se možda drugačije zvati ubuduće, analogon "izvrši" s IP,
			virtual ~AST(){}					// osim što sad ne interpretiram, nego optimiziram i compiliram
	};

	class Program : public AST
	{
		public:
			Program() { setKorijen(this); setRoditelj(nullptr); }
			Program(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			
			virtual void compiliraj() override {/* compiliraj svu djecu */};
			virtual ~Program(){}
	};

	class UseDirektiva : public AST
	{
		protected:
			std::string path;

		public:
			UseDirektiva(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			
			virtual void compiliraj() override {/* compiliraj datoteku koja se nalazi na path*/};
			virtual ~UseDirektiva(){}
	};

	class DeklaracijaFunkcije : public AST
	{
		protected:
			std::string m_povratniTip;
			std::string m_ime;
			
		public:
			DeklaracijaFunkcije(std::list<AST*>&& djeca) : AST(std::move(djeca)){}
			
			virtual void compiliraj() override {/* compiliraj svoju djecu */};
			virtual ~DeklaracijaFunkcije(){}
	};

	class DefinicijaFunkcije : public DeklaracijaFunkcije
	{
		protected:
			ASTList* tijelo;

		public:
			DefinicijaFunkcije(std::list<AST*>&& djeca) : DeklaracijaFunkcije(std::move(djeca)) {}
			virtual void compiliraj() override {/* compiliraj svoju djecu */ };
			virtual ~DefinicijaFunkcije() {}
	};

	class Varijabla : public AST
	{
		protected:
			std::string m_tip;
			std::string m_ime;

		public:
			Varijabla(std::initializer_list<AST*> parametri);

			virtual void compiliraj() override {/* zapamti varijablu i njenu vrijednost */};
			virtual ~Varijabla(){}
	};

	class Leaf : public AST
	{
		protected:
			Token m_sadrzaj;

		public:
			explicit Leaf(Token const& sadrzaj) : AST(), m_sadrzaj(sadrzaj) {}
			operator Token() const { return m_sadrzaj; }

			virtual void compiliraj() override {};		// od svih praznih overrideova, samo ovaj treba ostati prazan
	};

	// "AST wrapper" za std::list<AST*>, zapravo je ovo najobičniji AST, koji
	// se pravi da je lista i da su njegova djeca zapravo njegovi elementi
	class ASTList : public AST
	{
		public:
			using iterator = std::list<AST*>::iterator;

			ASTList() : AST(){}
			void push_back(AST* novi) { m_djeca.push_back(novi); }
			virtual void compiliraj() override {/* compiliraj sve svoje elemente */};

			iterator begin() { return m_djeca.begin(); }
			iterator end() { return m_djeca.end(); }
	};
}

#endif