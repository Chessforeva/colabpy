
## Chelpy python library for Google Colab
v.1.0 nov.2024, Chessforeva

Written in pure C. Magic bitboards calculations. Performance shoud be really good, but not scalable as GPU cuda tensors or likewise. Just a single thread CPU only.

Notebook:
https://colab.research.google.com/drive/1y8cYkpbHDymLMZ2K9zJNyRhw3mVWEvTj?usp=sharing

Source to clone
https://github.com/Chessforeva/colabpy

In case if there is no local linux VM.
Upload chelpy.c, u64_chess.h to Google Shell linux and compile .so library for python
    
    gcc chelpy.c -shared -o chelpy.so -I/usr/include/python3.12 -fPIC 

May work also on Anaconda Jupyters or AI Lightnings.

#### Load library in Colab

    #get library from online site
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
    chelpy.movegen()
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
    chelpy.parsepgn("1.Nf3 Ng8-f6 2.g2g3 e5 3.Bg2 Nc6 4. 0-0 ")
    print(chelpy.sboard())

#### Iterations

    
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
    

    #see performance
    for i in range(1*1000*1000):
      chelpy.setstartpos()
      chelpy.movegen()
      # or write a C function
      White_king_on_square = chelpy.freaknow()
    

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
    
    

