all:
	swig -python zenohc.i

	gcc -fpic zenohc_wrap.c\
		-L /usr/local/lib\
		-lzenohc\
		-L /usr/local/Cellar/python/3.7.3/Frameworks/Python.framework/Versions/3.7/lib/python3.7/config-3.7m-darwin\
		-lpython3.7\
		-shared\
		-o _zenohc.so\
	 	-DZENOH_MACOS=1\
		-I ../include\
		-I/usr/local/Cellar/python/3.7.3/Frameworks/Python.framework/Versions/3.7/include/python3.7m\

clean:
	$(RM) -Rf __pycache__ *.so *.dylib *.c *.py
