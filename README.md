
## Chelpy python chess library for Google Colab
v.1.2 nov.2024, Chessforeva

Written in pure C. Magic bitboards calculations. Performance shoud be really good, but not scalable as GPU cuda tensors or likewise. Just a single thread CPU only.

Notebook and usage samples:
https://colab.research.google.com/drive/1y8cYkpbHDymLMZ2K9zJNyRhw3mVWEvTj?usp=sharing

Source to clone
https://github.com/Chessforeva/colabpy

In case if there is no local linux VM.
Upload chelpy.c, u64_chess.h, u64_polyglot.h to Google Shell linux and compile .so library for python.

Obtain include path in python, look for 'include:' property.

    import sysconfig
    print( sysconfig.get_paths() )

Compile library.

    gcc chelpy.c -shared -o chelpy.so -I/usr/include/python3.12 -fPIC 


May work also on Anaconda Jupyters or AI Lightnings.

#### Load library in Colab

    # get library from online site
    ! wget 'https://chessforeva.gitlab.io/colab/chelpy.so'
    
    import chelpy


#### Library method functions



    #set starting position
    chelpy.setstartpos()
    
    #get chess board in a string variable
    s = chelpy.sboard()
    print(s)
    
    #get current FEN
    fen = chelpy.getfen()
    print(fen)
    
    #set position from FEN
    chelpy.setfen("7R/1B1N4/8/3r4/1K2k3/8/5Q2/8 w");
    print(chelpy.sboard())
    
    #perform move generation
    n = chelpy.movegen()
    print(n)
    #get string of legal moves after movegen
    s = chelpy.legalmoves()
    print(s)
    
    #basic things, no comments
    chelpy.setstartpos()
    chelpy.ucimove("f2f4")
    chelpy.ucimove("e7e5")
    chelpy.ucimove("g2g4")
    chelpy.movegen()
    print(chelpy.legalmoves())
    chelpy.parsepgn("2...Qh4")
    print(chelpy.sboard())
    
    print("Is check+ now?")
    print(chelpy.ischeck())
    print("Is checkmate now?")
    print(chelpy.ischeckmate())
    
    chelpy.undomove()
    
    # working with a pgn-notated chess game
    chelpy.setstartpos()
    ucis_made = chelpy.parsepgn("1.Nf3 Ng8-f6 2.g2g3 e5 3.Bg2 Nc6 4. 0-0 ")
    print(ucis_made)
    print(chelpy.sboard())
    
    # slow square check e1
    chelpy.setstartpos()
    s = chelpy.sboard().split('\n')[7][4]
    print(s)
    # to move
    s = chelpy.sboard()[72]
    print(s)

    # see chelpy polyglot key
    # unique for chess openings, opensource files .bin format, also Stockfish compatible  
    chelpy.setfen("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3")
    print(hex(chelpy.polyglotkey()))

    chelpy.setstartpos()
    ucis_made = chelpy.parsepgn("1.Nf3 Nc6 2.Rg1 Rb8 3.a2-a4 ")
    # board into variables (fast)
    print( chelpy.getboard() )
    # board castlings possible now (fast)
    print( chelpy.getcastlings() )

    # get tuple of occupied squares, after movegen()
    # parameter:
    #  0 - OCC (all pieces)
    #  1 - WOCC (whites)
    #  2 - WOCC (blacks)
    #  3 - NOCC (free squares = not occupied)
    #  4 - NWOCC (not whites occupied)
    #  5 - NBOCC (not blacks occupied)
    #  6 - EOCC (OCC with en-passant)
    #  7 - EWOCC (Whites with en-passant)
    #  8 - EBOCC (Blacks with en-passant)
    
    import torch

    chelpy.setstartpos()
    chelpy.movegen()
    Tuple = chelpy.getoccupancies(7)
    Tensor = torch.tensor(Tuple)
    print(Tensor)

    #Look white pawns
    Bo = chelpy.getboard()
    Tuple = Bo['wp']
    Tensor = torch.tensor(Tuple)
    print(Tensor)
    


