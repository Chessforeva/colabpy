/*
 ---------------------------------------------

    u64_chess C chess v.1.0

        by Chessforeva, feb.2022


    A fast uint64 chess MoveGen logic.

    Setup position, perform fastest move generation with magics, make & unmake move.
    Good for fast performance chess calculations.

    Also intended for WebAssembly cases, as there is no memory allocation at all.
    May be improved by anyone in any project.

    Does not count moves nor keeps notation history.
    Fast calculation for current check+,checkmate cases right in MoveGen.

    ----------------
    for copy+paste on MS Studio C++, or GCC, or Clang projects
    take a look at_samples Sample1(),Sample2()
    no large memory allocation, but can be used easily
    no libraries to include at all
    no classes*, sorry, or even better for compatibility reasons

    *Note:
        Make sure the same variables are not declared elsewhere.

Compilers ok: gcc, clang, MS Visual Studio C++
---------------------------------------------
*/

//
// for MS Studio projects should disable
// warning C4146: unary minus operator applied to
//    unsigned type, result still unsigned
// caused by: -unsigned & something...
//
//#pragma warning(disable:4146);


#ifndef _INC_u64_CHESS
#define _INC_u64_CHESS

#define CONST static const

typedef unsigned long long U64;
typedef unsigned int U32;
typedef unsigned short U16;
typedef unsigned char U8;

extern void Init_u64_chess();               /* Init, should call once on start */
extern void SetStartPos();                  /* Clear and set chess beginning position */
extern void PutPiece( char *pieceAt );      /* Can put pieces directly on board "Pe2" */
extern void SetByFEN( char *pos );          /* UCI interface, set FEN position, slow */
extern void sGetFEN( char *buffer );        /* Get current FEN string into buffer */
extern U8 uciMove( char *mstr );            /* UCI move as "e2e4", ret.1=ok,0=error, MoveGen,slow */
extern void MoveGen( U8 *mvlist );          /* Generate list of Legal moves */
extern U8 IsCheckNow();                     /* Returns: 1=check+, 0=no */
extern U8 IsCheckMateNow();                 /* Returns: 1=checkmate#, 0=no */

// output to string
extern void sBoard( char *buffer );                  /* Current Board to string */
extern void sLegalMoves( char *buffer, U8 *mvlist );  /* Moves to string, after MoveGen */


// The main Board variables
//=============================
U64 WK, WQ, WR, WB, WN, WP; // White pieces
U64 BK, BQ, BR, BB, BN, BP; // Black pieces
U64 ENPSQ;                  // EnPassant square
U64 CASTLES;                // Castling bits of kings, rooks
U8  ToMove;                 // 0-white/1-black to move

// IMPORTANT!!!
// set to undobuffer on each position reset
U64 *undo_p;                // pointer to undo buffer

//--------------------
//  MoveGen global variables
//  -----------------
U8  *mg_po;   // pointers to the list of moves MoveGen(&list)
U8  *mg_cnt;    // pointer to count of legal moves

// Structure of list:
// [0] Cnt = list counter byte (max 255, or code it better)
// [1-4] move0 ( ptype,  from, to squares(0..63),  flags)
// [5-9] move1
// ..... moveCnt
//
// ptype:
//  first 4 bits is moving piece by type
//  next 4 bits is captured piece by type
//
// flags bits:
// bit 0 - capture
// bit 1 - promotion
// bit 2-3 - 00 -queen, 01 -rook, 10 -bishop, 11 -knight
// bit 4 - en-passant capture
// bit 5 - castling move
// bit 6 - check+       ( if mg_CHECK_FLAG on )
// bit 7 - checkmate#   ( if mg_CKMATE_FLAG on, after check+ detected )

// can increase performance, if legal moves only needed,
//  by disabling check+,checkmate# status flag calculations:
#define mg_CHECK_FLAG 1
#define mg_CKMATE_FLAG 1

  // flags atck king, captured piece, promoted, en-passant
U8 mg_fl, mg_fA, mg_fC, mg_fP, mg_fE;
U64 mg_fr, mg_to;               // bits of move squares
U64 mg_P, *mg_Pc;                // piece
U64 mg_nf, mg_nt;

U64 mg_sQ,mg_ss,mg_sk,mg_sr;
U64 mg_sR,mg_sB,mg_sN,mg_sP;     // save-restore vars.
U64 mg_sPc, mg_sOcc;

U8 mg_sq, mg_sqTo, mg_sqA;      // squares
U8 mg_ty, mg_tc;                // piece type: which moves, which captured

U8 mg_1mv = 0;              // internal to verify checkmates (1-exit loops on 1st found)
U8 mg_ck_list[4<<8];         // when checking checkmates

U8 mg_pI, mg_pN, *mg_pm;    // checkmate flag checking
U8 *mg_c2, *mg_p2;          // saves pointer and counter

U64 undobuffer[14<<10];       //114688 bytes,
                            // contains variables for fast undo

//-------------------------------------------------------
// uciMove variables
//      uci moves list and pointer, can be identified after MoveGen
//
U8 mg_uci_list[1024];       // uci move list
U8 *mg_uci_po;              // uci move pointer to move

//--------------------

// occupancy, only when calculating MoveGen
U64 WOCC, BOCC, OCC, NOCC, NWOCC, NBOCC, EOCC, EWOCC, EBOCC;

// to access board variables directly
U64 *PIECES[] = {
    &WQ, &WR, &WB, &WN, &WP, &WK, 0LL, 0LL,
    &BQ, &BR, &BB, &BN, &BP, &BK, 0LL, 0LL
};

CONST char pieces[] = {
    'Q','R','B','N','P','K',' ',' ',
    'q','r','b','n','p','k',' ',' '
};

// castling constants
CONST U64 castle_E1H1 = 144LL;
CONST U64 castle_E1C1 = 17LL;
CONST U64 castle_E8H8 = (9LL<<60);
CONST U64 castle_E8C8 = (17LL<<56);
CONST U64 Not_castle_WHITE = ~(145LL);
CONST U64 Not_castle_BLACK = ~(145LL<<56);
CONST U64 castle_ALL = (145LL<<56)|(145LL);

CONST U64 sqF1G1 = 96LL;
CONST U64 sqD1C1B1 = 14LL;
CONST U64 sqF8G8 = (6LL<<60);
CONST U64 sqD8C8B8 = (14LL<<56);


//================================

/* Count of 0s */
CONST U8 trailingZerosTable[] = {
    63, 0,  58, 1,  59, 47, 53, 2,
    60, 39, 48, 27, 54, 33, 42, 3,
    61, 51, 37, 40, 49, 18, 28, 20,
    55, 30, 34, 11, 43, 14, 22, 4,
    62, 57, 46, 52, 38, 26, 32, 41,
    50, 36, 17, 19, 29, 10, 13, 21,
    56, 45, 25, 31, 35, 16, 9,  12,
    44, 24, 15, 8,  23, 7,  6,  5
    };

#define trail0(mask) trailingZerosTable[(U8)(((mask & (-mask)) * 0x07EDD5E59A4E28C2LL) >> 58)]

/* Magic MeveGen  constants and arrays */

#define B2G7 0x007E7E7E7E7E7E00LL

// pre-generated constants, magicgen bc16

