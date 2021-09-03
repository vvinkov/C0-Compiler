#ifndef AST_HPP
#define AST_HPP

#include <list>
#include <string>
#include <fstream>
#include <initializer_list>
#include <memory>
#include <deque>

#include "../Token/Token.hpp"
#include "../LLVM-Compatibility/LLVM-Compatibility.hpp"

#include "../LLVM/include/llvm/IR/Value.h"
//#include "../LLVM/include/llvm/IR/LLVMContext.h"
#include "../LLVM/include/llvm/IR/Function.h"
#include "../LLVM/include/llvm/IR/Module.h"
#include "../LLVM/include/llvm/IR/IRBuilder.h"
#include "../LLVM/include/llvm/IR/Verifier.h"
#include "../LLVM/include/llvm/IR/Instructions.h"
#include "../LLVM/include/llvm/IR/Constants.h"

#include "../LLVM/include/llvm-c/Core.h"

namespace C0Compiler
{
	class Parser;
	class ASTList;
	// opća klasa za AST, da ih mogu sve pobacati u jedan container

	// svaki AST je čvor u 
	// jednom ogromnom globalnom AST i zna tko mu je roditelj, tko djeca i tko je korijen.
	// zasad koristim list jer mi ne treba ništa jače

	class AST : public std::enable_shared_from_this<AST>
	{
		private:
			static std::shared_ptr<AST> korijen;				// veliki, glavni korijen
			static std::list<std::shared_ptr<AST>> sirocad;		// AST-ovi bez roditelja

			int m_redak;			// redak prvog tokena u AST-u
			int m_stupac;			// stupac prvog tokena u AST-u
			AST* m_roditelj;		// pointer na roditelja, nullptr ako se radi o korijenu (program)
			
			void ucitajDjecu(std::list<std::shared_ptr<AST>> const& djeca);
			void pobrisiDjecu();


		protected:
			static LLVMCompatibility::Context context;

			AST(int redak, int stupac);
			AST(AST const& drugi);
			AST(AST&& drugi);
			AST(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);

			virtual void Ispisi(std::ostream& out) const = 0; // služi funkcija koju operator<< poziva, ispisuje ime AST-a i njegove djece
			void IspisiDjecu(std::ostream& out) const;

		public:
			std::list<std::shared_ptr<AST>> m_djeca;			// lista djece, pointeri su zato da izbjegnem slicing
			
			void dodajDijete(std::shared_ptr<AST> dijete);		// dodaje granu u AST (čitaj: push_back u listu djece)
			void Roditelj(AST* roditelj);
			static void setKorijen(std::shared_ptr<AST> novi_korijen) { korijen = novi_korijen; }

			AST* Roditelj() const { return m_roditelj; }
			int Redak() const { return m_redak; }
			int Stupac() const { return m_stupac; }

			bool IsRoot() const { return m_roditelj == nullptr; }

			AST& operator=(AST const& drugi);
			AST&& operator=(AST&& drugi);

			virtual llvm::Value* GenerirajKodIzraz() = 0;
			virtual llvm::Function* GenerirajKodFunkcija() = 0;
			virtual llvm::Type* GenerirajKodTip() = 0;

			static LLVMCompatibility::Context& Context() { return context; }
			static llvm::IRBuilder<>& Builder() { return *context.Builder(); }
			static std::unique_ptr<llvm::Module>& Module() { return context.Module(); }
			static std::shared_ptr<AST> Korijen() { return korijen; }

			void ProvjeraUvjeta(llvm::Value* uvjetProlaza, std::string const& opisIznimke = "nepoznata iznimka", bool assert = false);
			void ProvjeraVrijednosti(llvm::Value* vrijednost, std::string const& imeOperatora = "", int vrsta = 0);
			void ProvjeraVrijednosti(llvm::Value* prvaVrijednost, llvm::Value* drugaVrijednost, std::string const& imeOperatora = "");
			void ProvjeraTipa(llvm::Value* vrijednost, llvm::Type* ocekivaniTip);
			void ProvjeraPolja(llvm::Value* vrijednost);

			friend std::ostream& operator<<(std::ostream& f, AST const& ast);

			virtual ~AST();
	};
	
