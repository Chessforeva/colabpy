/*

	CHELPY.SO
	v.1.5

	This is a python chess library intended for Google Colab.
		
	Or other pythons, of course.
	Develop as needed.
	dec.2024
	
	
	To compile:

	python
	>>>import sysconfig
	>>>sysconfig.get_paths()

	gcc chelpy.c -shared -o chelpy.so -I<python 'include': path> -fPIC 
	
	That's it.
	
	
	Online available linux binary:
	!wget("https://chessforeva.gitlab.io/colab/chelpy.so")

*/


#include <Python.h>
#include <stdio.h>

#include "u64_chess.h"

// This is needed for polyglot key generation
#include "u64_polyglot.h"

char lib_buffer[102400];
char lib_mv[4<<8];
int depth;

// freaking iterations
char lib_ii_mv[(4<<8)*32];
U8 *lib_ii_po[32];

Py_ssize_t isquares[64];

const int matdifvals[] = { 900, 500, 320, 318, 110, 20000, 0,0, -900, -500, -320, -318, -110, -20000, 0,0 };

//------------------------------------------
//
//	Parse PGN part
//
void parse_pgn_moves( char *pgn ) {

    char *s=pgn; 
    char *u=lib_buffer;
	
    int q=0;
    int mn=1;

    U8 mv[4<<8];
    char Pc=0, Pp=0;
    int w, i, N, fcastle=0, prom=0, found=0;
    char *P;
    char c, h1=0,v1=0,h2=0,v2=0;
    U8 ty=0, fl=0, sq1=0, sq2=0;
    U8 *po;

    for(;*s!=0;s++) {
        c = *s;
        if(c=='{') q++;
        if(q>0) *s=' ';       // remove comments
        if(c=='}') q--;
        if(c<=13) *s=' ';   // remove TABs etc...
        if(*s=='e' && *(s+1)=='p') { *s=' '; *(s+1)=' '; }      // ep case
        if(c=='*') *s=' ';
    }
    s=pgn;
	*u = 0;

    MoveGen(mv);

    for(;*s!=0;s++) {
        while(*s==' ') s++;
        if(*s==0) break;

        // skip number
        c = *s;
        if (c >= '1' && c <= '9') {
            while (*s != 0 && *s != '.' && *s != ' ') s++;
        }
        while (*s == '.' || *s == ' ') s++;
        if (*s == 0) break;

        fcastle = 0;
        if((strncmp(s,"0-0-0",5)==0)||(strncmp(s,"O-O-O",5)==0)) {
            fcastle = 2;
        }
        else {
            if((strncmp(s,"0-0",3)==0)||(strncmp(s,"O-O",3)==0)) {
                fcastle = 1;
            }
        }

        // obtain chess move from pgn string

        if(!fcastle) {
            Pc = 0; w = 0; Pp = 0;
            h1 = 0; v1 = 0; h2 = 0; v2 = 0;
            while(*s!=' ' && *s!=0) {
                c = *s;
                if(c=='.') w=0;
                if(Pc==0) {
                    P = strchr("QRBNPK",c);
                    if(P!=NULL) { Pc = *P; w=1; }
                }
                if(w<3 && (c=='x'||c=='-'||c==':')) w=3;

                if(c>='a' && c<='h') {
                    if(w<3 && h1==0) { h1=c; w=2; }
                    else if(w<6) { h2=c; w=4; }
                }
                if(w>0 && c>='1' && c<='8') {
                    if(w<3 && v1==0) { v1=c; w=3; }
                    else if(w<6) { v2=c; w=5; }
                }
                if(c=='=') { w=6; c = *(++s); }
                if(w>4) {
                    P = strchr("QRBN",c);
                    if(P!=NULL) Pp = (*P)+32;
                    else {
                        P = strchr("qrbn",c);
                        if(P!=NULL) Pp = *P;
                    }
                }

                s++;
            }
            if(h2==0) { h2=h1; v2=v1; h1=0; v1=0; }
            if(h2==0 && v2==0) continue;
        }

        po = mv;
        N = *(po++);
        found = 0;

        // find this move in the movegen list

        for(i=0; i<N; i++) {

            ty = (*po) & 7;
            sq1 = *(po+1);
            sq2 = *(po+2);
            fl = *(po+3);

            if(fl&32) {
              if (sq1>sq2 && fcastle==2) found = 1;
              if (sq1<sq2 && fcastle==1) found = 1;
            }

            if(!found && ((ty==4 && Pc==0)||(pieces[ty]==Pc))) {
                found = 1;
                if(h1!=0 && h1!=((sq1&7)+'a')) found=0;
                if(v1!=0 && v1!=((sq1>>3)+'1')) found=0;
                if(h2!=0 && h2!=((sq2&7)+'a')) found=0;
                if(v2!=0 && v2!=((sq2>>3)+'1')) found=0;

                prom = 0;
                if(fl&2) {   // promotion
                    prom = pieces[ 8+((fl>>2)&3)];
                    if(Pp!=0 && Pp != prom) found=0;
                    }
                }
            if(found) {
                    DoMove(po);
                    *(u++) = ((sq1&7)+'a');
                    *(u++) = ((sq1>>3)+'1');
                    *(u++) = ((sq2&7)+'a');
                    *(u++) = ((sq2>>3)+'1');
                    if(prom) *(u++) = prom;
                    *(u++) = ' ';
                    *u=0;
                    MoveGen(mv);
                    if(!ToMove) mn++;
                    break;
                    }
            po += 4;
        }

    }
}




