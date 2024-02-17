#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "manage.h"

#define MAX_LINE_LENGTH 256

// Fonction lit les info du fichier.csv
Room readFile(int salle)
{
    FILE *sensorFile = fopen("sensor_data.csv", "r");
    if (sensorFile == NULL)
    {
        perror("Erreur : Impossible d'ouvrir le fichier de capteurs");
        exit(EXIT_FAILURE);
    }

    Room room;
    char line[MAX_LINE_LENGTH];

    // Initialiser room avec des valeurs par défaut
    room.salle = -1;
    room.temperature = 0.0;
    room.Door_Status = 0;
    room.Staff_Presence = 0;
    room.authorization_badge = 0;
    room.key=0;
    room.alarm=0;

    // Utiliser une boucle while pour lire toutes les lignes du fichier
    while (fgets(line, MAX_LINE_LENGTH, sensorFile) != NULL)
    {
        char *token = strtok(line, ",");
        if (token != NULL)
        {
            room.salle = atoi(token);

            token = strtok(NULL, ",");
            if (token != NULL)
            {
                room.temperature = atof(token);
            }

            token = strtok(NULL, ",");
            if (token != NULL)
            {
                room.Door_Status = atoi(token);
            }

            token = strtok(NULL, ",");
            if (token != NULL)
            {
                room.Staff_Presence = atoi(token);
            }

            token = strtok(NULL, ",");
            if (token != NULL)
            {
                room.authorization_badge = atoi(token);
            }

            // token = strtok(NULL, ",");
            // if (token != NULL) {
            //     room.ActiveAlarm = atoi(token);

            token = strtok(NULL, ",");
            if (token != NULL)
            {
                room.key = atoi(token);
            }

            // Vérifier si la salle recherchée a été trouvée
            if (room.salle == salle)
            {
                // Fermer le fichier et retourner les données
                fclose(sensorFile);
                return room;
            }
        }
    }

    // Fermer le fichier puisque la salle recherchée n'a pas été trouvée
    fclose(sensorFile);

    // Afficher les valeurs par défaut (à des fins de débogage)
    printf("Valeurs par défaut : salle = %d, temperature = %f\n", room.salle, room.temperature);

    // Retourner la structure room avec les valeurs par défaut
    return room;
}

// Fonction pour modifier l'état de la porte dans le fichier CSV
void modifier_etat_porte(const char *fichier_entree, int numero_salle, int nouvelle_etat_porte)
{
    //time_t tab;
    FILE *file_in, *file_temp;
    char ligne[1000]; // Assumption: A line in the CSV file won't exceed 1000 characters

    // Ouverture du fichier en lecture
    file_in = fopen(fichier_entree, "r");
    if (file_in == NULL)
    {
        perror("Erreur lors de l'ouverture du fichier d'entrée");
        exit(EXIT_FAILURE);
    }

    // Création d'un fichier temporaire en écriture
    file_temp = fopen("temp.csv", "w");
    if (file_temp == NULL)
    {
        perror("Erreur lors de la création du fichier temporaire");
        fclose(file_in);
        exit(EXIT_FAILURE);
    }
    fprintf(file_temp, "%s,%s,%s,%s,%s,%s,%s\n", "id","temp","etat_porte", "presence_personnel"," autorisation_badge", "key","alarm");
    fgets(ligne, sizeof(ligne), file_in);
    while (fgets(ligne, sizeof(ligne), file_in) != NULL)
    {
        // Supprimer le caractère de nouvelle ligne s'il est présent
        ligne[strcspn(ligne, "\n")] = '\0';
        int salle;
        float temperature;
        int etat_porte;
        int presence_personnel;
        int autorisation_badge;
        int key;
        int alarm;

        // Analyse des valeurs de la ligne
        sscanf(ligne, "%d,%f,%d,%d,%d,%d,%d", &salle, &temperature, &etat_porte, &presence_personnel, &autorisation_badge, &key, &alarm);

        // Modification de l'état de la porte pour la salle spécifiée
        if (salle == numero_salle)
        {
            fprintf(file_temp, "%d,%.1f,%d,%d,%d,%d,%d\n", salle, temperature, nouvelle_etat_porte, presence_personnel, autorisation_badge, key, alarm);
        }
        else
        {
            fprintf(file_temp, "%d,%.1f,%d,%d,%d,%d;%d\n", salle, temperature, etat_porte, presence_personnel, autorisation_badge, key, alarm);
        }
    }

    // Fermeture des fichiers
    fclose(file_in);
    fclose(file_temp);

    // Remplacement du fichier d'origine par le fichier temporaire
    if (remove(fichier_entree) == 0 && rename("temp.csv", fichier_entree) == 0)
    {
        printf("Modification réussie. Vérifiez le fichier %s\n", fichier_entree);
    }
    else
    {
        perror("Erreur lors du remplacement du fichier d'origine");
    }
}