	// malo konkretnije klase, ali i dalje ne dovoljno konkretne.
	// služe za razlikovanje izraza, funkcija i tipova, jer ih LLVM razlikuje
	class IzrazAST : public AST 
	{
		public:
			IzrazAST(int redak, int stupac) : AST(redak, stupac) { /* setKorijen(shared_from_this()); */ }
			IzrazAST(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : AST(redak, stupac, std::move(djeca)) {}
			IzrazAST(IzrazAST const& drugi) : AST(drugi) {}
			
			llvm::Value* GenerirajKod() { return GenerirajKodIzraz(); }

			virtual llvm::Function* GenerirajKodFunkcija() override { return nullptr; }
			virtual llvm::Type* GenerirajKodTip() override { return nullptr; }
			
			virtual ~IzrazAST() {}
	};	

	class FunkcijaAST : public AST
	{
		public:
			FunkcijaAST(int redak, int stupac) : AST(redak, stupac) { /* setKorijen(shared_from_this()); */ }
			FunkcijaAST(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : AST(redak, stupac, std::move(djeca)) {}
			FunkcijaAST(FunkcijaAST const& drugi) : AST(drugi) {}

			llvm::Function* GenerirajKod() { return GenerirajKodFunkcija(); }

			virtual llvm::Type* GenerirajKodTip() override { return nullptr; }
			virtual llvm::Value* GenerirajKodIzraz() override { return nullptr; }

			virtual ~FunkcijaAST() {}
	};

	class TipAST : public AST
	{
		public:
			TipAST(int redak, int stupac) : AST(redak, stupac) { /* setKorijen(shared_from_this()); */ }
			TipAST(int redak, int stupac, std::list <std::shared_ptr<AST>>&& djeca) : AST(redak, stupac, std::move(djeca)) {}
			TipAST(TipAST const& drugi) : AST(drugi) {}

			llvm::Type* GenerirajKod() { return GenerirajKodTip(); }	

			virtual llvm::Function* GenerirajKodFunkcija() override { return nullptr; }
			virtual llvm::Value* GenerirajKodIzraz() override { return nullptr; }

			virtual ~TipAST() {}
	};

	// služi samo za globalni korijen AST-a; nije predviđeno
	// da se koristi igdje dalje
	class Program : public AST
	{
		public:
			Program();
			Program(int redak, int stupac, std::list <std::shared_ptr<AST>>&& djeca) = delete;
			Program(TipAST const& drugi) : AST(drugi) {}


			// trivijalni override-ovi čistih virtualnih funkcija
			virtual void Ispisi(std::ostream& out) const override { IspisiDjecu(out); }
			virtual llvm::Function* GenerirajKodFunkcija() override { return nullptr; }
			virtual llvm::Value* GenerirajKodIzraz() override { return nullptr; }
			virtual llvm::Type* GenerirajKodTip() override { return nullptr; }

			void GenerirajKod();
	};

	// napokon, skroz konkretne klase
	class UseDirektiva : public IzrazAST
	{
		private:
			std::string path;

		protected:
			// trivijalni override jer se ovo neće nikad ispisivati
			virtual void Ispisi(std::ostream& out) const override {}	

		public:
			UseDirektiva(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			UseDirektiva(UseDirektiva const& drugi);
			
			// use direktiva se mora izvršiti odmah, a ne tek kad prevođenje dođe na red. zato imamo izvrsi()
			std::deque<std::shared_ptr<Token>> izvrsi(Parser& parser);	
			virtual llvm::Value* GenerirajKodIzraz() override { return nullptr; } // do nothing
			virtual ~UseDirektiva(){}
	};

	class DeklaracijaStrukture : public TipAST
	{
		protected:
			std::string m_ime;

			virtual void Ispisi(std::ostream& out) const override;

		public:
			DeklaracijaStrukture(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			DeklaracijaStrukture(DeklaracijaStrukture const& drugi) : TipAST(drugi) {}

			virtual llvm::Type* GenerirajKodTip() override;
			virtual ~DeklaracijaStrukture() {}
	};
	
	class DefinicijaStrukture : public DeklaracijaStrukture
	{
		protected:
			std::vector<std::string> m_tipoviElemenata;
			std::vector<std::string> m_imenaElemenata;

			virtual void Ispisi(std::ostream& out) const override;

		public:
			DefinicijaStrukture(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			DefinicijaStrukture(DefinicijaStrukture const& drugi);

			virtual llvm::Type* GenerirajKodTip() override;
			virtual ~DefinicijaStrukture() {}
	};

	class DeklaracijaFunkcije : public FunkcijaAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			DeklaracijaFunkcije(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			DeklaracijaFunkcije(DeklaracijaFunkcije const& drugi);

			virtual llvm::Function* GenerirajKodFunkcija() override;
			virtual ~DeklaracijaFunkcije(){}
	};

