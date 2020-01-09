#include "Token/Token.hpp"
#include "Lekser/Lekser.hpp"
#include "Parser/Parser.hpp"
#include "Greska/Greska.hpp"

#include <iostream>
#include <fstream>
#include <deque>
#include <memory>

int main(int argc, char** argv)
{
	if (argc != 2) // zasad 2, kasnije možda više
	{
		std::cerr << "Korištenje: " << *argv << " kod.c0" << std::endl;
		exit(1);
	}

	std::shared_ptr<std::ifstream> code = std::make_shared<std::ifstream>(argv[1]);
	if (!code->is_open())
	{
		std::cerr << "Ne mogu naći datoteku " << argv[1] << std::endl;
		exit(1);
	}

	std::deque<std::shared_ptr<C0Compiler::Token>> tokeni;
	C0Compiler::Lekser lex(code);

	try 
	{
		tokeni = lex.leksiraj();
		for (std::deque<std::shared_ptr<C0Compiler::Token>>::iterator it = tokeni.begin(); it != tokeni.end(); ++it)
			std::cout << **it << std::endl;
	}
	catch (C0Compiler::LeksickaGreska const& e)
	{
		std::cout << e.what();
		//lex.pocisti();
	}

	C0Compiler::Parser pars(std::move(tokeni));
	try
	{
		pars.parsiraj();
	}
	catch (C0Compiler::SintaksnaGreska const& e)
	{
		std::cout << e.what();
	}

	return 0;
}
