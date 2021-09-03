#ifndef TOKEN_HPP
#define TOKEN_HPP

// Tokeni koje planiram koristiti:

// IME			|	ZNAČENJE	|			OPIS
//--------------+---------------+-------------------------------
// OOTV			|		(		|
// OZATV		|		)		|
// UOTV			|		[		|
// UZATV		|		]		|
// VOTV			|		{		|
// VZATV		|		}		|
// ZAREZ		|		,		|
// TZAREZ		|		:		|
// USKL			|		!		|
// TILDA		|		~		|
// MINUS		|		-		|
// ZVJ			|		*		|
// TOCKA		|		.		|
// STRELICA		|		->		|
// SLASH		|		/		|
// MOD			|		%		|
// PLUS			|		+		|
// LSHIFT		|		<<		|
// RSHIFT		|		>>		|
// LESS			|		<		|
// LESSEQ		|		<=		|
// GRT			|		>		|
// GRTEQ		|		>=		|
// EQ			|		==		|
// NEQ			|		!=		|
// BITAND		|		&		|
// BITXOR		|		^		|
// BITOR		|		|		|
// LAND			|		&&		|
// LOR			|		||		|
// CONDQ		|		?		| conditional question mark
// DTOCKA		|		:		|
// PLUSEQ		|		+=		|
// MINUSEQ		|		-=		|
// ZVJEQ		|		*=		|
// SLASHEQ		|		/=		|
// MODEQ		|		%=		|
// LSHIFTEQ		|		<<=		|
// RSHIFTEQ		|		>>=		|
// ASSIGN		|		=		|
// BANDEQ		|		&=		|
// BXOREQ		|		^=		|
// BOREQ		|		|=		|
// DECR			|		--		|
// INCR			|		++		|
// BASKSP		|		\b		|
// ALERT		|		\a		|
// QUOTE		|		'		|
// DQUOTE		|		"		|
// BSLASH		|		\		|
// COMMENT		|				|	sve između /* i */ ili // i \n
// IF			|		if		|
// ELSE			|	   else		|
// WHILE		|	   while	|
// FOR			|		for		|
// ASSERT		|	  assert	|
// ERROR		|	   error	|
// ALLOC		|	   alloc	|
// ALLOCARRAY	|	alloc_array	|
// IDENTIFIER	|				|	identifikator varijable, funkcije itd.
// DEKADSKI		|				|	broj u dekadskom zapisu
// HEKSADEKADSKI|				|	broj u heksadekadskom zapisu
// CHRLIT		|				|	char literal
// STRLIT		|				|	string literal
// BOOLEAN		|				|	true/false
// NUL			|	   NULL		|	ovo je C++ i NULL je rezervirana riječ pa koristim NUL
// BREAK		|	  break		|
// CONTINUE		|	 continue	|
// RETURN		|	  return	|
// INT			|		int		|
// BOOL			|	   bool		|
// CHAR			|	   char		|
// STRING		|	  string	|
// VOID			|	   void		|
// STRUCT		|	  struct	|
// INTPOINT		|				|	int pointer
// BOOLPOINT	|				|	bool pointer
// CHARPOINT	|				|	char pointer
// STRINGPOINT	|				|	string pointer
// VOIDPOINT	|				|	void pointer
// INTARRAY		|				|	int array
// BOOLARRAY	|				|	bool array
// CHARARRAY	|				|	char array
// STRINGARRAY	|				|	string array
// USE			|	   #use		|
// PRAZNO		|				|	znak ' '
// TYPEDEF		|	 typedef	|	znak ' '
// POCETAK		|				|	početak datoteke
// KRAJ			|				|	kraj datoteke
//--------------+---------------+-------------------------------

#include <string>
#include <vector>

namespace C0Compiler
{
	// Leaf je list u apstraktnom sintaksnom stablu,
	// zamišljen je kao tanki AST wrapper za token.
	// definicija u AST.hpp
	class Leaf;

	enum TokenTip
	{
		// separatori
		OOTV, OZATV, UOTV, UZATV, VOTV, VZATV, ZAREZ, TZAREZ,

		// unarni operatori
		USKL, TILDA, MINUS, ZVJ,