// Fontion qui verifie si l'employé est autorisé à rentrer dans la salle
char *OpenDoors(int salle, int key)
{
    Room room = readFile(salle);
    char *res;
    if (room.authorization_badge == 1 && room.key == key)
    {
        modifier_etat_porte("sensor_data.csv", salle, 1);
        res = "la salle est ouverte vous pouvez entrer. \n \n Si vous ne fermez pas la porte, elle vas être fermer automatiquement après 2min";
        return res;
    }
    else if (room.key != key)
    {
        res = "le mot de passe est incorrect\n";
        return res;
    }
    else
    {
        res = "Vous n'êtes pas autorisé à entrer à cette salle\n";
        return res;
    }
}

char *CloseDoors(int salle,int key){

    Room room = readFile(salle);
    char *res;

    if (room.key == key)
    {
        if(room.Door_Status==0){
            res = "la salle est déja fermé.\n";
        }else{
            modifier_etat_porte("sensor_data.csv", salle, 0);
        res = "la salle est fermé. ";
        }  
    }
    else
    {
        res = "le mot de passe est incorrect\n";
    }
    return res;
}


char *manageAlarms()
{
    char line[MAX_LINE_LENGTH];
    Room room;
    char *res = NULL;
    ;
    //char salle[1000];
    FILE *sensorFile = fopen("sensor_data.csv", "r");
    if (sensorFile == NULL)
    {
        perror("Erreur : Impossible d'ouvrir le fichier de capteurs");
        exit(EXIT_FAILURE);
    }
    while (fgets(line, MAX_LINE_LENGTH, sensorFile) != NULL)
    {
        char *token = strtok(line, ",");
        if (token != NULL)
        {
            room.salle = atoi(token);

            token = strtok(NULL, ",");
            if (token != NULL)
            {
                room.temperature = atof(token);
            }

            token = strtok(NULL, ",");
            if (token != NULL)
            {
                room.Door_Status = atoi(token);
            }

            token = strtok(NULL, ",");
            if (token != NULL)
            {
                room.Staff_Presence = atoi(token);
            }

            token = strtok(NULL, ",");
            if (token != NULL)
            {
                room.authorization_badge = atoi(token);
            }

            token = strtok(NULL, ",");
            if (token != NULL)
            {
                room.key = atoi(token);
            }
            /*
            if (room.temperature > 4)
            {
                sprintf(salle, "%d\n", room.salle);
                res = "la temperature de le salle est léve dans la salle: ";
                strcat(res, salle);

            }*/
            if (room.temperature > 4)
            {
                // Allouer de la mémoire pour la nouvelle ligne à afficher
                char *newLine = malloc(strlen("La température est basse dans la salle: ") + 20 + 1); // Taille de la chaîne + taille de salle + '\0'
                if (newLine == NULL)
                {
                    perror("Erreur d'allocation mémoire");
                    exit(EXIT_FAILURE);
                }
                sprintf(newLine, "La température est levé dans la salle: %d\n", room.salle);

                // Concaténer la nouvelle ligne à res
                if (res == NULL)
                {
                    res = newLine;
                }
                else
                {
                    char *temp = malloc(strlen(res) + strlen(newLine) + 1);
                    if (temp == NULL)
                    {
                        perror("Erreur d'allocation mémoire");
                        exit(EXIT_FAILURE);
                    }
                    strcpy(temp, res);
                    strcat(temp, newLine);
                    free(res);
                    res = temp;
                }
            }
        }
    }
    fclose(sensorFile);
    return res;
}

/*int main()
{

    printf("%s", manageAlarms());
}*/