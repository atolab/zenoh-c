JDK_HOME=/Library/Java/JavaVirtualMachines/jdk1.8.0_181.jdk/Contents/Home
default:
	swig   -o zenohc_wrap.c -python zenohc.i 
	gcc -fpic python/wrap/zenohc_wrap.c\
		-L /usr/local/lib\
		-lzenohc\
		-L /usr/local/Cellar/python/3.7.3/Frameworks/Python.framework/Versions/3.7/lib/python3.7/config-3.7m-darwin\
		-lpython3.7\
		-shared\
		-o _zenohc.so\
	 	-DZENOH_MACOS=1\
		-I ../include\
		-I/usr/local/Cellar/python/3.7.3/Frameworks/Python.framework/Versions/3.7/include/python3.7m\

# 
# At some point we can enable fast dispatch with: -fastdispatch
#
python:
	mkdir -p python &>/dev/null 
	mkdir -p python/wrap &>/dev/null 	
	swig -Wall -Wextra  -outdir python/  -o python/wrap/zenohc_wrap.c -python zenohc.i 
	

	gcc -fpic python/wrap/zenohc_wrap.c\
		-L /usr/local/lib\
		-lzenohc\
		-L /usr/local/Cellar/python/3.7.3/Frameworks/Python.framework/Versions/3.7/lib/python3.7/config-3.7m-darwin\
		-lpython3.7\
		-shared\
		-o python/_zenohc.so\
	 	-DZENOH_MACOS=1\
		-I ../include\
		-I/usr/local/Cellar/python/3.7.3/Frameworks/Python.framework/Versions/3.7/include/python3.7m\

java:
	mkdir -p java/src &>/dev/null 
	mkdir -p java/wrap &>/dev/null 
	mkdir -p java/lib &>/dev/null 	
	swig -outdir java/src -o java/wrap/zenohc_wrap.c -java zenohc.i 
	

	gcc -fpic java/wrap/zenohc_wrap.c\
		-L /usr/local/lib\
		-lzenohc\
		-shared\
		-o java/_zenohc.so\
	 	-DZENOH_MACOS=1\
		-I ../include\
		-I ${JDK_HOME}/include\
		-I ${JDK_HOME}/include/darwin


clean:
	$(RM) -Rf __pycache__ *.so *.dylib *.c *.py *.java python java 