#### Iterations (slow)

    
    # Functions i_movegen, i_domove, undomove are iterable in depth.
    # ucimove, movegen, legalmoves are slower. Do not use them in for-loops.
    # Anyway, write it in C if really need performance.
    
    print("Solve checkmate in 2 moves")
    
    chelpy.setfen("7R/1B1N4/8/3r4/1K2k3/8/5Q2/8 w")
    print(chelpy.sboard())
    
    # depth 0 white 1. move
    for i0 in range(chelpy.i_movegen(0)):
      chelpy.i_domove(0)
    
      can_escape = True;
    
      #depth 1 black 1... move
      for i1 in range(chelpy.i_movegen(1)):
        chelpy.i_domove(1)
    
        # depth 2 white 2. move
        yee = False
        #if no moves then stalemate
        for i2 in range(chelpy.i_movegen(2)):
          chelpy.i_domove(2)
    
          # or write a C function
          chelpy.freaknow()
    
          if chelpy.ischeckmate():
            # i0,i1,i2 represent moves
            yee = True
    
    
          chelpy.undomove()
          if(yee):
            break
    
        chelpy.undomove()
    
        can_escape = not yee
        if can_escape:
          break
    
      chelpy.undomove()
    
      if(not can_escape):
        chelpy.movegen()
        mv = chelpy.legalmoves().split(" ")
        print(mv[i0]);
        break

    # selective iterations
    # scan 1st.move knight moves only and all black knight moves too
    # this is too slow in python, especially dictionaries with string keys
    
    chelpy.setstartpos()
    # depth 0 white 1. move
    for i0 in range(chelpy.i_movegen(0)):
    
      # obtain move information at depth 0
      # returned values:
      # piece types =  Q=0,R=1,B=2,N=3,P=4,K=5, q=8,r=9,b=10,n=11,p=12,k=13
      # promoted pieces = 0=Q,1=R,2=B,3=N
      # other are various flags of movegen
    
      m0 = chelpy.i_moveinfo(0)
      if(m0['pieceTypeFrom'] == 3):
        print(m0)
        chelpy.i_domove(0)
        #depth 1 black 1... move
        for i1 in range(chelpy.i_movegen(1)):
          m1 = chelpy.i_moveinfo(1)
          if(m1['pieceTypeFrom'] == 11):
            print(m1)
            chelpy.i_domove(1)
            # do something
            chelpy.undomove()
          else:
            # other pawns moves, skip to next move
            chelpy.i_skipmove(1)
        
        chelpy.undomove()
        
      else:
        # other pawns moves
        chelpy.i_skipmove(0)

    # selective iterations
    # search check, too slow
    
    chelpy.setstartpos()
    chelpy.parsepgn("1.g4 e5 2.f3")
    # depth 0 white 1. move
    for i0 in range(chelpy.i_movegen(0)):
      m0 = chelpy.i_moveinfo(0)
      if(m0['check'] == 1):
        print(m0)
        chelpy.i_domove(0)
        print(chelpy.sboard())
        chelpy.undomove()
    
      else:
        chelpy.i_skipmove(0)
    

    #see performance
    #scans e2-e4 move k times
    
    k = 0
    for i in range(100*1000):
      chelpy.setstartpos()
      for i0 in range(chelpy.i_movegen(0)):
        m0 = chelpy.i_moveinfo(0)
        if(m0['squareFrom'] == 12 and m0['squareTo'] == 28):
          k = k + 1
          chelpy.i_domove(0)
          # or write a C function
          White_king_on_square = chelpy.freaknow()
          chelpy.undomove()
        else:
          chelpy.i_skipmove(0)
    print(k)
    

#### Other utils available

    
    ! wget 'https://chessforeva.gitlab.io/colab/pgn2uci'
    ! wget 'https://theweekinchess.com/assets/files/pgn/chusa24.pgn'
    
    # grant permission
    ! chmod 755 ./pgn2uci
    # create a file of ucis
    !./pgn2uci chusa24.pgn chusa24_ucis.txt
    
    

#### Anaconda Jupyter workaround

    #rename chelpy.so to chelpy then file can be uploaded
    #upload chelpy and refresh folders view
    #perform commands to make a chelpy.so
    
    ! rm chelpy.so
    ! cp chelpy chelpy.so
    ! chmod 755 ./chelpy.so
    ! ls
    
    #now it works
    import chelpy
    print(chelpy.sboard())
    
    