//------------------------------------------
//
//	python interface

PyObject *setstartpos ( PyObject *self, PyObject *args ) {
	SetStartPos();
	return Py_BuildValue( "", NULL );
}

PyObject *sboard ( PyObject *self, PyObject *args ) {
	sBoard( lib_buffer );
	return Py_BuildValue( "s", lib_buffer );
}

PyObject *sboard64 ( PyObject *self, PyObject *args ) {
	U8 c,j,sq;
	char *s = lib_buffer;
	for(j=0;j<64;j++) s[j]=' ';
	for(j=0;j<14;j++) {
		if(j==6) j=8;
		c = pieces[j];
		U64 o = *(PIECES[j]);
		while(o) {
			s[trail0(o)] = c;
			o &= o-1;
		}
	}
	s[64]=0;
	return Py_BuildValue( "s", lib_buffer );
}

// An unique position string 70 bytes long to save position somewhere
// (kinda key code for convinience)
PyObject *uniq ( PyObject *self, PyObject *args ) {
	U8 c,j,sq;
	char *s = lib_buffer;
	for(j=0;j<64;j++) s[j]=' ';
	for(j=0;j<14;j++) {
		if(j==6) j=8;
		c = pieces[j];
		U64 o = *(PIECES[j]);
		while(o) {
			s[trail0(o)] = c;
			o &= o-1;
		}
	}
	s[64]= ( ToMove ? 'b' : 'w' );
	s[65]= ( ((CASTLES&castle_E1H1)==castle_E1H1) ? 'K' : ' ' );
	s[66]= ( ((CASTLES&castle_E1C1)==castle_E1C1) ? 'Q' : ' ' );
	s[67]= ( ((CASTLES&castle_E8H8)==castle_E8H8) ? 'k' : ' ' );
	s[68]= ( ((CASTLES&castle_E8C8)==castle_E8C8) ? 'q' : ' ' );

	if(!ENPSQ) {
		s[69] = ' ';
		}
	else {
		sq = trail0(ENPSQ);
		s[69] = (( (((sq>>3)==2) ? 'A' : 'a' ) + (sq&7)) );
		}
	s[70]=0;
	return Py_BuildValue( "s", lib_buffer );
}

