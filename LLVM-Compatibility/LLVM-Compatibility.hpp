#ifndef LLVM_COMPATIBILITY_HPP
#define LLVM_COMPATIBILITY_HPP

#include "../LLVM/include/llvm-c/Core.h"
#include "../LLVM/include/llvm/IR/Type.h"
#include "../LLVM/include/llvm/IR/IRBuilder.h"
#include "../LLVM/include/llvm/IR/LLVMContext.h"
#include "../LLVM/include/llvm/ExecutionEngine/GenericValue.h"
#include "../LLVM/include/llvm/IR/Instructions.h"

#include <deque>
#include <memory>

// funkcije i klase za urednije baratanje LLVM-om

namespace C0Compiler
{
	class AST;
	namespace LLVMCompatibility
	{
		//static llvm::LLVMContext& AST::Context() { return  *llvm::unwrap(LLVMGetGlobalContext()); }

		class Blok
		{
			private:
				llvm::BasicBlock* m_sadrzaj;
				Blok* m_roditelj;
				std::map<std::string, llvm::AllocaInst*> m_varijable;
				static Blok* trenutniBlok;	// pointer na zadnji otvoreni i nezatvoreni blok

			public:
				void Sadrzaj(llvm::BasicBlock* sadrzaj) { m_sadrzaj = sadrzaj; }
				llvm::BasicBlock* Sadrzaj() { return m_sadrzaj; }
				std::map<std::string, llvm::AllocaInst*>& Varijable() { return m_varijable; }

				Blok* Roditelj() const { return m_roditelj; }
				void Roditelj(Blok* roditelj) { m_roditelj = roditelj; }
				llvm::AllocaInst* Trazi(std::string const& imeVarijable);

				static Blok* TrenutniBlok() { return trenutniBlok; }
				static void TrenutniBlok(Blok* noviTrenutni) { trenutniBlok = noviTrenutni; }
		};
		
		class Context
		{
			private:
				std::deque<std::shared_ptr<Blok>> m_blokovi;
				std::unique_ptr<llvm::LLVMContext> m_context;
				std::unique_ptr<llvm::Module> m_modul;
				std::unique_ptr<llvm::IRBuilder<>> m_irBuilder;

				//std::unique_ptr<std::map<llvm::Value*, std::string>> m_tipoviPointera;

				std::map<std::string, std::map<std::string, int>> m_strukture;
				std::map<std::string, llvm::Type*> m_poznatiTipovi;
				std::map<llvm::Type*, std::string> m_imenaTipova;
				std::map<std::string, std::string> m_aliasiTipova;
				llvm::Function* m_funkcijaMain;

			public:
				// kreiramo novi LLVMContext i Module te ih spremamo u ovaj custom LLVMContext
				Context();
				
				void DodajBlok(llvm::BasicBlock* noviBlok);
				void ZatvoriBlok() { Blok::TrenutniBlok(Blok::TrenutniBlok()->Roditelj()); }
				void GenerirajKod(AST& korijenStabla);
				void IzbrisiBlok() { m_blokovi.pop_back(); }
				void DodajStrukturu(std::string const& imeStrukture);
				void DodajElementStrukture(std::string const& imeStrukture, std::string const& imeElementa);
				void DodajNoviTip(std::string const& imeTipa, llvm::Type* tip);
				
				int DohvatiIndeksElementaStrukture(std::string const& imeStrukture, std::string const& imeElementa) { return m_strukture[imeStrukture][imeElementa]; }

				std::map<std::string, llvm::AllocaInst*> LokalneVarijable() { return m_blokovi.back()->Varijable(); }
				std::map<std::string, std::string>& AliasiTipova() { return m_aliasiTipova; }
				Blok* TrenutniBlok() { return Blok::TrenutniBlok(); }
				std::unique_ptr<llvm::Module>& Module() { return m_modul; }
				std::unique_ptr<llvm::IRBuilder<>>& Builder() { return m_irBuilder; }
				std::unique_ptr<llvm::LLVMContext>& LLVMContext() { return m_context; }
				//std::unique_ptr<std::map<llvm::Value*, std::string>>& TipoviPointera() { return m_tipoviPointera; }

				// PrevediTip funkcije prevode tip iz stringa u LLVM tip i natrag u string
				llvm::Type* PrevediTip(std::string const& tip);
				std::string const& PrevediTip(llvm::Type* tip) { return m_imenaTipova[tip]; }

				llvm::AllocaInst* StackAlokacija(llvm::Function* funkcija, llvm::Type* tipVarijable, std::string const& imeVarijable);
				//llvm::GenericValue PokreniKod();
		};
	}
}

#endif