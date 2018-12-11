#ifndef _TOKEN_HPP_
#define _TOKEN_HPP_

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
// HASH			|		#		|
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
// COMMENT		|				|	sve između // i \n ili /* i */
// IF			|		if		|
// ELSE			|	   else		|
// WHILE		|	   while	|
// FOR			|		for		|
// ASSERT		|	  assert	|
// ERROR		|	   error	|
// ALLOC		|	   alloc	|
// ALLOCA		|	alloc_array	|
// IDENTIFIER	|				|	identifier varijable, funkcije itd.
// DECIMALNI	|				|	broj u decimalnom zapisu
// HEKSADEKADSKI|				|	broj u heksadekadskom zapisu
// CHRLIT		|				|	char literal
// STRLIT		|				|	string literal
// BOOLEAN		|				|	true-false
// NUL			|	   NULL		|
// BREAK		|	  break		|
// CONTINUE		|	 continue	|
// RETURN		|	  return	|
// INT			|		int		|
// BOOL			|	   bool		|
// CHAR			|	   char		|
// STRING		|	  string	|
// VOID			|	   void		|
// POINTER		|				|	pointer
// ARRAY		|				|	array
// PRAZNO		|				|	znak ' '

#include <string>

namespace C0Compiler
{
	enum TokenTip
	{
		// separatori
		OOTV, OZATV, UOTV, UZATV, VOTV, VZATV, ZAREZ, TZAREZ, HASH,

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
		COMMENT, COMBEGIN, COMEND,

		// statementi
		IF, ELSE, WHILE, FOR, ASSERT, ERROR,

		// razno
		ALLOC, ALLOCA, IDENTIFIER, DECIMALNI, HEKSADEKADSKI, CHRLIT, STRLIT, BOOLEAN,
		NUL, BREAK, CONTINUE, RETURN, INT, BOOL, CHAR, STRING, VOID, POINTER, ARRAY, 
		PRAZNO
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
			Token(TokenTip tip, std::string const& sadrzaj, int redak, int stupac) : m_tip(tip), m_sadrzaj(sadrzaj), m_redak(redak), m_stupac(stupac) {}
			friend std::ostream& operator<<(std::ostream& out, Token const&);
	};
	// kad se pokaže potreba (a mogla bi), ovdje ću vjerojatno napraviti da 'razno' tokeni
	// nasljeđuju od općeg tokena i imaju svoje metode

	
}

#endif