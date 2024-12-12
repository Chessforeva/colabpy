
## Chelpy python chess library for Google Colab
v1.5 dec.2024, Chessforeva

Written in pure C. Magic bitboards calculations, as chess developers mean it. Intended to improve performance. Not sure on results.

Movegen is not scalable as GPU cuda tensors do or likewise. Just a single thread CPU only.

Notebook and usage samples:

[![Open in colab](https://chessforeva.gitlab.io/colab/opencolab.gif 'Open in colab')](https://colab.research.google.com/drive/1y8cYkpbHDymLMZ2K9zJNyRhw3mVWEvTj?usp=sharing)

Source to clone
https://github.com/Chessforeva/colabpy

In case if there is no local linux VM.
Upload

	chelpy.c
	u64_chess.h
	u64_polyglot.h
	
to the Google Shell linux and compile .so library for python.

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
	chelpy.setstartpos()
	chelpy.ucimove("e2e4")
	

Sorry, nothing more in readme there.

All samples of usage are well described in the shared colab notebook above.


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


#### Microsoft Windows dll libraries

https://github.com/Chessforeva/Cpp4chess

1. [u64chesslib.dll](https://github.com/Chessforeva/Cpp4chess/tree/master/DLL_u64chesslib)
 is a shortened chess library similar to chelpy.so

2. [ExeHandler64.dll](https://github.com/Chessforeva/Cpp4chess/tree/master/DLL_for_handling_exe_processes/ExeHandler64)
 is a library to manage Stockfish execution and stdin-stdout processing

There are python samples too.