CONST U64 BishopMagics[] = {
// Magics for bishops

0x6CFFD8B9D37E06BBLL, 0x7C178C7BF7F57CAELL, 0xBD98AC81272FB5B8LL, 0xEDFAE51870C9F19FLL, 0x87F47FB1C074593ALL, 0xD7C16D85CA270CF5LL, 0x5BA782D5B7C17BB3LL, 0x3A4C9A5ACF80510DLL,
0x6D57FAB6246EF2B3LL, 0x0BD74D62FC617A7BLL, 0x9584DA4D20B959BDLL, 0x6F723DEBECA069F4LL, 0xF4ACC795CB2D5CB5LL, 0x43C4F193D04AD6A9LL, 0x612085F344704B15LL, 0xE1A9E4CF7060D21FLL,
0x596BEA7F05C0FE36LL, 0x6214025A7E63AD2FLL, 0x15912315DA5016A0LL, 0xBB3A96FA5092215FLL, 0x51C771E8BDF392C0LL, 0x7AEB8FC497D0BC32LL, 0x36FB5CF010EC579ELL, 0x34EB7C64AE28C243LL,
0x4212BD8C83A6E4C0LL, 0xE0412592D1C1C3FFLL, 0x7751B104DA2A81FBLL, 0xF730B59B8571381CLL, 0x634900EA035C3400LL, 0xC840D3F3B6F9B929LL, 0x39C239F3FC93CB76LL, 0x66C5D96788357B03LL,
0xD9824BEFC9415C11LL, 0xCD7DF0EFE183E6E3LL, 0xD79C76F086E33E64LL, 0x4DBA5F8501CE2400LL, 0xCBAF744A8043F100LL, 0x617DBA463E646CFFLL, 0x61B7B064856AD941LL, 0x5E682D8D0B916E5DLL,
0xFD9CACF9DA01803ELL, 0xF6779696FF3E7EE0LL, 0x6FE2E5338E0C1248LL, 0x72165A308C570B6ELL, 0xAF7756B732E846E2LL, 0x6B1A5D734E172568LL, 0x2197A3345514B9D9LL, 0x678DBC7501FB2C50LL,
0x78612467DABD1F04LL, 0x33966792F76720D1LL, 0x96C66374DDFB3360LL, 0x53860F0EE0C8CD45LL, 0x534290A02B766B2CLL, 0x5595C747518329FBLL, 0x4370788E0145B955LL, 0xFD35BBAF299013D9LL,
0x6E266965ADAF646CLL, 0xA1DEEFF0A585674FLL, 0xD262607972DD3A36LL, 0xD46BC2B72BF218A5LL, 0x562CDC3F02EC43F5LL, 0x2AA80F72248BE656LL, 0x91F23AC46A1EE715LL, 0x013FB443B15EB41ALL
};

CONST U64 RookMagics[] = {
// Magics for rooks
0xFF260527D2CAC700LL, 0x4CF0F24E7D5A5800LL, 0x829576FC130A3080LL, 0x5CFB475FC91E1100LL, 0x1C6E08D3004F2600LL, 0xF1B1998208AC1078LL, 0x06FCBDB2CB1BC4A8LL, 0x938C7FC1E07D6298LL,
0x4E2D37B8F9E42E39LL, 0x45719CD9ACBBEB00LL, 0x29FBB48EFBF98AC0LL, 0xBBE3DBB8542C8C80LL, 0x1B0CBBC889E0A010LL, 0x784844442DBFABB8LL, 0x08320F627EE206D0LL, 0xE651EECBDD946AE0LL,
0xBD8AEA197A509A03LL, 0x6412AE5F67893D00LL, 0x4CD4742FDEAA6A00LL, 0x324FDAC71DBAE100LL, 0x0D6F612384FF4DB0LL, 0xC0322570866933F0LL, 0xFE527633BA19DCCCLL, 0x65DD6D0E8C0A245CLL,
0xCCD36B245D1A34BFLL, 0x301B25F9D1808F00LL, 0xCDD5B0E696401E00LL, 0xAA567462A1BFE7C0LL, 0xCB8D9FFE2B0A4540LL, 0xB002D050508A8808LL, 0x90DBCB9AA74E5DECLL, 0x52BD607AD16823CELL,
0x6789D8D2F2B06E12LL, 0x84DBDAD1C0C9E180LL, 0xECA9A368B64154C0LL, 0xCB663F3098641DE0LL, 0x900DB76EA2B3CE30LL, 0x09A70669D4010E20LL, 0xC512D6EE770DE5A0LL, 0x797423BE8627BC94LL,
0x8A3D575077F24F63LL, 0x0EC9B8F218AB9D00LL, 0x0C8587D6C0DD33C0LL, 0x60AF95CADD982560LL, 0xACC5169EB71CA590LL, 0x160D1C32B7BC6398LL, 0x88B6F3A8886C2D2ALL, 0x4C5BC7A588A0AB5CLL,
0x0A2F0CEF3B7D0C39LL, 0xEF4D5C4858599E00LL, 0x1B041BF13B674200LL, 0xFB6B678E03BAAE00LL, 0xC277382FB0280FE0LL, 0x5C8971741F006128LL, 0x8A909A7AC999AAACLL, 0x5405CD9DAE7D3E08LL,
0x9D4D6B7CC2DAD786LL, 0xCE3D0AD07A35766ALL, 0xC0EA25A977591592LL, 0xC5C9BB5E7D8EF022LL, 0xB1552E04EC9C49E9LL, 0x9C16B80885B53F9ELL, 0xDE285EF1DAC7D27ALL, 0x441830E566CDF37ALL
};

// pre-generated constants, pawnMagicGen created

CONST U64 PawnMagicsWhite[] = {
// Pawn Magics for white
0x6CFFD8BCD3813025LL, 0xD124186042012412LL, 0x47D1CB98DC98C626LL, 0x9960A3319A0370A7LL, 0xF57AA11C65FA8AECLL, 0x666F4312B4C037E1LL, 0x2A68DDD475354CE9LL, 0xA2E0E09B5C0EE01DLL,
0xE2791197D71749EFLL, 0x8041E0F89AAFD8F1LL, 0xC4E86860077C1E6BLL, 0xF21CA852246F6593LL, 0x2494C41F7D900347LL, 0x6674B60D9A900A65LL, 0xBF709C4103B32D43LL, 0x7E7C73602722894BLL,
0x753A0BE05B7409DELL, 0xFAD801465CEBC598LL, 0x75491B383EB986CELL, 0x160836D6E157ADEFLL, 0x4293770388AFED51LL, 0xFC00457C53A7A8CCLL, 0x933A416F7FA1BEFBLL, 0x3A6DD28A944A89A4LL,
0x2B32C9EDE5D36625LL, 0x598807C071807290LL, 0x51F67A2B4FCD94BELL, 0x30468DEB19182555LL, 0x52502DB0AE2A8854LL, 0xC766DFDBFA0EE2B9LL, 0x138F57656ADC76B5LL, 0x096D4FAF27875385LL,
0xF6C08BB990BB080DLL, 0xEA57DFA529A2C4FELL, 0x5159A8BE8414092BLL, 0x2F7650DCB9C84A47LL, 0xCB9D650D37ED81BDLL, 0xC98D4D7C35F95288LL, 0x9AFC4746431B8281LL, 0xAC7E6366E9AEC902LL,
0xE50204A876F2910CLL, 0xDCDA62861C5E9DF2LL, 0xB3E7DCF35BC01861LL, 0xDF5BC8E4CB5AC47DLL, 0x985E5D3573EE5DA0LL, 0x3F622C3D95F8D13FLL, 0x9CE96D4D9B47C5E4LL, 0x47C859E4A93F3B86LL,
0x8B9EE7CFD60C44A9LL, 0x48CD5724C7FD3EF1LL, 0xEBE321BE5396A4C7LL, 0x781EF3D4A0CBADCFLL, 0x74B6985859DD0E89LL, 0xAB099F864255121ALL, 0x4B32732AFF896361LL, 0x3C4755EB59E83C65LL,
0x0D08957E08521372LL, 0xFDF19B39AD6BC6B2LL, 0xF4D721133D240DA9LL, 0x8C3481A7C4E3D2BFLL, 0x06CFF88D337EFF50LL, 0x1D7AC438742547ACLL, 0xCDDBF6FB9AA9832DLL, 0x6B9E69C6883FA50BLL
};

