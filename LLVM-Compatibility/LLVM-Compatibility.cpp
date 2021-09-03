#include "LLVM-Compatibility.hpp"
#include "../AST/AST.hpp"
#include "../Greska/Greska.hpp"

namespace C0Compiler
{
	namespace LLVMCompatibility
	{
		Block* Block::trenutniBlok = nullptr;
		// u aliase tipova spremimo dijagonalu

		llvm::Value* Block::Trazi(std::string const& imeVarijable)
		{
			if (m_varijable.find(imeVarijable) != m_varijable.end())
				return m_varijable[imeVarijable];

			else if (m_roditelj != nullptr)
				return m_roditelj->Trazi(imeVarijable);

			else
				return nullptr;
		}

		std::string const& Block::Trazi(llvm::Value* varijabla)
		{
			// traženje imena varijable je zeznuto jer LLVM koristi SSA.
			// ime varijable koju smo dobili kao argument sadži ime originalne varijable
			// kojem je konkateniran neki broj. kao takvo, nećemo ga nikad moći naći s
			// jednostavnim find(), nego se moramo malo pomučiti. ne spremamo rezultate
			// ni u kakav cache jer su ionako sva imena jednokratna zbog SSA

			// trčimo po svim lokalnim varijablama
			for (std::map<std::string, llvm::Value*>::iterator it = m_varijable.begin(); it != m_varijable.end(); ++it)
			{
				// i tražimo one čije ime je početni komad imena varijable arugmenta
				if (varijabla->getName().find(it->first) == 0)
				{
					// za one za koje to vrijedi, tražimo onu koja ima usera s istim imenom kao varijabla argument
					for (llvm::Value::use_iterator jt = it->second->use_begin(); jt != it->second->use_end(); ++jt)
					{
						// ako smo je našli, vratimo ime lokalne varijable
						if (jt->getUser()->getName() == varijabla->getName())
							return it->first;
					}
				}
			}
			
			// ako nijedna lokalna varijabla nema ime koje je početni komad varijable argumenta ili nijedna nema 
			// usera s istim imenom kao varijabla argument, tražimo među lokalnim varijablama roditelja (ako postoji)
			if(m_roditelj != nullptr)
				return m_roditelj->Trazi(varijabla);

			// ako nismo našli ime varijable i roditelj ne postoji, vraćamo pazan string (zapravo nevažeću referencu)
			return std::string();
		}

		llvm::Value* Block::Dodaj(llvm::Value* varijabla, std::string const& ime)
		{
			if (m_varijable.find(ime) != m_varijable.end())
				return nullptr;

			m_varijable[ime] = varijabla;

			return varijabla;
		}

		Context::Context() : 
			m_context(std::make_unique<llvm::LLVMContext>()),
			m_modul(std::make_unique<llvm::Module>("main", *m_context.get())),
			m_irBuilder(std::make_unique<llvm::IRBuilder<>>(*m_context)),
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
			
			// stringove implementiramo kao struct{int, char*, bool}, ali se ponašamo kao da nisu strukture
			m_poznatiTipovi["string"] = llvm::StructType::create(*m_context, "string");
			std::vector<llvm::Type*> tempTijelo;
			tempTijelo.push_back(llvm::Type::getInt32Ty(*m_context));
			tempTijelo.push_back(llvm::Type::getInt8PtrTy(*m_context));
			tempTijelo.push_back(llvm::Type::getInt1Ty(*m_context));
			static_cast<llvm::StructType*>(m_poznatiTipovi["string"])->setBody(tempTijelo);
			tempTijelo.clear();

			m_poznatiTipovi["int*"] = llvm::Type::getInt32PtrTy(*m_context);
			m_poznatiTipovi["bool*"] = llvm::Type::getInt1PtrTy(*m_context);
			m_poznatiTipovi["char*"] = llvm::Type::getInt8PtrTy(*m_context);
			m_poznatiTipovi["string*"] = llvm::PointerType::getUnqual(m_poznatiTipovi["string"]);

