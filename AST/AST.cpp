#include "AST.hpp"
#include <typeinfo>

namespace C0Compiler
{
	////////////////////////////////////////////////////////////////////////////////////
	// AST
	std::list<AST*> AST::sirocad = std::list<AST*>();

	void AST::ucitajDjecu(std::list<AST*> const& djeca)
	{
		for (std::list<AST*>::const_iterator it = djeca.begin(); it != djeca.end(); ++it)
		{
			AST* novoDijete;
			// zapamti tip it-tog djeteta
			type_info const& tipDjeteta = typeid(**it);

			// 13.05.2019. note to self: u nekom trenutku bi možda valjalo napraviti full fledged AST factory i ovo prebaciti u njega

			// i copy-konstruiraj dijete ispravnog tipa
			if (tipDjeteta == typeid(Program))
				novoDijete = new Program((Program const&)**it);

			else if (tipDjeteta == typeid(UseDirektiva))
				novoDijete = new UseDirektiva((UseDirektiva const&)**it);

			else if (tipDjeteta == typeid(DeklaracijaFunkcije))
				novoDijete = new DeklaracijaFunkcije((DeklaracijaFunkcije const&)**it);

			else if (tipDjeteta == typeid(DefinicijaFunkcije))
				novoDijete = new DefinicijaFunkcije((DefinicijaFunkcije const&)**it);

			else if (tipDjeteta == typeid(If))
				novoDijete = new If((If const&)**it);

			else if (tipDjeteta == typeid(IfElse))
				novoDijete = new IfElse((IfElse const&)**it);

			else if (tipDjeteta == typeid(While))
				novoDijete = new While((While const&)**it);

			else if (tipDjeteta == typeid(For))
				novoDijete = new For((For const&)**it);

			else if (tipDjeteta == typeid(Return))
				novoDijete = new Return((Return const&)**it);

			else if (tipDjeteta == typeid(Break))
				novoDijete = new Break((Break const&)**it);

			else if (tipDjeteta == typeid(Continue))
				novoDijete = new Continue((Continue const&)**it);

			else if (tipDjeteta == typeid(Assert))
				novoDijete = new Assert((Assert const&)**it);

			else if (tipDjeteta == typeid(Error))
				novoDijete = new Error((Error const&)**it);

			else if (tipDjeteta == typeid(Varijabla))
				novoDijete = new Varijabla((Varijabla const&)**it);

			else if (tipDjeteta == typeid(DeklaracijaVarijable))
				novoDijete = new DeklaracijaVarijable((DeklaracijaVarijable const&)**it);

			else if (tipDjeteta == typeid(TernarniOperator))
				novoDijete = new TernarniOperator((TernarniOperator const&)**it);

			else if (tipDjeteta == typeid(LogickiOperator))
				novoDijete = new LogickiOperator((LogickiOperator const&)**it);

			else if (tipDjeteta == typeid(BitwiseOperator))
				novoDijete = new BitwiseOperator((BitwiseOperator const&)**it);

			else if (tipDjeteta == typeid(OperatorJednakost))
				novoDijete = new OperatorJednakost((OperatorJednakost const&)**it);

			else if (tipDjeteta == typeid(OperatorUsporedbe))
				novoDijete = new OperatorUsporedbe((OperatorUsporedbe const&)**it);

			else if (tipDjeteta == typeid(BinarniOperator))
				novoDijete = new BinarniOperator((BinarniOperator const&)**it);

			else if (tipDjeteta == typeid(OperatorPridruzivanja))
				novoDijete = new OperatorPridruzivanja((OperatorPridruzivanja const&)**it);

			else if (tipDjeteta == typeid(Alokacija))
				novoDijete = new Alokacija((Alokacija const&)**it);

			else if (tipDjeteta == typeid(AlokacijaArray))
				novoDijete = new AlokacijaArray((AlokacijaArray const&)**it);

			else if (tipDjeteta == typeid(Negacija))
				novoDijete = new Negacija((Negacija const&)**it);

			else if (tipDjeteta == typeid(Tilda))
				novoDijete = new Tilda((Tilda const&)**it);

			else if (tipDjeteta == typeid(Minus))
				novoDijete = new Minus((Minus const&)**it);

			else if (tipDjeteta == typeid(Dereferenciranje))
				novoDijete = new Dereferenciranje((Dereferenciranje const&)**it);

			else if (tipDjeteta == typeid(PozivFunkcije))
				novoDijete = new PozivFunkcije((PozivFunkcije const&)**it);

			else if (tipDjeteta == typeid(Inkrement))
				novoDijete = new Inkrement((Inkrement const&)**it);

			else if (tipDjeteta == typeid(Dekrement))
				novoDijete = new Dekrement((Dekrement const&)**it);

			else if (tipDjeteta == typeid(UglateZagrade))
				novoDijete = new UglateZagrade((UglateZagrade const&)**it);

			else if (tipDjeteta == typeid(Leaf))
				novoDijete = new Leaf((Leaf const&)**it);

			else if (tipDjeteta == typeid(ASTList))
				novoDijete = new ASTList((ASTList const&)**it);

			dodajDijete(novoDijete);
		}
	}

