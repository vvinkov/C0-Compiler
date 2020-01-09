#include <typeinfo>
#include <vector> // potreban zato što LLVM očekuje da su neke stvari poredane na slijednim memorijskim lokacijama

#include "../Lekser/Lekser.hpp"
#include "../Parser/Parser.hpp"
#include "AST.hpp"
#include "../LLVM-Compatibility/LLVM-Compatibility.hpp"

namespace C0Compiler
{
	////////////////////////////////////////////////////////////////////////////////////
	// AST

	LLVMCompatibility::Context AST::context;
	std::list<std::shared_ptr<AST>> AST::sirocad;
	std::shared_ptr<AST> AST::korijen = nullptr;

	void AST::ucitajDjecu(std::list<std::shared_ptr<AST>> const& djeca)
	{
		for (std::list<std::shared_ptr<AST>>::const_iterator it = djeca.begin(); it != djeca.end(); ++it)
		{
			std::shared_ptr<AST> novoDijete;

			// zapamti tip it-tog djeteta
			type_info const& tipDjeteta = typeid(**it);

			// i copy-konstruiraj dijete ispravnog tipa
			//if (tipDjeteta == typeid(Program))
			//	novoDijete = std::make_shared<Program>((Program const&)**it);

			if (tipDjeteta == typeid(UseDirektiva))
				novoDijete = std::make_shared<UseDirektiva>((UseDirektiva const&)**it);

			else if (tipDjeteta == typeid(DeklaracijaFunkcije))
				novoDijete = std::make_shared<DeklaracijaFunkcije>((DeklaracijaFunkcije const&)**it);

			else if (tipDjeteta == typeid(DefinicijaFunkcije))
				novoDijete = std::make_shared<DefinicijaFunkcije>((DefinicijaFunkcije const&)**it);

			else if (tipDjeteta == typeid(IfElse))
				novoDijete = std::make_shared<IfElse>((IfElse const&)**it);

			else if (tipDjeteta == typeid(While))
				novoDijete = std::make_shared<While>((While const&)**it);

			else if (tipDjeteta == typeid(For))
				novoDijete = std::make_shared<For>((For const&)**it);

			else if (tipDjeteta == typeid(Return))
				novoDijete = std::make_shared<Return>((Return const&)**it);

			else if (tipDjeteta == typeid(Break))
				novoDijete = std::make_shared<Break>((Break const&)**it);

			else if (tipDjeteta == typeid(Continue))
				novoDijete = std::make_shared<Continue>((Continue const&)**it);

			else if (tipDjeteta == typeid(Assert))
				novoDijete = std::make_shared<Assert>((Assert const&)**it);

			// moram provjeriti postoji li error kao takav uopće.
			// dotad je ovo zakomentirano
			//else if (tipDjeteta == typeid(Error)) 
			//	novoDijete = std::make_shared<Error((Error const&)**it);

			else if (tipDjeteta == typeid(Varijabla))
				novoDijete = std::make_shared<Varijabla>((Varijabla const&)**it);

			else if (tipDjeteta == typeid(DeklaracijaVarijable))
				novoDijete = std::make_shared<DeklaracijaVarijable>((DeklaracijaVarijable const&)**it);

			else if (tipDjeteta == typeid(TernarniOperator))
				novoDijete = std::make_shared<TernarniOperator>((TernarniOperator const&)**it);

			else if (tipDjeteta == typeid(LogickiOperator))
				novoDijete = std::make_shared<LogickiOperator>((LogickiOperator const&)**it);

			else if (tipDjeteta == typeid(BitovniOperator))
				novoDijete = std::make_shared<BitovniOperator>((BitovniOperator const&)**it);

			else if (tipDjeteta == typeid(OperatorJednakost))
				novoDijete = std::make_shared<OperatorJednakost>((OperatorJednakost const&)**it);

			else if (tipDjeteta == typeid(OperatorUsporedbe))
				novoDijete = std::make_shared<OperatorUsporedbe>((OperatorUsporedbe const&)**it);

			else if (tipDjeteta == typeid(BinarniOperator))
				novoDijete = std::make_shared<BinarniOperator>((BinarniOperator const&)**it);

			else if (tipDjeteta == typeid(OperatorPridruzivanja))
				novoDijete = std::make_shared<OperatorPridruzivanja>((OperatorPridruzivanja const&)**it);

			else if (tipDjeteta == typeid(Alokacija))
				novoDijete = std::make_shared<Alokacija>((Alokacija const&)**it);

			else if (tipDjeteta == typeid(AlokacijaArray))
				novoDijete = std::make_shared<AlokacijaArray>((AlokacijaArray const&)**it);

			else if (tipDjeteta == typeid(Negacija))
				novoDijete = std::make_shared<Negacija>((Negacija const&)**it);

			else if (tipDjeteta == typeid(Tilda))
				novoDijete = std::make_shared<Tilda>((Tilda const&)**it);

			else if (tipDjeteta == typeid(Minus))
				novoDijete = std::make_shared<Minus>((Minus const&)**it);

			else if (tipDjeteta == typeid(Dereferenciranje))
				novoDijete = std::make_shared<Dereferenciranje>((Dereferenciranje const&)**it);

			else if (tipDjeteta == typeid(PozivFunkcije))
				novoDijete = std::make_shared<PozivFunkcije>((PozivFunkcije const&)**it);

			else if (tipDjeteta == typeid(Inkrement))
				novoDijete = std::make_shared<Inkrement>((Inkrement const&)**it);

			else if (tipDjeteta == typeid(Dekrement))
				novoDijete = std::make_shared<Dekrement>((Dekrement const&)**it);

			else if (tipDjeteta == typeid(UglateZagrade))
				novoDijete = std::make_shared<UglateZagrade>((UglateZagrade const&)**it);

			else if (tipDjeteta == typeid(Leaf))
				novoDijete = std::make_shared<Leaf>((Leaf const&)**it);

			else if (tipDjeteta == typeid(ASTList))
				novoDijete = std::make_shared<ASTList>((ASTList const&)**it);

			else 
				// ovo se ne bi nikad trebalo dogoditi, ali ne želim da mi 
				// kompajler prigovara da mi nedostaje povratna vrijednost
				novoDijete = nullptr; 

			dodajDijete(novoDijete);
		}
	}

	void AST::pobrisiDjecu()
	{
		for (std::list<std::shared_ptr<AST>>::iterator it = m_djeca.begin(); it != m_djeca.end();)
			(it++)->reset();
	}

	AST::AST(int redak, int stupac) : m_djeca()
	{
		m_redak = redak;
		m_stupac = stupac;
		m_roditelj = nullptr;
		sirocad.push_back(std::move(std::shared_ptr<AST>(this)));
	}

	AST::AST(AST const& drugi)
	{
		m_redak = drugi.m_redak;
		m_stupac = drugi.m_stupac;
		m_roditelj = nullptr;
		ucitajDjecu(drugi.m_djeca);
		sirocad.push_back(std::move(std::shared_ptr<AST>(this)));
	}

	AST::AST(AST&& drugi) : m_djeca(std::move(drugi.m_djeca)) 
	{
		// svakom djetetu stavi sebe za roditelja
		for (std::list<std::shared_ptr<AST>>::iterator it = m_djeca.begin(); it != m_djeca.end(); ++it)
			(*it)->setRoditelj(std::move(std::shared_ptr<AST>(this)));

		m_redak = drugi.m_redak;
		m_stupac = drugi.m_stupac;
		m_roditelj = drugi.m_roditelj;
	}

