/*

	CHELPY.SO
	v.1.0

	This is a python chess library intended for Google Colab.
		
	Or other pythons, of course.
	Develop as needed.
	nov.2024
	
	
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


char lib_buffer[102400];
char lib_mv[4<<8];

// freaking iterations
char lib_ii_mv[(4<<8)*32];
U8 *lib_ii_po[32];

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
	PyArg_ParseTuple( args,  "" );
	SetStartPos();
	return Py_BuildValue( "", NULL );
}

PyObject *sboard ( PyObject *self, PyObject *args ) {
	PyArg_ParseTuple( args,  "" );
	sBoard( lib_buffer );
	return Py_BuildValue( "s", lib_buffer );
}

PyObject *getfen ( PyObject *self, PyObject *args ) {
	PyArg_ParseTuple( args,  "" );
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
	PyArg_ParseTuple( args,  "" );
	MoveGen(lib_mv);
	int count = lib_mv[0];
	return Py_BuildValue( "i", count );
}

PyObject *legalmoves ( PyObject *self, PyObject *args ) {
	PyArg_ParseTuple( args,  "" );
	sLegalMoves( lib_buffer, lib_mv );
	return Py_BuildValue( "s", lib_buffer );
}


PyObject *ucimove ( PyObject *self, PyObject *args ) {
	char *ucistr;
	PyArg_ParseTuple( args,  "s", &ucistr );
	return Py_BuildValue( "i", uciMove( ucistr ) );
}

PyObject *undomove ( PyObject *self, PyObject *args ) {
	PyArg_ParseTuple( args,  "" );
	UnDoMove();
	return Py_BuildValue( "", NULL );
}

PyObject *parsepgn ( PyObject *self, PyObject *args ) {
	char *pgnstr;
	PyArg_ParseTuple( args,  "s", &pgnstr );
	parse_pgn_moves( pgnstr );
	return Py_BuildValue( "s", lib_buffer );
}

PyObject *ischeck ( PyObject *self, PyObject *args ) {
	PyArg_ParseTuple( args,  "" );
	return ( IsCheckNow() ? Py_True : Py_False );
}

PyObject *ischeckmate ( PyObject *self, PyObject *args ) {
	PyArg_ParseTuple( args,  "" );
	return ( IsCheckMateNow() ? Py_True : Py_False );
}

// Freak mode. Iterations in depth


PyObject *i_movegen ( PyObject *self, PyObject *args ) {
	int depth;
	PyArg_ParseTuple( args,  "i", &depth );
	
	U8 *dp = &lib_ii_mv[(4<<8)*depth];
	
	MoveGen(dp);
	int count = *(dp++);
	
	lib_ii_po[depth] = dp;
	
	return Py_BuildValue( "i", count );
}


PyObject *i_domove ( PyObject *self, PyObject *args ) {
	int depth;
	PyArg_ParseTuple( args,  "i", &depth );
	
	DoMove( lib_ii_po[depth] );
	lib_ii_po[depth]+=4;
	
	return Py_BuildValue( "", NULL );
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
	{ "getfen", getfen, METH_VARARGS, "Get the FEN of current chess position on board." },
	{ "setfen", setfen, METH_VARARGS, "Set the chess position by FEN." },
	{ "movegen", movegen, METH_VARARGS, "Force resources consuming legal chess moves generator routine." },
	{ "legalmoves", legalmoves, METH_VARARGS, "To display legal chess moves generated now." },
	{ "ucimove", ucimove, METH_VARARGS, "Perform an uci move on chess board." },
	{ "undomove", undomove, METH_VARARGS, "Undo the last move. Also iterations." },
	{ "parsepgn", parsepgn, METH_VARARGS, "Parse PGN and perform moves. Returns uci string." },
	{ "ischeck", ischeck, METH_VARARGS, "Is check+ now or not." },
	{ "i_movegen", i_movegen, METH_VARARGS, "Iterations. Fast MoveGen at depth." },
	{ "i_domove", i_domove, METH_VARARGS, "Iterations. Fast DoMove at depth." },
	{ "freaknow", freaknow, METH_VARARGS, "Iterations. C route sample returns occupancy of white king." },
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