			// polja implementiramo kao struct{int, <tip>*}*, jer LLVM-ovi ArrayType-ovi imaju
			// fiksnu veličinu. slično kao za stringove, pravimo se da je to normalno i da nisu 
			// pointeri na strukture. biram pointere jer se dinamički alociraju
			llvm::StructType* tempTip = llvm::StructType::create(*m_context, "int[]");
			tempTijelo.push_back(llvm::Type::getInt32Ty(*m_context));
			tempTijelo.push_back(llvm::Type::getInt32PtrTy(*m_context));
			static_cast<llvm::StructType*>(tempTip)->setBody(tempTijelo);
			m_poznatiTipovi["int[]"] = llvm::PointerType::getUnqual(tempTip);
			tempTijelo.clear();

			tempTip = llvm::StructType::create(*m_context, "char[]");
			tempTijelo.push_back(llvm::Type::getInt32Ty(*m_context));
			tempTijelo.push_back(llvm::Type::getInt8PtrTy(*m_context));
			static_cast<llvm::StructType*>(tempTip)->setBody(tempTijelo);
			m_poznatiTipovi["char[]"] = llvm::PointerType::getUnqual(tempTip);
			tempTijelo.clear();

			tempTip = llvm::StructType::create(*m_context, "bool[]");
			tempTijelo.push_back(llvm::Type::getInt32Ty(*m_context));
			tempTijelo.push_back(llvm::Type::getInt1PtrTy(*m_context));
			static_cast<llvm::StructType*>(tempTip)->setBody(tempTijelo);
			m_poznatiTipovi["bool[]"] = llvm::PointerType::getUnqual(tempTip);
			tempTijelo.clear();

			tempTip = llvm::StructType::create(*m_context, "string[]");
			tempTijelo.push_back(llvm::Type::getInt32Ty(*m_context));
			tempTijelo.push_back(m_poznatiTipovi["string"]);
			static_cast<llvm::StructType*>(tempTip)->setBody(tempTijelo);
			m_poznatiTipovi["string[]"] = llvm::PointerType::getUnqual(tempTip);
			tempTijelo.clear();

			m_imenaTipova[llvm::Type::getVoidTy(*m_context)] = "void";
			m_imenaTipova[llvm::Type::getInt32Ty(*m_context)] = "int";
			m_imenaTipova[llvm::Type::getInt1Ty(*m_context)] = "bool";
			m_imenaTipova[llvm::Type::getInt8Ty(*m_context)] = "char";
			m_imenaTipova[m_poznatiTipovi["string"]] = "string";
			
			m_imenaTipova[llvm::Type::getInt32PtrTy(*m_context)] = "int*";
			m_imenaTipova[llvm::Type::getInt1PtrTy(*m_context)] = "bool*";
			m_imenaTipova[llvm::Type::getInt8PtrTy(*m_context)] = "char*";
			m_imenaTipova[llvm::PointerType::getUnqual(m_poznatiTipovi["string"])] = "string*";

			m_imenaTipova[m_poznatiTipovi["int[]"]] = "int[]";
			m_imenaTipova[m_poznatiTipovi["char[]"]] = "char[]";
			m_imenaTipova[m_poznatiTipovi["bool[]"]] = "bool[]";
			m_imenaTipova[m_poznatiTipovi["string[]"]] = "string[]";
		}

		void Context::DodajBlok(llvm::BasicBlock* noviBlok)
		{
			m_blokovi.push_back(std::make_shared<Block>());
			m_blokovi.back()->Sadrzaj(noviBlok);
			m_blokovi.back()->Roditelj(Block::TrenutniBlok());
			Block::TrenutniBlok(m_blokovi.back().get());
		}