	AST::AST(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : m_djeca(std::move(djeca))
	{
		// svakom svom djetetu stavi sebe za roditelja
		for (std::list<std::shared_ptr<AST>>::iterator it = djeca.begin(); it != djeca.end(); ++it)
			(*it)->setRoditelj(std::move(std::shared_ptr<AST>(this)));

		m_redak = redak;
		m_stupac = stupac;
		m_roditelj = nullptr;
		sirocad.push_back(std::move(std::shared_ptr<AST>(this)));
	}

	void AST::dodajDijete(std::shared_ptr<AST> dijete)
	{
		// dodaj si dijete
		m_djeca.push_back(dijete);

		// i reci tom djetetu da si mu sad ti roditelj
		m_djeca.back()->setRoditelj(std::move(std::shared_ptr<AST>(this)));
	}

	void AST::setRoditelj(std::shared_ptr<AST> roditelj)
	{
		// postavi roditelja
		m_roditelj = roditelj;

		// i izbriši se iz siročadi
		for (std::list<std::shared_ptr<AST>>::iterator it = sirocad.begin(); it != sirocad.end(); ++it)
		{
			if (*it == shared_from_this())
			{
				sirocad.erase(it);
				break;
			}
		}
	}
	
	AST& AST::operator=(AST const& drugi)
	{
		if (this != &drugi)
		{
			pobrisiDjecu();
			ucitajDjecu(drugi.m_djeca);
		}
		return *this;
	}

	AST&& AST::operator=(AST&& drugi)
	{
		if (this != &drugi)
		{
			pobrisiDjecu();
			m_djeca = std::move(drugi.m_djeca);
			
			for (std::list<std::shared_ptr<AST>>::iterator it = m_djeca.begin(); it != m_djeca.end(); ++it)
				(*it)->setRoditelj(shared_from_this());
		}
		return std::move(*this);
	}

	//llvm::Value* AST::compilirajDjecu(llvm::LLVMContext& _context)
	//{
	//	llvm::Value* zadnji = nullptr;

	//	for (std::list<std::shared_ptr<AST>>::iterator it = m_djeca.begin(); it != m_djeca.end(); ++it) 
	//		zadnji = (*it)->GenerirajKod(_context);
	//	
	//	return zadnji;
	//}

	AST::~AST()
	{
		//pobrisiDjecu();

		// ako si siroče, izbriši se iz popisa siročadi
		for (std::list<std::shared_ptr<AST>>::iterator it = sirocad.begin(); it != sirocad.end(); ++it)
		{
			if (*it == shared_from_this())
			{
				sirocad.erase(it);
				break;
			}
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////
	// Program

	//Program::Program() : AST(0, 0)
	//{
	//	setKorijen(shared_from_this()); 
	//	modul = new llvm::Module("main", AST::Context());
	//}

	//llvm::Value* Program::GenerirajKod(llvm::LLVMContext& _context)
	//{
	//	m_doseg = std::make_shared<Doseg>();
	//	m_doseg->setRoditelj(nullptr);
	//	Doseg::setZadnji(m_doseg);
	//	// I JOŠ NEŠTA MOŽDA
	//}

	////////////////////////////////////////////////////////////////////////////////////
	// Use direktiva

	UseDirektiva::UseDirektiva(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	UseDirektiva::UseDirektiva(UseDirektiva const& drugi) : 
		IzrazAST(drugi) 
	{ 
		path = drugi.path; 
	}


	std::deque<std::shared_ptr<Token>> UseDirektiva::izvrsi(Parser& parser)
	{
		std::shared_ptr<std::ifstream> datoteka = std::make_shared<std::ifstream>(path);
		if (!datoteka->is_open())
			throw Greska("Greška", Redak(), Stupac(), "Ne mogu otvoriti datoteku " + path);

		// leksiraj datoteku 
		std::deque<std::shared_ptr<Token>> drugiTokeni;
		Lekser drugiLex(datoteka);
		try
		{
			drugiTokeni = drugiLex.leksiraj();
		}
		catch (LeksickaGreska const& e)
		{
			std::cout << e.what();
			//drugiLex.pocisti();
		}

		return drugiTokeni;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Deklaracija strukture

	DeklaracijaStrukture::DeklaracijaStrukture(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		TipAST(redak, stupac, std::move(djeca)), 
		m_ime(std::dynamic_pointer_cast<Leaf>(m_djeca.front())->Sadrzaj()) 
	{
		// možeš popnuti front jer si dohvatio ime (prvo dijete u AST-u)
		m_djeca.pop_front();
	}

	llvm::Type* DeklaracijaStrukture::GenerirajKodTip()
	{
		llvm::StructType* novaStruktura = llvm::StructType::create(*Context().LLVMContext(), m_ime);
		Context().DodajStrukturu(m_ime);
		Context().DodajNoviTip("struct " + m_ime, novaStruktura);

		return novaStruktura;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Definicija strukture

	DefinicijaStrukture::DefinicijaStrukture(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		DeklaracijaStrukture(redak, stupac, std::move(djeca))
	{}
	
	DefinicijaStrukture::DefinicijaStrukture(DefinicijaStrukture const& drugi) :
		DeklaracijaStrukture(drugi),
		m_tipoviElemenata(drugi.m_tipoviElemenata),
		m_imenaElemenata(drugi.m_imenaElemenata)
	{}

	llvm::Type* DefinicijaStrukture::GenerirajKodTip()
	{
		llvm::StructType* novaStruktura = static_cast<llvm::StructType*>(DeklaracijaStrukture::GenerirajKod());

		// dohvati referencu na elemente da ne moramo svaki put pisati cijelo ime djeteta
		ASTList& elementi = *std::dynamic_pointer_cast<ASTList>(m_djeca.front());
		std::vector<llvm::Type*> tijelo;

		while(!elementi.empty())
		{
			// spremi tip elementa za LLVM
			llvm::Type* tipElementa = (*elementi.begin())->GenerirajKodTip();
			tijelo.push_back(tipElementa);

			// izbriši ga s liste
			elementi.pop_front();

			// spremi ime elementa za nas, jer LLVM nije briga za imena
			Context().DodajElementStrukture(m_ime, std::dynamic_pointer_cast<Leaf>(*elementi.begin())->Sadrzaj());

			// izbriši i njega s liste
			elementi.pop_front();
		}

		// na kraju izbriši (sada praznu) listu djece
		m_djeca.pop_front();

		novaStruktura->setBody(tijelo);
		return novaStruktura;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Deklaracija funkcije

	DeklaracijaFunkcije::DeklaracijaFunkcije(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) :
		FunkcijaAST(redak, stupac, std::move(djeca))
	{}

	DeklaracijaFunkcije::DeklaracijaFunkcije(DeklaracijaFunkcije const& drugi) :
		FunkcijaAST(drugi)
	{}

	llvm::Function* DeklaracijaFunkcije::GenerirajKodFunkcija()
	{
		// spremi povratni tip funkcije
		llvm::Type* povratniTip = m_djeca.front()->GenerirajKodTip();
		m_djeca.pop_front();

		// dohvati ime funkcije pa popni i njega
		std::string ime = std::dynamic_pointer_cast<Leaf>(*m_djeca.begin())->Sadrzaj();
		m_djeca.pop_front();

		// dijelimo duljinu liste s 2 jer ona sadrži i tipove i imena
		std::vector<llvm::Type*> tipoviArgumenata(std::dynamic_pointer_cast<ASTList>(m_djeca.front())->size() / 2);
		std::vector<std::string> imenaArgumenata(std::dynamic_pointer_cast<ASTList>(m_djeca.front())->size() / 2);

		for (int i = 0; i < tipoviArgumenata.size(); ++i)
		{
			// prevedi tip iz stringa u llvm::Type* i pushaj ga u tipove
			tipoviArgumenata.push_back(m_djeca.front()->GenerirajKodTip());
			m_djeca.pop_front();

			// i spremi mu ime za kasnije
			imenaArgumenata.push_back(std::dynamic_pointer_cast<Leaf>(m_djeca.front())->Sadrzaj());
			m_djeca.pop_front();
		}

		// sad znamo dovoljno da možemo generirati tip funkcije
		llvm::FunctionType* tipFunkcije = llvm::FunctionType::get(povratniTip, tipoviArgumenata, false);

		// konstruiramo funkciju
		llvm::Function* funkcija = llvm::Function::Create(tipFunkcije, llvm::Function::ExternalLinkage, ime, Module().get());

		// spremimo imena argumenata
		int i = 0;
		for (llvm::Function::arg_iterator it = funkcija->arg_begin(); it != funkcija->arg_end(); ++it)
			it->setName(imenaArgumenata[i++]);

		return funkcija;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Definicija funkcije 
	DefinicijaFunkcije::DefinicijaFunkcije(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) :
		DeklaracijaFunkcije(redak, stupac, std::move(djeca))
		//m_tijelo(*std::dynamic_pointer_cast<ASTList>(m_djeca.front()))
	{}
	
	DefinicijaFunkcije::DefinicijaFunkcije(DefinicijaFunkcije const& drugi) : 
		DeklaracijaFunkcije(drugi)
		//m_tijelo(drugi.m_tijelo)
	{}

	llvm::Function* DefinicijaFunkcije::GenerirajKodFunkcija()
	{
		// skočimo na dijete koje sadrži ime funkcije
		ASTList::iterator it = m_djeca.begin();
		++it;
		std::string ime = std::dynamic_pointer_cast<Leaf>(*it)->Sadrzaj();
		
		// provjeri je li funkcija s tim imenom već deklarirana
		llvm::Function* funkcija = Module()->getFunction(ime);

		// ako ne, deklariraj je
		if (funkcija == nullptr)
			funkcija = DeklaracijaFunkcije::GenerirajKod();

		// ako je funkcija već definirana, prijavi grešku
		else if (!funkcija->empty())
			throw SemantickaGreska(Redak(), Stupac(), "Funkcija " + ime + " je već definirana.");

		// ako je funkcija deklarirana i još nema tijelo, provjeri ima li isti tip
		else if (funkcija->getReturnType() != (*m_djeca.begin())->GenerirajKodTip())
			throw SemantickaGreska(Redak(), Stupac(), "Funkcija '" + ime + "' nema isti povratni tip kao u deklaraciji.");

		// ako je funkcija deklarirana, još nema tijelo i ima isti tip, provjeri prima li iste argumente
		else
		{
			// popni povratni tip i ime funkcije; više nam ne trebaju
			m_djeca.pop_front();
			m_djeca.pop_front();

			// zatim izvuci tipove argumenata iz zadnjeg djeteta
			std::vector<llvm::Type*> tipoviArgumenata(std::dynamic_pointer_cast<ASTList>(m_djeca.front())->size() / 2);
			for (int i = 0; i < tipoviArgumenata.size(); ++i)
			{
				// prevedi tip iz stringa u llvm::Type* i pushaj ga u tipove
				tipoviArgumenata.push_back(m_djeca.front()->GenerirajKodTip());
				m_djeca.pop_front();

				// trebaju nam samo tipovi pa imena argumenata odbacujemo
				m_djeca.pop_front();
			}

			int i = 0;
			for (llvm::Function::arg_iterator it = funkcija->arg_begin(); it != funkcija->arg_end(); ++it)
			{
				if (it->getType() != tipoviArgumenata[i++])
					throw SemantickaGreska(Redak(), Stupac(), "Funkcija '" + ime + "' nema iste argumente kao u deklaraciji.");
			}
		}

		// otvori blok za funkciju
		llvm::BasicBlock* blok = llvm::BasicBlock::Create(*Context().LLVMContext(), "pocetak funkcije", funkcija);
		Context().DodajBlok(blok);
		Builder().SetInsertPoint(blok);

		// stavi imena argumenata u doseg funkcije
		for (llvm::Function::arg_iterator it = funkcija->arg_begin(); it != funkcija->arg_end(); ++it)
		{
			llvm::AllocaInst* alokacija = Context().StackAlokacija(funkcija, it->getType(), it->getName());
			Builder().CreateStore(&(*it), alokacija);
			Context().LokalneVarijable()[it->getName()] = alokacija;
		}

		try
		{
			llvm::Value* povratnaVrijednost = m_djeca.front()->GenerirajKodIzraz();
			if (povratnaVrijednost != nullptr)
			{
				// na kraju, provjeri je li funkcija ispravno konstruirana
				llvm::verifyFunction(*funkcija);
				Context().ZatvoriBlok();

				return funkcija;
			}

			else
			{
				Context().ZatvoriBlok();
				funkcija->eraseFromParent();
				return nullptr;
			}
		}

		// ako ne uspiješ generirati kod za tijelo funkcije, obriši je
		catch (SemantickaGreska const& e)
		{
			std::cout << e.what();
			funkcija->eraseFromParent();
			return nullptr;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Typedef

	TypeDef::TypeDef(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) :
		IzrazAST(redak, stupac, std::move(djeca))
	{
		m_tip = *std::dynamic_pointer_cast<Tip>(m_djeca.front());
		m_djeca.pop_front();
		m_alias = std::dynamic_pointer_cast<Leaf>(m_djeca.front())->Sadrzaj();
		m_djeca.pop_front();
	}

	TypeDef::TypeDef(TypeDef const& drugi) :
		IzrazAST(drugi),
		m_tip(drugi.m_tip),
		m_alias(drugi.m_alias)
	{}

	llvm::Value* TypeDef::GenerirajKodIzraz()
	{
		// ako nismo u globalnom dosegu, bacamo grešku
		if (Context().TrenutniBlok()->Roditelj() != nullptr)
			throw SemantickaGreska(Redak(), Stupac(), "typedef se smije pojavljivati samo u globalnom dosegu.");
		
		Context().AliasiTipova()[m_alias] = m_tip;
		return nullptr;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// IfElse
	
	IfElse::IfElse(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	IfElse::IfElse(IfElse const& drugi) : IzrazAST(drugi) {}

	llvm::Value* IfElse::GenerirajKodIzraz()
	{
		// prvo kompajliramo uvjet
		llvm::Value* uvjet = m_djeca.front()->GenerirajKodIzraz();
		if (uvjet == nullptr)
			return nullptr;

		m_djeca.pop_front();

		if (Context().PrevediTip(uvjet->getType()) != "bool")
			throw SemantickaGreska(Redak(), Stupac(), "Uvjet u 'if'-naredbi mora biti tipa bool!");

		// pretvaramo vrijednost u bool tako da je uspoređujemo s 0.0
		uvjet = Builder().CreateFCmpONE(uvjet, llvm::ConstantFP::get(*Context().LLVMContext(), llvm::APFloat(0.0)), "uvjet");

		// dohvati funkciju u kojoj se "if" nalazi
		llvm::Function* funkcija = Builder().GetInsertBlock()->getParent();

		// napravimo blok s naredbama u slučaju true
		llvm::BasicBlock* ifTrueBlok = llvm::BasicBlock::Create(*Context().LLVMContext(), "ifTrue", funkcija);
		Context().DodajBlok(ifTrueBlok);

		// napravimo i blok za else i ono što slijedi poslije if-a, ali ih ne stavljamo u funkciju
		llvm::BasicBlock* elseBlok = llvm::BasicBlock::Create(*Context().LLVMContext(), "else");
		llvm::BasicBlock* nastavakBlok = llvm::BasicBlock::Create(*Context().LLVMContext(), "nastavak");

		// i u builder dodamo grananje s danim uvjetom i granama
		Builder().CreateCondBr(uvjet, ifTrueBlok, elseBlok);

		// uđemo u if blok i popunimo ga
		Builder().SetInsertPoint(ifTrueBlok);

		llvm::Value *ifTrueVrijednost = m_djeca.front()->GenerirajKodIzraz();
		if (ifTrueVrijednost == nullptr)
			return nullptr;

		m_djeca.pop_front();
		bool imamElse = m_djeca.front().get() != nullptr;
		Context().ZatvoriBlok();

		// i bezuvjetno granamo u originalnu funkciju
		Builder().CreateBr(nastavakBlok);
		ifTrueBlok = Builder().GetInsertBlock();

		// dodamo else blok
		funkcija->getBasicBlockList().push_back(elseBlok);
		Context().DodajBlok(elseBlok);
		Builder().SetInsertPoint(elseBlok);

		llvm::Value* elseVrijednost = nullptr;

		// ako imamo else dio, kompajliramo ga
		if (imamElse)
		{
			elseVrijednost = m_djeca.front()->GenerirajKodIzraz();
			if (!elseVrijednost)
				return nullptr;
		}
	
		// granamo u originalnu funkciju
		Builder().CreateBr(nastavakBlok);
		elseBlok = Builder().GetInsertBlock();

		funkcija->getBasicBlockList().push_back(nastavakBlok);
		Builder().SetInsertPoint(nastavakBlok);

		Context().ZatvoriBlok();
		Context().DodajBlok(nastavakBlok);

		// dodamo samo grananje u builder
		llvm::PHINode* grananje = Builder().CreatePHI(llvm::Type::getDoubleTy(*Context().LLVMContext()), 2, "grananje");

		// i dodamo mu grane
		grananje->addIncoming(ifTrueVrijednost, ifTrueBlok);
		grananje->addIncoming(elseVrijednost, elseBlok);
		return grananje;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// While

	While::While(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}
	
	While::While(While const& drugi) : 
		IzrazAST(drugi) 
	{}

	llvm::Value* While::GenerirajKodIzraz()
	{
		// napravimo blok za tijelo petlje
		llvm::Function* funkcija = Builder().GetInsertBlock()->getParent();
		m_pocetakPetlje = llvm::BasicBlock::Create(*Context().LLVMContext(), "tijeloPetlje", funkcija);
		Context().DodajBlok(m_pocetakPetlje);

		// bezuvjetno granamo u tijelo petlje
		Builder().CreateBr(m_pocetakPetlje);
		Builder().SetInsertPoint(m_pocetakPetlje);

		// kompajliramo uvjet petlje
		llvm::Value *uvjetZaustavljanja = m_djeca.front()->GenerirajKodIzraz();
		if (uvjetZaustavljanja == nullptr)
			return nullptr;

		if (Context().PrevediTip(uvjetZaustavljanja->getType()) != "bool")
			throw SemantickaGreska(Redak(), Stupac(), "Uvjet u 'while'-petlji mora biti tipa bool!");

		m_djeca.pop_front();

		// dodamo blok za dio funkcije koji slijedi nakon petlje
		//llvm::BasicBlock* krajPetlje = Builder().GetInsertBlock();
		m_krajPetlje = llvm::BasicBlock::Create(*Context().LLVMContext(), "nastavakFunkcije", funkcija);

		// pretvaramo vrijednost u bool tako da je uspoređujemo s 0.0
		uvjetZaustavljanja = Builder().CreateFCmpONE(uvjetZaustavljanja, llvm::ConstantFP::get(*Context().LLVMContext(), llvm::APFloat(0.0)), "uvjetZaustavljanja");
		
		// dodamo grananje na kraj bloka "krajPetlje"
		Builder().CreateCondBr(uvjetZaustavljanja, m_pocetakPetlje, m_krajPetlje);

		// kompajliramo tijelo petlje
		llvm::Value* tijelo = m_djeca.front()->GenerirajKodIzraz();
		if (tijelo == nullptr)
			return nullptr;

		m_djeca.pop_front();

		// zatvorimo blok s tijelom petlje te otvorimo blok koji slijedi
		Context().ZatvoriBlok();
		Context().DodajBlok(m_krajPetlje);
		Builder().SetInsertPoint(m_krajPetlje);

		// LLVM kaže da izraz while uvijek vraća 0 
		return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*Context().LLVMContext()));
	}
	
	////////////////////////////////////////////////////////////////////////////////////
	// For

	For::For(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}
	
	For::For(For const& drugi) 
		: IzrazAST(drugi) 
	{}

	llvm::Value* For::GenerirajKodIzraz()
	{
		llvm::Function* funkcija = Builder().GetInsertBlock()->getParent();
		llvm::BasicBlock* prethodniBlok = Builder().GetInsertBlock();

		// dodajemo blok u kojem je samo inicijalizacija brojača
		llvm::BasicBlock* inicijalizacijaBlok = llvm::BasicBlock::Create(*Context().LLVMContext(), "inicijalizacija", funkcija);
		Context().DodajBlok(inicijalizacijaBlok);
		
		// bezuvjetno granamo u inicijalizacijski blok (iz funkcije)
		Builder().CreateBr(inicijalizacijaBlok);
		Builder().SetInsertPoint(inicijalizacijaBlok);

		// generiramo kod za inicijalizaciju brojača
		llvm::Value* brojac = m_djeca.front()->GenerirajKodIzraz();
		if (brojac == nullptr)
			return nullptr;

		m_djeca.pop_front();

		// dodajemo blok u kojem je samo provjera uvjeta zaustavljanja
		llvm::BasicBlock* uvjetBlok = llvm::BasicBlock::Create(*Context().LLVMContext(), "provjeraUvjeta", funkcija);
		Context().DodajBlok(uvjetBlok);

		// bezuvjetno granamo u blok s provjerom uvjeta
		Builder().CreateBr(uvjetBlok);
		Builder().SetInsertPoint(uvjetBlok);
		
		// generiramo kod za uvjet zaustavljanja
		llvm::Value* uvjetZaustavljanja = m_djeca.front()->GenerirajKodIzraz();
		if (uvjetZaustavljanja == nullptr)
			return nullptr;

		if (Context().PrevediTip(uvjetZaustavljanja->getType()) != "bool")
			throw SemantickaGreska(Redak(), Stupac(), "Uvjet u 'for'-petlji mora biti tipa bool!");

		m_djeca.pop_front();

		// pretvaramo vrijednost u bool tako da je uspoređujemo s 0.0
		uvjetZaustavljanja = Builder().CreateFCmpONE(uvjetZaustavljanja, llvm::ConstantFP::get(*Context().LLVMContext(), llvm::APFloat(0.0)), "uvjetZaustavljanja");
		
		// dodajemo blokove za tijelo petlje, inkrement petlje te nastavak
		// funkcije nakon završetka petlje (ali inkrement i nastavak još ne stavljamo u funkciju).
		// pamtimo gdje su inkrement i kraj petlje tako da break i continue znaju kamo skaču
		llvm::BasicBlock* tijeloPetlje = llvm::BasicBlock::Create(*Context().LLVMContext(), "tijeloPetlje", funkcija);
		m_inkrement = llvm::BasicBlock::Create(*Context().LLVMContext(), "inkrementPetlje");
		m_krajPetlje = llvm::BasicBlock::Create(*Context().LLVMContext(), "nastavakFunkcije");
		Builder().CreateCondBr(uvjetZaustavljanja, tijeloPetlje, m_krajPetlje);

		// zatvorimo uvjetBlok i otvorimo tijeloPetlje
		Context().ZatvoriBlok();
		Context().DodajBlok(tijeloPetlje);
		Builder().SetInsertPoint(tijeloPetlje);  

		// generiramo kod za tijelo petlje
		llvm::Value* tijelo = m_djeca.front()->GenerirajKodIzraz();
		if (tijelo == nullptr)
			return nullptr;

		m_djeca.pop_front();
		
		// bezuvjetno granamo u inkrement blok
		Builder().CreateBr(m_inkrement);

		// zatvaramo tijeloPetlje i otvaramo inkrementBlok
		Context().ZatvoriBlok();
		funkcija->getBasicBlockList().push_back(m_inkrement);
		Context().DodajBlok(m_inkrement);
		Builder().SetInsertPoint(m_inkrement);

		// generiramo kod za inkrement
		llvm::Value* inkrement = m_djeca.front()->GenerirajKodIzraz();
		if (inkrement == nullptr)
				return nullptr;
		
		m_djeca.pop_front();
		// i bezuvjetno granamo u uvjetBlok
		Builder().CreateBr(uvjetBlok);
		Context().ZatvoriBlok();

		// na kraju zatvorimo inicijalizacijaBlok
		Context().ZatvoriBlok();

		funkcija->getBasicBlockList().push_back(m_krajPetlje);
		Builder().SetInsertPoint(m_krajPetlje);

		// stavimo deklaraciju prije petlje i stavimo da se nakon nje ulazi u petlju
		//llvm::PHINode* grananje = Builder().CreatePHI(brojac->getType(), 2, brojac->getName());
		//grananje->addIncoming(brojac, prethodniBlok);
		
		// llvm::Value* trenutnaVrijednostBrojaca = Builder().CreateLoad(brojac, brojac->getName());
		
		// TODO: KAD DOĐEM DO INKREMENTA, POGLEDATI KOJA JE FORA S OVIM I JE LI MI UOPĆE POTREBNO OVDJE
		//llvm::Value* sljedecaVrijednostBrojca = Builder().CreateFAdd(trenutnaVrijednostBrojaca, inkrement, "sljedecaVrijednostBrojaca");
		//Builder().CreateStore(sljedecaVrijednostBrojca, brojac);


		// dodamo blok za dio funkcije koji slijedi nakon petlje
		//llvm::BasicBlock* krajPetlje = Builder().GetInsertBlock();
		//Context().ZatvoriBlok();

		//llvm::BasicBlock* nakonPetlje = llvm::BasicBlock::Create(*Context().LLVMContext(), "nastavakFunkcije", funkcija);
		//Context().DodajBlok(nakonPetlje);

		//// dodamo grananje na kraj bloka "krajPetlje"
		//Builder().CreateCondBr(uvjetZaustavljanja, tijeloPetlje, nakonPetlje);
		//Builder().SetInsertPoint(nakonPetlje);

		//brojac->addIncoming(sljedecaVrijednostBrojca, krajPetlje);

		// nakon petlje vrati prešaranu varijablu
		//if (staraVarijabla != nullptr)
		//	m_doseg->Imena()[imeBrojaca] = staraVarijabla;
		//else
		//	m_doseg->Imena().erase(imeBrojaca);

		// LLVM kaže da izraz for uvijek vraća 0
		return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*Context().LLVMContext()));
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Return

	Return::Return(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	Return::Return(Return const& drugi) : 
		IzrazAST(drugi) 
	{}

	llvm::Value* Return::GenerirajKodIzraz() 
	{
		// ako funkcija vraća nešto, vidi što je to
		if (!m_djeca.empty())
		{
			llvm::Value* povratnaVrijednost = m_djeca.front()->GenerirajKodIzraz();

			// ako tip dobivene vrijednosti odgovara povratnom tipu funkcije, vratimo ga
			if(povratnaVrijednost->getType() == Builder().GetInsertBlock()->getParent()->getReturnType())
				return Builder().CreateRet(povratnaVrijednost);

			// inače bacamo grešku
			std::stringstream poruka;
			poruka << "Tip vrijednosti ne odgovara povratnom tipu funkcije! Dobio: '"
				<< Context().PrevediTip(povratnaVrijednost->getType()) << "', a očekujem: '"
				<< Context().PrevediTip(Builder().GetInsertBlock()->getParent()->getReturnType()) << "'.";

			throw new SemantickaGreska(Redak(), Stupac(), poruka.str());
		}
		else
			// inače "vrati" void
			return Builder().CreateRetVoid();
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Break
	
	Break::Break(int redak, int stupac) : 
		IzrazAST(redak, stupac) 
	{}

	Break::Break(Break const& drugi) : 
		IzrazAST(drugi) 
	{}

	llvm::Value* Break::GenerirajKodIzraz() 
	{ 
		std::shared_ptr<AST> predakPetlja = getRoditelj();

		// zapamtimo typeid od For i While. ne pamtimo typeid od
		// petlje pretka jer type_info ima izbrisan operator=
		type_info const& tipFor = typeid(For);
		type_info const& tipWhile = typeid(While);

		// penjemo se po stablu dok ne nađemo For ili While
		while (typeid(*predakPetlja) != tipFor && typeid(*predakPetlja) != tipWhile)
		{
			// ako smo došli do korijena i nismo našli petlju, bacamo grešku
			if (predakPetlja->isRoot())
				throw new SemantickaGreska(Redak(), Stupac(), "Naredba break se ne nalazi u petlji.");

			// inače se penjemo jedan korak gore
			predakPetlja = predakPetlja->getRoditelj();
		}
		
		// sad možemo zapamtiti tip petlje
		type_info const& tipPetlja = typeid(*predakPetlja);

		// ako je petlja For, skačemo van iz petlje na jedan način 
		if (tipPetlja == tipFor)
			Builder().CreateBr(std::dynamic_pointer_cast<For>(predakPetlja)->BlokNakonPetlje());
		
		// ako je petlja While, skačemo van iz petlje na drugi način
		if (tipPetlja == tipWhile)
			Builder().CreateBr(std::dynamic_pointer_cast<While>(predakPetlja)->KrajPetlje());

		// povratna vrijednost nam je nebitna jer je ionako nigdje ne koristimo pa može nullptr
		return nullptr; 
	}; 

	////////////////////////////////////////////////////////////////////////////////////
	// Continue

	Continue::Continue(int redak, int stupac) : 
		IzrazAST(redak, stupac) 
	{}

	Continue::Continue(Continue const& drugi) : 
		IzrazAST(drugi) 
	{}

	llvm::Value* Continue::GenerirajKodIzraz() 
	{
		std::shared_ptr<AST> predakPetlja = getRoditelj();

		// zapamtimo typeid od For i While. ne pamtimo typeid od
		// petlje pretka jer type_info ima izbrisan operator=
		type_info const& tipFor = typeid(For);
		type_info const& tipWhile = typeid(While);

		// penjemo se po stablu dok ne nađemo For ili While
		while (typeid(*predakPetlja) != tipFor && typeid(*predakPetlja) != tipWhile)
		{
			// ako smo došli do korijena i nismo našli petlju, bacamo grešku
			if (predakPetlja->isRoot())
				throw new SemantickaGreska(Redak(), Stupac(), "Continue se ne nalazi u petlji.");

			// inače se penjemo jedan korak gore
			predakPetlja = predakPetlja->getRoditelj();
		}

		// sad možemo zapamtiti tip petlje
		type_info const& tipPetlja = typeid(*predakPetlja);

		// ako je petlja For, skačemo na inkrement
		if (tipPetlja == tipFor)
			Builder().CreateBr(std::dynamic_pointer_cast<For>(predakPetlja)->BlokInkrement());

		// ako je petlja While, skačemo na provjeru uvjeta (početak petlje)
		if (tipPetlja == tipWhile)
			Builder().CreateBr(std::dynamic_pointer_cast<While>(predakPetlja)->PocetakPetlje());

		// povratna vrijednost nam je nebitna jer je ionako nigdje ne koristimo pa može nullptr
		return nullptr;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// Varijabla

	Varijabla::Varijabla(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)),
		m_ime(std::dynamic_pointer_cast<Leaf>(m_djeca.front())->Sadrzaj())
	{
		m_djeca.pop_front();
	}

	Varijabla::Varijabla(Varijabla const& drugi) :
		IzrazAST(drugi),
		m_ime(drugi.m_ime)
	{}

	llvm::Value* Varijabla::GenerirajKodIzraz()
	{
		llvm::Value* varijabla = Context().TrenutniBlok()->Trazi(m_ime);
		if (varijabla == nullptr)
		{
			throw SemantickaGreska(Redak(), Stupac(), "Varijabla " + m_ime + " nije deklarirana.");
			return nullptr;
		}

		// učitaj varijablu sa stacka
		return Builder().CreateLoad(varijabla, m_ime);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// DeklaracijaVarijable

	DeklaracijaVarijable::DeklaracijaVarijable(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}
	
	DeklaracijaVarijable::DeklaracijaVarijable(DeklaracijaVarijable const& drugi) 
		: IzrazAST(drugi)
	{}

	llvm::Value* DeklaracijaVarijable::GenerirajKodIzraz()
	{
		// iz dohvati tip, ime i pridruženu vrijednost varijabli (ako postoji)
		llvm::Type* tip = m_djeca.front()->GenerirajKodTip();
		m_djeca.pop_front();

		std::string ime = std::dynamic_pointer_cast<Leaf>(m_djeca.front())->Sadrzaj();
		m_djeca.pop_front();

		if (Context().LokalneVarijable().find(ime) != Context().LokalneVarijable().end())
			throw SemantickaGreska(Redak(), Stupac(), "Varijabla " + ime + " je već deklarirana.");

		llvm::Function* funkcija = Builder().GetInsertBlock()->getParent();
		llvm::Value* inicijalnaVrijednost = nullptr;

		// ako imamo inicijalnu vrijednost, kompajliramo je
		if (!m_djeca.empty())
			inicijalnaVrijednost = m_djeca.front()->GenerirajKodIzraz();

		// ako ne, koristimo defaultnu ovisno o tipu (C0)
		else if (Context().PrevediTip(tip) == "int")
			inicijalnaVrijednost = llvm::ConstantInt::get(Context().PrevediTip("int"), 0, true);

		else if (Context().PrevediTip(tip) == "char")
			inicijalnaVrijednost = llvm::ConstantInt::get(Context().PrevediTip("char"), '\0');

		else if (Context().PrevediTip(tip) == "bool")
			inicijalnaVrijednost = llvm::ConstantInt::get(Context().PrevediTip("bool"), 0);

		else if (Context().PrevediTip(tip) == "string")
			inicijalnaVrijednost = llvm::ConstantDataArray::getString(*Context().LLVMContext(), "");

		else if (Context().PrevediTip(tip) == "int*")
			inicijalnaVrijednost = llvm::ConstantPointerNull::get(static_cast<llvm::PointerType*>(Context().PrevediTip("int*")));

		else if (Context().PrevediTip(tip) == "char*")
			inicijalnaVrijednost = llvm::ConstantPointerNull::get(static_cast<llvm::PointerType*>(Context().PrevediTip("char*")));

		else if (Context().PrevediTip(tip) == "bool*")
			inicijalnaVrijednost = llvm::ConstantPointerNull::get(static_cast<llvm::PointerType*>(Context().PrevediTip("bool*")));

		else if (Context().PrevediTip(tip) == "string*")
			inicijalnaVrijednost = llvm::ConstantPointerNull::get(static_cast<llvm::PointerType*>(Context().PrevediTip("string*")));

		// dodamo stack alokaciju varijable
		llvm::AllocaInst* alokacija = Context().StackAlokacija(funkcija, tip, ime);
		Builder().CreateStore(inicijalnaVrijednost, alokacija);
		Context().LokalneVarijable()[ime] = alokacija;

		return alokacija;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// TernarniOperator

	TernarniOperator::TernarniOperator(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) :
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	TernarniOperator::TernarniOperator(TernarniOperator const& drugi) : 
		IzrazAST(drugi) 
	{}

	llvm::Value* TernarniOperator::GenerirajKodIzraz()
	{	
		// implementiramo ga kao if-else jer ne vidim pametniji način kako bih to napravio
		llvm::Value* uvjet = std::dynamic_pointer_cast<IzrazAST>(m_djeca.front())->GenerirajKodIzraz();
		m_djeca.pop_front();
		llvm::Value* granaTrue = std::dynamic_pointer_cast<IzrazAST>(m_djeca.front())->GenerirajKodIzraz();
		m_djeca.pop_front();
		llvm::Value* granaFalse = std::dynamic_pointer_cast<IzrazAST>(m_djeca.front())->GenerirajKodIzraz();
		m_djeca.pop_front();

		// LLVM nema ternarni operator pa ga implementiramo sami:
		// 1. bacamo grešku ako izrazi koje operator vraća nisu istog tipa
		if (granaTrue->getType() != granaFalse->getType())
			throw SemantickaGreska(Redak(), Stupac(), "Izrazi koje operator ?: vraća nisu istog tipa.");

		// 2. saznamo kojeg su izrazi tipa
		llvm::Type* tipIzraza = granaTrue->getType();

		// 3. inkrementiramo statički brojač, tako da nemamo konflikte između ternarnih operatora
		static int brojac = 0;
		std::string imeOperatora = "__ternarniOperator" + std::to_string(brojac) + "__";
		++brojac;

		// 4. deklariramo funkciju tipa tipIzraza(bool, tipIzraza, tipIzraza) 
		// (u budućnosti bi valjalo pogledati postoji li takva funkcija i ne praviti novu
		// ako postoji, ali malo sam u stisci s vremenom pa zasad to otpada)
		std::vector<llvm::Type*> tipoviArugmenata;
		tipoviArugmenata.push_back(Context().PrevediTip("bool"));
		tipoviArugmenata.push_back(tipIzraza);
		tipoviArugmenata.push_back(tipIzraza);

		llvm::FunctionType* tipOperatora = llvm::FunctionType::get(tipIzraza, tipoviArugmenata, false);
		llvm::Function* ternarniOperator = llvm::Function::Create(tipOperatora, llvm::Function::ExternalLinkage, imeOperatora, Module().get());
		
		// 5. implementiramo ternarni operator kao if(uvjet) return granaTrue; else return granaFalse;
		llvm::BasicBlock* tijeloOperatora = llvm::BasicBlock::Create(*Context().LLVMContext(), imeOperatora, ternarniOperator);
		Builder().SetInsertPoint(tijeloOperatora);

		uvjet = Builder().CreateFCmpONE(uvjet, llvm::ConstantFP::get(*Context().LLVMContext(), llvm::APFloat(0.0)), "uvjet");
		llvm::BasicBlock* blokTrue = llvm::BasicBlock::Create(*Context().LLVMContext(), "granaTrue", ternarniOperator);
		llvm::BasicBlock* blokFalse = llvm::BasicBlock::Create(*Context().LLVMContext(), "granaFalse");
		
		Builder().CreateCondBr(uvjet, blokTrue, blokFalse);
		Builder().SetInsertPoint(blokTrue);
		Builder().CreateRet(granaTrue);
		blokTrue = Builder().GetInsertBlock();

		ternarniOperator->getBasicBlockList().push_back(blokFalse);
		Builder().SetInsertPoint(blokFalse);
		Builder().CreateRet(granaFalse);
		blokFalse = Builder().GetInsertBlock();

		llvm::PHINode* grananje = Builder().CreatePHI(llvm::Type::getDoubleTy(*Context().LLVMContext()), 2, "grananje");
		grananje->addIncoming(granaTrue, blokTrue);
		grananje->addIncoming(granaFalse, blokFalse);

		// 6. pozovemo funkciju __ternarniOperator<brojac>__
		std::vector<llvm::Value*> argumentiZaPoziv;
		argumentiZaPoziv.push_back(uvjet);
		argumentiZaPoziv.push_back(granaTrue);
		argumentiZaPoziv.push_back(granaFalse);

		// 7. vratimo ono što vrati poziv ternarnog operatora
		return llvm::CallInst::Create(ternarniOperator, argumentiZaPoziv, "", Context().TrenutniBlok()->Sadrzaj());
	}

	////////////////////////////////////////////////////////////////////////////////////
	// LogickiOperator

	LogickiOperator::LogickiOperator(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	LogickiOperator::LogickiOperator(LogickiOperator const& drugi) : 
		IzrazAST(drugi) 
	{}

	// manje-više ista stvar kao bitwise operatori, osim što
	// rade samo s jednim bitom (jer sam tako implementirao bool)
	llvm::Value* LogickiOperator::GenerirajKodIzraz()
	{
		llvm::Value* lijevo = m_djeca.front()->GenerirajKodIzraz();
		if (Context().PrevediTip(lijevo->getType()) != "bool")
			throw SemantickaGreska(m_djeca.front()->Redak(), m_djeca.front()->Stupac(), "Prvi operand logičkog operatora mora biti tipa bool.");
		
		m_djeca.pop_front();

		llvm::Value* desno = m_djeca.front()->GenerirajKodIzraz();
		if (Context().PrevediTip(desno->getType()) != "bool")
			throw SemantickaGreska(m_djeca.front()->Redak(), m_djeca.front()->Stupac(), "Drugi operand logičkog operatora mora biti tipa bool.");

		m_djeca.pop_front();

		if (lijevo == nullptr || desno == nullptr)
			// ili baci grešku?
			return nullptr;

		Leaf const& operacija = *std::dynamic_pointer_cast<Leaf>(m_djeca.front());
		std::string imeOperatora;

		switch (operacija.TipTokena())
		{
			case LAND:
				imeOperatora = "ogickiAnd";
				lijevo = Builder().CreateAnd(lijevo, desno, "l" + imeOperatora);
				break;

			case LOR:
				imeOperatora = "ogickiOr";
				lijevo = Builder().CreateOr(lijevo, desno, "l" + imeOperatora);
				break;
		}
		// pretvorimo bool u broj 0.0 ili 1.0 (jer LLVM tako kaže)
		return Builder().CreateUIToFP(lijevo, llvm::Type::getDoubleTy(*Context().LLVMContext().get()), "rezultatL" + imeOperatora);

	}

	////////////////////////////////////////////////////////////////////////////////////
	// BitovniOperator

	BitovniOperator::BitovniOperator(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	BitovniOperator::BitovniOperator(BitovniOperator const& drugi) : 
		IzrazAST(drugi) 
	{}

	llvm::Value* BitovniOperator::GenerirajKodIzraz()
	{
		llvm::Value* lijevo = m_djeca.front()->GenerirajKodIzraz();
		if (lijevo->getType() != Context().PrevediTip("int"))
			throw SemantickaGreska(m_djeca.front()->Redak(), m_djeca.front()->Stupac(), "Prvi argument bitovnog operatora mora biti tipa int.");
		
		m_djeca.pop_front();

		llvm::Value* desno = m_djeca.front()->GenerirajKodIzraz();
		if (desno->getType() != Context().PrevediTip("int"))
			throw SemantickaGreska(m_djeca.front()->Redak(), m_djeca.front()->Stupac(), "Drugi argument bitovnog operatora mora biti tipa int.");

		m_djeca.pop_front();

		if (lijevo == nullptr || desno == nullptr)
			// ili baci grešku?
			return nullptr;

		Leaf const& operacija = *std::dynamic_pointer_cast<Leaf>(m_djeca.front());
		std::string imeOperatora;

		switch (operacija.TipTokena())
		{
			case BITAND:
				imeOperatora = "itAnd";
				lijevo = Builder().CreateAnd(lijevo, desno, "b" + imeOperatora);
				break;

			case BITOR:
				imeOperatora = "itOr";
				lijevo = Builder().CreateOr(lijevo, desno, "b" + imeOperatora);
				break;

			case BITXOR:
				imeOperatora = "itXor";
				lijevo = Builder().CreateXor(lijevo, desno, "b" + imeOperatora);
				break;
		}
		// pretvorimo bool u broj 0.0 ili 1.0
		return Builder().CreateUIToFP(lijevo, llvm::Type::getDoubleTy(*Context().LLVMContext().get()), "rezultatB" + imeOperatora);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// OperatorJednakost

	OperatorJednakost::OperatorJednakost(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	OperatorJednakost::OperatorJednakost(OperatorJednakost const& drugi) : 
		IzrazAST(drugi) 
	{}

	llvm::Value* OperatorJednakost::GenerirajKodIzraz()
	{
		llvm::Value* lijevo = m_djeca.front()->GenerirajKodIzraz();
		m_djeca.pop_front();

		llvm::Value* desno = m_djeca.front()->GenerirajKodIzraz();
		m_djeca.pop_front();

		if (lijevo == nullptr || desno == nullptr)
			// ili baci grešku?
			return nullptr;

		Leaf const& operacija = *std::dynamic_pointer_cast<Leaf>(m_djeca.front());
		std::string imeOperatora;
		switch (operacija.TipTokena())
		{
			case NEQ:
				imeOperatora = "Razlicito";
				lijevo = Builder().CreateICmpNE(lijevo, desno, "usporedba" + imeOperatora);
				break;

			case EQ:
				imeOperatora = "Jedako";
				lijevo = Builder().CreateICmpEQ(lijevo, desno, "usporedba" + imeOperatora);
				break;
		}
		// pretvorimo bool u broj 0.0 ili 1.0
		return Builder().CreateUIToFP(lijevo, llvm::Type::getDoubleTy(*Context().LLVMContext().get()), "rezultat" + imeOperatora);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// OperatorUsporedbe

	OperatorUsporedbe::OperatorUsporedbe(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	OperatorUsporedbe::OperatorUsporedbe(OperatorUsporedbe const& drugi) : 
		IzrazAST(drugi) 
	{}

	llvm::Value* OperatorUsporedbe::GenerirajKodIzraz()
	{
		llvm::Value* lijevo = m_djeca.front()->GenerirajKodIzraz();
		m_djeca.pop_front();

		llvm::Value* desno = m_djeca.front()->GenerirajKodIzraz();
		m_djeca.pop_front();

		if (lijevo == nullptr || desno == nullptr)
			// ili baci grešku?
			return nullptr;

		Leaf const& operacija = *std::dynamic_pointer_cast<Leaf>(m_djeca.front());
		std::string imeOperatora;
		switch (operacija.TipTokena())
		{
			case LESS:
				imeOperatora = "Manje";
				lijevo = Builder().CreateICmpSLT(lijevo, desno, "usporedba" + imeOperatora);
				break;
				
			case LESSEQ:
				imeOperatora = "ManjeJednako";
				lijevo = Builder().CreateICmpSLE(lijevo, desno, "usporedba" + imeOperatora);
				break;

			case GRT:
				imeOperatora = "Vece";
				lijevo = Builder().CreateICmpSGT(lijevo, desno, "usporedba" + imeOperatora);
				break;

			case GRTEQ:
				imeOperatora = "VeceJednako";
				lijevo = Builder().CreateICmpSGE(lijevo, desno, "usporedba" + imeOperatora);
				break;
		}

		// pretvorimo bool u broj 0.0 ili 1.0
		return Builder().CreateUIToFP(lijevo, llvm::Type::getDoubleTy(*Context().LLVMContext().get()), "rezultat" + imeOperatora);
	}
	
	////////////////////////////////////////////////////////////////////////////////////
	// BinarniOperator

	BinarniOperator::BinarniOperator(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) :
		IzrazAST(redak, stupac, std::move(djeca))
	{}

	BinarniOperator::BinarniOperator(BinarniOperator const& drugi) :
		IzrazAST(drugi)
	{}

	llvm::Value* BinarniOperator::GenerirajKodIzraz()
	{
		llvm::Value* lijevo = m_djeca.front()->GenerirajKodIzraz();
		m_djeca.pop_front();

		llvm::Value* desno = m_djeca.front()->GenerirajKodIzraz();
		m_djeca.pop_front();

		if (lijevo == nullptr || desno == nullptr)
			// ili baci grešku?
			return nullptr;

		Leaf const& operacija = *std::dynamic_pointer_cast<Leaf>(m_djeca.front());
		switch (operacija.TipTokena())
		{
			case PLUS:
				return Builder().CreateAdd(lijevo, desno, "zbrajanje");

			case MINUS:
				return Builder().CreateSub(lijevo, desno, "oduzimanje");

			case ZVJ:
				return Builder().CreateMul(lijevo, desno, "mnozenje");

			case SLASH:
				return Builder().CreateSDiv(lijevo, desno, "dijeljenje");

			case LSHIFT:
			{
				// castamo desni broj u constant int, zatim u int i napokon ga reduciramo modulo 32 jer
				// C0 uzima samo 5 najmanje značajnih znamenki drugog argumenta
				int smanjenDesni = llvm::dyn_cast<llvm::ConstantInt>(desno)->getValue().getSExtValue() % 32;
				return Builder().CreateShl(lijevo, smanjenDesni, "lijeviPomak");
			}
			case RSHIFT:
			{
				// vidi gore
				int smanjenDesni = llvm::dyn_cast<llvm::ConstantInt>(desno)->getValue().getSExtValue() % 32;
				return Builder().CreateAShr(lijevo, smanjenDesni, "desniPomak");
			}
			case MOD:
				return Builder().CreateSRem(lijevo, desno, "modulo"); // nadam se da je srem signed remainder

			default:
				throw SemantickaGreska(Redak(), Stupac(), "Nepoznat binarni operator!");
		}
	}

	////////////////////////////////////////////////////////////////////////////////////
	// OperatorPridruzivanja

	OperatorPridruzivanja::OperatorPridruzivanja(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	OperatorPridruzivanja::OperatorPridruzivanja(OperatorPridruzivanja const& drugi) : 
		IzrazAST(drugi) 
	{}

	llvm::Value* OperatorPridruzivanja::GenerirajKodIzraz()
	{
		// ako s lijeve strane nije lijeva vrijednost, bacamo grešku
		type_info const& tipLijevo = typeid(*m_djeca.front());
		if (tipLijevo != typeid(Varijabla) &&
			tipLijevo != typeid(Dereferenciranje) &&
			tipLijevo != typeid(UglateZagrade) &&
			tipLijevo != typeid(Strelica) &&
			tipLijevo != typeid(Tocka))
		{
			throw SemantickaGreska(Redak(), Stupac(), "S lijeve strane operatora pridruživanja mora biti lijeva vrijednost.");
		}

		// kompajliramo lijevu stranu
		llvm::Value* lijevo = m_djeca.front()->GenerirajKodIzraz();
		m_djeca.pop_front();

		// kompajliramo desnu stranu
		llvm::Value* desno = m_djeca.front()->GenerirajKodIzraz();
		m_djeca.pop_front();

		if (desno == nullptr)
				return nullptr;

		// dohvati poznatu vrijednost lijeve strane
		Builder().CreateLoad(lijevo, lijevo->getName());
		if (lijevo == nullptr)
			throw SemantickaGreska(Redak(), Stupac(), "Varijabla " + std::string(lijevo->getName()) + " nije deklarirana.");

		Leaf const& operacija = *std::dynamic_pointer_cast<Leaf>(m_djeca.front());

		llvm::Value* novoLijevo;
		switch (operacija.TipTokena())
		{
			case ASSIGN:
				novoLijevo = desno;
				break;

			case PLUSEQ:
				novoLijevo = Builder().CreateAdd(lijevo, desno);
				break;

			case MINUSEQ:
				novoLijevo = Builder().CreateSub(lijevo, desno);
				break;

			case ZVJEQ:
				novoLijevo = Builder().CreateMul(lijevo, desno);
				break;

			case SLASHEQ:
				novoLijevo = Builder().CreateSDiv(lijevo, desno);
				break;

			case MODEQ:
				novoLijevo = Builder().CreateSRem(lijevo, desno);
				break;

			case LSHIFTEQ:
			{
				int smanjenDesni = llvm::dyn_cast<llvm::ConstantInt>(desno)->getValue().getSExtValue() % 32;
				novoLijevo = Builder().CreateShl(lijevo, smanjenDesni);
				break;
			}
			case RSHIFTEQ:
			{
				int smanjenDesni = llvm::dyn_cast<llvm::ConstantInt>(desno)->getValue().getSExtValue() % 32;
				novoLijevo = Builder().CreateAShr(lijevo, smanjenDesni);
				break;
			}
			case BANDEQ:
				novoLijevo = Builder().CreateAnd(lijevo, desno);
				break;

			case BOREQ:
				novoLijevo = Builder().CreateOr(lijevo, desno);
				break;

			case BXOREQ:
				novoLijevo = Builder().CreateXor(lijevo, desno);
				break;
		}

		Builder().CreateStore(novoLijevo, lijevo);
		return novoLijevo;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Alokacija

	Alokacija::Alokacija(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	Alokacija::Alokacija(Alokacija const& drugi) : 
		IzrazAST(drugi) 
	{}

	llvm::Value* Alokacija::GenerirajKodIzraz()
	{
		// dohvati tip
		llvm::Type* tip = m_djeca.front()->GenerirajKodTip();
		llvm::Type* intTip = Context().PrevediTip("int");
		std::string tipString = Context().PrevediTip(tip);
		m_djeca.pop_front();

		// dohvati sizeof(tip)
		llvm::Constant* velicinaObjekta = llvm::ConstantInt::get(intTip, Module()->getDataLayout().getTypeAllocSize(tip));
		
		// i kreiraj poziv malloc(tip*, sizeof(tip))
		llvm::Instruction* pozivMalloc = llvm::CallInst::CreateMalloc(Builder().GetInsertBlock(), tip->getPointerTo(), tip, velicinaObjekta, nullptr, nullptr, "");
		Builder().Insert(pozivMalloc);
		//(*Context().TipoviPointera())[pozivMalloc] = tipString;

		return pozivMalloc;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// AlokacijaArray

	AlokacijaArray::AlokacijaArray(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) 
		: IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	AlokacijaArray::AlokacijaArray(AlokacijaArray const& drugi) : 
		IzrazAST(drugi) 
	{}

	llvm::Value* AlokacijaArray::GenerirajKodIzraz()
	{
		// dohvati tip arraya i zapamti Tip* za int
		llvm::Type* tip = m_djeca.front()->GenerirajKodTip();
		llvm::Type* intTip = Context().PrevediTip("int");
		std::string tipString = std::dynamic_pointer_cast<Leaf>(m_djeca.front())->Sadrzaj();
		m_djeca.pop_front();
		
		llvm::Constant* kolicina = static_cast<llvm::Constant*>(m_djeca.front()->GenerirajKodIzraz());

		// dohvati sizeof(tip)
		llvm::Constant* velicinaObjekta = llvm::ConstantInt::get(intTip, Module()->getDataLayout().getTypeAllocSize(tip));
		llvm::Constant* velicinaPolja = llvm::ConstantExpr::getMul(velicinaObjekta, kolicina);

		// kreiraj poziv malloc(tip*, sizeof(tip))
		llvm::Instruction* pozivMalloc = llvm::CallInst::CreateMalloc(Builder().GetInsertBlock(), tip->getPointerTo(), tip, velicinaObjekta, nullptr, nullptr, "");
		Builder().Insert(pozivMalloc);
		//(*Context().TipoviPointera())[pozivMalloc] = tipString;

		return pozivMalloc;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Negacija

	Negacija::Negacija(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}
	
	Negacija::Negacija(Negacija const& drugi) : 
		IzrazAST(drugi) 
	{}

	llvm::Value* Negacija::GenerirajKodIzraz()
	{
		llvm::Value* operand = m_djeca.front()->GenerirajKodIzraz();

		if (operand->getType() != Context().PrevediTip("bool"))
			throw SemantickaGreska(m_djeca.front()->Redak(), m_djeca.front()->Stupac(), "Operand operatora negacije mora biti tipa bool.");

		m_djeca.pop_front();
		operand = Builder().CreateNot(operand, "logickaNegacija");

		// pretvorimo bool u broj 0.0 ili 1.0 (jer LLVM tako kaže)
		return Builder().CreateUIToFP(operand, llvm::Type::getDoubleTy(*Context().LLVMContext().get()), "rezultatLogickaNegacija");
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Tilda

	Tilda::Tilda(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	Tilda::Tilda(Tilda const& drugi) : 
		IzrazAST(drugi) 
	{}

	llvm::Value* Tilda::GenerirajKodIzraz()
	{
		llvm::Value* operand = m_djeca.front()->GenerirajKodIzraz();
		
		if (operand->getType() != Context().PrevediTip("int"))
			throw SemantickaGreska(m_djeca.front()->Redak(), m_djeca.front()->Stupac(), "Operand bitovne negacije mora biti tipa int.");

		m_djeca.pop_front();
		operand = Builder().CreateNot(operand, "bitovnaNegacija");

		// pretvorimo bool u broj 0.0 ili 1.0 (jer LLVM tako kaže)
		return Builder().CreateUIToFP(operand, llvm::Type::getDoubleTy(*Context().LLVMContext().get()), "rezultatBitovnaNegacija");
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Minus

	Minus::Minus(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	Minus::Minus(Minus const& drugi) : 
		IzrazAST(drugi) 
	{}

	llvm::Value* Minus::GenerirajKodIzraz()
	{
		llvm::Value* operand = m_djeca.front()->GenerirajKodIzraz();

		if (operand->getType() != Context().PrevediTip("int"))
			throw SemantickaGreska(m_djeca.front()->Redak(), m_djeca.front()->Stupac(), "Operand aritmetičke negacije mora biti tipa int.");

		m_djeca.pop_front();
		operand = Builder().CreateNot(operand, "aritmetickaNegacija");

		// pretvorimo bool u broj 0.0 ili 1.0 (jer LLVM tako kaže)
		return Builder().CreateUIToFP(operand, llvm::Type::getDoubleTy(*Context().LLVMContext().get()), "rezultatAritmetickaNegacija");
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Dereferenciranje
	
	Dereferenciranje::Dereferenciranje(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	Dereferenciranje::Dereferenciranje(Dereferenciranje const& drugi) : 
		IzrazAST(drugi) 
	{}

	llvm::Value* Dereferenciranje::GenerirajKodIzraz()
	{
		llvm::Value* pointerKaoBroj = m_djeca.front()->GenerirajKodIzraz();
		llvm::Type* tipPointera = Context().PrevediTip(Context().PrevediTip(pointerKaoBroj->getType()) + "*");
		llvm::Value* pointer = Builder().CreateIntToPtr(pointerKaoBroj, tipPointera);
		return Builder().CreateLoad(pointer, tipPointera);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// PozivFunkcije

	PozivFunkcije::PozivFunkcije(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca))
	{}

	PozivFunkcije::PozivFunkcije(PozivFunkcije const& drugi) :
		IzrazAST(drugi)
	{}

	llvm::Value* PozivFunkcije::GenerirajKodIzraz()
	{
		std::string ime = std::dynamic_pointer_cast<Leaf>(m_djeca.front())->Sadrzaj();
		m_djeca.pop_front();

		llvm::Function* funkcija = Context().Module()->getFunction(ime);
		if (funkcija == nullptr) 
			throw SemantickaGreska(Redak(), Stupac(), "Ne postoji funkcija s imenom " + ime + ".");

		ASTList& dobiveniArgumenti = *std::dynamic_pointer_cast<ASTList>(m_djeca.front());
		
		if (dobiveniArgumenti.size() != funkcija->arg_size())
		{
			std::stringstream poruka;
			poruka << "Funkcija " << ime << " prima " << funkcija->arg_size()
				<< " argumenata (poslano " << dobiveniArgumenti.size() << ").";
			throw SemantickaGreska(Redak(), Stupac(), poruka.str());
		}

		std::vector<llvm::Value*> argumenti;
		for (ASTList::iterator it = dobiveniArgumenti.begin(); it != dobiveniArgumenti.end(); ++it)
			argumenti.push_back((*it)->GenerirajKodIzraz());
		
		int i = 0;
		for (llvm::Function::arg_iterator it = funkcija->arg_begin(); it != funkcija->arg_end(); ++it)
		{
			if (it->getType() != argumenti[i++]->getType())
			{
				std::stringstream poruka;
				poruka << i << ". argument u pozivu funkcije " << ime << " ne odgovara definiciji. "
					<< "Očekujem '" << Context().PrevediTip(it->getType()) << "', dobio ' "
					<< Context().PrevediTip(argumenti[--i]->getType()) << "'.";
				throw new SemantickaGreska(Redak(), Stupac(), poruka.str());
			}
		}

		llvm::CallInst* poziv = llvm::CallInst::Create(funkcija, argumenti, "", Context().TrenutniBlok()->Sadrzaj());
		return poziv;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Inkrement

	Inkrement::Inkrement(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	Inkrement::Inkrement(Inkrement const& drugi) : 
		IzrazAST(drugi) 
	{}

	llvm::Value* Inkrement::GenerirajKodIzraz()
	{
		llvm::Value* operand = m_djeca.front()->GenerirajKodIzraz();
		llvm::Constant* jedan = llvm::ConstantInt::get(Context().PrevediTip("int"), 1);

		m_djeca.pop_front();

		//// dohvati poznatu vrijednost operanda
		//Builder().CreateLoad(operand, operand->getName());
		//if (operand == nullptr)
		//	throw SemantickaGreska(Redak(), Stupac(), "Varijabla " + std::string(operand->getName()) + " nije deklarirana.");

		llvm::Value* novaVrijednost = Builder().CreateAdd(operand, jedan);
		Builder().CreateStore(novaVrijednost, operand);

		return novaVrijednost;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Dekrement

	Dekrement::Dekrement(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) :
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	Dekrement::Dekrement(Dekrement const& drugi) : 
		IzrazAST(drugi) 
	{}

	llvm::Value* Dekrement::GenerirajKodIzraz()
	{
		llvm::Value* operand = m_djeca.front()->GenerirajKodIzraz();
		llvm::Constant* jedan = llvm::ConstantInt::get(Context().PrevediTip("int"), 1);

		m_djeca.pop_front();

		//// dohvati poznatu vrijednost operanda
		//Builder().CreateLoad(operand, operand->getName());
		//if (operand == nullptr)
		//	throw SemantickaGreska(Redak(), Stupac(), "Varijabla " + std::string(operand->getName()) + " nije deklarirana.");

		llvm::Value* novaVrijednost = Builder().CreateSub(operand, jedan);
		Builder().CreateStore(novaVrijednost, operand);

		return novaVrijednost;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// UglateZagrade

	UglateZagrade::UglateZagrade(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	UglateZagrade::UglateZagrade(UglateZagrade const& drugi) : 
		IzrazAST(drugi) 
	{}

	llvm::Value* UglateZagrade::GenerirajKodIzraz()
	{
		llvm::Value* polje = m_djeca.front()->GenerirajKodIzraz();
		m_djeca.pop_front();

		llvm::Value* indeks = m_djeca.front()->GenerirajKodIzraz();
		if (Context().PrevediTip(indeks->getType()) != "int")
			throw SemantickaGreska(Redak(), Stupac(), "Indeks mora biti tipa 'int'!");

		m_djeca.pop_front();
		
		llvm::Value* dohvaceniElement = Builder().CreateInBoundsGEP(polje, indeks);

		return dohvaceniElement;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Tip

	Tip::Tip(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		TipAST(redak, stupac, std::move(djeca)),
		m_ime(std::dynamic_pointer_cast<Leaf>(*m_djeca.begin())->Sadrzaj())
	{
		m_djeca.pop_front();
		
		// ako imaš više od jednog djeteta, onda ime ima više riječi. konkateniraj ih sve;
		// to je ime tipa. (kroz ovo nikad neće proći više od jednom, ali nek je while za
		// svaki slučaj
		while (!m_djeca.empty())
		{
			m_ime += ' ';
			m_ime += std::dynamic_pointer_cast<Leaf>(*m_djeca.begin())->Sadrzaj();
			m_djeca.pop_front();
		}
	}

	Tip::Tip(Tip const& drugi) : 
		TipAST(drugi) 
	{}

	llvm::Type* Tip::GenerirajKodTip()
	{
		llvm::Type* ret = Context().PrevediTip(m_ime);
		if (ret == nullptr)
			throw new SemantickaGreska(Redak(), Stupac(), "Nepoznat tip '" + m_ime + "'!");

		return ret;
	}
	
	////////////////////////////////////////////////////////////////////////////////////
	// Tocka

	Tocka::Tocka(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	Tocka::Tocka(Tocka const& drugi) : 
		IzrazAST(drugi) 
	{}

	llvm::Value* Tocka::GenerirajKodIzraz()
	{
		// dohvatimo ime varijable 
		llvm::Value* varijabla = m_djeca.front()->GenerirajKodIzraz();
		std::string imeVarijable = varijabla->getName();
		m_djeca.pop_front();

		// dohvatimo ime strukture (zapravo tipa varijable)
		std::string imeStrukture = Context().PrevediTip(Context().LokalneVarijable()[imeVarijable]->getType());

		// dohvatimo ime elementa koji nas zanima
		llvm::Value* element = m_djeca.front()->GenerirajKodIzraz();
		m_djeca.pop_front();

		// pročitamo indeks tog elementa u strukturi
		int indeksElementa = Context().DohvatiIndeksElementaStrukture(imeStrukture, element->getName());
		
		// prevedemo taj indeks u LLVM jezik
		llvm::Value* indeks = llvm::ConstantInt::get(*Context().LLVMContext(), llvm::APInt(32, indeksElementa, true));

		llvm::AllocaInst* alokacija = Builder().CreateAlloca(Context().PrevediTip(imeStrukture), 0, "");
		Builder().CreateStore(varijabla, alokacija);

		std::vector<llvm::Value*> indeksi(2);
		// prvi indeks uvijek mora biti 0 jer LLVM tako kaže
		indeksi[0] = llvm::ConstantInt::get(*Context().LLVMContext().get(), llvm::APInt(32, 0, true));
		
		// a drugi je zapravo indeks elementa kojeg dohvaćamo
		indeksi[1] = indeks;

		// dohvatimo pointer na element
		llvm::Value* elementStrukture = Builder().CreateGEP(Context().PrevediTip(imeStrukture), alokacija, indeksi, "");

		// i onda dohvatimo element
		llvm::Value* ucitanElement = Builder().CreateLoad(elementStrukture, "");

		return ucitanElement;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Strelica

	Strelica::Strelica(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) :
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	Strelica::Strelica(Strelica const& drugi) :
		IzrazAST(drugi) 
	{}
	
	llvm::Value* Strelica::GenerirajKodIzraz()
	{
		// dereferenciraj lijevu stranu
		llvm::Value* pointerKaoBroj = m_djeca.front()->GenerirajKodIzraz();
		llvm::Type* tipPointera = Context().PrevediTip(Context().PrevediTip(pointerKaoBroj->getType()) + "*");
		llvm::Value* pointer = Builder().CreateIntToPtr(pointerKaoBroj, tipPointera);
		llvm::Value* lijevo = Builder().CreateLoad(pointer, tipPointera);

		// i vrati odgovarajući element strukture
		std::string imeVarijable = lijevo->getName();
		m_djeca.pop_front();

		std::string imeStrukture = Context().PrevediTip(Context().LokalneVarijable()[imeVarijable]->getType());
		llvm::Value* element = m_djeca.front()->GenerirajKodIzraz();
		m_djeca.pop_front();
		
		int indeksElementa = Context().DohvatiIndeksElementaStrukture(imeStrukture, element->getName());
		llvm::Value* indeks = llvm::ConstantInt::get(*Context().LLVMContext(), llvm::APInt(32, indeksElementa, true));

		llvm::AllocaInst* alokacija = Builder().CreateAlloca(Context().PrevediTip(imeStrukture), 0, "");
		Builder().CreateStore(lijevo, alokacija);

		std::vector<llvm::Value*> indeksi(2);
		indeksi[0] = llvm::ConstantInt::get(*Context().LLVMContext().get(), llvm::APInt(32, 0, true));
		indeksi[1] = indeks;

		llvm::Value* elementStrukture = Builder().CreateGEP(Context().PrevediTip(imeStrukture), alokacija, indeksi, "");
		llvm::Value* ucitanElement = Builder().CreateLoad(elementStrukture, "");

		return ucitanElement;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// BrojLiteral

	BrojLiteral::BrojLiteral(Token const& sadrzaj) :
		Leaf(sadrzaj)
	{}

	llvm::Value* BrojLiteral::GenerirajKodIzraz()
	{
		return llvm::ConstantInt::get(Context().PrevediTip("int"), (int)*this, true);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// CharLiteral

	CharLiteral::CharLiteral(Token const& sadrzaj) :
		Leaf(sadrzaj)
	{}

	llvm::Value* CharLiteral::GenerirajKodIzraz()
	{
		return llvm::ConstantInt::get(Context().PrevediTip("char"), (char)*this, true);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// BoolLiteral

	BoolLiteral::BoolLiteral(Token const& sadrzaj) :
		Leaf(sadrzaj)
	{}

	BoolLiteral::operator bool() const
	{
		if (m_sadrzaj.Sadrzaj() == "true")
			return true;

		if (m_sadrzaj.Sadrzaj() == "false")
			return false;
	}

	llvm::Value* BoolLiteral::GenerirajKodIzraz()
	{
		return llvm::ConstantInt::get(Context().PrevediTip("bool"), (bool)*this);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// StringLiteral

	StringLiteral::StringLiteral(Token const& sadrzaj) :
		Leaf(sadrzaj)
	{}

	llvm::Value* StringLiteral::GenerirajKodIzraz()
	{
		return llvm::ConstantDataArray::getString(*Context().LLVMContext(), (const char*)(*this));
	}

}