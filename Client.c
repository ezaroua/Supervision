#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 1024

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    // déclaration des variables
    int client_socket;
    int choix;
    char buffer[1000];
    int retourWrite;
    int salle;
    int key;

    // création de la socket client
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        // echec de la création de la socket
        fprintf(stderr, "erreur de la création \n");
        exit(EXIT_FAILURE);
    }
    else
    {
        // succès de la création
        fprintf(stdout, "création réussie de la socket \n");

        // adresse du serveur
        struct hostent *hote = gethostbyname("127.0.0.1");
        if (hote == NULL)
        {
            fprintf(stderr, "Erreur lors de l'allocation de l'addresse de l'hôte\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            // déclaration des variables serveur
            struct sockaddr_in s; // adresse machine
            s.sin_family = AF_INET;
            memcpy(&s.sin_addr.s_addr, hote->h_addr_list[0], hote->h_length);
            s.sin_port = htons(PORT);

            // Demande de connexion avec le serveur
            int connex = connect(client_socket, (struct sockaddr *)&s, sizeof(s));
            if (connex == -1)
            {
                fprintf(stderr, "erreur de la connexion \n");
                exit(EXIT_FAILURE);
            }
            else
            {
                fprintf(stdout, "connexion réussie %d\n", connex);

                while (1)
                {
                    read(client_socket, &buffer, sizeof(buffer));
                    printf("%s", buffer);

                    printf("\n--------------------- Menu -----------------------\n");
                    printf("1-ouvrir une salle\n");
                    printf("2-fermer une salle\n");
                    printf("4-Quitter\n");
                    printf("veuillez entrer un choix \n");
                    scanf("%d", &choix);

                    /*Envoie de la requête*/
                    retourWrite = write(client_socket, &choix, sizeof(choix));
                    if (retourWrite == -1)
                    {
                        perror("Fail write\n");
                        exit(EXIT_FAILURE);
                    }
                    else
                    {
                        switch (choix)
                        {
                        case 5:
                            exit(EXIT_SUCCESS);
                        case 1:
                            printf("Quelle salle voulez-vous ouvrir?\n");
                            scanf("%d", &salle);
                            write(client_socket, &salle, sizeof(salle));
                            printf("Veuillez saisir le mot de passe\n");
                            scanf("%d", &key);
                            write(client_socket, &key, sizeof(key));
                            read(client_socket, &buffer, sizeof(buffer));
                            printf("%s", buffer);
                            break;
                        case 2:
                            printf("Quelle salle voulez-vous fermer?\n");
                            scanf("%d", &salle);
                            write(client_socket, &salle, sizeof(salle));
                            printf("Veuillez saisir le mot de passe\n");
                            scanf("%d", &key);
                            write(client_socket, &key, sizeof(key));
                            read(client_socket, &buffer, sizeof(buffer));
                            printf("%s", buffer);
                            break;
                        case 3:
                            int m = 0;
                            printf("da5al chi l3iba");
                            scanf("%d", &m);
                            printf("\nhahia l3ibtk %d\n", choix);
                            break;
                        case 4:
                            exit(EXIT_SUCCESS);
                        }
                    }
                }
            }
        }
        close(client_socket);  // Fermez la socket avant de quitter le programme
        return 0;
    }
}
