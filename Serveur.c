#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include "manage.h"

#define PORT 1024

void handler(int sig)
{
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0)
    {
        continue;
    }
}

int main()
{

    int serveur_socket;
    int acceptesocket;
    int retourRead;
    char buffer[1000];
    int choix;
    int salle;
    int key;
    struct sockaddr_in s1, s2;

    // Adresse du serveur
    s2.sin_family = AF_INET;
    s2.sin_addr.s_addr = htonl(INADDR_ANY);
    s2.sin_port = htons(PORT);

    // Création de la socket
    serveur_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (serveur_socket == -1)
    {
        fprintf(stderr, "erreur de la creation \n");
        exit(EXIT_FAILURE);
    }

    // Succès de la création
    fprintf(stdout, "creation reussie de la socket \n");

    // La fonction bind
    if (bind(serveur_socket, (struct sockaddr *)&s2, sizeof(s2)) == -1)
    {
        fprintf(stderr, "erreur du bind \n");
        exit(EXIT_FAILURE);
    }

    // Succès du bind
    fprintf(stdout, "Succes du bind\n");

    // Fonction listen
    if (listen(serveur_socket, 5) == -1)
    {
        fprintf(stderr, "Echec de l'écoute \n");
        exit(EXIT_FAILURE);
    }

    // Succès de l'écoute
    fprintf(stdout, "L'écoute est réussie\n");

    // Boucle infinie
    while (1)
    {
        fprintf(stdout, "On rentre dans la boucle\n");
        socklen_t len = sizeof(s1);
        acceptesocket = accept(serveur_socket, (struct sockaddr *)&s1, &len);
        if (acceptesocket == -1)
        {
            fprintf(stderr, "Refus de connexion\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            fprintf(stdout, "Connexion établie avec le client \n");

            // Création d'un fils
            int p = fork();
            switch (p)
            {
            case -1:
                fprintf(stderr, "Erreur de création du fils\n");
                exit(EXIT_FAILURE);

            case 0:
                // Processus fils
                while (1)
                {
                    strcpy(buffer, "Message du serveur : Bienvenue!\n");
                    write(acceptesocket, &buffer, sizeof(buffer));
                    retourRead = read(acceptesocket, &choix, sizeof(choix));
                    if (retourRead == -1)
                    {
                        perror("Erreur de lecture depuis le client");
                        close(acceptesocket);
                        exit(EXIT_FAILURE);
                    }
                    // Traitement des requêtes
                    switch (choix)
                    {
                    case 1:
                        read(acceptesocket, &salle, sizeof(salle));
                        read(acceptesocket, &key, sizeof(key));
                        strcpy(buffer, "Traitement de la requête d'ouverture de salle\n");
                        write(acceptesocket, &buffer, sizeof(buffer));
                        strcpy(buffer, OpenDoors(salle, key));
                        write(acceptesocket, &buffer, sizeof(buffer));
                        break;
                    case 2:
                        read(acceptesocket, &salle, sizeof(salle));
                        read(acceptesocket, &key, sizeof(key));
                        strcpy(buffer, "Traitement de la requête de fermeture de salle\n");
                        write(acceptesocket, &buffer, sizeof(buffer));
                        strcpy(buffer, CloseDoors(salle, key));
                        write(acceptesocket, &buffer, sizeof(buffer));
                        break;
                    case 3:
                        strcpy(buffer, "Traitement de la requête 3\n");
                        write(acceptesocket, &buffer, sizeof(buffer));
                        break;
                    case 4:
                        strcpy(buffer, "Serveur déconnecté\n");
                        write(acceptesocket, &buffer, sizeof(buffer));
                        close(acceptesocket);
                        exit(EXIT_SUCCESS);
                    case 5:
                        break;
                    default:
                        break;
                    }
                }
                close(acceptesocket);
                exit(EXIT_SUCCESS);

            default:
                // Processus parent
                close(acceptesocket);
                signal(SIGCHLD, handler);
            }
        }
    }
    close(serveur_socket);
    return 0;
}