		void Context::GenerirajKod(Program& korijenStabla)
		{
			// prazan vector jer funkcija main ne prima argumente
			std::vector<llvm::Type*> tipoviArgumenata;

			// main je tipa int(void)
			llvm::FunctionType* tipMaina = llvm::FunctionType::get(PrevediTip("int"), tipoviArgumenata, false);

			// sad kad imamo tip i argumente, kreiramo funkciju main
			m_funkcijaMain = llvm::Function::Create(tipMaina, llvm::GlobalValue::ExternalLinkage, "main", m_modul.get());

			// dodamo blok za main i pridružimo ga funkciji
			//llvm::BasicBlock* mainBlok = llvm::BasicBlock::Create(*m_context, "pocetakMain", m_funkcijaMain, 0);
			//DodajBlok(mainBlok);

			korijenStabla.GenerirajKod();
			//IzbrisiBlok();
		}

		void Context::DodajStrukturu(std::string const& imeStrukture)
		{ 
			// inicijaliziramo praznu strukturu
			m_strukture[imeStrukture] = std::map<std::string, int>(); 
		}

		void Context::DodajClanStrukture(std::string const& imeStrukture, std::string const& imeClana)
		{
			// zapamtimo indeks elementa imeElementa u strukturi imeStrukture jer LLVM
			// ne zna da elementi struktura imaju imena, nego ih inače pamti po indeksu
			m_strukture[imeStrukture][imeClana] = m_strukture[imeStrukture].size();
		}
		
		void Context::DodajNoviTip(std::string const& imeTipa, llvm::Type* tip)
		{
			// generiramo tipove tip* i tip[]
			llvm::PointerType* pointerTip = llvm::PointerType::getUnqual(tip);
			llvm::StructType* arrayTip = llvm::StructType::create(*m_context, imeTipa + "[]");
			
			std::vector<llvm::Type*> tempTijelo;
			tempTijelo.push_back(llvm::Type::getInt32Ty(*m_context));
			tempTijelo.push_back(pointerTip);
			arrayTip->setBody(tempTijelo);
			tempTijelo.clear();

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

		int Context::DohvatiIndeksClanaStrukture(std::string const& imeStrukture, std::string const& imeClana)
		{
			if (m_strukture.find(imeStrukture) == m_strukture.end())
				// ako nema strukture s danim imenom, vratimo -1
				return -1;

			std::map<std::string, int> const& struktura = m_strukture[imeStrukture];
			if (struktura.find(imeClana) == struktura.end())
				// ako struktura nema traženi element, vratimo -2
				return -2;

			// inače samo vratimo element
			return m_strukture[imeStrukture][imeClana];
		}

		llvm::Value* Context::DohvatiClanStrukture(llvm::Value* struktura, int indeksClana)
		{
			// prevedemo indeks u LLVM jezik i dohvatimo strukturu
			llvm::Value* indeks = llvm::ConstantInt::get(*Context().LLVMContext(), llvm::APInt(32, indeksClana, true));

			// dohavimo tip člana strukture kojeg dohvaćamo
			llvm::StructType* tipStrukture = llvm::dyn_cast<llvm::StructType>(struktura->getType());
			llvm::Type* tipClana = tipStrukture->getElementType(indeksClana);

			std::vector<llvm::Value*> indeksi(2);
			// prvi indeks uvijek mora biti 0 jer LLVM strukture pamti kao struct* pa moramo prvo dereferencirati taj pointer
			indeksi[0] = llvm::ConstantInt::get(*Context().LLVMContext().get(), llvm::APInt(32, 0, true));

			// a drugi je zapravo indeks člana kojeg dohvaćamo
			indeksi[1] = indeks;

			// dohvatimo pointer na član
			llvm::Value* clanStrukture = Builder()->CreateGEP(struktura, indeksi, "pointerNaClanStrukture");

			// i onda dohvatimo element
			return Builder()->CreateLoad(clanStrukture, "elementStrukture");
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
