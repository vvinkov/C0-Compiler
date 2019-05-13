#include "Token/Token.hpp"
#include "Lekser/Lekser.hpp"
#include "Greska/Greska.hpp"

#include <iostream>
#include <fstream>
#include <deque>

int main(int argc, char** argv)
{
	if (argc != 2) // zasad 2, kasnije možda više
	{
		std::cerr << "Korištenje: " << *argv << " kod.c0" << std::endl;
		exit(1);
	}

	std::ifstream code(argv[1]);
	if (!code.is_open())
	{
		std::cerr << "Ne mogu naći datoteku " << argv[1] << std::endl;
		exit(1);
	}

	C0Compiler::Lekser lex(&code);
	std::deque<C0Compiler::Token*> tokeni;

	try 
	{
		tokeni = lex.leksiraj();
		for (auto it = tokeni.begin(); it != tokeni.end(); ++it)
			std::cout << **it << std::endl;
	}
	catch (C0Compiler::LeksickaGreska const& e)
	{
		std::cout << e.what();
		lex.pocisti();
	}

	lex.pocisti();

	return 0;
}
