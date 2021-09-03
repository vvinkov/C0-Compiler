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
	class Program;
	namespace LLVMCompatibility
	{
		//static llvm::LLVMContext& AST::Context() { return  *llvm::unwrap(LLVMGetGlobalContext()); }

		class Block
		{
			private:
				llvm::BasicBlock* m_sadrzaj;
				Block* m_roditelj;
				std::map<std::string, llvm::Value*> m_varijable;

				static Block* trenutniBlok;	// pointer na zadnji otvoreni i nezatvoreni blok

			public:
				void Sadrzaj(llvm::BasicBlock* sadrzaj) { m_sadrzaj = sadrzaj; }
				llvm::BasicBlock* Sadrzaj() { return m_sadrzaj; }
				std::map<std::string, llvm::Value*>& Varijable() { return m_varijable; }

				Block* Roditelj() const { return m_roditelj; }
				void Roditelj(Block* roditelj) { m_roditelj = roditelj; }
				llvm::Value* Trazi(std::string const& imeVarijable);
				std::string const& Trazi(llvm::Value* varijabla);
				llvm::Value* Dodaj(llvm::Value* varijabla, std::string const& ime);

				static Block* TrenutniBlok() { return trenutniBlok; }
				static void TrenutniBlok(Block* noviTrenutni) { trenutniBlok = noviTrenutni; }
		};
		
		class Context
		{
			private:
				std::deque<std::shared_ptr<Block>> m_blokovi;
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
				void ZatvoriBlok() { Block::TrenutniBlok(Block::TrenutniBlok()->Roditelj()); }
				void GenerirajKod(Program& korijenStabla);
				void IzbrisiBlok() { m_blokovi.pop_back(); }
				void DodajStrukturu(std::string const& imeStrukture);
				void DodajClanStrukture(std::string const& imeStrukture, std::string const& imeClana);
				void DodajNoviTip(std::string const& imeTipa, llvm::Type* tip);
				
				int DohvatiIndeksClanaStrukture(std::string const& imeStrukture, std::string const& imeClana);
				llvm::Value* DohvatiClanStrukture(llvm::Value* struktura, int indeksClana);
				bool StrukturaDefinirana(std::string const& imeStrukture) { return m_strukture.find(imeStrukture) != m_strukture.end() && !m_strukture[imeStrukture].empty(); }
				std::map<std::string, llvm::Value*>& LokalneVarijable() { return TrenutniBlok()->Varijable(); }
				std::map<std::string, std::string>& AliasiTipova() { return m_aliasiTipova; }
				Block* TrenutniBlok() { return Block::TrenutniBlok(); }
				std::unique_ptr<llvm::Module>& Module() { return m_modul; }
				std::unique_ptr<llvm::IRBuilder<>>& Builder() { return m_irBuilder; }
				std::unique_ptr<llvm::LLVMContext>& LLVMContext() { return m_context; }

				// PrevediTip funkcije prevode tip iz stringa u LLVM tip i natrag u string
				llvm::Type* PrevediTip(std::string const& tip);
				std::string const& PrevediTip(llvm::Type* tip) { return m_imenaTipova[tip]; }

				llvm::AllocaInst* StackAlokacija(llvm::Function* funkcija, llvm::Type* tipVarijable, std::string const& imeVarijable);
				//llvm::GenericValue PokreniKod();
		};
	}
}

#endif
