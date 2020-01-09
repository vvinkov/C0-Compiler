#include "LLVM-Compatibility.hpp"
#include "../AST/AST.hpp"

namespace C0Compiler
{
	namespace LLVMCompatibility
	{
		Blok* Blok::trenutniBlok = nullptr;
		// u aliase tipova spremimo dijagonalu

		llvm::AllocaInst* Blok::Trazi(std::string const& imeVarijable)
		{
			if (m_varijable.find(imeVarijable) != m_varijable.end())
				return m_varijable[imeVarijable];

			else if (m_roditelj != nullptr)
				return m_roditelj->Trazi(imeVarijable);

			else
				return nullptr;
		}

		Context::Context() : 
			m_context(std::make_unique<llvm::LLVMContext>()),
			m_modul(std::make_unique<llvm::Module>("main", *m_context.get())),
			m_aliasiTipova(
				{
					// svaki tip je ujedno i sam sebi alias
					{"int", "int"},
					{"char", "char"},
					{"bool", "bool"},
					{"string", "string"},
					{"void", "void"},
					{"int*", "int*"},
					{"char*", "char*"},
					{"bool*", "bool*"},
					{"string*", "string*"},
					{"int[]", "int[]"},
					{"char[]", "char[]"},
					{"bool[]", "bool[]"},
					{"string[]", "string[]"}
				}
			)
		{
			m_poznatiTipovi["void"] = llvm::Type::getVoidTy(*m_context);
			m_poznatiTipovi["int"] = llvm::Type::getInt32Ty(*m_context);
			m_poznatiTipovi["bool"] = llvm::Type::getInt1Ty(*m_context);
			m_poznatiTipovi["char"] = llvm::Type::getInt8Ty(*m_context);
			// da bismo razlikovali string od char[], tip string karakteriziramo kao polje charova duljine 1
			m_poznatiTipovi["string"] = llvm::ArrayType::get(m_poznatiTipovi["char"], 1);
			
			m_poznatiTipovi["int*"] = llvm::Type::getInt32PtrTy(*m_context);
			m_poznatiTipovi["char*"] = llvm::Type::getInt8PtrTy(*m_context);
			m_poznatiTipovi["bool*"] = llvm::Type::getInt1PtrTy(*m_context);
			m_poznatiTipovi["string*"] = llvm::PointerType::getUnqual(m_poznatiTipovi["string"]);
			m_poznatiTipovi["int[]"] = llvm::ArrayType::get(m_poznatiTipovi["int"], 0);
			m_poznatiTipovi["char[]"] = llvm::ArrayType::get(m_poznatiTipovi["char"], 0);
			m_poznatiTipovi["bool[]"] = llvm::ArrayType::get(m_poznatiTipovi["bool"], 0);
			m_poznatiTipovi["string[]"] = llvm::ArrayType::get(m_poznatiTipovi["string"], 0);
		}

		void Context::DodajBlok(llvm::BasicBlock* noviBlok)
		{
			m_blokovi.push_back(std::make_shared<Blok>());
			m_blokovi.back()->Sadrzaj(noviBlok);
			m_blokovi.back()->Roditelj(Blok::TrenutniBlok());
			Blok::TrenutniBlok(m_blokovi.back().get());
		}

		void Context::GenerirajKod(AST& korijenStabla)
		{
			// prazan vector jer funkcija main ne prima argumente
			std::vector<llvm::Type*> tipoviArgumenata;

			// main je tipa void()
			llvm::FunctionType* tipMaina = llvm::FunctionType::get(PrevediTip("void"), tipoviArgumenata, false);

			// sad kad imamo tip i argumente, kreiramo funkciju main
			m_funkcijaMain = llvm::Function::Create(tipMaina, llvm::GlobalValue::InternalLinkage, "main", m_modul.get());

			// dodamo blok za main i pridružimo ga funkciji
			llvm::BasicBlock* mainBlok = llvm::BasicBlock::Create(*m_context, "pocetakMain", m_funkcijaMain, 0);
			DodajBlok(mainBlok);

			korijenStabla.GenerirajKodIzraz();
			IzbrisiBlok();
		}

		void Context::DodajStrukturu(std::string const& imeStrukture)
		{ 
			// inicijaliziramo praznu strukturu
			m_strukture[imeStrukture] = std::map<std::string, int>(); 
		}

		void Context::DodajElementStrukture(std::string const& imeStrukture, std::string const& imeElementa)
		{
			// zapamtimo indeks elementa imeElementa u strukturi imeStrukture jer LLVM
			// ne zna da elementi struktura imaju imena; pamti ih po indeksu
			m_strukture[imeStrukture][imeElementa] = m_strukture[imeStrukture].size();
		}
		
		void Context::DodajNoviTip(std::string const& imeTipa, llvm::Type* tip)
		{
			// generiramo tipove tip* i tip[]
			llvm::PointerType* pointerTip = llvm::PointerType::getUnqual(tip);
			llvm::ArrayType* arrayTip = llvm::ArrayType::get(tip, 0);

			// dodamo tip, tip* i tip[] u poznate tipove
			m_poznatiTipovi[imeTipa] = tip; 
			m_poznatiTipovi[imeTipa + "*"] = pointerTip;
			m_poznatiTipovi[imeTipa + "[]"] = arrayTip;

			// dodamo mogućnost prevođenja llvm::Type u string za tip, tip* i tip[] 
			m_imenaTipova[tip] = imeTipa;
			m_imenaTipova[pointerTip] = imeTipa + "*";
			m_imenaTipova[arrayTip] = imeTipa + "[]";
			
			// i dodamo tip, tip* i tip[] u aliase tipova
			m_aliasiTipova[imeTipa] = imeTipa;
			m_aliasiTipova[imeTipa + "*"] = imeTipa + "*";
			m_aliasiTipova[imeTipa + "[]"] = imeTipa + "[]";
		}

		llvm::Type* Context::PrevediTip(std::string const& tip)
		{
			// ako postoji alias tipa tip i ako je to stvarno alias nekog tipa
			if (m_aliasiTipova.find(tip) != m_aliasiTipova.end() && m_poznatiTipovi.find(m_aliasiTipova[tip]) != m_poznatiTipovi.end())
				// onda vrati taj tip
				return m_poznatiTipovi[m_aliasiTipova[tip]];

			// inače signaliziraj da nešto ne štima
			else
				return nullptr;
		}

		llvm::AllocaInst* Context::StackAlokacija(llvm::Function* funkcija, llvm::Type* tipVarijable, std::string const& imeVarijable)
		{
			llvm::IRBuilder<> tempBuilder(&funkcija->getEntryBlock(), funkcija->getEntryBlock().begin());
			return tempBuilder.CreateAlloca(tipVarijable, 0, imeVarijable);
		}
	}
}