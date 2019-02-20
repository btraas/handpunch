
handpunch: handpunch.o hgu.o crc_ccitt.o
	$(CC) $(LDFLAGS) $^ -o $@

handpunch.o: handpunch.c hgu.h
	$(CC) $(CFLAGS) -c $<

hgu.o: hgu.c hgu.h 
	$(CC) $(CFLAGS) -c $<

crc_ccitt.o: crc_ccitt.c crc_ccitt.h
	$(CC) $(CFLAGS) -c $<

debug:
	$(MAKE) clean
	$(MAKE) handpunch "CFLAGS=-g -DDEBUG -Wall -Wextra -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC"

clean: 
	rm -f handpunch handpunch.o hgu.o crc_ccitt.o
