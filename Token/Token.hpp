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
// CONDQ		|		?		|
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
// COMMENT		|				|	sve između /* i */
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
// POINTER		|				|	pointer		->21.12.2018. premišljam se da umjesto pointer i array imam token za
// ARRAY		|				|	array		->			svaki mogući tip arraya i tokena
// USE			|	   #use		|
// PRAZNO		|				|	znak ' '
// POCETAK		|				|	početak datoteke
// KRAJ			|				|	kraj datoteke

#include <string>
#include <map>

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

		// komentari
		COMMENT,

		// statementi
		IF, ELSE, WHILE, FOR, ASSERT, ERROR,

		// razno
		ALLOC, ALLOCARRAY, IDENTIFIER, DEKADSKI, HEKSADEKADSKI, CHRLIT, STRLIT, BOOLEAN,
		NUL, BREAK, CONTINUE, RETURN, INT, BOOL, CHAR, STRING, VOID, POINTER, ARRAY, 
		USE, PRAZNO, POCETAK, KRAJ
	};

	std::map<TokenTip, std::string> tokenString;

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
			
			TokenTip getTip() { return m_tip; }
			int getRedak() { return m_redak; }
			int getStupac() { return m_stupac; }

			std::string const& getSadrzaj() { return m_sadrzaj; }
			bool isOfType(TokenTip tip) { return m_tip == tip; }
			bool isOfType(Token const& token) { return m_tip == token.m_tip; }

			operator Leaf() const;	// zamotaj Token u Leaf

			friend std::ostream& operator<<(std::ostream& out, Token const&);
	};
	// kad se pokaže potreba (a mogla bi), ovdje ću vjerojatno napraviti da 'razno' tokeni
	// nasljeđuju od općeg tokena i imaju svoje metode
}

#endif