// faster position setup from uniq code
PyObject *setasuniq ( PyObject *self, PyObject *args ) {
	char *ustr;
	PyArg_ParseTuple( args,  "s",  &ustr);
	
    U8 sq, j, v;
	char c;

    WK=WQ=WR=WB=WN=WP=0LL;
    BK=BQ=BR=BB=BN=BP=0LL;

	for(sq=0; sq<64; sq++)
        {
		c = ustr[sq];
		if(c!=' ') {
			for(j=0;j<14;j++) {
                if(j==6) j=8;
                if(pieces[j]==c) { *(PIECES[j])|=(1LL<<sq); }
                }
            }
		}
	ToMove = (ustr[64]=='w'? 0: 1);
	CASTLES = 0LL;
	if( ustr[65]== 'K' ) CASTLES|=castle_E1H1;
	if( ustr[66]== 'Q' ) CASTLES|=castle_E1C1;
	if( ustr[67]== 'k' ) CASTLES|=castle_E8H8;
	if( ustr[68]== 'q' ) CASTLES|=castle_E8C8;;
	
	if( ustr[69]!= ' ' ) {
		c = ustr[69];
		v = 5;
		if(c<'a') {
			c+=32;
			v = 2;
			}
        sq = ((v<<3) | ((c-'a')&7));
        ENPSQ = (1LL<<sq);
        }
	else {
		ENPSQ = 0LL;
		}
	
	undo_p = undobuffer;
    mg_po = mg_cnt = mg_uci_list;
	
	return Py_BuildValue( "", NULL );
}

PyObject *getfen ( PyObject *self, PyObject *args ) {
	sGetFEN( lib_buffer );
	return Py_BuildValue( "s", lib_buffer );
}

PyObject *setfen ( PyObject *self, PyObject *args ) {
	char *fenstr;
	PyArg_ParseTuple( args,  "s",  &fenstr);
	SetByFEN( fenstr );
	return Py_BuildValue( "", NULL );
}

PyObject *movegen ( PyObject *self, PyObject *args ) {
	MoveGen(lib_mv);
	int count = lib_mv[0];
	return Py_BuildValue( "i", count );
}

PyObject *legalmoves ( PyObject *self, PyObject *args ) {
	sLegalMoves( lib_buffer, lib_mv );
	return Py_BuildValue( "s", lib_buffer );
}

PyObject *ucimove ( PyObject *self, PyObject *args ) {
	char *ucistr;
	PyArg_ParseTuple( args,  "s", &ucistr );
	return Py_BuildValue( "i", uciMove( ucistr ) );
}

PyObject *parseucimoves ( PyObject *self, PyObject *args ) {
	char *ucisstr;
	PyArg_ParseTuple( args,  "s", &ucisstr );
	char *s = ucisstr;
	char uci[8];
	int i, r = 0;
	while( (*s)>13 ) {
		while((*s)==32) s++;
		for( i=0; ((*s)>13) && (i<4); i++) uci[i]=(*(s++));
		if( ((*s)>13) && (*s)!=32 ) uci[i++]=(*(s++));
		uci[i]=0;
		r += uciMove( uci );
		}
	return Py_BuildValue( "i", r );
}


PyObject *undomove ( PyObject *self, PyObject *args ) {
	UnDoMove();
	return Py_BuildValue( "", NULL );
}

PyObject *swaptomove ( PyObject *self, PyObject *args ) {
	ToMove^=1;
	return Py_BuildValue( "i", ToMove );
}

PyObject *sidetomove ( PyObject *self, PyObject *args ) {
	return Py_BuildValue( "i", ToMove );
}

// Counts pieces by given occupancy
int bitCount(U64 o)
{
	int n = 0;
    while(o) {
		n++;
		o &= o-1;
	}
	return n;
}