CONST U64 PawnMagicsBlack[] = {
// Pawn Magics for black
0x3BE7FC46927CF82FLL, 0x4AAC763FA470CE98LL, 0x0FF62BEE8FBD5F76LL, 0xB2589E054CC0E43CLL, 0x0D88BB0FD4C3F231LL, 0x20F213C013DD9FEALL, 0xD0B2B77848F0A66DLL, 0xEB14C34F54F2D57ALL,
0x19D7C8BAC44A9A6FLL, 0x9E090BB094DEE86DLL, 0xBA473B4EB44158E8LL, 0xA844702091EB8067LL, 0x23EC2A26EF0BE169LL, 0x4EF88CEE76CE3FC9LL, 0x0F17C5CCB32A834DLL, 0xC4D1814E4C75D21BLL,
0x03976178DFD226F3LL, 0xB117C3E4260AEE44LL, 0xD9A5F62FFCC2D7EELL, 0x4391D38271B166B8LL, 0xE843C4FF9888ABA1LL, 0x121B595647031BBDLL, 0x2D28D420366E2A36LL, 0xC932750F8F70B654LL,
0x021A35782B70B577LL, 0x22484DC1D25713C4LL, 0x0E921CE58FA7D771LL, 0xA4454E470A24AC31LL, 0xA1A0D233233A1672LL, 0xECBB9B320FD66DD3LL, 0xFF52D3BAFC80874BLL, 0x016A9AD5C6274070LL,
0x6761EA34FECAFF77LL, 0xCB99473867611CA4LL, 0x93E2A89D1DAF3327LL, 0xB0C4D93293A295C1LL, 0xC80CF6EC257F7312LL, 0x5AFA753D0ACD0130LL, 0x96AF80A64DEF717DLL, 0xF8ACCB6A2FE5196FLL,
0xDD0A4D5B5C1D215ELL, 0xC34300BB46EB9347LL, 0x5411B3BE16DA5F92LL, 0x684585F768A7CE1BLL, 0x941465C6672E1F57LL, 0x2D7C51046B0EAA08LL, 0x648D13FAED26DA3ELL, 0xA4EC588D84466058LL,
0x49130F7B82174554LL, 0xAAA5518D3961B481LL, 0xB27780EAA746F79FLL, 0xD1D092A030E42126LL, 0x7C0C9FF0C8720D80LL, 0x38B93FAD85A2150DLL, 0x1E7F6AF95FC31136LL, 0x1CFB3E0C75045016LL,
0x05A7788C6DC71739LL, 0x0C32324921486AA6LL, 0xFDB67D5B6271DEF2LL, 0x76711792B83C3F48LL, 0xA57A37C961F4B050LL, 0x24AB64AD89927642LL, 0x8140B8DA4374A11FLL, 0x01D5067165EACDA4LL
};
// BitCount=4  >>60


// MoveGen arrays

U64 BishopMask[64];
U64 RookMask[64];

U64 BishopLegalsTable[64][1<<16];
U64 RookLegalsTable[64][1<<16];

U64 KnightLegals[64];
U64 KingLegals[64];

U64 PawnMaskWhite[64];
U64 PawnMaskBlack[64];

U64 PawnWhiteLegalsTable[64][1<<4];
U64 PawnBlackLegalsTable[64][1<<4];

U64 PawnWhiteAtck[64];      // Square attacked by opposite pawns
U64 PawnBlackAtck[64];

//
// These can be declared as functions too, instead of fast defines
//
// rays of moves combined with occupancy mask
#define getBishopMove(square,occupancy) BishopLegalsTable[(U8)square][(U16)(((occupancy&BishopMask[square])*BishopMagics[square])>>48)]
#define getRookMove(square,occupancy) RookLegalsTable[(U8)square][(U16)(((occupancy&RookMask[square])*RookMagics[square])>>48)]
#define getWhitePawnMove(square,occupancy) PawnWhiteLegalsTable[square][((occupancy&PawnMaskWhite[square])*PawnMagicsWhite[square])>>60]
#define getBlackPawnMove(square,occupancy) PawnBlackLegalsTable[square][((occupancy&PawnMaskBlack[square])*PawnMagicsBlack[square])>>60]
// is check+
#define sqAttackedByWhites() {mg_fA=(((KingLegals[mg_sqA]&WK)||(KnightLegals[mg_sqA]&WN)||(getRookMove(mg_sqA,OCC)&(WR|WQ))||(getBishopMove(mg_sqA,OCC)&(WB|WQ))||(PawnWhiteAtck[mg_sqA]&WP))?1:0);}
#define sqAttackedByBlacks() {mg_fA=(((KingLegals[mg_sqA]&BK)||(KnightLegals[mg_sqA]&BN)||(getRookMove(mg_sqA,OCC)&(BR|BQ))||(getBishopMove(mg_sqA,OCC)&(BB|BQ))||(PawnBlackAtck[mg_sqA]&BP))?1:0);}


// ---------------------- prepares arrays
//
// variables one-time, but needed

U64 Bo1,Bo2;    // boards
U8 SqI; // square to prepare
U8 b_r; // 1-bishops, 0-rooks
U8 b_w; // 1-black, 0-white (for pawns)

U8 legalck = 0; // Calculate: 0-allmoves-rays, 1-legalmoves

// -------

void gdir(int dv, int dh, U8 loop) {

        int V=(SqI>>3),H=(SqI&7);
        V+=dv; H+=dh;
        while( (V>=0 && V<8) && (H>=0 && H<8) ) {
                U8 sq = (V<<3)|H;
                U64 B = (1LL<<sq);
                if(legalck) {
            Bo2 |= B;
            if( Bo1 & B ) return;
                }
                else { Bo1|=B; }
        if(!loop) return;
                V+=dv; H+=dh;
                }
        }

void gen2dir() {

    if(b_r) {   //bishops
        gdir(-1,-1,1); gdir(+1,-1,1); gdir(-1,+1,1); gdir(+1,+1,1);
    }
    else {  // rooks
        gdir(-1,0,1); gdir(+1,0,1); gdir(0,+1,1); gdir(0,-1,1);
    }
}

