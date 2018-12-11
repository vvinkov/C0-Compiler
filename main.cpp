#include "Token/Token.hpp"
#include "Lekser/Lekser.hpp"

#include <iostream>
#include <fstream>
#include <deque>

int main(int argc, char** argv)
{
	if (argc != 2) // zasad 2, kasnije vjerojatno više
	{
		std::cerr << "Koristenje: " << *argv << " kod.c0" << std::endl;
		exit(1);
	}

	std::ifstream code(argv[1]);
	if (!code.is_open())
	{
		std::cerr << "Ne mogu naci datoteku " << argv[1] << std::endl;
		exit(1);
	}

	C0Compiler::Lekser lex(&code);
	std::deque<C0Compiler::Token*> tokeni = lex.leksiraj();

	for (auto it = tokeni.begin(); it != tokeni.end(); ++it)
		std::cout << **it << std::endl;

	lex.pocisti();

	return 0;
}