// to verify chess position for normality
U8 seemslegit () {
	U8 b = 1;
	U64 pawns = WP | BP;
	
	b &= (( (pawns & ((1LL<<0)|(1LL<<1)|(1LL<<2)|(1LL<<3)|(1LL<<4)|(1LL<<5)|(1LL<<6)|(1LL<<7)|
						(1LL<<56)|(1LL<<57)|(1LL<<58)|(1LL<<59)|(1LL<<60)|(1LL<<61)|(1LL<<62)|(1LL<<63) )) ==0 ) ? 1 : 0);

	b &= ((( bitCount(WK)==1 ) && ( bitCount(BK)==1 ) && (WK!=BK) )? 1: 0);	
	b &= ((( bitCount(WP)<9 ) && ( bitCount(BP)<9 ) && ( bitCount(WN)<3 ) && ( bitCount(BN)<3 )) ? 1: 0);
	b &= ((( bitCount(WB)<3 ) && ( bitCount(BB)<3 ) && ( bitCount(WR)<3 ) && ( bitCount(BR)<3 )) ? 1: 0);
	if(b) {
		// avoid same bishop squares
		U64 o;
		int sq, v, h, d, D;
		for(D=9, o=WB; o;) {
			sq = (U8)trail0(o);
			v = (sq>>3); h = (sq&7); d = (v+h)&1;
			if(D==9) D=d;
			else if(D==d) b=0;
			o &= o-1;
		}
		for(D=9, o=BB; o;) {
			sq = (U8)trail0(o);
			v = (sq>>3); h = (sq&7); d = (v+h)&1;
			if(D==9) D=d;
			else if(D==d) b=0;
			o &= o-1;
		}
	}
		
	// can not be both check+
	if(b){
		if(IsCheckNow()) {
			ToMove^=1;
			if(IsCheckNow()) b=0;
			ToMove^=1;
		}
	}
	// should be pawn normality verify too, but not today
	return b;
}


PyObject *seemslegitpos ( PyObject *self, PyObject *args ) {
	return ( seemslegit() ? Py_True : Py_False );
}

PyObject *parsepgn ( PyObject *self, PyObject *args ) {
	char *pgnstr;
	PyArg_ParseTuple( args,  "s", &pgnstr );
	parse_pgn_moves( pgnstr );
	return Py_BuildValue( "s", lib_buffer );
}

PyObject *ischeck ( PyObject *self, PyObject *args ) {
	return ( IsCheckNow() ? Py_True : Py_False );
}

PyObject *ischeckmate ( PyObject *self, PyObject *args ) {
	return ( IsCheckMateNow() ? Py_True : Py_False );
}

PyObject *polyglotkey ( PyObject *self, PyObject *args ) {
	return Py_BuildValue( "K", getPolyglotKey() );
}


// Freak mode. Iterations in depth


PyObject *i_movegen ( PyObject *self, PyObject *args ) {
	PyArg_ParseTuple( args,  "i", &depth );
	
	U8 *dp = &lib_ii_mv[(4<<8)*depth];
	
	MoveGen(dp);
	int count = *(dp++);
	
	lib_ii_po[depth] = dp;
	
	return Py_BuildValue( "i", count );
}


PyObject *i_domove ( PyObject *self, PyObject *args ) {
	PyArg_ParseTuple( args,  "i", &depth );
	
	DoMove( lib_ii_po[depth] );
	lib_ii_po[depth]+=4;
	
	return Py_BuildValue( "", NULL );
}

// create a list of tuples
PyObject *Tu( U64 o ) {
	
	int n = 0;

	while(o) {
		// which square?
		isquares[n++] = (U8)trail0(o);
		o &= o-1;
		}
	
	PyObject *tuple = PyTuple_New(n);
	for(int i=0; i<n; i++)
	{
		PyTuple_SET_ITEM(tuple, i, PyLong_FromSsize_t( isquares[i] ) );
	}
	
	return tuple;
}


// get current board in object
PyObject *getboard ( PyObject *self, PyObject *args ) {

	return Py_BuildValue( "{s:O,s:O,s:O,s:O,s:O,s:O,s:O,s:O,s:O,s:O,s:O,s:O,s:i,s:i}",
		"wk", Tu(WK), "wq", Tu(WQ), "wr",	Tu(WR),		"wb", Tu(WB), "wn", Tu(WN), "wp", Tu(WP),
		"bk", Tu(BK), "bq", Tu(BQ), "br",	Tu(BR),		"bb", Tu(BB), "bn", Tu(BN), "bp", Tu(BP),
		"tomove",  ToMove, "enpsq", trail0(ENPSQ)
		);
}