	void AST::pobrisiDjecu()
	{
		for (std::list<AST*>::iterator it = m_djeca.begin(); it != m_djeca.end();)
			delete *(it++);
	}

	AST::AST(AST&& drugi) : m_djeca(std::move(drugi.m_djeca)) 
	{
		// svakom djetetu stavi sebe za roditelja
		for (std::list<AST*>::iterator it = m_djeca.begin(); it != m_djeca.end(); ++it)
			(*it)->setRoditelj(this);
	}

	AST::AST(std::list<AST*>&& djeca) : m_djeca(std::move(djeca)) 
	{
		// svakom svom djetetu stavi sebe za roditelja
		for (std::list<AST*>::iterator it = djeca.begin(); it != djeca.end(); ++it)
			(*it)->setRoditelj(this);

		sirocad.push_back(this);
	}

	void AST::dodajDijete(AST* dijete)
	{
		// dodaj si dijete
		m_djeca.push_back(dijete);

		// i reci tom djetetu da si mu sad ti roditelj
		m_djeca.back()->setRoditelj(this);
	}

	void AST::setRoditelj(AST* roditelj)
	{
		// postavi roditelja
		m_roditelj = roditelj;

		// i izbriši se iz siročadi
		for (std::list<AST*>::iterator it = sirocad.begin(); it != sirocad.end(); ++it)
		{
			if (*it == this)
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
			
			for (std::list<AST*>::iterator it = m_djeca.begin(); it != m_djeca.end(); ++it)
				(*it)->setRoditelj(this);
		}
		return std::move(*this);
	}

	AST::~AST()
	{
		pobrisiDjecu();

		// ako si siroče, izbriši se iz popisa siročadi
		for (std::list<AST*>::iterator it = sirocad.begin(); it != sirocad.end(); ++it)
		{
			if (*it == this)
			{
				sirocad.erase(it);
				break;
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Program - placeholder

	////////////////////////////////////////////////////////////////////////////////////
	// Use direktiva - placeholder

	////////////////////////////////////////////////////////////////////////////////////
	// Deklaracija funkcije - placeholder

	////////////////////////////////////////////////////////////////////////////////////
	// Definicija funkcije - placeholder

	////////////////////////////////////////////////////////////////////////////////////
	// Varijabla

	//Varijabla::Varijabla(std::initializer_list<AST*> parametri)
	//{
	//	std::initializer_list<AST*>::iterator it = parametri.begin();
	//	
	//	// užasno, znam.
	//	m_tip = static_cast<Token>(*dynamic_cast<Leaf const*>(*it++)).getSadrzaj();
	//	m_ime = static_cast<Token>(*dynamic_cast<Leaf const*>(*it++)).getSadrzaj();

	//	for (; it != parametri.end(); ++it)
	//	{

	//	}
	//}
}