U8 BoSet(U8 sq, U8 capt) {
    U8 b = 0;
    U64 B = (1LL<<sq);
    if(legalck) {
        if(Bo1 & B) b = 1;
        if(capt==b) Bo2 |= B;
        }
    else { Bo1|=B; }
    return b;
}

void gen_pawnmoves() {

    int V=(SqI>>3),H=(SqI&7);
    if(V>0 && V<7) {
        if(b_w) V--; else V++;

        U8 sq = (V<<3)|H;
        U8 f = BoSet(sq,0);
        if(H>0) BoSet(sq-1,1);
        if(H<7) BoSet(sq+1,1);
        if(!f) {
            if(b_w) { if(V==5) BoSet(sq-8,0); }
            else { if(V==2) BoSet(sq+8,0); }
            }
    }
}

void gen_pawn_atck() {

    int V=(SqI>>3),H=(SqI&7);
    if(b_w) V++; else V--;
    if(V>0 && V<7) {
        U8 sq = (V<<3)|H;
        if(H>0) BoSet(sq-1,1);
        if(H<7) BoSet(sq+1,1);
        }
}

// Scan occupancy cases
void Permutate(U8 pawncase) {

    U64 mult;
    U16 idx_16;
    U8 idx_8;

        U8 bits[64];
        U8 n=0,sq=0;
        for(;sq<64;sq++) {
                if( Bo1 & (1LL<<sq) ) bits[n++]=sq;
                }

        U16 LEN = (1<<n);
        for(U16 i=0;i<LEN;i++) {

        Bo1 = 0LL;
        for(U8 j=0;j<n;j++)     // scan as bits
                {
            if(i&(1<<j)) Bo1|=(1LL<<bits[j]);
                }

                Bo2 = 0LL;      // find legal moves for square, put in Bo2

                if(pawncase)
            {
            mult = Bo1 * (b_w ? PawnMagicsBlack[SqI] : PawnMagicsWhite[SqI]);
            idx_8 = (U8)(mult >> 60);
            gen_pawnmoves();
            if(b_w) PawnBlackLegalsTable[SqI][idx_8] = Bo2;
            else PawnWhiteLegalsTable[SqI][idx_8] = Bo2;
                }
                else
            {
            mult = Bo1 * (b_r ? BishopMagics[SqI] : RookMagics[SqI]);
            idx_16 = (U16)(mult >> 48);
            gen2dir();
            if(b_r) BishopLegalsTable[SqI][idx_16] = Bo2;
            else RookLegalsTable[SqI][idx_16] = Bo2;
        }
        }
}

void prepare_tables() {

    for(SqI=0;SqI<64;SqI++) {
        for(b_r=0; b_r<2; b_r++) {

            legalck=0;
            Bo1 = 0;
            gen2dir();
            if((1LL<<SqI)&B2G7) Bo1&=B2G7;
            if(b_r) BishopMask[SqI]= Bo1;
            else RookMask[SqI]= Bo1;
            legalck=1;
            Permutate(0);
        }

        for(b_w=0; b_w<2; b_w++) {

            legalck = 0;
            Bo1 = 0LL;
            gen_pawnmoves();
            if(b_w) PawnMaskBlack[SqI]= Bo1;
            else PawnMaskWhite[SqI]= Bo1;
            legalck = 1;
            Permutate(1);

            legalck = 0;
            Bo1 = 0LL;
            gen_pawn_atck();
            if(b_w) PawnBlackAtck[SqI]= Bo1;
            else PawnWhiteAtck[SqI]= Bo1;
        }
    }
}

void prepare_knights() {

    for(SqI=0;SqI<64;SqI++) {
        Bo1=0LL;
        legalck=0;
        gdir(-1,-2,0); gdir(+1,-2,0); gdir(-2,+1,0); gdir(+2,-1,0);
        gdir(-1,+2,0); gdir(+1,+2,0); gdir(-2,-1,0); gdir(+2,+1,0);
        KnightLegals[SqI]=Bo1;
    }
}

void prepare_kings() {

    for(SqI=0;SqI<64;SqI++) {
        Bo1=0LL;
        legalck=0;
        gdir(-1,-1,0); gdir(-1,0,0); gdir(-1,+1,0); gdir(0,-1,0);
        gdir(+1,-1,0); gdir(+1,0,0); gdir(+1,+1,0); gdir(0,+1,0);
        KingLegals[SqI]=Bo1;
    }
}

/*
    King square attacks check, see IsCheckNow, IsCheckMateNow
*/
void isCheckedKing() {

    WOCC = WK|WQ|WR|WB|WN|WP;
    BOCC = BK|BQ|BR|BB|BN|BP;
    OCC = WOCC|BOCC;
    if(ToMove) {
        mg_sqA=trail0(BK); sqAttackedByWhites();
        }
    else {
        mg_sqA=trail0(WK); sqAttackedByBlacks();
        }
}

/*
    Make a chess move on board
*/

void DoMove( U8 *mv ) {

    *(undo_p++)=WQ; *(undo_p++)=WR; *(undo_p++)=WB;
    *(undo_p++)=WN; *(undo_p++)=WP; *(undo_p++)=WK;
    *(undo_p++)=BQ; *(undo_p++)=BR; *(undo_p++)=BB;
    *(undo_p++)=BN; *(undo_p++)=BP; *(undo_p++)=BK;
    *(undo_p++)=CASTLES; *(undo_p++)=ENPSQ;

    mg_ty = *(mv++);
    mg_sq = *(mv++);
    mg_sqTo = *(mv++);
    mg_fr = (1LL<<mg_sq);
    mg_to = (1LL<<mg_sqTo);
    mg_fl = *(mv);

    if(mg_fl&1) {   // capture
        mg_tc = (mg_ty>>4)&15;
        mg_ty &= 15;
        (*PIECES[mg_tc]) &= (~mg_to);
    }
    mg_Pc = PIECES[mg_ty];
    (*mg_Pc) &= (~mg_fr);   // move from
    if(mg_fl&2) {   // promotion
        (*PIECES[ (ToMove?8:0) + ((mg_fl>>2)&3)]) |= mg_to;
        }
    else {
        (*mg_Pc) |= mg_to;  // move to
        }
    if(mg_fl&16) {   // en-passant
        if(ToMove) WP &= ~(1LL<<(mg_sqTo+8));
        else BP &= ~(1LL<<(mg_sqTo-8));
    }
    if(CASTLES) {
        CASTLES &= ~(mg_fr|mg_to);
    }

    if(mg_fl&32) {   // castling
     if(ToMove) {
        if(mg_sqTo>mg_sq) { BK=(1LL<<62); BR^=(1LL<<63); BR|=(1LL<<61); }
        else { BK=(1LL<<58); BR^=(1LL<<56); BR|=(1LL<<59); }
        CASTLES &= Not_castle_BLACK;
        }
     else {
        if(mg_sqTo>mg_sq) { WK=64LL; WR^=128LL; WR|=32LL; }
        else { WK=4LL; WR^=1LL; WR|=8LL; }
        CASTLES &= Not_castle_WHITE;
        }
    }
                    // if pawn, set en-passant square
    ENPSQ = 0LL;
    if((mg_ty&7)==4) {
        if(ToMove) {
            if(mg_sq>47 && mg_sqTo<40) ENPSQ = (1LL<<(mg_sq-8));
            }
        else {
            if(mg_sq<16 && mg_sqTo>23) ENPSQ = (1LL<<(mg_sq+8));
            }
        }
    ToMove^=1;
}