// get current board in object
PyObject *getboardU64 ( PyObject *self, PyObject *args ) {
	return Py_BuildValue( "(KKKKKKKKKKKKii)",
		WK,WQ,WR,WB,WN,WP,BK,BQ,BR,BB,BN,BP, ToMove, trail0(ENPSQ) );
}


// get information on possible castlings
PyObject *getcastlings ( PyObject *self, PyObject *args ) {

	return Py_BuildValue( "{s:i,s:i,s:i,s:i}",
		"e1c1", ( ((CASTLES&castle_E1C1)==castle_E1C1) ? 1 : 0),
		"e1h1", ( ((CASTLES&castle_E1H1)==castle_E1H1) ? 1 : 0),
		"e8c8", ( ((CASTLES&castle_E8C8)==castle_E8C8) ? 1 : 0),
		"e8h8", ( ((CASTLES&castle_E8H8)==castle_E8H8) ? 1 : 0) );
}

// get information on possible castlings
PyObject *getcastlingsU64 ( PyObject *self, PyObject *args ) {

	return Py_BuildValue( "(iiii)",
		( ((CASTLES&castle_E1C1)==castle_E1C1) ? 1 : 0),
		( ((CASTLES&castle_E1H1)==castle_E1H1) ? 1 : 0),
		( ((CASTLES&castle_E8C8)==castle_E8C8) ? 1 : 0),
		( ((CASTLES&castle_E8H8)==castle_E8H8) ? 1 : 0) );
}

// get occupancies information
PyObject *getoccupancies ( PyObject *self, PyObject *args ) {

    WOCC = WK|WQ|WR|WB|WN|WP;
    BOCC = BK|BQ|BR|BB|BN|BP;
    OCC = WOCC|BOCC;
    NOCC = ~OCC;
    NWOCC = ~WOCC;
    NBOCC = ~BOCC;
    EOCC = OCC|ENPSQ;
    EWOCC = WOCC|ENPSQ;
    EBOCC = BOCC|ENPSQ;
	
	return Py_BuildValue( "{s:O,s:O,s:O,s:O,s:O,s:O,s:O,s:O,s:O}",
		"occ",Tu(OCC), "wocc", Tu(WOCC), "bocc", Tu(BOCC), "nocc", Tu(NOCC),
		"nwocc", Tu(NWOCC), "nbocc", Tu(NBOCC), "eocc", Tu(EOCC), "ewocc", Tu(EWOCC), "ebocc", Tu(EBOCC) );
}

// get occupancies information
PyObject *getoccupanciesU64 ( PyObject *self, PyObject *args ) {
	
    WOCC = WK|WQ|WR|WB|WN|WP;
    BOCC = BK|BQ|BR|BB|BN|BP;
    OCC = WOCC|BOCC;
    NOCC = ~OCC;
    NWOCC = ~WOCC;
    NBOCC = ~BOCC;
    EOCC = OCC|ENPSQ;
    EWOCC = WOCC|ENPSQ;
    EBOCC = BOCC|ENPSQ;
	
	return Py_BuildValue( "(KKKKKKKKK)",
		OCC, WOCC, BOCC, NOCC, NWOCC, NBOCC, EOCC, EWOCC, EBOCC );
}

