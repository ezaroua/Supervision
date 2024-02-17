CFLAGS = -Wall -Wextra -g

all: serveur client surveillancePorte surveillanceTemperature

serveur: Serveur.o manage.o
	gcc $(CFLAGS) -o serveur Serveur.o manage.o

client: Client.o manage.o
	gcc $(CFLAGS) -o client Client.o manage.o

surveillanceTemperature: surveillanceTemperature.o
	gcc $(CFLAGS) -o surveillanceTemperature surveillanceTemperature.o

surveillancePorte: surveillancePorte.o
	gcc $(CFLAGS) -o surveillancePorte surveillancePorte.o

Serveur.o: Serveur.c manage.h
	gcc $(CFLAGS) -c -o Serveur.o Serveur.c

Client.o: Client.c manage.h
	gcc $(CFLAGS) -c -o Client.o Client.c

manage.o: manage.c manage.h
	gcc $(CFLAGS) -c -o manage.o manage.c

surveillanceTemperature.o: surveillanceTemperature.c 
	gcc $(CFLAGS) -c -o surveillanceTemperature.o surveillanceTemperature.c

surveillancePorte.o: surveillancePorte.c
	gcc $(CFLAGS) -c -o surveillancePorte.o surveillancePorte.c

clean:
	rm -f serveur client surveillanceTemperature surveillancePorte Serveur.o Client.o manage.o surveillanceTemperature.o surveillancePorte.o

.PHONY: all clean