/*
    UnMake a chess move (fast)
*/

void UnDoMove() {

    ToMove^=1;
    ENPSQ = *(--undo_p); CASTLES = *(--undo_p);
    BK = *(--undo_p); BP = *(--undo_p); BN = *(--undo_p);
    BB = *(--undo_p); BR = *(--undo_p); BQ = *(--undo_p);
    WK = *(--undo_p); WP = *(--undo_p); WN = *(--undo_p);
    WB = *(--undo_p); WR = *(--undo_p); WQ = *(--undo_p);
}

/*
    Sets Check+ flags
*/

void getFlags() {

    if(mg_1mv) return;

    if(ToMove) {
        mg_sqA = trail0(WK);
        sqAttackedByBlacks();       // is opposite king checked+?
        if(mg_fA) {
            *(mg_po)|=64;
        }
    }
    else {
        mg_sqA = trail0(BK);
        sqAttackedByWhites();       // is opposite king checked+?
        if(mg_fA) {
            *(mg_po)|=64;
        }
    }
}

/*
    Verifies checkmate cases, if check+ flag is set.
*/

void getCheckMateFlags( U8 *mvlist ) {

    mg_pm = mvlist;
    mg_pN = *(mg_pm++);

    for(mg_pI=0; mg_pI<mg_pN; mg_pI++) {

     if( (*(mg_pm+3)) & 64 ) {  // if check+

        DoMove(mg_pm);
        mg_1mv = 1;
        mg_p2 = mg_po;
        mg_c2 = mg_cnt;

        MoveGen(mg_ck_list);

        if(!( *mg_cnt)) {
            *(mg_pm+3) |= 128;      // add checkmate flag, if could not escape check
        }
        mg_1mv = 0;
        mg_po = mg_p2;
        mg_cnt = mg_c2;
        UnDoMove();

        }
        mg_pm+=4;
    }
}

/*
    Tries to move the piece to all mo squares,
    verifies if king left under attack,
    saves in the list, if ok.
*/

void addMove( U64 mo ) {

    mg_fr = (1LL<<mg_sq);
    mg_Pc = PIECES[ mg_ty ];
    mg_sPc = *mg_Pc;   // save
    mg_sOcc = OCC;

    while(mo) {

        mg_nf = (~mg_fr);
        *mg_Pc &= mg_nf;           // moved from
        OCC &= mg_nf;
        mg_sqTo = trail0(mo);
        mg_to = (1LL<<mg_sqTo);
        mg_nt = (~mg_to);
        mg_fC = mg_fP = mg_fE = mg_tc = 0;

            // try move piece, verify attacks to our king
        *mg_Pc |= mg_to;
        OCC |= mg_to;

        if(ToMove) {               // black
            mg_sqA = trail0(BK);
            if(EWOCC & mg_to) {
                mg_fC = 1;
                mg_sQ=WQ; mg_sR=WR; mg_sB=WB; mg_sN=WN; mg_sP=WP;   // save
                WQ &= mg_nt; WR &= mg_nt; WB &= mg_nt; WN &= mg_nt; WP &= mg_nt;
                if((mg_to==ENPSQ) && (mg_ty==12)) {
                    WP &= ~(ENPSQ<<8);
                    mg_fE = 16;
                    }
                mg_tc=( WR!=mg_sR?1: (WB!=mg_sB?2 :(WN!=mg_sN?3 :(WP!=mg_sP?4: 0))));
                }
            sqAttackedByWhites();       // is black king ok?
            if(!mg_fA) {
                if((mg_ty==12) && (mg_sqTo<8))  {   // promoted pawn
                    mg_fP=2;
                    BP &= mg_nt;
                    mg_ss=BQ;
                    BQ |= mg_to;      // put a Queen
                }
                *(mg_po++)=mg_ty|(mg_tc<<4);
                *(mg_po++)=mg_sq;
                *(mg_po++)=mg_sqTo;
                *(mg_po)=(mg_fC|mg_fP|mg_fE);
                (*mg_cnt)++;
                #ifdef mg_CHECK_FLAG
                    if(!mg_1mv) getFlags();
                #endif
                mg_po++;

                if(mg_fP) {
                    BQ=mg_ss;

                    mg_ss=BR;
                    BR|=mg_to;      // put a Rook
                    *(mg_po++)=mg_ty|(mg_tc<<4);
                    *(mg_po++)=mg_sq;
                    *(mg_po++)=mg_sqTo;
                    *(mg_po)=(mg_fC|(mg_fP|4)|mg_fE);
                    (*mg_cnt)++;
                #ifdef mg_CHECK_FLAG
                    if(!mg_1mv) getFlags();
                #endif
                    mg_po++;
                    BR=mg_ss;

                    mg_ss=BB;
                    BB|=mg_to;      // put a Bishop
                    *(mg_po++)=mg_ty|(mg_tc<<4);
                    *(mg_po++)=mg_sq;
                    *(mg_po++)=mg_sqTo;
                    *(mg_po)=(mg_fC|(mg_fP|8)|mg_fE);
                    (*mg_cnt)++;
                #ifdef mg_CHECK_FLAG
                    if(!mg_1mv) getFlags();
                #endif
                    mg_po++;
                    BB=mg_ss;

                    mg_ss=BN;
                    BN|=mg_to;      // put a Knight
                    *(mg_po++)=mg_ty|(mg_tc<<4);
                    *(mg_po++)=mg_sq;
                    *(mg_po++)=mg_sqTo;
                    *(mg_po)=(mg_fC|(mg_fP|12)|mg_fE);
                    (*mg_cnt)++;
                #ifdef mg_CHECK_FLAG
                    if(!mg_1mv) getFlags();
                #endif
                    mg_po++;
                    BN=mg_ss;
                }

                }
            if(mg_fC) {
                WQ=mg_sQ; WR=mg_sR; WB=mg_sB; WN=mg_sN; WP=mg_sP;   // restore
                }

            }
        else {                      // white
            mg_sqA = trail0(WK);
            if(EBOCC & mg_to) {
                mg_fC = 1;
                mg_sQ=BQ; mg_sR=BR; mg_sB=BB; mg_sN=BN; mg_sP=BP;   // save
                BQ &= mg_nt; BR &= mg_nt; BB &= mg_nt; BN &= mg_nt; BP &= mg_nt;
                if((mg_to==ENPSQ) && (mg_ty==4)) {
                    BP &= ~(ENPSQ>>8);
                    mg_fE = 16;
                    }
                mg_tc=( BR!=mg_sR?9: (BB!=mg_sB?10 :(BN!=mg_sN?11 :(BP!=mg_sP?12: 8))));
                }
            sqAttackedByBlacks();       // is white king ok?
            if(!mg_fA) {
                if((mg_ty==4) && (mg_sqTo>55))  {   // promoted pawn
                    mg_fP=2;
                    WP &= mg_nt;
                    mg_ss=WQ;
                    WQ |= mg_to;      // put a Queen
                }
                *(mg_po++)=mg_ty|(mg_tc<<4);
                *(mg_po++)=mg_sq;
                *(mg_po++)=mg_sqTo;
                *(mg_po)=(mg_fC|mg_fP|mg_fE);
                (*mg_cnt)++;

                #ifdef mg_CHECK_FLAG
                    if(!mg_1mv) getFlags();
                #endif
                mg_po++;

                if(mg_fP) {
                    WQ=mg_ss;

                    mg_ss=WR;
                    WR|=mg_to;      // put a Rook
                    *(mg_po++)=mg_ty|(mg_tc<<4);
                    *(mg_po++)=mg_sq;
                    *(mg_po++)=mg_sqTo;
                    *(mg_po)=(mg_fC|(mg_fP|4)|mg_fE);
                    (*mg_cnt)++;
                #ifdef mg_CHECK_FLAG
                    if(!mg_1mv) getFlags();
                #endif
                    mg_po++;
                    WR=mg_ss;

                    mg_ss=WB;
                    WB|=mg_to;      // put a Bishop
                    *(mg_po++)=mg_ty|(mg_tc<<4);
                    *(mg_po++)=mg_sq;
                    *(mg_po++)=mg_sqTo;
                    *(mg_po)=(mg_fC|(mg_fP|8)|mg_fE);
                    (*mg_cnt)++;
                #ifdef mg_CHECK_FLAG
                    if(!mg_1mv) getFlags();
                #endif
                    mg_po++;
                    WB=mg_ss;

                    mg_ss=WN;
                    WN|=mg_to;      // put a Knight
                    *(mg_po++)=mg_ty|(mg_tc<<4);
                    *(mg_po++)=mg_sq;
                    *(mg_po++)=mg_sqTo;
                    *(mg_po)=(mg_fC|(mg_fP|12)|mg_fE);
                    (*mg_cnt)++;
                #ifdef mg_CHECK_FLAG
                    if(!mg_1mv) getFlags();
                #endif
                    mg_po++;
                    WN=mg_ss;
                }

                }
            if(mg_fC) {
                BQ=mg_sQ; BR=mg_sR; BB=mg_sB; BN=mg_sN; BP=mg_sP;   // restore
                }
            }

        *mg_Pc=mg_sPc;     // restore
        OCC = mg_sOcc;

        mo &= mo-1;
        }

}