	class DefinicijaFunkcije : public DeklaracijaFunkcije
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			DefinicijaFunkcije(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			DefinicijaFunkcije(DefinicijaFunkcije const& drugi);
			
			virtual llvm::Function* GenerirajKodFunkcija() override;
			virtual ~DefinicijaFunkcije() {}
	};

	class TypeDef : public IzrazAST
	{
		protected:
			std::string m_tip;
			std::string m_alias;

			virtual void Ispisi(std::ostream& out) const override;

		public:
			TypeDef(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			TypeDef(TypeDef const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override;
			virtual ~TypeDef() {}
	};

	class IfElse : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			IfElse(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			IfElse(IfElse const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override;
			virtual ~IfElse() {}
	};

	class While : public IzrazAST
	{
		private:
			llvm::BasicBlock* m_uvjetPetlje;
			llvm::BasicBlock* m_tijeloPetlje;
			llvm::BasicBlock* m_krajPetlje;

		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			While(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			While(While const& drugi);

			llvm::BasicBlock* UvjetPetlje() { return m_uvjetPetlje; }
			//llvm::BasicBlock* TijeloPetlje() {	return m_tijeloPetlje;	};
			llvm::BasicBlock* BlokNakonPetlje() { return m_krajPetlje; }

            virtual llvm::Value* GenerirajKodIzraz() override;
            virtual ~While() {}
	};

	class For : public IzrazAST
	{
		private:
			llvm::BasicBlock* m_krajPetlje;
			llvm::BasicBlock* m_inkrement;

		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			For(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			For(For const& drugi);

			llvm::BasicBlock* BlokNakonPetlje() { return m_krajPetlje; }
			llvm::BasicBlock* BlokInkrement() { return m_inkrement; }

			virtual llvm::Value* GenerirajKodIzraz() override;
			virtual ~For(){}
	};

	class Return : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			Return(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			Return(Return const& drugi);

			virtual llvm::Value* GenerirajKodIzraz();
			virtual ~Return(){}
	};

	class Break : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			Break(int redak, int stupac);
			Break(Break const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override; // placeholder
			virtual ~Break(){}
	};

	class Continue : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			Continue(int redak, int stupac);
			Continue(Continue const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override;
			virtual ~Continue(){}
	};

	class Assert : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			Assert(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			Assert(Assert const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override;
			virtual ~Assert(){}
	};
	
	class Varijabla : public IzrazAST
	{
		protected:
			std::string m_ime;
			virtual void Ispisi(std::ostream& out) const override;

		public:
			Varijabla(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			Varijabla(Varijabla const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override;
			virtual ~Varijabla(){}
	};

	class DeklaracijaVarijable : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			DeklaracijaVarijable(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			DeklaracijaVarijable(DeklaracijaVarijable const& drugi);

			//Varijabla const& getVarijabla() const { return *(std::dynamic_pointer_cast<Varijabla>)(m_djeca.front()); }	// poseban geter za ovo jer je odvratno
			virtual llvm::Value* GenerirajKodIzraz() override;
			virtual ~DeklaracijaVarijable() {}
	};

	class TernarniOperator : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			TernarniOperator(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			TernarniOperator(TernarniOperator const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override;
			virtual ~TernarniOperator() {}
	};

	class LogickiOperator : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			LogickiOperator(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			LogickiOperator(LogickiOperator const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override;
			virtual ~LogickiOperator() {}
	};

	// ovo su sad binarni operatori
	//class BitovniOperator : public IzrazAST
	//{
	//	protected:
	//		virtual void Ispisi(std::ostream& out) const override;

	//	public:
	//		BitovniOperator(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
	//		BitovniOperator(BitovniOperator const& drugi);

	//		virtual llvm::Value* GenerirajKodIzraz() override;
	//		virtual ~BitovniOperator() {}
	//};

	class OperatorJednakost : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			OperatorJednakost(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			OperatorJednakost(OperatorJednakost const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override; 
			virtual ~OperatorJednakost() {}
	};

	class OperatorUsporedbe : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			OperatorUsporedbe(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			OperatorUsporedbe(OperatorUsporedbe const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override; 
			virtual ~OperatorUsporedbe() {}
	};

	class BinarniOperator : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			BinarniOperator(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			BinarniOperator(BinarniOperator const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override; 
			virtual ~BinarniOperator() {}
	};

	class OperatorPridruzivanja : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			OperatorPridruzivanja(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			OperatorPridruzivanja(OperatorPridruzivanja const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override;
			virtual ~OperatorPridruzivanja() {}
	};

	class Alokacija : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			Alokacija(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			Alokacija(Alokacija const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override;
			virtual ~Alokacija() {}
	};

	class AlokacijaArray : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			AlokacijaArray(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			AlokacijaArray(AlokacijaArray const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override; 
			virtual ~AlokacijaArray() {}
	};

	class Negacija : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			Negacija(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			Negacija(Negacija const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override; 
			virtual ~Negacija() {}
	};

	class Tilda : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			Tilda(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			Tilda(Tilda const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override; 
			virtual ~Tilda() {}
	};

	class Minus : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			Minus(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			Minus(Minus const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override;
			virtual ~Minus() {}
	};

	class Dereferenciranje : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			Dereferenciranje(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			Dereferenciranje(Dereferenciranje const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override; 
			virtual ~Dereferenciranje() {}
	};

	class PozivFunkcije : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			PozivFunkcije(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			PozivFunkcije(PozivFunkcije const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override;
			virtual ~PozivFunkcije() {}
	};

	class Inkrement : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			Inkrement(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			Inkrement(Inkrement const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override;
			virtual ~Inkrement() {}
	};

	class Dekrement : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			Dekrement(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			Dekrement(Dekrement const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override;
			virtual ~Dekrement() {}
	};

	class UglateZagrade : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			UglateZagrade(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			UglateZagrade(UglateZagrade const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override; 
			virtual ~UglateZagrade() {}
	};

	class Tip : public TipAST
	{
		protected:
			std::string m_ime;
			virtual void Ispisi(std::ostream& out) const override;

		public:
			Tip(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			Tip(Tip const& drugi);
			
			operator std::string() { return m_ime; }

			virtual llvm::Type* GenerirajKodTip() override;

			virtual ~Tip() {}
	};

	class Tocka : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			Tocka(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			Tocka(Tocka const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override;
			virtual ~Tocka() {}
	};

	class Strelica : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			Strelica(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			Strelica(Strelica const& drugi);

			virtual llvm::Value* GenerirajKodIzraz() override;
			virtual ~Strelica() {}
	};

	class Leaf : public IzrazAST
	{
		protected:
			Token m_sadrzaj;
			virtual void Ispisi(std::ostream& out) const override;

		public:
			Leaf(Token const& sadrzaj);
			Leaf(Leaf const& drugi);

			operator Token() const { return m_sadrzaj; }
			std::string const& Sadrzaj() const { return m_sadrzaj.Sadrzaj(); }
			TokenTip TipTokena() const { return m_sadrzaj.Tip(); }

			virtual llvm::Value* GenerirajKodIzraz() override { return nullptr; };	// od svih praznih overrideova, samo ovaj treba ostati prazan. vedran iz budućnosti: treba li stvarno? DA. ne.
	};

	class IntLiteral : public Leaf
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			IntLiteral(Token const& sadrzaj);
			operator int() const { return std::stoi(m_sadrzaj.Sadrzaj()); }

			virtual llvm::Value* GenerirajKodIzraz() override;
	};

	class CharLiteral : public Leaf
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			CharLiteral(Token const& sadrzaj);
			operator char() const { return m_sadrzaj.Sadrzaj()[0]; }

			virtual llvm::Value* GenerirajKodIzraz() override;
	};

	class BoolLiteral : public Leaf
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			BoolLiteral(Token const& sadrzaj);
			operator bool() const;

			virtual llvm::Value* GenerirajKodIzraz() override;
	};

	class StringLiteral : public Leaf
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			StringLiteral(Token const& sadrzaj);
			operator std::string() const { return Sadrzaj(); }
			operator const char*() const { return Sadrzaj().c_str(); }

			virtual llvm::Value* GenerirajKodIzraz() override;
	};

	// AST "wrapper" za std::list<std::shared_ptr<AST>>; zapravo je ovo najobičniji AST, koji
	// se pravi da je lista i da su njegova djeca zapravo njegovi elementi
	class ASTList : public IzrazAST
	{
		protected:
			virtual void Ispisi(std::ostream& out) const override;

		public:
			using iterator = std::list<std::shared_ptr<AST>>::iterator;

			ASTList(int redak, int stupac);
			ASTList(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca);
			ASTList(ASTList const& drugi);

			void push_back(std::shared_ptr<AST> novi) { dodajDijete(novi); }
			virtual llvm::Value* GenerirajKodIzraz() override;

			iterator begin() { return m_djeca.begin(); }
			iterator end() { return m_djeca.end(); }
			bool empty() { return m_djeca.empty(); }
			size_t size() { return m_djeca.size(); }
			void pop_front() { m_djeca.pop_front(); }
	};
}

#endif
