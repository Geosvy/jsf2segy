CC = gcc 
OBJECTS = jsf2segy.c  ascebc.c 
CFLAGS=-g  -m64 $(OPTFLAGS) -Wall -Wimplicit -Wimplicit-int -Wimplicit-function-declaration -W -Wstrict-prototypes -Wnested-externs  
LIBS = -lm -lc


jsf2segy:$(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) -o jsf2segy