		// binarni operatori
		TOCKA, STRELICA, SLASH, MOD, PLUS, LSHIFT, RSHIFT, LESS, LESSEQ, GRT, GRTEQ,
		EQ, NEQ, BITAND, BITXOR, BITOR, LAND, LOR, CONDQ, DTOCKA,

		// operatori pridruživanja
		PLUSEQ, MINUSEQ, ZVJEQ, SLASHEQ, MODEQ, LSHIFTEQ, RSHIFTEQ, ASSIGN, BANDEQ,
		BXOREQ, BOREQ,

		// postfix operatori
		DECR, INCR,

		// escape sekvence
		BACKSP, ALERT, QUOTE, DQUOTE, BSLASH,

		// statementi
		IF, ELSE, WHILE, FOR, ASSERT,

		// razno
		ALLOC, ALLOCARRAY, IDENTIFIER, DEKADSKI, HEKSADEKADSKI, CHRLIT, STRLIT, BOOLEAN,
		NUL, BREAK, CONTINUE, RETURN, INT, BOOL, CHAR, STRING, VOID, STRUCT, INTPOINT,
		BOOLPOINT, CHARPOINT, STRINGPOINT, VOIDPOINT, INTARRAY, BOOLARRAY, CHARARRAY,
		STRINGARRAY, USE, PRAZNO, TYPEDEF, POCETAK, KRAJ
	};

	// popis svih tipova tokena tako da ih mogu lakše ispisivati. isto što i ovo gore,
	// samo što je neurednije i nije enum, nego vector stringova
	static std::vector<std::string> tokenString = 
	{
		"OOTV", "OZATV", "UOTV", "UZATV", "VOTV", "VZATV", "ZAREZ", "TZAREZ", "USKL", 
		"TILDA", "MINUS", "ZVJ", "TOCKA", "STRELICA", "SLASH", "MOD", "PLUS", "LSHIFT", 
		"RSHIFT", "LESS", "LESSEQ", "GRT", "GRTEQ", "EQ", "NEQ", "BITAND", "BITXOR", 
		"BITOR", "LAND", "LOR", "CONDQ", "DTOCKA", "PLUSEQ", "MINUSEQ", "ZVJEQ", "SLASHEQ", 
		"MODEQ", "LSHIFTEQ", "RSHIFTEQ", "ASSIGN", "BANDEQ", "BXOREQ", "BOREQ", "DECR", 
		"INCR", "BACKSP", "ALERT", "QUOTE", "DQUOTE", "BSLASH", "IF", "ELSE", "WHILE", 
		"FOR", "ASSERT", "ALLOC", "ALLOCARRAY", "IDENTIFIER", "DEKADSKI", "HEKSADEKADSKI", 
		"CHRLIT", "STRLIT", "BOOLEAN", "NUL", "BREAK", "CONTINUE", "RETURN", "INT", "BOOL", 
		"CHAR", "STRING", "VOID", "STRUCT", "INTPOINT", "BOOLPOINT", "CHARPOINT", 
		"STRINGPOINT", "VOIDPOINT", "INTARRAY", "BOOLARRAY", "CHARARRAY", "STRINGARRAY", 
		"USE", "PRAZNO", "TYPEDEF", "POCETAK", "KRAJ"
	};

	// Općenita klasa za token
	class Token
	{
		private:
			TokenTip m_tip;
			std::string m_sadrzaj;
			int m_redak;
			int m_stupac;

		public:
			Token(TokenTip tip = PRAZNO, std::string const& sadrzaj = "", int redak = -1, int stupac = -1) :
				m_tip(tip), m_sadrzaj(sadrzaj), m_redak(redak), m_stupac(stupac) {}

			TokenTip Tip() const { return m_tip; }
			int Redak() const { return m_redak; }
			int Stupac() const { return m_stupac; }

			std::string const& Sadrzaj() const { return m_sadrzaj; }
			bool OfType(TokenTip tip) const { return m_tip == tip; }
			bool OfType(Token const& token) const { return m_tip == token.m_tip; }

			operator Leaf() const;	// zamotaj Token u Leaf

			friend std::ostream& operator<<(std::ostream& out, Token const&);
	};
}

#endif