/*
    Castlings case
*/
void addCastleMove() {

    if(ToMove) {
        mg_sr=BR;
        mg_sk=BK;
        if(mg_sqA>mg_sq) { BK=(1LL<<62); BR^=(1LL<<63); BR|=(1LL<<61); }
        else { BK=(1LL<<58); BR^=(1LL<<56); BR|=(1LL<<59); }
        }
    else {
        mg_sr=WR;
        mg_sk=WK;
        if(mg_sqA>mg_sq) { WK=64LL; WR^=128LL; WR|=32LL; }
        else { WK=4LL; WR^=1LL; WR|=8LL; }
        }
    *(mg_po++) = mg_ty;
    *(mg_po++) = mg_sq;
    *(mg_po++)  =mg_sqA;
    *(mg_po) = 32;
                #ifdef mg_CHECK_FLAG
                    if(!mg_1mv) getFlags();
                #endif
    (*mg_cnt)++;
    mg_po++;
    if(ToMove) {
        BK=mg_sk;
        BR=mg_sr;
        }
    else {
        WK=mg_sk;
        WR=mg_sr;
        }
}

/*
    Cenerates the list of legal chess moves on current board
*/

void MoveGen( U8 *listStoreTo ) {

    WOCC = WK|WQ|WR|WB|WN|WP;
    BOCC = BK|BQ|BR|BB|BN|BP;
    OCC = WOCC|BOCC;
    NOCC = ~OCC;
    NWOCC = ~WOCC;
    NBOCC = ~BOCC;
    EOCC = OCC|ENPSQ;
    EWOCC = WOCC|ENPSQ;
    EBOCC = BOCC|ENPSQ;

    mg_cnt = mg_po = listStoreTo;
    mg_po++;
    *(mg_cnt)=0;

    if(ToMove){   // Black to move

        // King moves
        mg_P = BK; mg_ty = 13;
        mg_sq = trail0(mg_P);
        addMove(KingLegals[mg_sq] & NBOCC);
        if(mg_1mv&&(*mg_cnt)) return;

        if(CASTLES && (!mg_1mv)) {
            if(((NOCC & sqF8G8)== sqF8G8) &&
            ((CASTLES & castle_E8H8)==castle_E8H8)) {
                mg_sqA = mg_sq;
                sqAttackedByWhites();   // is E8 under check+?
                if(!mg_fA) {
                    mg_sqA++;
                    sqAttackedByWhites();   // is F8 check+?
                    if(!mg_fA) {
                        mg_sqA++;
                        sqAttackedByWhites();   // is G8 check+?
                        if(!mg_fA) {
                            addCastleMove();    // add castling
                        }
                    }
                }
            }
            if(((NOCC & sqD8C8B8)== sqD8C8B8) &&
            ((CASTLES & castle_E8C8)==castle_E8C8)) {
                mg_sqA = mg_sq;
                sqAttackedByWhites();   // is E8 under check+?
                if(!mg_fA) {
                    mg_sqA--;
                    sqAttackedByWhites();   // is D8 check+?
                    if(!mg_fA) {
                        mg_sqA--;
                        sqAttackedByWhites();   // is C8 check+?
                        if(!mg_fA) {
                            addCastleMove();    // add castling
                        }
                    }
                }
            }
        }

        mg_P = BR; mg_ty = 9;
        while(mg_P) {
            // Rook moves
            mg_sq = trail0(mg_P);
            addMove(getRookMove(mg_sq,OCC) & NBOCC);
            mg_P &= mg_P-1;
            }

        mg_P = BB; mg_ty = 10;
        while(mg_P) {
            // Bishop moves
            mg_sq = trail0(mg_P);
            addMove(getBishopMove(mg_sq,OCC) & NBOCC);
            mg_P &= mg_P-1;
            }

        mg_P = BQ; mg_ty = 8;
        while(mg_P) {
            // Queen moves
            mg_sq = trail0(mg_P);
            addMove(getRookMove(mg_sq,OCC) & NBOCC);
            addMove(getBishopMove(mg_sq,OCC) & NBOCC);
            mg_P &= mg_P-1;
            }
        if(mg_1mv&&(*mg_cnt)) return;

        mg_P = BN; mg_ty = 11;
        while(mg_P) {
            // Knight moves
            mg_sq = trail0(mg_P);
            addMove(KnightLegals[mg_sq] & NBOCC);
            mg_P &= mg_P-1;
            }

        mg_P = BP; mg_ty = 12;
        while(mg_P) {
            // Pawn moves
            mg_sq = trail0(mg_P);
            addMove(getBlackPawnMove(mg_sq,EOCC) & NBOCC);
            mg_P &= mg_P-1;
            }

    } else {        // White to move

        // King moves
        mg_P = WK; mg_ty = 5;
        mg_sq = trail0(mg_P);
        addMove(KingLegals[mg_sq] & NWOCC);
        if(mg_1mv&&(*mg_cnt)) return;

        if(CASTLES && (!mg_1mv)) {
            if(((NOCC & sqF1G1)== sqF1G1) &&
            ((CASTLES & castle_E1H1)==castle_E1H1)) {
                mg_sqA = mg_sq;
                sqAttackedByBlacks();   // is E1 under check+?
                if(!mg_fA) {
                    mg_sqA++;
                    sqAttackedByBlacks();   // is F1 check+?
                    if(!mg_fA) {
                        mg_sqA++;
                        sqAttackedByBlacks();   // is G1 check+?
                        if(!mg_fA) {
                            addCastleMove();    // add castling
                        }
                    }
                }
            }
            if(((NOCC & sqD1C1B1)== sqD1C1B1) &&
            ((CASTLES & castle_E1C1)==castle_E1C1)) {
                mg_sqA = mg_sq;
                sqAttackedByBlacks();   // is E1 under check+?
                if(!mg_fA) {
                    mg_sqA--;
                    sqAttackedByBlacks();   // is D1 check+?
                    if(!mg_fA) {
                        mg_sqA--;
                        sqAttackedByBlacks();   // is C1 check+?
                        if(!mg_fA) {
                            addCastleMove();    // add castling
                        }
                    }
                }
            }
        }

        mg_P = WR; mg_ty = 1;
        while(mg_P) {
            // Rook moves
            mg_sq = trail0(mg_P);
            addMove(getRookMove(mg_sq,OCC) & NWOCC);
            mg_P &= mg_P-1;
            }

        mg_P = WB; mg_ty = 2;
        while(mg_P) {
            // Bishop moves
            mg_sq = trail0(mg_P);
            addMove(getBishopMove(mg_sq,OCC) & NWOCC);
            mg_P &= mg_P-1;
            }

        mg_P = WQ; mg_ty = 0;
        while(mg_P) {
            // Queen moves
            mg_sq = trail0(mg_P);
            addMove(getRookMove(mg_sq,OCC) & NWOCC);
            addMove(getBishopMove(mg_sq,OCC) & NWOCC);
            mg_P &= mg_P-1;
            }
        if(mg_1mv&&(*mg_cnt)) return;

        mg_P = WN; mg_ty = 3;
        while(mg_P) {
            // Knight moves
            mg_sq = trail0(mg_P);
            addMove(KnightLegals[mg_sq] & NWOCC);
            mg_P &= mg_P-1;
            }

        mg_P = WP; mg_ty = 4;
        while(mg_P) {
            // Pawn moves
            mg_sq = trail0(mg_P);
            addMove(getWhitePawnMove(mg_sq,EOCC) & NWOCC);
            mg_P &= mg_P-1;
            }
    }

#ifdef mg_CHECK_FLAG
    #ifdef mg_CKMATE_FLAG
        if(!(mg_1mv)) getCheckMateFlags( listStoreTo );
    #endif
#endif

}