// get more data on current move to iterate 
PyObject *i_moveinfo ( PyObject *self, PyObject *args ) {
	PyArg_ParseTuple( args,  "i", &depth );
	
	U8 *p = lib_ii_po[depth];
	
	U8 t1 = ((*p)&15);
	U8 t2 = ((*(p++))>>4)&15;
	U8 f_sq = (*(p++));
	U8 t_sq = (*(p++));
	U8 flags = (*p);
	U8 capt = (flags&1);
	U8 pr = ((flags&(1<<1))?1:0);
	U8 pr_pc = ((flags>>2)&3);
	U8 ecapt = ((flags&(1<<4))?1:0);
	U8 cs = ((flags&(1<<5))?1:0);
	U8 ck = ((flags&(1<<6))?1:0);
	U8 cm = ((flags&(1<<7))?1:0);
	
	return Py_BuildValue( "{s:i,s:i,s:i,s:i,s:i,s:i,s:i,s:i,s:i,s:i,s:i}",
		"pieceTypeFrom", t1,
		"pieceTypeTo", t2,
		"squareFrom", f_sq,
		"squareTo", t_sq,
		"capture", capt,
		"promote", pr,
		"promPiece", pr_pc,
		"enPassCapture", ecapt,
		"castling", cs,
		"check", ck,
		"checkmate", cm );
}


// get more data on current move to iterate 
PyObject *i_moveinfoU64 ( PyObject *self, PyObject *args ) {
	PyArg_ParseTuple( args,  "i", &depth );
	
	U8 *p = lib_ii_po[depth];
	
	U8 t1 = ((*p)&15);
	U8 t2 = ((*(p++))>>4)&15;
	U8 f_sq = (*(p++));
	U8 t_sq = (*(p++));
	U8 flags = (*p);
	U8 capt = (flags&1);
	U8 pr = ((flags&(1<<1))?1:0);
	U8 pr_pc = ((flags>>2)&3);
	U8 ecapt = ((flags&(1<<4))?1:0);
	U8 cs = ((flags&(1<<5))?1:0);
	U8 ck = ((flags&(1<<6))?1:0);
	U8 cm = ((flags&(1<<7))?1:0);
	
	return Py_BuildValue( "(iiiiiiiiiii)",
		t1, t2, f_sq, t_sq, capt, pr, pr_pc, ecapt, cs, ck, cm );
}

PyObject *i_skipmove ( PyObject *self, PyObject *args ) {
	PyArg_ParseTuple( args,  "i", &depth );
	lib_ii_po[depth]+=4;
	return Py_BuildValue( "", NULL );
}

int pieces_cnt(int cz) {
	
    WOCC = WK|WQ|WR|WB|WN|WP;
    BOCC = BK|BQ|BR|BB|BN|BP;
    OCC = WOCC|BOCC;
	
	U64 o = (cz==0 ? OCC :(cz==1 ? WOCC : BOCC));	//occupancies
	return bitCount(o);
}

PyObject *piecescount ( PyObject *self, PyObject *args ) {
	return Py_BuildValue( "i", pieces_cnt(0));
}
PyObject *whitecount ( PyObject *self, PyObject *args ) {
	return Py_BuildValue( "i", pieces_cnt(1));
}
PyObject *blackcount ( PyObject *self, PyObject *args ) {
	return Py_BuildValue( "i", pieces_cnt(2));
}

PyObject *materialdiff ( PyObject *self, PyObject *args ) {
	
	int diff = 0;
	for(U8 j=0;j<14;j++) {
		if(j==6) j=8;
		U64 o = *(PIECES[j]);
		while(o) {
			diff += matdifvals[j];
			o &= o-1;
		}
	}
	return Py_BuildValue( "i", diff);
}

//
// Here is a sample function that can be advanced as chess evaluation or something else.
//
PyObject *freaknow ( PyObject *self, PyObject *args ) {
	PyArg_ParseTuple( args,  "" );
	
	U64 occupancies = OCC;
	U64 whites = WOCC;
	U64 blacks = BOCC;
	//  or use other variables and arrays to scan bits WK,WQ,....
	//  develop own chess engine at C level
	// ...
	
	U64 o = WK;	// white king occupancies

	int sq = -1;

	while(o) {
		// which square?
		sq = trail0(o);
		o &= o-1;
		}

	return Py_BuildValue( "i", sq);
}



//------------------------------------------


