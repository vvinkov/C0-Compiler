#ifndef AST_HPP
#define AST_HPP

#include <list>
#include <string>
#include <initializer_list>

#include "../Token/Token.hpp"

namespace C0Compiler
{

	// opća klasa za AST, da ih mogu sve pobacati u jedan container

	// 26.12.2018. deque ne prikazuje stablastu strukturu dovoljno dobro, sad je svaki AST čvor u 
	// jednom ogromnom globalnom AST i zna tko mu je roditelj, tko djeca i tko je veliki korijen.
	// zasad koristim list jer mi ne treba ništa jače

	class AST
	{
		private:
			static AST* korijen;				// veliki, glavni korijen
			static std::list<AST*> sirocad;		// AST-ovi bez roditelja
			AST* m_roditelj;					// pointer na roditelja, nullptr ako se radi o korijenu (program)
			
			void ucitajDjecu(std::list<AST*> const& djeca);
			void pobrisiDjecu();

		protected:
			std::list<AST*> m_djeca;			// lista djece, pointeri su zato da izbjegnem slicing

			AST() : m_djeca() { sirocad.push_back(this); }
			AST(AST const& drugi) { ucitajDjecu(drugi.m_djeca); }
			AST(AST&& drugi);
			AST(std::list<AST*>&& djeca);

		public:
			void dodajDijete(AST* dijete);		// dodaje granu u AST (čitaj: push_back u listu djece)
			void setKorijen(AST* novi_korijen) { korijen = novi_korijen; }
			void setRoditelj(AST* roditelj);

			AST* getRoditelj() { return m_roditelj; }
			bool isRoot() { return m_roditelj == nullptr; }

			AST& operator=(AST const& drugi);
			AST&& operator=(AST&& drugi);
			virtual void compiliraj() = 0;		// ovo će se možda drugačije zvati ubuduće, analogon "izvrši" s IP, osim što sad ne interpretiram, nego optimiziram i compiliram

			virtual ~AST();
	};

	class Program : public AST
	{
		public:
			Program() : AST() { setKorijen(this); setRoditelj(nullptr); }
			Program(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			Program(Program const& drugi) : AST(drugi) {}
			
			virtual void compiliraj() override {/* compiliraj svu djecu */};
			virtual ~Program(){}
	};

	class UseDirektiva : public AST
	{
		protected:
			std::string path;

		public:
			UseDirektiva(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			UseDirektiva(UseDirektiva const& drugi) : AST(drugi) { path = drugi.path; }
			
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
			DeklaracijaFunkcije(DeklaracijaFunkcije const& drugi) : AST(drugi) { m_povratniTip = drugi.m_povratniTip; m_ime = drugi.m_ime; }

			virtual void compiliraj() override {/* compiliraj svoju djecu */};
			virtual ~DeklaracijaFunkcije(){}
	};

	class DefinicijaFunkcije : public DeklaracijaFunkcije
	{
		public:
			DefinicijaFunkcije(std::list<AST*>&& djeca) : DeklaracijaFunkcije(std::move(djeca)) {}
			DefinicijaFunkcije(DefinicijaFunkcije const& drugi) : DeklaracijaFunkcije(drugi) {}
			
			virtual void compiliraj() override {/* compiliraj svoju djecu */ };
			virtual ~DefinicijaFunkcije() {}
	};

	class If : public AST
	{
		public:
			If(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			If(If const& drugi) : AST(drugi) {}
			
			virtual void compiliraj() override {/* compiliraj uvjet i tijelo */ }
			virtual ~If() {}
	};

	class IfElse : public If
	{
		public:
			IfElse(std::list<AST*>&& djeca) : If(std::move(djeca)) {}
			IfElse(IfElse const& drugi) : If(drugi) {}
			
			virtual void compiliraj() override {/* compiliraj if preko parenta, else sam */}
			virtual ~IfElse() {}
	};

	class While : public AST
	{
		public:
            While(std::list<AST*>&& djeca) : AST(std::move(djeca)){}
			While(While const& drugi) : AST(drugi) {}

            virtual void compiliraj() override {/*you know what to do*/}
            virtual ~While() {}
	};

	class For : public AST
	{
		public:
			For(std::list<AST*>&& djeca) : AST(std::move(djeca)){}
			For(For const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {}
			virtual ~For(){}
	};

	class Return : public AST
	{
		public:
			Return(std::list<AST*>&& djeca) : AST(std::move(djeca)){}
			Return(Return const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {}
			virtual ~Return(){}
	};

	// 08.01.2019. možda bi bilo dobro staviti da break i continue nasljeđuju od leaf
	class Break : public AST
	{
		public:
			Break() : AST(){}
			Break(Break const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {}
			virtual ~Break(){}
	};

	class Continue : public AST
	{
		public:
			Continue() : AST(){}
			Continue(Continue const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {}
			virtual ~Continue(){}
	};

