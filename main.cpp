#include "Token/Token.hpp"
#include "Lekser/Lekser.hpp"
#include "Parser/Parser.hpp"
#include "Greska/Greska.hpp"
#include "AST/AST.hpp"

#include "llvm/Bitcode/BitcodeWriter.h"

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

	std::error_code errorCode;
	std::ofstream lekserOut("lekserOut.txt");
	std::ofstream parserOut("parserOut.txt");
	llvm::raw_fd_ostream llvmBC("llvmBC.bc", errorCode);
	llvm::raw_fd_ostream llvmIR("llvmIR.ll", errorCode);
	std::ofstream binaryOut("binaryOut.exe");

	std::deque<std::shared_ptr<C0Compiler::Token>> tokeni;
	C0Compiler::Lekser lex(code);

	try
	{
		tokeni = lex.leksiraj();
		for (std::deque<std::shared_ptr<C0Compiler::Token>>::iterator it = tokeni.begin(); it != tokeni.end(); ++it)
			lekserOut << **it << std::endl;

		lekserOut.flush();
	}
	catch (C0Compiler::LeksickaGreska const& e)
	{
		std::cout << e.what();
		//lex.pocisti();
	}

	std::shared_ptr<C0Compiler::Program> korijenAST = std::make_shared<C0Compiler::Program>();
	C0Compiler::AST::setKorijen(korijenAST);
	C0Compiler::Parser pars(std::move(tokeni), korijenAST);
	try
	{
		pars.parsiraj();
		parserOut << *std::dynamic_pointer_cast<C0Compiler::AST>(korijenAST);
		parserOut.flush();
	}
	catch (C0Compiler::SintaksnaGreska const& e)
	{
		std::cout << e.what();
	}
	catch (C0Compiler::Greska const& e)
	{
		std::cout << e.what();
	}

	// nakon što izgradimo apstraktno sintaksno stablo, 
	// generiramo LLVM IR kod za program
	try
	{
		korijenAST->GenerirajKod();
		llvm::WriteBitcodeToFile(*korijenAST->Module().get(), llvmBC);
		llvmIR << *korijenAST->Module().get();
		llvmIR.flush();
	}		

	catch (C0Compiler::SemantickaGreska const& e)
	{
		std::cout << e.what();
	}
	catch (std::exception const& e)
	{
		std::cout << e.what();
	}

	return 0;
}