static PyMethodDef methods[] = {
	{ "setstartpos", setstartpos, METH_VARARGS, "Set starting chess position on board." },
	{ "sboard", sboard, METH_VARARGS, "To display the chess board." },
	{ "sboard64", sboard64, METH_VARARGS, "sboard into string[64] fast without ck,cm." },
	{ "getboard", getboard, METH_VARARGS, "Get variables of board into tuples." },
	{ "getboardU64", getboardU64, METH_VARARGS, "getboard into unsigned long long (fast)" },
	{ "getfen", getfen, METH_VARARGS, "Get the FEN of current chess position on board." },
	{ "setfen", setfen, METH_VARARGS, "Set the chess position by FEN." },
	{ "movegen", movegen, METH_VARARGS, "Force resources consuming legal chess moves generator routine." },
	{ "legalmoves", legalmoves, METH_VARARGS, "To display legal chess moves generated now." },
	{ "sidetomove", sidetomove, METH_VARARGS, "Side to move 0-white,1-black" },
	{ "swaptomove", swaptomove, METH_VARARGS, "Swap side to move" },
	{ "ucimove", ucimove, METH_VARARGS, "Perform an uci move on chess board." },
	{ "undomove", undomove, METH_VARARGS, "Undo the last move. Also iterations." },
	{ "parsepgn", parsepgn, METH_VARARGS, "Parse PGN and perform moves. Returns uci string." },
	{ "parseucimoves", parseucimoves, METH_VARARGS, "Parse string of ucis for faster performance. Returns count of moves made." },
	{ "ischeck", ischeck, METH_VARARGS, "Is check+ now or not." },
	{ "ischeckmate", ischeckmate, METH_VARARGS, "Is checkmate# now or not." },
	{ "polyglotkey", polyglotkey, METH_VARARGS, "Get a the polyglot opening book key for position." },
	{ "i_movegen", i_movegen, METH_VARARGS, "Iterations. Fast MoveGen at depth." },
	{ "i_domove", i_domove, METH_VARARGS, "Iterations. Fast DoMove at depth." },
	{ "i_moveinfo", i_moveinfo, METH_VARARGS, "Iterations. Get move details into variables." },
	{ "i_moveinfoU64", i_moveinfoU64, METH_VARARGS, "moveinfoU64 into unsigned long long  (fast)" },	
	{ "i_skipmove", i_skipmove, METH_VARARGS, "Iterations. Skip move." },
	{ "getcastlings", getcastlings, METH_VARARGS, "Get castling variables." },
	{ "getcastlingsU64", getcastlingsU64, METH_VARARGS, "getcastlings into unsigned long long  (fast)" },
	{ "getoccupancies", getoccupancies, METH_VARARGS, "Get variables of board occupancies into tuples." },
	{ "getoccupanciesU64", getoccupanciesU64, METH_VARARGS, "getoccupancies all into unsigned long long  (fast)" },
	{ "piecescount", piecescount, METH_VARARGS, "Get count of pieces." },
	{ "whitecount", whitecount, METH_VARARGS, "Get count of white pieces." },
	{ "blackcount", blackcount, METH_VARARGS, "Get count of black pieces." },
	{ "materialdiff", materialdiff, METH_VARARGS, "To indicate material difference, not 0." },
	{ "uniq", uniq, METH_VARARGS, "Get 70 bytes unique string of position." },
	{ "setasuniq", setasuniq, METH_VARARGS, "Set position as unig string." },
	{ "seemslegitpos", seemslegitpos, METH_VARARGS, "Verify chess position normality" },
	{ "freaknow", freaknow, METH_VARARGS, "C route sample returns occupancy of white king." },
	{ NULL, NULL, 0, NULL }
};

static struct PyModuleDef chelpy = {
	PyModuleDef_HEAD_INIT,
	"chelpy",
	"This is a chess for python on colab or elsewhere.",
	-1,
	methods
};

//------------------------------------------
PyMODINIT_FUNC PyInit_chelpy() {
	Init_u64_chess();

	printf("Chess library ready.\n");
	return PyModule_Create( &chelpy );
}