/*
    Sets the initial chess position
*/
void SetStartPos() {
    WK=16LL; WQ=8LL; WR=129LL; WB=36LL; WN=66LL; WP=65280LL;
    BK=(1LL<<60); BQ=(1LL<<59);
    BR=(1LL<<56)|(1LL<<63);
    BB=(1LL<<58)|(1LL<<61);
    BN=(1LL<<57)|(1LL<<62);
    BP=(255LL<<48);
    ENPSQ = 0LL;
    CASTLES = castle_ALL;
    ToMove = 0;
    undo_p = undobuffer;
    mg_po = mg_cnt = mg_uci_list;
}

/*
    Board to String
*/
void sBoard( char *buffer ) {

    U64 P;
    int V,H;
    U8 i,sq;
    char B[64], *s=buffer, c;

    for(i=0;i<64;i++) B[i]='.';
    for(i=0;i<14;i++) {
        if(i==6) i=8;
        c = pieces[i];
        P = *(PIECES[i]);
        while(P) {
            sq = trail0(P);
            if(B[sq]=='.') {
                B[sq]= c;
                }
            else {
                B[sq]='?';      // wtf cases
                }
            P&=P-1;
            }
        }
    for(V=7; V>=0; V--) {
        for(H=0; H<8; H++) *(s++)= B[(V<<3)|H];
        *(s++)=10;
    }
    *(s++)=(ToMove ? 'b':'w');
    *(s++)=(IsCheckMateNow() ? '#' : (IsCheckNow() ? '+' : ' '));
    *(s++)=10;
    *s=0;

}

/*
    Legal moves to String
*/
void sLegalMoves( char *buffer, U8 *mvlist ) {
    U8 *p = mvlist;
    U8 i=0, n = *(p++);
    U8 ty, fr, to, flags;
    char P, *s=buffer;

    while((i++)<n) {
        ty = (*(p++))&7;
        fr = *(p++); to = *(p++);
        flags = *(p++);
        if(flags&32) { *(s++)='O'; *(s++)='-';      // castlings
                if(fr>to) {*(s++)='O'; *(s++)='-';}
                *(s++)='O'; }
        else
        {

        P = pieces[ty];
        if(P!='P' && P!='p') *(s++)=P;
        *(s++)='a'+(fr&7);
        *(s++)='1'+(fr>>3);
        *(s++)=((flags&1)?'x':'-');
        *(s++)='a'+(to&7);
        *(s++)='1'+(to>>3);
                // add "ep" for en-passant capture
        if(flags&16) { *(s++)='e'; *(s++)='p'; }
                // display promoted piece =q,=r,=b,=n
        if(flags&2) { *(s++)='='; *(s++)=pieces[(flags>>2)&3]; }

        }

        if(flags&128) *(s++)='#';       // checkmate
        else if (flags&64) *(s++)='+';  // check+

        *(s++)=' ';
    }
    *s=0;
}

/*
    Put pieces directly on board - testing capabilities
        pieceAt = "pa2", "Ne3", "ng8", or clear " a3"
*/
void PutPiece( char *pieceAt ) {
    char pc = pieceAt[0], sH = pieceAt[1], sV = pieceAt[2];
    U8 sq = ((sV-'1')<<3)|((sH-'a')&7);
    U64 b = (1LL<<sq);
    for(U8 i=0; i<14; i++) {
        if(i==6) i=8;
        if(pc==' ') (*PIECES[i])&=(~b);
        else if(pieces[i]==pc) (*PIECES[i])|=b;
        }
}

/*
    Set position by uci FEN, but it is slow style
*/
void SetByFEN( char *pos ) {

    int i, j, y=7, x=0;
    U8 sq;
        char c;

    WK=WQ=WR=WB=WN=WP=0LL;
    BK=BQ=BR=BB=BN=BP=0LL;

        for(i=0;;i++)
        {
        c = pos[i];
        if(c==' ') break;
        if(c==0) return;
        if(c!='/')
                {
                if(c>'0'&&c<'9') x+=(c-'1');
                else if((c>'a'&&c<'z')||(c>'A'&&c<'Z')) {
                    sq = ((y<<3)+x);
            for(j=0;j<14;j++) {
                if(j==6) j=8;
                if(pieces[j]==c) { *(PIECES[j])|=(1LL<<sq); }
                }
            }
                if(x>6) { x=0; y--; } else x++;
                }
        }
        c = pos[++i];
        ToMove = (c=='w'? 0: 1);
        CASTLES = 0LL;
        for(i+=2;;i++)
        {
        c = pos[i];
        if(c==' ') break;
        if(c==0) return;
        if(c=='K') CASTLES|=castle_E1H1;
        if(c=='Q') CASTLES|=castle_E1C1;
        if(c=='k') CASTLES|=castle_E8H8;
        if(c=='q') CASTLES|=castle_E8C8;
        }
        c = pos[++i];
        ENPSQ = 0LL;
        if((c!=0) && (c!='-')) {
        sq = (((pos[i+1]-'1')<<3) | ((pos[i]-'a')&7));
        ENPSQ = (1LL<<sq);
        }

    undo_p = undobuffer;
    mg_po = mg_cnt = mg_uci_list;

}