	class Assert : public AST
	{
		public:
			Assert(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			Assert(Assert const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {}
			virtual ~Assert(){}
	};

	class Error : public AST
	{
		public:
			Error(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			Error(Error const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {}
			virtual ~Error(){}
	};

	class Varijabla : public AST
	{
		protected:
			std::string m_tip;
			std::string m_ime;

		public:
			Varijabla(std::list<AST*>&& djeca) : AST(std::move(djeca)){};
			Varijabla(Varijabla const& drugi) : AST(drugi) { m_tip = drugi.m_tip; m_ime = drugi.m_ime; }

			virtual void compiliraj() override {/* radi nešto s varijablom */};
			virtual ~Varijabla(){}
	};

	class DeklaracijaVarijable : public Varijabla
	{
		protected:
			std::string m_desno;

		public:
			DeklaracijaVarijable(std::list<AST*>&& djeca) : Varijabla(std::move(djeca)){}
			DeklaracijaVarijable(DeklaracijaVarijable const& drugi) : Varijabla(drugi) { m_desno = drugi.m_desno; }

			virtual void compiliraj() override {/* zapamti varijablu i njenu vrijednost */ };
			virtual ~DeklaracijaVarijable() {}
	};

	class TernarniOperator : public AST
	{
		public:
			TernarniOperator(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			TernarniOperator(TernarniOperator const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {/* ternarno operiraj */};
			virtual ~TernarniOperator() {}
	};

	class LogickiOperator : public AST
	{
		public:
			LogickiOperator(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			LogickiOperator(LogickiOperator const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {/* logicki operiraj */};
			virtual ~LogickiOperator() {}
	};

	class BitwiseOperator : public AST
	{
		public:
			BitwiseOperator(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			BitwiseOperator(BitwiseOperator const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {/* bitwise operiraj */};
			virtual ~BitwiseOperator() {}
	};

	class OperatorJednakost : public AST
	{
		public:
			OperatorJednakost(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			OperatorJednakost(OperatorJednakost const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {/* operiraj jednakost */};
			virtual ~OperatorJednakost() {}
	};

	class OperatorUsporedbe : public AST
	{
		public:
			OperatorUsporedbe(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			OperatorUsporedbe(OperatorUsporedbe const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {/* operiraj usporedbu */};
			virtual ~OperatorUsporedbe() {}
	};

	class BinarniOperator : public AST
	{
		public:
			BinarniOperator(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			BinarniOperator(BinarniOperator const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {/* binarno operiraj */ };
			virtual ~BinarniOperator() {}
	};

	class OperatorPridruzivanja : public AST
	{
		public:
			OperatorPridruzivanja(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			OperatorPridruzivanja(OperatorPridruzivanja const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {/* pridruži */ };
			virtual ~OperatorPridruzivanja() {}
	};

	class Alokacija : public AST
	{
		public:
			Alokacija(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			Alokacija(Alokacija const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {/* alociraj */ };
			virtual ~Alokacija() {}
	};

	class AlokacijaArray : public AST
	{
		public:
			AlokacijaArray(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			AlokacijaArray(AlokacijaArray const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {/* alociraj array */ };
			virtual ~AlokacijaArray() {}
	};

	class Negacija : public AST
	{
		public:
			Negacija(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			Negacija(Negacija const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {/* negiraj */ };
			virtual ~Negacija() {}
	};

	class Tilda : public AST
	{
		public:
			Tilda(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			Tilda(Tilda const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {/* iztildači */ };
			virtual ~Tilda() {}
	};

	class Minus : public AST
	{
		public:
			Minus(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			Minus(Minus const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {/* promijeni predznak */ };
			virtual ~Minus() {}
	};

	class Dereferenciranje : public AST
	{
		public:
			Dereferenciranje(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			Dereferenciranje(Dereferenciranje const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {/* dereferenciraj */ };
			virtual ~Dereferenciranje() {}
	};

	class PozivFunkcije : public AST
	{
		public:
			PozivFunkcije(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			PozivFunkcije(PozivFunkcije const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {/* pozovi funkciju */ };
			virtual ~PozivFunkcije() {}
	};

	class Inkrement : public AST
	{
		public:
			Inkrement(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			Inkrement(Inkrement const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {/* inkrementiraj */ };
			virtual ~Inkrement() {}
	};

	class Dekrement : public AST
	{
		public:
			Dekrement(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			Dekrement(Dekrement const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {/* dekrementiraj */ };
			virtual ~Dekrement() {}
	};

	class UglateZagrade : public AST
	{
		public:
			UglateZagrade(std::list<AST*>&& djeca) : AST(std::move(djeca)) {}
			UglateZagrade(UglateZagrade const& drugi) : AST(drugi) {}

			virtual void compiliraj() override {/* dohvati što je u uglatim zagradama */ };
			virtual ~UglateZagrade() {}
	};

	class Leaf : public AST
	{
		protected:
			Token m_sadrzaj;

		public:
			explicit Leaf(Token const& sadrzaj) : AST(), m_sadrzaj(sadrzaj) {}
			Leaf(Leaf const& drugi) : AST(drugi) { m_sadrzaj = drugi.m_sadrzaj; }

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
			ASTList(ASTList const& drugi) : AST(drugi) {}
			
			void push_back(AST* novi) { dodajDijete(novi); }	// premišljam se da ostavim dodajDijete ili da napišem novu funkciju koja dodaje dijete i kao parenta mu stavi svog parenta umjesto sebe
			virtual void compiliraj() override {/* compiliraj sve svoje elemente */};

			iterator begin() { return m_djeca.begin(); }
			iterator end() { return m_djeca.end(); }
	};
}

#endif