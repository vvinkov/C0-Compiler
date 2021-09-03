#include <typeinfo>
#include <vector> // potreban zato što LLVM očekuje da su neke stvari poredane na slijednim memorijskim lokacijama

#include "../Lekser/Lekser.hpp"
#include "../Parser/Parser.hpp"
#include "AST.hpp"
#include "../LLVM-Compatibility/LLVM-Compatibility.hpp"
#include "llvm/Support/ErrorHandling.h"

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

			// ovo su sad binarni operatori
			//else if (tipDjeteta == typeid(BitovniOperator))
			//	novoDijete = std::make_shared<BitovniOperator>((BitovniOperator const&)**it);

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
	//	//sirocad.push_back(shared_from_this());
	}

	AST::AST(AST const& drugi)
	{
		m_redak = drugi.m_redak;
		m_stupac = drugi.m_stupac;
		m_roditelj = nullptr;
		ucitajDjecu(drugi.m_djeca);
		//sirocad.push_back(shared_from_this());
	}

	AST::AST(AST&& drugi) : m_djeca(std::move(drugi.m_djeca)) 
	{
		// svakom djetetu stavi sebe za roditelja
		for (std::list<std::shared_ptr<AST>>::iterator it = m_djeca.begin(); it != m_djeca.end(); ++it)
			(*it)->Roditelj(this);

		m_redak = drugi.m_redak;
		m_stupac = drugi.m_stupac;
		m_roditelj = drugi.m_roditelj;
	}

	AST::AST(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : m_djeca(std::move(djeca))
	{
		// svakom svom djetetu stavi sebe za roditelja
		for (std::list<std::shared_ptr<AST>>::iterator it = m_djeca.begin(); it != m_djeca.end();)
		{
			if (*it != nullptr)
				(*it++)->Roditelj(this);
			else
				m_djeca.erase(it++);
		}

		m_redak = redak;
		m_stupac = stupac;
		m_roditelj = nullptr;
		//sirocad.push_back(shared_from_this());
	}

	void AST::IspisiDjecu(std::ostream& out) const
	{
		int visinaPodstabla = 0;
		// pogledaj koliko si duboko u stablu
		for(AST const* trenutni = this; !trenutni->IsRoot(); trenutni = trenutni->m_roditelj)
			++visinaPodstabla;

		// zatim ispiši djecu
		for (std::list<std::shared_ptr<AST>>::const_iterator it = m_djeca.begin(); it != m_djeca.end(); ++it)
		{
			// prije svakog djeteta nalijepi onoliko tabulatora 
			// kolika je dubina te grane u glavnom AST-u
			for (int i = 0; i < visinaPodstabla; ++i)
				out << '\t';

			// pa ispiši dijete
			(*it)->Ispisi(out);
		}
	}

	void AST::dodajDijete(std::shared_ptr<AST> dijete)
	{
		if (dijete)
		{
			// dodaj si dijete
			m_djeca.push_back(dijete);

			// i reci tom djetetu da si mu sad ti roditelj
			m_djeca.back()->Roditelj(this);
		}
	}

	void AST::Roditelj(AST* roditelj)
	{
		// postavi roditelja
		m_roditelj = roditelj;

		// i izbriši se iz siročadi
		//for (std::list<std::shared_ptr<AST>>::iterator it = sirocad.begin(); it != sirocad.end(); ++it)
		//{
		//	if (*it == shared_from_this())
		//	{
		//		sirocad.erase(it);
		//		break;
		//	}
		//}
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
				(*it)->Roditelj(this);
		}
		return std::move(*this);
	}

	void AST::ProvjeraUvjeta(llvm::Value* uvjetProlaza, std::string const& opisIznimke, bool assert)
	{
		return;
	}

	void AST::ProvjeraVrijednosti(llvm::Value* vrijednost, std::string const& imeOperatora, int vrsta)
	{
		if(vrijednost == nullptr)
			throw new SemantickaGreska(Redak(), Stupac(), std::string("Operand operatora \"") + imeOperatora + "\" je neispravna vrijednost");
	}
	// ovdje smo stali - 3. argument mora biti std::string
	void AST::ProvjeraVrijednosti(llvm::Value* prvaVrijednost, llvm::Value* drugaVrijednost, std::string const& imeOperatora)
	{
		if(prvaVrijednost == nullptr)
			throw SemantickaGreska(Redak(), Stupac(), "Lijevi operand operatora \"" + imeOperatora + "\" je neispravna vrijednost");
		
		if(drugaVrijednost == nullptr)
			throw SemantickaGreska(Redak(), Stupac(), "Desni operand operatora \"" + imeOperatora + "\" je neispravna vrijednost");
	}

	void AST::ProvjeraTipa(llvm::Value* vrijednost, llvm::Type* ocekivaniTip)
	{
		if(vrijednost->getType() != ocekivaniTip)
			throw SemantickaGreska(Redak(), Stupac(), "Pogrešan tip! Očekujem " + Context().PrevediTip(ocekivaniTip));
	}

	void AST::ProvjeraPolja(llvm::Value* vrijednost)
	{
		std::string tipVrijednosti = Context().PrevediTip(vrijednost->getType());
		if(tipVrijednosti != "string" && 
			(tipVrijednosti[tipVrijednosti.size()-2] != '[' || tipVrijednosti[tipVrijednosti.size()-1] != ']'))
				throw SemantickaGreska(Redak(), Stupac(), "Pogrešan tip! Očekujem polje ili string");
	}

	std::ostream& operator<<(std::ostream& out, AST const& ast)
	{
		ast.Ispisi(out);
		return out;
	}

	AST::~AST()
	{
		//pobrisiDjecu();

		// ako si siroče, izbriši se iz popisa siročadi
		//for (std::list<std::shared_ptr<AST>>::iterator it = sirocad.begin(); it != sirocad.end(); ++it)
		//{
		//	if (*it == shared_from_this())
		//	{
		//		sirocad.erase(it);
		//		break;
		//	}
		//}
	}
	
	////////////////////////////////////////////////////////////////////////////////////
	// Program
	// DONE

	Program::Program() : AST(0, 0)
	{
		//setKorijen(shared_from_this()); 
	}

	void Program::GenerirajKod()
	{
		for (std::list<std::shared_ptr<AST>>::iterator it = m_djeca.begin(); it != m_djeca.end(); ++it)
		{
			// uvijek je točno jedan od sljedeća 3 poziva netrivijalan i
			// to nikad nije GenerirajKodIzraz; taj je tu da bude očito
			// da negdje nešto ne štima ako taj ispadne netrivijalan
			//(*it)->GenerirajKodIzraz();
			(*it)->GenerirajKodTip();
			(*it)->GenerirajKodFunkcija();
		}
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Use direktiva
	// DONE

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
		std::ofstream drugiLexOut(path + "LekserOut.txt");
		try
		{
			drugiTokeni = drugiLex.leksiraj();
			for (std::deque<std::shared_ptr<C0Compiler::Token>>::iterator it = drugiTokeni.begin(); it != drugiTokeni.end(); ++it)
				drugiLexOut << **it << std::endl;
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
	// DONE

	DeklaracijaStrukture::DeklaracijaStrukture(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		TipAST(redak, stupac, std::move(djeca)), 
		m_ime(std::dynamic_pointer_cast<Leaf>(m_djeca.front())->Sadrzaj()) 
	{
		// možeš popnuti front jer si dohvatio ime (prvo dijete u AST-u)
		m_djeca.pop_front();
	}

	void DeklaracijaStrukture::Ispisi(std::ostream& out) const
	{
		out << "DeklaracijaStrukture" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Type* DeklaracijaStrukture::GenerirajKodTip()
	{
		if (!Context().StrukturaDefinirana(m_ime))
		{
			llvm::StructType* novaStruktura = llvm::StructType::create(*Context().LLVMContext(), m_ime);
			Context().DodajStrukturu(m_ime);
			Context().DodajNoviTip("struct " + m_ime, novaStruktura);
			return novaStruktura;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Definicija strukture
	// DONE

	DefinicijaStrukture::DefinicijaStrukture(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		DeklaracijaStrukture(redak, stupac, std::move(djeca))
	{}
	
	DefinicijaStrukture::DefinicijaStrukture(DefinicijaStrukture const& drugi) :
		DeklaracijaStrukture(drugi),
		m_tipoviElemenata(drugi.m_tipoviElemenata),
		m_imenaElemenata(drugi.m_imenaElemenata)
	{}

	void DefinicijaStrukture::Ispisi(std::ostream& out) const
	{
		out << "DefinicijaStrukture" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Type* DefinicijaStrukture::GenerirajKodTip()
	{
		if(Context().StrukturaDefinirana(m_ime))
			throw SemantickaGreska(Redak(), Stupac(), "Struktura " + m_ime + " je već definirana");

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
			Context().DodajClanStrukture(m_ime, std::dynamic_pointer_cast<Leaf>(*elementi.begin())->Sadrzaj());

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

	void DeklaracijaFunkcije::Ispisi(std::ostream& out) const
	{
		out << "DeklaracijaFunkcije" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Function* DeklaracijaFunkcije::GenerirajKodFunkcija()
	{
		// spremi povratni tip funkcije
		llvm::Type* povratniTip = m_djeca.front()->GenerirajKodTip();
		m_djeca.pop_front();

		// dohvati ime funkcije pa popni i njega
		std::string ime = std::dynamic_pointer_cast<Leaf>(*m_djeca.begin())->Sadrzaj();
		m_djeca.pop_front();

		// dijelimo duljinu liste s 2 jer ona sadrži i tipove i imena
		ASTList& argumenti = *std::dynamic_pointer_cast<ASTList>(m_djeca.front());
		std::vector<llvm::Type*> tipoviArgumenata(argumenti.size() / 2);
		std::vector<std::string> imenaArgumenata(argumenti.size() / 2);

		for (int i = 0; !argumenti.empty(); ++i)
		{
			// prevedi tip iz stringa u llvm::Type* i pushaj ga u tipove
			tipoviArgumenata[i] = ((*argumenti.begin())->GenerirajKodTip());
			argumenti.pop_front();

			// i spremi mu ime za kasnije
			imenaArgumenata[i] = (std::dynamic_pointer_cast<Leaf>((*argumenti.begin()))->Sadrzaj());
			argumenti.pop_front();
		}
		m_djeca.pop_front();

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

	void DefinicijaFunkcije::Ispisi(std::ostream& out) const
	{
		out << "DefinicijaFunkcije" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Function* DefinicijaFunkcije::GenerirajKodFunkcija()
	{
		// skočimo na dijete koje sadrži ime funkcije
		ASTList::iterator it = m_djeca.begin();
		++it;
		std::string ime = std::dynamic_pointer_cast<Leaf>(*it)->Sadrzaj();
		
		// Provjera je li funkcija s tim imenom već deklarirana
		llvm::Function* funkcija = Module()->getFunction(ime);

		// ako ne, deklariraj je
		if (funkcija == nullptr)
			funkcija = DeklaracijaFunkcije::GenerirajKodFunkcija();

		// ako je funkcija već definirana, prijavi grešku
		else if (!funkcija->empty())
			throw SemantickaGreska(Redak(), Stupac(), "Funkcija " + ime + " je već definirana");

		// ako je funkcija deklarirana i još nema tijelo, Provjera ima li isti tip
		else if (funkcija->getReturnType() != (*m_djeca.begin())->GenerirajKodTip())
			throw SemantickaGreska(Redak(), Stupac(), "Funkcija '" + ime + "' nema isti povratni tip kao u deklaraciji");

		// ako je funkcija deklarirana, još nema tijelo i ima isti tip, Provjera prima li iste argumente
		else
		{
			// popni povratni tip i ime funkcije; više nam ne trebaju
			m_djeca.pop_front();
			m_djeca.pop_front();

			// zatim izvuci tipove argumenata iz zadnjeg djeteta
			ASTList& argumenti = *std::dynamic_pointer_cast<ASTList>(m_djeca.front());
			std::vector<llvm::Type*> tipoviArgumenata;
			std::vector<std::string> imenaArgumenata;
			int maxSize = std::dynamic_pointer_cast<ASTList>(m_djeca.front())->size() / 2;
			tipoviArgumenata.reserve(maxSize);
			while (!argumenti.empty())
			{
				// prevedi tip iz stringa u llvm::Type* i pushaj ga u tipove
				tipoviArgumenata.push_back((*argumenti.begin())->GenerirajKodTip());
				argumenti.pop_front();

				// zapamti i ime za slučaj da svi tipovi odgovaraju
				imenaArgumenata.push_back(std::dynamic_pointer_cast<Leaf>(*argumenti.begin())->Sadrzaj());
				argumenti.pop_front();
			}

			int i = 0;
			for (llvm::Function::arg_iterator it = funkcija->arg_begin(); it != funkcija->arg_end(); ++it)
			{
				// ako tip ne odgovara, bacamo semantičku grešku. nema overloadanja u C0!
				if (it->getType() != tipoviArgumenata[i])
					throw SemantickaGreska(Redak(), Stupac(), "Funkcija '" + ime + "' nema iste argumente kao u deklaraciji");

				// ako tip odgovara, onda ime argumenta u deklarciji setiramo na ime argumenta u definiciji.
				// kad nešto budemo radili s njima, uvijek ćemo raditi s imenom iz definicije
				it->setName(imenaArgumenata[i]);
				++i;
			}

			// popnemo argumente jer nam više ne trebaju; već smo ih dobili kad je funkcija bila deklarirana
			m_djeca.pop_front();
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
			Context().TrenutniBlok()->Dodaj(alokacija, it->getName());
		}

		try
		{
			llvm::Value* povratnaVrijednost = m_djeca.front()->GenerirajKodIzraz();
			if (povratnaVrijednost != nullptr)
			{
				// na kraju, Provjera je li funkcija ispravno konstruirana (vraća false ako je sve ok)
				std::string error;
				llvm::raw_string_ostream errorStream(error);
				llvm::verifyFunction(*funkcija, &errorStream);
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
			funkcija->eraseFromParent();
			// propagiraj error do main-a
			throw;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Typedef
	// DONE

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

	void TypeDef::Ispisi(std::ostream& out) const
	{
		out << "TypeDef" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Value* TypeDef::GenerirajKodIzraz()
	{
		// ako nismo u globalnom dosegu, bacamo grešku
		if (Context().TrenutniBlok()->Roditelj() != nullptr)
			throw SemantickaGreska(Redak(), Stupac(), "typedef se smije pojavljivati samo u globalnom dosegu");
		
		Context().AliasiTipova()[m_alias] = m_tip;
		return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*Context().LLVMContext()));
	}

	////////////////////////////////////////////////////////////////////////////////////
	// IfElse
	// DONE
	IfElse::IfElse(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	IfElse::IfElse(IfElse const& drugi) : 
		IzrazAST(drugi) 
	{}

	void IfElse::Ispisi(std::ostream& out) const
	{
		out << "IfElse" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Value* IfElse::GenerirajKodIzraz()
	{
		// prvo kompajliramo uvjet
		llvm::Value* uvjet = m_djeca.front()->GenerirajKodIzraz();
		m_djeca.pop_front();

		ProvjeraVrijednosti(uvjet);
		ProvjeraTipa(uvjet, Context().PrevediTip("bool"));

		// dohvati funkciju u kojoj se "if" nalazi i njen blok
		//auto blokFunkcije = Context().TrenutniBlok();
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
		bool imamElse = !m_djeca.empty();

		// i bezuvjetno granamo u originalnu funkciju
		Builder().CreateBr(nastavakBlok);
		Context().ZatvoriBlok();
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
		Context().ZatvoriBlok();
	
		funkcija->getBasicBlockList().push_back(nastavakBlok);
		Builder().SetInsertPoint(nastavakBlok);

		//Context().TrenutniBlok()->Roditelj(blokFunkcije);
		// treba namjestiti da početni blok bude parent nastavku

		// phi čvor u builder. sad je void
		//llvm::PHINode* phi = Builder().CreatePHI(llvm::Type::getVoidTy(*Context().LLVMContext()), 2);
		//
		//// i dodamo mu grane
		//phi->addIncoming(ifTrueVrijednost, ifTrueBlok);
		//
		//if(elseVrijednost != nullptr)
		//	phi->addIncoming(elseVrijednost, elseBlok);
		//
		//// LLVM vraćanje nullptr-a tretira kao grešku pa vraćam konstantu 0
		//return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*Context().LLVMContext()));;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// While
	// DONE
	While::While(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}
	
	While::While(While const& drugi) : 
		IzrazAST(drugi) 
	{}

	void While::Ispisi(std::ostream& out) const
	{
		out << "While" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Value* While::GenerirajKodIzraz()
	{
		llvm::Function* funkcija = Builder().GetInsertBlock()->getParent();

		// inicijaliziramo blokove
		m_uvjetPetlje = llvm::BasicBlock::Create(*Context().LLVMContext(), "uvjetPetlje", funkcija);
		m_tijeloPetlje = llvm::BasicBlock::Create(*Context().LLVMContext(), "tijeloPetlje");
		m_krajPetlje = llvm::BasicBlock::Create(*Context().LLVMContext(), "nastavakFunkcije");

		// otvorimo blok za uvjet petlje
		Builder().SetInsertPoint(m_uvjetPetlje);

		// kompajliramo uvjet petlje
		llvm::Value *uvjetNastavka = m_djeca.front()->GenerirajKodIzraz();
		if (uvjetNastavka == nullptr)
			return nullptr;

		if (Context().PrevediTip(uvjetNastavka->getType()) != "bool")
			throw SemantickaGreska(Redak(), Stupac(), "Uvjet u 'while'-petlji mora biti tipa bool");

		m_djeca.pop_front();
		// pretvaramo vrijednost u bool tako da je uspoređujemo s 0.0
		uvjetNastavka = Builder().CreateICmpNE(uvjetNastavka, llvm::ConstantInt::get(*Context().LLVMContext(), llvm::APInt(1, 0)), "uvjetNastava");

		// ovisno o uvjetu, granamo u tijelo petlje ili na kraj petlje
		Builder().CreateCondBr(uvjetNastavka, m_tijeloPetlje, m_krajPetlje);

		// napravimo blok za tijelo petlje
		Context().DodajBlok(m_tijeloPetlje);
		funkcija->getBasicBlockList().push_back(m_tijeloPetlje);
		Builder().SetInsertPoint(m_tijeloPetlje);

		// kompajliramo tijelo petlje
		llvm::Value* tijelo = m_djeca.front()->GenerirajKodIzraz();
		if (tijelo == nullptr)
			return nullptr;

		m_djeca.pop_front();

		// nakon što se izvrši tijelo petlje, bezuvjetno granamo u uvjet petlje
		Builder().CreateBr(m_uvjetPetlje);

		// zatvorimo blok s tijelom petlje te otvorimo blok koji slijedi
		Context().ZatvoriBlok();
		Context().DodajBlok(m_krajPetlje);
		Builder().SetInsertPoint(m_krajPetlje);
		funkcija->getBasicBlockList().push_back(m_krajPetlje);

		// LLVM kaže da izraz while uvijek vraća 0 
		return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*Context().LLVMContext()));
	}
	
	////////////////////////////////////////////////////////////////////////////////////
	// For

	For::For(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}
	
	For::For(For const& drugi) 
		: IzrazAST(drugi) 
	{}

	void For::Ispisi(std::ostream& out) const
	{
		out << "For" << std::endl;
		IspisiDjecu(out);
	}

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
		llvm::Value* uvjetNastavka = m_djeca.front()->GenerirajKodIzraz();
		m_djeca.pop_front();
		if (uvjetNastavka == nullptr)
			return nullptr;

		if (Context().PrevediTip(uvjetNastavka->getType()) != "bool")
			throw SemantickaGreska(Redak(), Stupac(), "Uvjet u 'for'-petlji mora biti tipa bool");

		// pretvaramo vrijednost u bool tako da je uspoređujemo s 0.0
		uvjetNastavka = Builder().CreateICmpNE(uvjetNastavka, llvm::ConstantInt::get(*Context().LLVMContext(), llvm::APInt(1, 0)), "uvjetNastavka");
		
		// dodajemo blokove za tijelo petlje, inkrement petlje te nastavak
		// funkcije nakon završetka petlje (ali inkrement i nastavak još ne stavljamo u funkciju).
		// pamtimo gdje su inkrement i kraj petlje tako da break i continue znaju kamo skaču
		llvm::BasicBlock* tijeloPetlje = llvm::BasicBlock::Create(*Context().LLVMContext(), "tijeloPetlje", funkcija);
		m_inkrement = llvm::BasicBlock::Create(*Context().LLVMContext(), "inkrementPetlje");
		m_krajPetlje = llvm::BasicBlock::Create(*Context().LLVMContext(), "nastavakFunkcije");
		Builder().CreateCondBr(uvjetNastavka, tijeloPetlje, m_krajPetlje);

		// zatvorimo uvjetBlok i otvorimo tijeloPetlje
		Context().ZatvoriBlok();
		Context().DodajBlok(tijeloPetlje);
		Builder().SetInsertPoint(tijeloPetlje);  

		// generiramo kod za tijelo petlje
		llvm::Value* tijelo = m_djeca.front()->GenerirajKodIzraz();
		if (tijelo == nullptr)
			return nullptr;

		m_djeca.pop_front();

		// iz tijela bezuvjetno granamo u inkrement blok
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
		//Builder().CreateCondBr(uvjetNastavka, tijeloPetlje, nakonPetlje);
		//Builder().SetInsertPoint(nakonPetlje);

		//brojac->addIncoming(sljedecaVrijednostBrojca, krajPetlje);

		// nakon petlje vrati prešaranu varijablu
		//if (staraVarijabla != nullptr)
		//	m_doseg->Imena()[imeBrojaca] = staraVarijabla;
		//else
		//	m_doseg->Imena().erase(imeBrojaca);

		// LLVM kaže da izraz for uvijek vraća 0
		return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*Context().LLVMContext()));
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Return

	Return::Return(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	Return::Return(Return const& drugi) : 
		IzrazAST(drugi) 
	{}

	void Return::Ispisi(std::ostream& out) const
	{
		out << "Return" << std::endl;
		IspisiDjecu(out);
	}

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
				<< Context().PrevediTip(Builder().GetInsertBlock()->getParent()->getReturnType()) << "'";

			throw new SemantickaGreska(Redak(), Stupac(), poruka.str());
		}
		else
			// inače "vrati" void
			return Builder().CreateRetVoid();
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Break
	// DONE
	Break::Break(int redak, int stupac) : 
		IzrazAST(redak, stupac) 
	{}

	Break::Break(Break const& drugi) : 
		IzrazAST(drugi) 
	{}

	void Break::Ispisi(std::ostream& out) const
	{
		out << "Break" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Value* Break::GenerirajKodIzraz() 
	{ 
		AST* predakPetlja = Roditelj();

		// zapamtimo typeid od For i While. ne pamtimo typeid od
		// petlje pretka jer type_info ima izbrisan operator=
		type_info const& tipFor = typeid(For);
		type_info const& tipWhile = typeid(While);

		// penjemo se po stablu dok ne nađemo For ili While
		while (typeid(*predakPetlja) != tipFor && typeid(*predakPetlja) != tipWhile)
		{
			// ako smo došli do korijena i nismo našli petlju, bacamo grešku
			if (predakPetlja->IsRoot())
				throw new SemantickaGreska(Redak(), Stupac(), "Naredba break se mora nalaziti u petlji");

			// inače se penjemo jedan korak gore
			predakPetlja = predakPetlja->Roditelj();
		}
		
		// sad možemo zapamtiti tip petlje
		type_info const& tipPetlja = typeid(*predakPetlja);

		// ako je petlja For, skačemo van iz petlje na jedan način 
		Builder().CreateBr(dynamic_cast<For*>(predakPetlja)->BlokNakonPetlje());
		
		// ako je petlja While, skačemo van iz petlje na drugi način
		if (tipPetlja == tipWhile)
			Builder().CreateBr(dynamic_cast<While*>(predakPetlja)->BlokNakonPetlje());

		// povratna vrijednost nam je nebitna jer je ionako nigdje ne koristimo pa može nullptr
		return nullptr; 
	}; 

	////////////////////////////////////////////////////////////////////////////////////
	// Continue
	// DONE
	Continue::Continue(int redak, int stupac) : 
		IzrazAST(redak, stupac) 
	{}

	Continue::Continue(Continue const& drugi) : 
		IzrazAST(drugi) 
	{}

	void Continue::Ispisi(std::ostream& out) const
	{
		out << "Continue" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Value* Continue::GenerirajKodIzraz() 
	{
		AST* predakPetlja = Roditelj();

		// zapamtimo typeid od For i While. ne pamtimo typeid od
		// petlje pretka jer type_info ima izbrisan operator=
		type_info const& tipFor = typeid(For);
		type_info const& tipWhile = typeid(While);

		// penjemo se po stablu dok ne nađemo For ili While
		while (typeid(*predakPetlja) != tipFor && typeid(*predakPetlja) != tipWhile)
		{
			// ako smo došli do korijena i nismo našli petlju, bacamo grešku
			if (predakPetlja->IsRoot())
				throw new SemantickaGreska(Redak(), Stupac(), "Continue se ne nalazi u petlji");

			// inače se penjemo jedan korak gore
			predakPetlja = predakPetlja->Roditelj();
		}

		// sad možemo zapamtiti tip petlje
		type_info const& tipPetlja = typeid(*predakPetlja);

		// ako je petlja For, skačemo na inkrement
		if (tipPetlja == tipFor)
			Builder().CreateBr(dynamic_cast<For*>(predakPetlja)->BlokInkrement());

		// ako je petlja While, skačemo na provjeru uvjeta
		if (tipPetlja == tipWhile)
			Builder().CreateBr(dynamic_cast<While*>(predakPetlja)->UvjetPetlje());

		// povratna vrijednost nam je nebitna jer je ionako nigdje ne koristimo pa može nullptr
		return nullptr;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// Assert
	// DONE
	Assert::Assert(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	Assert::Assert(Assert const& drugi) : 
		IzrazAST(drugi) 
	{}

	void Assert::Ispisi(std::ostream& out) const
	{
		out << "Assert" << std::endl;
		IspisiDjecu(out);
	}
	
	llvm::Value* Assert::GenerirajKodIzraz()
	{
		// izračunamo vrijednost izraza
		llvm::Value* uvjet = m_djeca.front()->GenerirajKodIzraz();
		m_djeca.pop_front();

		if(uvjet == nullptr)
			throw SemantickaGreska(Redak(), Stupac(), "Uvjet u naredbi \"assert\" je neispravna vrijednost");

		if (Context().PrevediTip(uvjet->getType()) != "bool")
			throw SemantickaGreska(Redak(), Stupac(), "Uvjet u naredbi \"assert\" mora biti tipa bool");

		// pripremimo tekst greške
		std::stringstream tekstGreske;
		tekstGreske << "Assert nije prošao! Redak: " << Redak() << ", Stupac: " << Stupac() << ".";

		// i prijavimo je ako je uvjet false
		ProvjeraUvjeta(uvjet, tekstGreske.str(), true);
		
		// i vratimo nullptr jer se ova vrijednost ne smije nigdje koristiti
		return nullptr;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Varijabla
	// DONE

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

	void Varijabla::Ispisi(std::ostream& out) const
	{
		out << "Varijabla (" << m_ime << ")" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Value* Varijabla::GenerirajKodIzraz()
	{
		llvm::Value* varijabla = Context().TrenutniBlok()->Trazi(m_ime);
		if (varijabla == nullptr)
		{
			throw SemantickaGreska(Redak(), Stupac(), "Varijabla " + m_ime + " nije deklarirana");
			return nullptr;
		}

		// učitaj varijablu sa stacka
		return Builder().CreateLoad(varijabla, m_ime);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// DeklaracijaVarijable
	// DONE

	DeklaracijaVarijable::DeklaracijaVarijable(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}
	
	DeklaracijaVarijable::DeklaracijaVarijable(DeklaracijaVarijable const& drugi) 
		: IzrazAST(drugi)
	{}

	void DeklaracijaVarijable::Ispisi(std::ostream& out) const
	{
		out << "DeklaracijaVarijable" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Value* DeklaracijaVarijable::GenerirajKodIzraz()
	{
		bool statickaAlokacija = true;
		// dohvati tip, ime i pridruženu vrijednost varijabli (ako postoji)
		// dohvatit ćemo samo i tip kao std::string. ovdje koristimo činjenicu da
		// pri deklaraciji varijable lijevo od njenog imena može biti samo ime tipa
		// i ništa drugo
		std::string tipSlovima = *std::dynamic_pointer_cast<Tip>(m_djeca.front());
		llvm::Type* tip = m_djeca.front()->GenerirajKodTip();
		m_djeca.pop_front();

		std::string ime = std::dynamic_pointer_cast<Leaf>(m_djeca.front())->Sadrzaj();
		m_djeca.pop_front();

		if (Context().LokalneVarijable().find(ime) != Context().LokalneVarijable().end())
			throw SemantickaGreska(Redak(), Stupac(), "Varijabla " + ime + " je već deklarirana");

		llvm::Function* funkcija = Builder().GetInsertBlock()->getParent();
		llvm::Value* inicijalnaVrijednost = nullptr;

		// ako imamo inicijalnu vrijednost, kompajliramo je
		if (!m_djeca.empty())
		{
			// provjerimo je li s desne strane alloc ili alloc_array. 
			// ako da, onda se radi o dinamičkoj alokaciji
			if (std::dynamic_pointer_cast<Alokacija>(m_djeca.front()) != nullptr ||
				std::dynamic_pointer_cast<AlokacijaArray>(m_djeca.front()) != nullptr)
				statickaAlokacija = false;

			inicijalnaVrijednost = m_djeca.front()->GenerirajKodIzraz();
			std::string tipDesno = Context().PrevediTip(inicijalnaVrijednost->getType());
		
			if (tipDesno != tipSlovima)
				throw SemantickaGreska(Redak(), Stupac(), "Varijabla tipa " + tipSlovima +
					" ne može biti inicijalizirana vrijednošću tipa " + tipDesno);
		}
		
		// ako ne, koristimo defaultnu ovisno o tipu (C0)
		else if (tipSlovima == "int")
			inicijalnaVrijednost = llvm::ConstantInt::get(Context().PrevediTip("int"), 0, true);

		else if (tipSlovima == "char")
			inicijalnaVrijednost = llvm::ConstantInt::get(Context().PrevediTip("char"), '\0');

		else if (tipSlovima == "bool")
			inicijalnaVrijednost = llvm::ConstantInt::get(Context().PrevediTip("bool"), 0);

		else if (tipSlovima == "string")
		{
			// string literale je teže konstruirati od ostalih literala pa ćemo taj posao outsource-ati
			// klasi StringLiteral
			StringLiteral literal(Token(STRLIT, "", Redak(), Stupac()));
			inicijalnaVrijednost = literal.GenerirajKodIzraz();
		}

		// ako završava na *, onda mu je defaultna vrijednost nullptr
		else if (tipSlovima[tipSlovima.size()-1] == '*')
			inicijalnaVrijednost = llvm::ConstantPointerNull::get(static_cast<llvm::PointerType*>(tip));

		// dodamo stack alokaciju varijable
		llvm::Value* vrijednost;
		if (statickaAlokacija)
		{
			llvm::AllocaInst* alokacija = Context().StackAlokacija(funkcija, tip, ime);
			Builder().CreateStore(inicijalnaVrijednost, alokacija);
			vrijednost = alokacija;
		}
		else
			vrijednost = inicijalnaVrijednost;

		if(Context().TrenutniBlok()->Dodaj(vrijednost, ime) == nullptr)
			throw SemantickaGreska(Redak(), Stupac(), "Varijabla imena " + ime + " je već deklarirana.");
			
		return vrijednost;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// TernarniOperator

	TernarniOperator::TernarniOperator(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) :
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	TernarniOperator::TernarniOperator(TernarniOperator const& drugi) : 
		IzrazAST(drugi) 
	{}

	void TernarniOperator::Ispisi(std::ostream& out) const
	{
		out << "TernarniOperator" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Value* TernarniOperator::GenerirajKodIzraz()
	{	
		// implementiramo ga kao if-else jer ne vidim pametniji način kako bih to napravio.

		// popišemo stvari koje će nam trebati kasnije
		llvm::BasicBlock* trenutniBlok = Builder().GetInsertBlock();
		llvm::Value* uvjet = std::dynamic_pointer_cast<IzrazAST>(m_djeca.front())->GenerirajKodIzraz();
		m_djeca.pop_front();
		llvm::Value* granaTrue = std::dynamic_pointer_cast<IzrazAST>(m_djeca.front())->GenerirajKodIzraz();
		m_djeca.pop_front();
		llvm::Value* granaFalse = std::dynamic_pointer_cast<IzrazAST>(m_djeca.front())->GenerirajKodIzraz();
		m_djeca.pop_front();

		// LLVM nema ternarni operator pa ga implementiramo sami:
		// 1. bacamo grešku ako izrazi koje operator vraća nisu istog tipa
		if (granaTrue->getType() != granaFalse->getType())
			throw SemantickaGreska(Redak(), Stupac(), "Izrazi koje operator ?: vraća nisu istog tipa");

		// 2. saznamo kojeg su izrazi tipa
		llvm::Type* tipIzraza = granaTrue->getType();

		// 3. inkrementiramo statički brojač, tako da nemamo konflikte između ternarnih operatora
		static int brojac = 1;
		std::string imeOperatora = "operator?:" + std::to_string(brojac);
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
		
		// 5. dodamo imena argumentima za lakše čitanje LLVM IR-a
		llvm::Function::arg_iterator it = ternarniOperator->arg_begin();
		it->setName(imeOperatora + " uvjet");
		++it;
		it->setName(imeOperatora + " vrijednostTrue");
		++it;
		it->setName(imeOperatora + " vrijednostFalse");

		// 6. implementiramo ternarni operator kao if(uvjet) return granaTrue; else return granaFalse;
		llvm::BasicBlock* tijeloOperatora = llvm::BasicBlock::Create(*Context().LLVMContext(), imeOperatora, ternarniOperator);
		Builder().SetInsertPoint(tijeloOperatora);

		llvm::Argument* argumentPoziva = ternarniOperator->args().begin();
		llvm::Value* uvjetUFunkciji = Builder().CreateICmpNE(argumentPoziva, llvm::ConstantInt::get(*Context().LLVMContext(), llvm::APInt(1, 0)), "uvjet");
		++argumentPoziva;
		llvm::Value* granaTrueUFunkciji = argumentPoziva;
		++argumentPoziva;
		llvm::Value* granaFalseUFunkciji = argumentPoziva;
		llvm::BasicBlock* blokTrue = llvm::BasicBlock::Create(*Context().LLVMContext(), "granaTrue", ternarniOperator);
		llvm::BasicBlock* blokFalse = llvm::BasicBlock::Create(*Context().LLVMContext(), "granaFalse");
		
		Builder().CreateCondBr(uvjetUFunkciji, blokTrue, blokFalse);
		Builder().SetInsertPoint(blokTrue);
		Builder().CreateRet(granaTrueUFunkciji);
		blokTrue = Builder().GetInsertBlock();

		ternarniOperator->getBasicBlockList().push_back(blokFalse);
		Builder().SetInsertPoint(blokFalse);
		Builder().CreateRet(granaFalseUFunkciji);
		blokFalse = Builder().GetInsertBlock();

		// budući da granaTrue i granaFalse provjereno imaju isti tip, možemo bez problema obećati
		// da će obje grane imati tip kao granaTrue
		//llvm::PHINode* grananje = Builder().CreatePHI(granaTrue->getType(), 2, "grananje");
		//grananje->addIncoming(granaTrue, blokTrue);
		//grananje->addIncoming(granaFalse, blokFalse);

		// 7. vratimo insert point natrag u funkciju iz koje smo došli
		Builder().SetInsertPoint(trenutniBlok);
		
		// 8. verify-amo funkciju
		std::string error;
		llvm::raw_string_ostream errorStream(error);
		llvm::verifyFunction(*ternarniOperator, &errorStream);

		// 9. pozovemo funkciju __ternarniOperator<brojac>__
		std::vector<llvm::Value*> argumentiZaPoziv;
		argumentiZaPoziv.push_back(uvjet);
		argumentiZaPoziv.push_back(granaTrue);
		argumentiZaPoziv.push_back(granaFalse);
		llvm::Value* poziv = llvm::CallInst::Create(ternarniOperator, argumentiZaPoziv, imeOperatora, Context().TrenutniBlok()->Sadrzaj());

		// 10. vratimo ono što vrati taj poziv 
		return poziv;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// LogickiOperator
	// DONE
	LogickiOperator::LogickiOperator(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	LogickiOperator::LogickiOperator(LogickiOperator const& drugi) : 
		IzrazAST(drugi) 
	{}

	void LogickiOperator::Ispisi(std::ostream& out) const
	{
		out << "LogickiOperator" << std::endl;
		IspisiDjecu(out);
	}

	// manje-više ista stvar kao bitwise operatori, osim što
	// rade samo s jednim bitom (jer sam tako implementirao bool)
	llvm::Value* LogickiOperator::GenerirajKodIzraz()
	{
		llvm::Value* lijevo = m_djeca.front()->GenerirajKodIzraz();
		if (Context().PrevediTip(lijevo->getType()) != "bool")
			throw SemantickaGreska(m_djeca.front()->Redak(), m_djeca.front()->Stupac(), "Prvi operand logičkog operatora mora biti tipa bool");
		m_djeca.pop_front();

		llvm::Value* desno = m_djeca.front()->GenerirajKodIzraz();
		if (Context().PrevediTip(desno->getType()) != "bool")
			throw SemantickaGreska(m_djeca.front()->Redak(), m_djeca.front()->Stupac(), "Drugi operand logičkog operatora mora biti tipa bool");
		m_djeca.pop_front();

		if (lijevo == nullptr)
			throw SemantickaGreska(Redak(), Stupac(), "S lijeve strane operatora je neispravna vrijednost"); 

		if (desno == nullptr)
			throw SemantickaGreska(Redak(), Stupac(), "S desne strane operatora je neispravna vrijednost");

		std::string lijevoTip = Context().PrevediTip(lijevo->getType());
		std::string desnoTip = Context().PrevediTip(lijevo->getType());
		
		Leaf const& operacija = *std::dynamic_pointer_cast<Leaf>(m_djeca.front());

		if(lijevoTip != "bool" || desnoTip != "bool" )
			throw SemantickaGreska(Redak(), Stupac(), "Operandi operatora " + operacija.Sadrzaj() + " moraju biti tipa int");

		switch (operacija.TipTokena())
		{
			case LAND:
				return Builder().CreateAnd(lijevo, desno, "logickiAnd");
				break;

			case LOR:
				return Builder().CreateOr(lijevo, desno, "logickiOr");
				break;
		}
		// pretvorimo bool u broj 0.0 ili 1.0 (jer LLVM tako kaže)
		//return Builder().CreateUIToFP(lijevo, llvm::Type::getInt1Ty(*Context().LLVMContext().get()), "rezultatL" + imeOperatora);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// BitovniOperator

	// ovo su sad binarni operatori
	//BitovniOperator::BitovniOperator(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
	//	IzrazAST(redak, stupac, std::move(djeca)) 
	//{}

	//BitovniOperator::BitovniOperator(BitovniOperator const& drugi) : 
	//	IzrazAST(drugi) 
	//{}

	//void BitovniOperator::Ispisi(std::ostream& out) const
	//{
	//	out << "BitovniOperator" << std::endl;
	//	IspisiDjecu(out);
	//}

	//llvm::Value* BitovniOperator::GenerirajKodIzraz()
	//{
	//	llvm::Value* lijevo = m_djeca.front()->GenerirajKodIzraz();
	//	if (lijevo->getType() != Context().PrevediTip("int"))
	//		throw SemantickaGreska(m_djeca.front()->Redak(), m_djeca.front()->Stupac(), "Prvi argument bitovnog operatora mora biti tipa int");
	//	
	//	m_djeca.pop_front();

	//	llvm::Value* desno = m_djeca.front()->GenerirajKodIzraz();
	//	if (desno->getType() != Context().PrevediTip("int"))
	//		throw SemantickaGreska(m_djeca.front()->Redak(), m_djeca.front()->Stupac(), "Drugi argument bitovnog operatora mora biti tipa int");

	//	m_djeca.pop_front();

	//	if (lijevo == nullptr || desno == nullptr)
	//		// ili baci grešku?
	//		return nullptr;

	//	Leaf const& operacija = *std::dynamic_pointer_cast<Leaf>(m_djeca.front());
	//	std::string imeOperatora;

	//	switch (operacija.TipTokena())
	//	{
	//		case BITAND:
	//			imeOperatora = "itAnd";
	//			lijevo = Builder().CreateAnd(lijevo, desno, "b" + imeOperatora);
	//			break;

	//		case BITOR:
	//			imeOperatora = "itOr";
	//			lijevo = Builder().CreateOr(lijevo, desno, "b" + imeOperatora);
	//			break;

	//		case BITXOR:
	//			imeOperatora = "itXor";
	//			lijevo = Builder().CreateXor(lijevo, desno, "b" + imeOperatora);
	//			break;
	//	}
	//	// pretvorimo bool u broj 0.0 ili 1.0
	//	return Builder().CreateUIToFP(lijevo, llvm::Type::getInt1Ty(*Context().LLVMContext().get()), "rezultatB" + imeOperatora);
	//}

	////////////////////////////////////////////////////////////////////////////////////
	// OperatorJednakost
	// DONE
	OperatorJednakost::OperatorJednakost(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	OperatorJednakost::OperatorJednakost(OperatorJednakost const& drugi) : 
		IzrazAST(drugi) 
	{}

	void OperatorJednakost::Ispisi(std::ostream& out) const
	{
		out << "OperatorJednakost" << std::endl;
		IspisiDjecu(out);
	}

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
				imeOperatora = "Jednako";
				lijevo = Builder().CreateICmpEQ(lijevo, desno, "usporedba" + imeOperatora);
				break;
		}
		// pretvorimo bool u broj 0.0 ili 1.0
		return Builder().CreateUIToFP(lijevo, llvm::Type::getInt1Ty(*Context().LLVMContext().get()), "rezultat" + imeOperatora);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// OperatorUsporedbe
	// DONE
	OperatorUsporedbe::OperatorUsporedbe(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	OperatorUsporedbe::OperatorUsporedbe(OperatorUsporedbe const& drugi) : 
		IzrazAST(drugi) 
	{}

	void OperatorUsporedbe::Ispisi(std::ostream& out) const
	{
		out << "OperatorUsporedbe" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Value* OperatorUsporedbe::GenerirajKodIzraz()
	{
		llvm::Value* lijevo = m_djeca.front()->GenerirajKodIzraz();
		m_djeca.pop_front();
		llvm::Value* desno = m_djeca.front()->GenerirajKodIzraz();
		m_djeca.pop_front();

		Leaf operacija = *std::dynamic_pointer_cast<Leaf>(m_djeca.front());
		m_djeca.pop_front();

		ProvjeraVrijednosti(lijevo, desno, operacija.Sadrzaj());
		ProvjeraTipa(lijevo, Context().PrevediTip("int"));
		ProvjeraTipa(desno, Context().PrevediTip("int"));

		switch (operacija.TipTokena())
		{
			case LESS:
				return Builder().CreateICmpSLT(lijevo, desno, "usporedbaManje");
				
			case LESSEQ:
				return Builder().CreateICmpSLE(lijevo, desno, "usporedbaManjeJednako");

			case GRT:
				return Builder().CreateICmpSGT(lijevo, desno, "usporedbaVece");

			case GRTEQ:
				return Builder().CreateICmpSGE(lijevo, desno, "usporedbaVeceJednako");
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////
	// BinarniOperator
	// DONE
	BinarniOperator::BinarniOperator(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) :
		IzrazAST(redak, stupac, std::move(djeca))
	{}

	BinarniOperator::BinarniOperator(BinarniOperator const& drugi) :
		IzrazAST(drugi)
	{}

	void BinarniOperator::Ispisi(std::ostream& out) const
	{
		out << "BinarniOperator" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Value* BinarniOperator::GenerirajKodIzraz()
	{
		llvm::Value* lijevo = m_djeca.front()->GenerirajKodIzraz();
		m_djeca.pop_front();

		llvm::Value* desno = m_djeca.front()->GenerirajKodIzraz();
		m_djeca.pop_front();
		desno->getName();

		if (lijevo == nullptr)
			throw SemantickaGreska(Redak(), Stupac(), "S lijeve strane operatora je neispravna vrijednost"); 

		if (desno == nullptr)
			throw SemantickaGreska(Redak(), Stupac(), "S desne strane operatora je neispravna vrijednost");

		std::string lijevoTip = Context().PrevediTip(lijevo->getType());
		std::string desnoTip = Context().PrevediTip(lijevo->getType());
		
		Leaf const& operacija = *std::dynamic_pointer_cast<Leaf>(m_djeca.front());

		if(lijevoTip != "int" || desnoTip != "int" )
			throw SemantickaGreska(Redak(), Stupac(), "Operandi operatora " + operacija.Sadrzaj() + " moraju biti tipa int");

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

			case MOD:
				return Builder().CreateSRem(lijevo, desno, "modulo");

			case BITAND:
				return Builder().CreateAnd(lijevo, desno, "bitAnd");

			case BITOR:
				return Builder().CreateOr(lijevo, desno, "bitOr");

			case BITXOR:
				return Builder().CreateXor(lijevo, desno, "bitXor");

			case LSHIFT:
			{
				// provjeravamo je li desni između 0 i 32 i dižemo iznimku ako nije
				llvm::Value* nula = llvm::ConstantInt::get(*Context().LLVMContext(), llvm::APInt(32, 0, true));
				llvm::Value* tridesetDva = llvm::ConstantInt::get(*Context().LLVMContext(), llvm::APInt(32, 32, true));
				llvm::Value* desniVeciOd0 = Builder().CreateICmpSGT(desno, nula);
				llvm::Value* desniManjiOd32 = Builder().CreateICmpSLT(desno, tridesetDva);
				ProvjeraUvjeta(desniVeciOd0, "Drugi operand lijevog pomaka mora biti veći od 0");
				ProvjeraUvjeta(desniManjiOd32, "Drugi operand lijevog pomaka mora biti manji od 32");
				
				return Builder().CreateShl(lijevo, desno, "lijeviPomak");
			}
			case RSHIFT:
			{
				// vidi gore
				llvm::Value* nula = llvm::ConstantInt::get(*Context().LLVMContext(), llvm::APInt(32, 0, true));
				llvm::Value* tridesetDva = llvm::ConstantInt::get(*Context().LLVMContext(), llvm::APInt(32, 32, true));
				llvm::Value* desniVeciOd0 = Builder().CreateICmpSGT(desno, nula);
				llvm::Value* desniManjiOd32 = Builder().CreateICmpSLT(desno, tridesetDva);
				ProvjeraUvjeta(desniVeciOd0, "Drugi operand desnog pomaka mora biti veći od 0");
				ProvjeraUvjeta(desniManjiOd32, "Drugi operand desnog pomaka mora biti manji od 32");

				return Builder().CreateAShr(lijevo, desno, "desniPomak");
			}

			default:
				throw SemantickaGreska(Redak(), Stupac(), "Nepoznat binarni operator");
		}
	}

	////////////////////////////////////////////////////////////////////////////////////
	// OperatorPridruzivanja
	// DONE
	OperatorPridruzivanja::OperatorPridruzivanja(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	OperatorPridruzivanja::OperatorPridruzivanja(OperatorPridruzivanja const& drugi) : 
		IzrazAST(drugi) 
	{}

	void OperatorPridruzivanja::Ispisi(std::ostream& out) const
	{
		out << "OperatorPridruzivanja" << std::endl;
		IspisiDjecu(out);
	}

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
			throw SemantickaGreska(Redak(), Stupac(), "Prvi operand operatora pridruživanja mora biti lijeva vrijednost");
		}

		// dohvatimo lijevu stranu (još ne kompajliramo) i maknemo je iz popisa djece
		std::shared_ptr<AST> lijevaStrana = m_djeca.front();
		m_djeca.pop_front();
		
		// kompajliramo desnu stranu
		llvm::Value* desno = m_djeca.front()->GenerirajKodIzraz();
		m_djeca.pop_front();

		// zatim kompajliramo lijevu stranu
		llvm::Value* lijevo;
		llvm::Value* adresaLijevog;


		// ako je s lijeve strane varijabla, onda preko njenog imena dohvatimo njenu adresu
		if (tipLijevo == typeid(Varijabla))
		{
			lijevo = lijevaStrana->GenerirajKodIzraz();
			std::string lijevoIme = Context().TrenutniBlok()->Trazi(lijevo);
			adresaLijevog = Context().TrenutniBlok()->Trazi(lijevoIme);
		}
		// inače je dobijemo od operatora
		else
		{
			adresaLijevog = lijevaStrana->GenerirajKodIzraz();
			lijevo = Builder().CreateLoad(adresaLijevog, "dereferenciranjeAdrese");
		}

		ProvjeraVrijednosti(lijevo);
		ProvjeraVrijednosti(desno);
		ProvjeraTipa(desno, lijevo->getType());

		llvm::Value* novoLijevo;
		Leaf const& operacija = *std::dynamic_pointer_cast<Leaf>(m_djeca.front());

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
				// provjeravamo je li desni između 0 i 32 i dižemo iznimku ako nije
				llvm::Value* nula = llvm::ConstantInt::get(*Context().LLVMContext(), llvm::APInt(32, 0, true));
				llvm::Value* tridesetDva = llvm::ConstantInt::get(*Context().LLVMContext(), llvm::APInt(32, 32, true));
				llvm::Value* desniVeciOd0 = Builder().CreateICmpSGT(desno, nula);
				llvm::Value* desniManjiOd32 = Builder().CreateICmpSLT(desno, tridesetDva);
				ProvjeraUvjeta(desniVeciOd0, "Drugi operand lijevog pomaka mora biti veći od 0");
				ProvjeraUvjeta(desniManjiOd32, "Drugi operand lijevog pomaka mora biti manji od 32");
				
				return Builder().CreateShl(lijevo, desno, "lijeviPomak");
			}
			case RSHIFTEQ:
			{
				// vidi gore
				llvm::Value* nula = llvm::ConstantInt::get(*Context().LLVMContext(), llvm::APInt(32, 0, true));
				llvm::Value* tridesetDva = llvm::ConstantInt::get(*Context().LLVMContext(), llvm::APInt(32, 32, true));
				llvm::Value* desniVeciOd0 = Builder().CreateICmpSGT(desno, nula);
				llvm::Value* desniManjiOd32 = Builder().CreateICmpSLT(desno, tridesetDva);
				ProvjeraUvjeta(desniVeciOd0, "Drugi operand desnog pomaka mora biti veći od 0");
				ProvjeraUvjeta(desniManjiOd32, "Drugi operand desnog pomaka mora biti manji od 32");

				return Builder().CreateAShr(lijevo, desno, "desniPomak");
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

		Builder().CreateStore(novoLijevo, adresaLijevog);
		return novoLijevo;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Alokacija
	// DONE
	Alokacija::Alokacija(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	Alokacija::Alokacija(Alokacija const& drugi) : 
		IzrazAST(drugi) 
	{}

	void Alokacija::Ispisi(std::ostream& out) const
	{
		out << "Alokacija" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Value* Alokacija::GenerirajKodIzraz()
	{
		// dohvati tip
		llvm::Type* tip = m_djeca.front()->GenerirajKodTip();
		llvm::Type* tipInt = Context().PrevediTip("int");
		std::string tipString = Context().PrevediTip(tip);
		m_djeca.pop_front();

		// dohvati sizeof(tip)
		llvm::Constant* velicinaObjekta = llvm::ConstantInt::get(tipInt, Module()->getDataLayout().getTypeAllocSize(tip));
		
		// i kreiraj poziv malloc(tip*, sizeof(tip))
		llvm::Instruction* pozivMalloc = llvm::CallInst::CreateMalloc(Builder().GetInsertBlock(), tipInt, tip, velicinaObjekta, nullptr, nullptr, "alokacija");
		Builder().Insert(pozivMalloc);
		//(*Context().TipoviPointera())[pozivMalloc] = tipString;

		return pozivMalloc;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// AlokacijaArray
	// DONE
	AlokacijaArray::AlokacijaArray(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) 
		: IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	AlokacijaArray::AlokacijaArray(AlokacijaArray const& drugi) : 
		IzrazAST(drugi) 
	{}

	void AlokacijaArray::Ispisi(std::ostream& out) const
	{
		out << "AlokacijaArray" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Value* AlokacijaArray::GenerirajKodIzraz()
	{
		llvm::Value* ret;
		llvm::Value* dereferenciraniRet;

		// dohvati tip arraya i zapamti Tip* za int
		llvm::Type* tip = m_djeca.front()->GenerirajKodTip();
		llvm::Type* tipInt = Context().PrevediTip("int");
		std::string tipSlovima = *std::dynamic_pointer_cast<Tip>(m_djeca.front());
		m_djeca.pop_front();

		// alociramo memoriju za struct{int, <tip>}*. zapravo pripremimo podatke za Alokaciju
		std::shared_ptr<Leaf> alokacijaTipSlovima = std::make_shared<Leaf>(Token(PRAZNO, tipSlovima + "[]", Redak(), Stupac()));
		std::list<std::shared_ptr<AST>> alokacijaTipDjeca;
		alokacijaTipDjeca.push_back(std::move(alokacijaTipSlovima));
		std::shared_ptr<Tip> alokacijaTip = std::make_shared<Tip>(Redak(), Stupac(), std::move(alokacijaTipDjeca));

		std::list<std::shared_ptr<AST>> alokacijaDjeca;
		alokacijaDjeca.push_back(std::move(alokacijaTip));
		Alokacija alokacija(Redak(), Stupac(), std::move(alokacijaDjeca));

		// i pustimo je da alocira memoriju za nas! 
		ret = alokacija.GenerirajKodIzraz();
		dereferenciraniRet = Builder().CreateLoad(ret, "dereferencirajStrukturu");

		// dohvatimo nulti element strukture polja - int koji će sadržavati njegovu duljinu i pridružimo mu odgovarajuću vrijednost
		llvm::Value* size = Context().DohvatiClanStrukture(ret, 0);
		if (size == nullptr)
			throw SemantickaGreska(Redak(), Stupac(), "Greška pri spremanju duljine novog polja");

		llvm::Constant* kolicinaObjekata = static_cast<llvm::Constant*>(m_djeca.front()->GenerirajKodIzraz());
		Builder().CreateStore(kolicinaObjekata, size);

		// dohvatimo praznu memoriju za elemente polja
		llvm::Value* elements = Context().DohvatiClanStrukture(ret, 1);
		if (elements == nullptr)
			throw SemantickaGreska(Redak(), Stupac(), "Greška pri spremanju sadržaja novog polja");

		// dohvatimo sizeof(tip)
		llvm::Constant* velicinaObjekta = llvm::ConstantInt::get(tipInt, Module()->getDataLayout().getTypeAllocSize(tip));
		llvm::Constant* velicinaPolja = llvm::ConstantExpr::getMul(velicinaObjekta, kolicinaObjekata);
		velicinaPolja = llvm::ConstantExpr::getTruncOrBitCast(velicinaPolja, tipInt);

		// kreiraj poziv malloc(tip*, sizeof(tip))
		llvm::Instruction* pozivMalloc = llvm::CallInst::CreateMalloc(Builder().GetInsertBlock(), tipInt, tip, velicinaPolja, nullptr, nullptr, "");
		Builder().Insert(pozivMalloc);

		// namjestimo da pokazivač u strukturi pokazuje na alociranu memoriju
		Builder().CreateStore(pozivMalloc, elements);

		// i vratimo pointer na strukturu
		return ret;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Negacija
	// DONE
	Negacija::Negacija(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}
	
	Negacija::Negacija(Negacija const& drugi) : 
		IzrazAST(drugi) 
	{}

	void Negacija::Ispisi(std::ostream& out) const
	{
		out << "Negacija" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Value* Negacija::GenerirajKodIzraz()
	{
		llvm::Value* operand = m_djeca.front()->GenerirajKodIzraz();

		if (operand->getType() != Context().PrevediTip("bool"))
			throw SemantickaGreska(m_djeca.front()->Redak(), m_djeca.front()->Stupac(), "Operand operatora negacije mora biti tipa bool");

		m_djeca.pop_front();
		operand = Builder().CreateNot(operand, "logickaNegacija");

		// pretvorimo bool u broj 0.0 ili 1.0 (jer LLVM tako kaže)
		return Builder().CreateUIToFP(operand, llvm::Type::getInt1Ty(*Context().LLVMContext().get()), "rezultatLogickaNegacija");
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Tilda
	// DONE
	Tilda::Tilda(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	Tilda::Tilda(Tilda const& drugi) : 
		IzrazAST(drugi) 
	{}

	void Tilda::Ispisi(std::ostream& out) const
	{
		out << "Tilda" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Value* Tilda::GenerirajKodIzraz()
	{
		llvm::Value* operand = m_djeca.front()->GenerirajKodIzraz();
		
		if (operand->getType() != Context().PrevediTip("int"))
			throw SemantickaGreska(m_djeca.front()->Redak(), m_djeca.front()->Stupac(), "Operand bitovne negacije mora biti tipa int");

		m_djeca.pop_front();
		operand = Builder().CreateNot(operand, "bitovnaNegacija");

		// pretvorimo bool u broj 0.0 ili 1.0 (jer LLVM tako kaže)
		return Builder().CreateUIToFP(operand, llvm::Type::getInt1Ty(*Context().LLVMContext().get()), "rezultatBitovnaNegacija");
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Minus
	// DONE
	Minus::Minus(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	Minus::Minus(Minus const& drugi) : 
		IzrazAST(drugi) 
	{}

	void Minus::Ispisi(std::ostream& out) const
	{
		out << "Minus" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Value* Minus::GenerirajKodIzraz()
	{
		llvm::Value* operand = m_djeca.front()->GenerirajKodIzraz();

		if(operand == nullptr)
			throw SemantickaGreska(Redak(), Stupac(), "Operand aritmetičke negacije je neispravna vrijednost");

		if (operand->getType() != Context().PrevediTip("int"))
			throw SemantickaGreska(m_djeca.front()->Redak(), m_djeca.front()->Stupac(), "Operand aritmetičke negacije mora biti tipa int");

		m_djeca.pop_front();
		return Builder().CreateNeg(operand, "aritmetickaNegacija");
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Dereferenciranje
	// DONE
	Dereferenciranje::Dereferenciranje(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	Dereferenciranje::Dereferenciranje(Dereferenciranje const& drugi) : 
		IzrazAST(drugi) 
	{}

	void Dereferenciranje::Ispisi(std::ostream& out) const
	{
		out << "Dereferenciranje" << std::endl;
		IspisiDjecu(out);
	}

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

	void PozivFunkcije::Ispisi(std::ostream& out) const
	{
		out << "PozivFunkcije" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Value* PozivFunkcije::GenerirajKodIzraz()
	{
		std::string ime = std::dynamic_pointer_cast<Leaf>(m_djeca.front())->Sadrzaj();
		m_djeca.pop_front();

		llvm::Function* funkcija = Context().Module()->getFunction(ime);
		if (funkcija == nullptr) 
			throw SemantickaGreska(Redak(), Stupac(), "Ne postoji funkcija s imenom " + ime + "");

		ASTList& dobiveniArgumenti = *std::dynamic_pointer_cast<ASTList>(m_djeca.front());
		
		if (dobiveniArgumenti.size() != funkcija->arg_size())
		{
			std::stringstream poruka;
			poruka << "Funkcija " << ime << " prima " << funkcija->arg_size()
				<< " argumenata (poslano " << dobiveniArgumenti.size() << ")";
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
					<< Context().PrevediTip(argumenti[--i]->getType()) << "'";
				throw new SemantickaGreska(Redak(), Stupac(), poruka.str());
			}
		}

		llvm::CallInst* poziv = llvm::CallInst::Create(funkcija, argumenti, "", Context().TrenutniBlok()->Sadrzaj());
		return poziv;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Inkrement
	// DONE
	Inkrement::Inkrement(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	Inkrement::Inkrement(Inkrement const& drugi) : 
		IzrazAST(drugi) 
	{}

	void Inkrement::Ispisi(std::ostream& out) const
	{
		out << "Inkrement" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Value* Inkrement::GenerirajKodIzraz()
	{
		llvm::Value* operand = m_djeca.front()->GenerirajKodIzraz();
		llvm::Constant* jedan = llvm::ConstantInt::get(Context().PrevediTip("int"), 1);

		m_djeca.pop_front();

		llvm::Value* novaVrijednost = Builder().CreateAdd(operand, jedan);
		Builder().CreateStore(novaVrijednost, Context().TrenutniBlok()->Trazi(Context().TrenutniBlok()->Trazi(operand)));
		//Builder().CreateStore(novaVrijednost, operand);

		return novaVrijednost;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Dekrement
	// DONE
	Dekrement::Dekrement(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) :
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	Dekrement::Dekrement(Dekrement const& drugi) : 
		IzrazAST(drugi) 
	{}

	void Dekrement::Ispisi(std::ostream& out) const
	{
		out << "Dekrement" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Value* Dekrement::GenerirajKodIzraz()
	{
		llvm::Value* operand = m_djeca.front()->GenerirajKodIzraz();
		llvm::Constant* jedan = llvm::ConstantInt::get(Context().PrevediTip("int"), 1);

		m_djeca.pop_front();

		llvm::Value* novaVrijednost = Builder().CreateSub(operand, jedan);
		Builder().CreateStore(novaVrijednost, operand);

		return novaVrijednost;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// UglateZagrade
	// DONE
	UglateZagrade::UglateZagrade(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	UglateZagrade::UglateZagrade(UglateZagrade const& drugi) : 
		IzrazAST(drugi) 
	{}

	void UglateZagrade::Ispisi(std::ostream& out) const
	{
		out << "UglateZagrade" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Value* UglateZagrade::GenerirajKodIzraz()
	{
		llvm::Value* polje = m_djeca.front()->GenerirajKodIzraz();

		ProvjeraVrijednosti(polje);
		ProvjeraPolja(polje);
		m_djeca.pop_front();
		
		llvm::Value* indeksPolja = m_djeca.front()->GenerirajKodIzraz();
		m_djeca.pop_front();
		ProvjeraTipa(indeksPolja, Context().PrevediTip("int"));

		// ako indeks nije između 0 i duljine polja, bacamo iznimku
		llvm::Value* nula = llvm::ConstantInt::get(*Context().LLVMContext().get(), llvm::APInt(32, 0, true));
		llvm::Value* veceOdNule = Builder().CreateICmpSLT(nula, indeksPolja, "usporedbaManje");
		ProvjeraUvjeta(veceOdNule, "Indeks polja mora biti veći od 0!");

		// sad dohvatimo duljinu polja
		llvm::Value* dohvacenaDuljina = Context().DohvatiClanStrukture(polje, 0);
		ProvjeraVrijednosti(dohvacenaDuljina);

		// i provjerimo je li indeks manji od duljine
		llvm::Value* manjeOdDuljine = Builder().CreateICmpSLT(indeksPolja, dohvacenaDuljina);
		ProvjeraUvjeta(manjeOdDuljine, "Indeks polja mora biti manji od maksimalne duljine polja!");
		
		// ako je sve u redu s duljinom, dohvatimo pointer na polje i dereferenciramo ga
		llvm::Value* dohvacenoPolje = Context().DohvatiClanStrukture(polje, 1);
		ProvjeraVrijednosti(dohvacenoPolje);

		// zatim konstruiramo indeks elementa
		std::vector<llvm::Value*> indeksElementa;
		indeksElementa.push_back(indeksPolja);

		// i dohvatimo element polja koji je na tom indeksu
		llvm::Value* dohvaceniElement = Builder().CreateGEP(dohvacenoPolje, indeksElementa, "pointerNaElement");
		dohvaceniElement = Builder().CreateLoad(dohvaceniElement, "elementPolja");
		return dohvaceniElement;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Tip
	// DONE

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
	
	void Tip::Ispisi(std::ostream& out) const
	{
		out << "Tip (" << m_ime << ")" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Type* Tip::GenerirajKodTip()
	{
		llvm::Type* ret = Context().PrevediTip(m_ime);
		if (ret == nullptr)
			throw new SemantickaGreska(Redak(), Stupac(), "Nepoznat tip '" + m_ime + "'");

		return ret;
	}
	
	////////////////////////////////////////////////////////////////////////////////////
	// Tocka
	// DONE

	Tocka::Tocka(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) : 
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	Tocka::Tocka(Tocka const& drugi) : 
		IzrazAST(drugi) 
	{}

	void Tocka::Ispisi(std::ostream& out) const
	{
		out << "Tocka" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Value* Tocka::GenerirajKodIzraz()
	{
		// dohvatimo ime varijable 
		llvm::Value* varijabla = m_djeca.front()->GenerirajKodIzraz();
		std::string imeVarijable = varijabla->getName();
		m_djeca.pop_front();

		// dohvatimo ime strukture (zapravo tipa varijable)
		std::string imeStrukture = Context().PrevediTip(Context().LokalneVarijable()[imeVarijable]->getType());

		// dohvatimo ime elementa koji nas zanima
		std::string imeClana = (std::dynamic_pointer_cast<Leaf>(m_djeca.front()))->Sadrzaj();
		m_djeca.pop_front();

		// pročitamo indeks tog elementa u strukturi
		int indeksClana = Context().DohvatiIndeksClanaStrukture(imeStrukture, imeClana);
		llvm::Value* dohvaceniClan = Context().DohvatiClanStrukture(Context().TrenutniBlok()->Trazi(imeStrukture), indeksClana);

		if (dohvaceniClan == nullptr)
			throw SemantickaGreska(Redak(), Stupac(), "Greška pri dohvaćanju člana " + imeClana + " strukture " + imeStrukture);

		return dohvaceniClan;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Strelica
	// DONE

	Strelica::Strelica(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) :
		IzrazAST(redak, stupac, std::move(djeca)) 
	{}

	Strelica::Strelica(Strelica const& drugi) :
		IzrazAST(drugi) 
	{}
	
	void Strelica::Ispisi(std::ostream& out) const
	{
		out << "Strelica" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Value* Strelica::GenerirajKodIzraz()
	{
		// dereferenciraj lijevu stranu
		llvm::Value* pointerKaoBroj = m_djeca.front()->GenerirajKodIzraz();
		llvm::Type* tipPointera = Context().PrevediTip(Context().PrevediTip(pointerKaoBroj->getType()) + "*");
		llvm::Value* pointer = Builder().CreateIntToPtr(pointerKaoBroj, tipPointera);
		llvm::Value* lijevo = Builder().CreateLoad(pointer, tipPointera);

		// i vrati odgovarajući član strukture
		std::string imeStrukture = lijevo->getName();
		m_djeca.pop_front();

		std::string tipStrukture = Context().PrevediTip(Context().LokalneVarijable()[imeStrukture]->getType());
		llvm::Value* clan = m_djeca.front()->GenerirajKodIzraz();
		m_djeca.pop_front();
		
		// pročitamo indeks tog elementa u strukturi
		int indeksClana = Context().DohvatiIndeksClanaStrukture(imeStrukture, clan->getName());
		llvm::Value* dohvaceniClan = Context().DohvatiClanStrukture(Context().TrenutniBlok()->Trazi(imeStrukture), indeksClana);

		if (dohvaceniClan == nullptr)
			throw SemantickaGreska(Redak(), Stupac(), "Greška pri dohvaćanju člana " + clan->getName().str() + " strukture " + imeStrukture);

		return dohvaceniClan;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Leaf
	// DONE

	Leaf::Leaf(Token const& sadrzaj) : 
		IzrazAST(sadrzaj.Redak(), sadrzaj.Stupac()), 
		m_sadrzaj(sadrzaj) 
	{}

	Leaf::Leaf(Leaf const& drugi) : 
		IzrazAST(drugi) 
	{ 
		m_sadrzaj = drugi.m_sadrzaj; 
	}

	void Leaf::Ispisi(std::ostream& out) const
	{
		out << "Leaf (" << m_sadrzaj << ")" << std::endl;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// IntLiteral
	// DONE

	IntLiteral::IntLiteral(Token const& sadrzaj) :
		Leaf(sadrzaj)
	{}

	void IntLiteral::Ispisi(std::ostream& out) const
	{
		out << "IntLiteral (" << Sadrzaj() << ")" << std::endl;
	}

	llvm::Value* IntLiteral::GenerirajKodIzraz()
	{
		return llvm::ConstantInt::get(Context().PrevediTip("int"), (int)*this, true);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// CharLiteral
	// DONE

	CharLiteral::CharLiteral(Token const& sadrzaj) :
		Leaf(sadrzaj)
	{}

	void CharLiteral::Ispisi(std::ostream& out) const
	{
		out << "CharLiteral (" << Sadrzaj() << ")" << std::endl;
	}

	llvm::Value* CharLiteral::GenerirajKodIzraz()
	{
		return llvm::ConstantInt::get(Context().PrevediTip("char"), (char)*this, true);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// BoolLiteral
	// DONE

	BoolLiteral::BoolLiteral(Token const& sadrzaj) :
		Leaf(sadrzaj)
	{}

	BoolLiteral::operator bool() const
	{
		if (Sadrzaj() == "true")
			return true;

		if (Sadrzaj() == "false")
			return false;
	}

	void BoolLiteral::Ispisi(std::ostream& out) const
	{
		out << "BoolLiteral (" << Sadrzaj() << ")" << std::endl;
	}

	llvm::Value* BoolLiteral::GenerirajKodIzraz()
	{
		return llvm::ConstantInt::get(Context().PrevediTip("bool"), (bool)*this);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// StringLiteral
	// DONE

	StringLiteral::StringLiteral(Token const& sadrzaj) :
		Leaf(sadrzaj)
	{
		// sadržaj sadrži znakove "", što ne želimo pa ćemo ih maknuti
		int size = Sadrzaj().size();
		std::string noviSadrzaj = Sadrzaj().substr(1, size - 2);
		m_sadrzaj = Token(sadrzaj.Tip(), noviSadrzaj, sadrzaj.Redak(), sadrzaj.Stupac());
	}

	void StringLiteral::Ispisi(std::ostream& out) const
	{
		out << "StringLiteral (\"" << Sadrzaj() << "\")" << std::endl;
	}

	llvm::Value* StringLiteral::GenerirajKodIzraz()
	{
		// dohvatimo funkciju u kojoj se nalazimo
		llvm::Function* funkcija = Builder().GetInsertBlock()->getParent();

		// alociramo memoriju za string
		llvm::Value* ret = Context().StackAlokacija(funkcija, Context().PrevediTip("string"), "");

		// dohvatimo nulti element strukture string te spremimo duljinu stringa u njega
		llvm::Value* size = Context().DohvatiClanStrukture(ret, 0);
		if (size == nullptr)
			throw SemantickaGreska(Redak(), Stupac(), "Greška pri spremanju duljine literala tipa string");

		Builder().CreateStore(llvm::ConstantInt::get(*Context().LLVMContext(), llvm::APInt(32, Sadrzaj().size(), true)), size);

		// dohvatimo prvi element strukture
		llvm::Value* data = Context().DohvatiClanStrukture(ret, 1);
		if (data == nullptr)
			throw SemantickaGreska(Redak(), Stupac(), "Greška pri dohvaćanju sadržaja literala tipa string");

		// prevedemo novu vrijednost elemenata strukture string u LLVM
		llvm::Constant* _data = Builder().CreateGlobalString((const char*)(*this), "noviString(" + Sadrzaj() + ")");
		// i castamo to u char*
		_data = llvm::ConstantExpr::getBitCast(_data, Context().PrevediTip("char*"));

		// spremimo nove vrijednosti elemenata u strukturu
		Builder().CreateStore(_data, data);

		// dereferenciramo pointer na memoriju gdje se nalazi string literral
		ret = Builder().CreateLoad(ret, "popunjenStringLiteral");

		// i vratimo ga
		return ret;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// ASTList
	// DONE

	ASTList::ASTList(int redak, int stupac) : 
		IzrazAST(redak, stupac) 
	{}

	ASTList::ASTList(int redak, int stupac, std::list<std::shared_ptr<AST>>&& djeca) :
		IzrazAST(redak, stupac, std::move(djeca))
	{}

	ASTList::ASTList(ASTList const& drugi) : 
		IzrazAST(drugi) 
	{}

	void ASTList::Ispisi(std::ostream& out) const
	{
		out << "ASTList" << std::endl;
		IspisiDjecu(out);
	}

	llvm::Value* ASTList::GenerirajKodIzraz()
	{
		for (iterator it = begin(); it != end(); ++it)
		{
			// uvijek je točno jedan od sljedeća 3 poziva netrivijalan i
			// to nikad nije GenerirajKodFunkcija; taj je tu da bude očito
			// da negdje nešto ne štima ako taj ispadne netrivijalan
			(*it)->GenerirajKodIzraz();
			(*it)->GenerirajKodTip();
			//(*it)->GenerirajKodFunkcija();
		}

		// vraćam nulu jer ionako nikad ne gledam što ova funkcija vraća.
		// ne vraćam nullptr jer tako LLVM označava greške pri generiranju koda
		return llvm::Constant::getNullValue(llvm::Type::getInt32Ty(*Context().LLVMContext()));
	}
}