/*
    Gets current uci FEN of position
*/

void sGetFEN( char *buffer ) {
        int i=0, a, j, y, x;
        char c, *s=buffer;
        U8 sq;
    for(y=7; y>=0; y-- )
                {
                a=0;
                for(x=0; x<8; x++ )
            {
                    sq = ((y<<3)+x);
                    c=0;
            for(j=0;j<14;j++) {
                if(j==6) j=8;
                if( *(PIECES[j]) & (1LL<<sq) ) {
                    c=pieces[j];
                    }
                }
            if(c==0) a++;
            else { if(a>0) s[i++]=('0'+a); a=0; s[i++]=c; }
                }
                if(a>0) s[i++]=('0'+a);
                if(y>0) s[i++]='/';
                }
    s[i++]=' ';
        s[i++]=(ToMove?'b':'w');
        s[i++]=' ';
    if(!CASTLES) s[i++]='-';
    else
                {
        if((CASTLES&castle_E1H1)==castle_E1H1) s[i++]='K';
        if((CASTLES&castle_E1C1)==castle_E1C1) s[i++]='Q';
        if((CASTLES&castle_E8H8)==castle_E8H8) s[i++]='k';
        if((CASTLES&castle_E8C8)==castle_E8C8) s[i++]='q';
                }
    s[i++]=' ';
        if(!ENPSQ) s[i++]='-';
        else
                {
                sq = trail0(ENPSQ);
                s[i++] = ('a'+(sq&7));
                s[i++] = ('1'+(sq>>3));
                }
        s[i++]=' ';
        s[i++]='0';
        s[i++]=' ';
        s[i++]='1';
        s[i]=0;
}



/*
    Is check+?
*/
U8 IsCheckNow() {
    isCheckedKing();
    return mg_fA;
}

/*
    Is checkmate#?
*/
U8 IsCheckMateNow() {

    isCheckedKing();
    if(mg_fA) {
        mg_1mv = 1;
        mg_p2 = mg_po;
        mg_c2 = mg_cnt;
        MoveGen(mg_ck_list);
        mg_fA = ((*mg_cnt) ? 0 : 1);
        mg_1mv = 0;
        mg_po = mg_p2;
        mg_cnt = mg_c2;
    }
    return mg_fA;
}

/*
    mstr is "e2e4", "b7a8q", "e1g1",...

    Returns 1 if made or 0 if error
*/

U8 uciMove( char *mstr ) {

    U8 sq1 = (((mstr[1]-'1')<<3)|((mstr[0]-'a')&7));
    U8 sq2 = (((mstr[3]-'1')<<3)|((mstr[2]-'a')&7));
    U8 pr = mstr[4];
    U8 *p = mg_uci_list;

    mg_p2 = mg_po;
    mg_c2 = mg_cnt;
    MoveGen(p++);
    mg_pN = *(mg_cnt);
    mg_po = mg_p2;
    mg_cnt = mg_c2;

    for(mg_pI=0; mg_pI<mg_pN; mg_pI++) {
        mg_sq = *(p+1);
        mg_sqTo = *(p+2);
        mg_fl = *(p+3);

        if( mg_sq==sq1 && mg_sqTo==sq2 &&
                ((pr==0)||(pr==pieces[8+((mg_fl>>2)&3)])) ) {
            DoMove(p);
            mg_uci_po = p;
            return 1;
            }
        p += 4;
        }
    return 0;
}

/*
// testing bits if ok
U8 assertBoardOk() {
    char bstr[200], *s=bstr;
    sBoard(s);
    while(*s!=0) {
        if(*(s++)=='?') {
            printf("ASSERT ERROR!\n%s\n\n",bstr);
            return 0;
        }
    }
    return 1;
}

*/

/*
//-------------------- Usage samples

//
//    Sample 1    Fast MoveGen with checkmate flag sample
//

void Sample1() {        // fool's mate

    char buf[1024];
    U8 mv[4<<8];

    uciMove("f2f3");
    uciMove("e7e5");
    uciMove("g2g4");

    sBoard(buf);
    printf("%s\n",buf);
    MoveGen(mv);
    sLegalMoves( buf,mv);
    printf("%s\n",buf);
    sGetFEN(buf);
    printf("%s\n\n",buf);
}


//
//    Sample 2    The Puzzle.
//

U8 samp2_seek_2w() {
    U8 mlist[4<<8];
    MoveGen(mlist);
    for(U8 i=0; i<mlist[0]; i++) {
        if( mlist[1+(i<<2)+3]&128 ) {
            return 1;       // if checkmate flag,
        }
    }
    return 0;   // nope, no checkmate at all
}

U8 samp2_seek_1b() {
    U8 mlist[4<<8];
    U8 can=0;           // can escape or not?
    MoveGen(mlist);
    if(!mlist[0]) return 0; // stalemate?
    for(U8 i=0; i<mlist[0]; i++) {
        DoMove( mlist+(1+(i<<2)) ) ;
          // can I avoid checkmate?
        can = (samp2_seek_2w() ? 0 : 1);
        UnDoMove();
        if(can) return 0;   // no checkmate here
    }
    return 1;   // checkmate here
}

void samp2_seek_1w() {
    U8 mlist[4<<8];
    U8 *p;
    MoveGen(mlist);
    for(U8 i=0; i<mlist[0]; i++) {
        p = mlist+(1+(i<<2));
        DoMove(p);
                // if black can't escape checkmate and here it is
        if(samp2_seek_1b()) {
            U8 sq1 = *(p+1);
            U8 sq2 = *(p+2);
            printf("Go %c%c%c%c to checkmate in 2 moves\n",
                   ((sq1&7)+'a'), ((sq1>>3)+'1'),
                   ((sq2&7)+'a'), ((sq2>>3)+'1'));
        }
        UnDoMove();
    }
}

void Sample2() {        // Set position and simple search

// Solve 2 move checkmate puzzle.
// This puzzle was created by Frank Healey,
// and published in 5 Family Herald on 7/17/1858.

// 7R/1B1N4/8/3r4/1K2k3/8/5Q2/8 w
// 1.Rd8 Kd3 2.Nc5#

    char buf[1024];
    SetByFEN("7R/1B1N4/8/3r4/1K2k3/8/5Q2/8 w");
    sBoard(buf);
    printf("%s\n",buf);
    samp2_seek_1w();
}

*/

/*
    OneTime Init
    Call in main.
*/
void Init_u64_chess() {
    prepare_tables();
    prepare_knights();
    prepare_kings();
    SetStartPos();

    //Sample1();
    //Sample2();
}

#endif  /* _INC_u64_CHESS */
