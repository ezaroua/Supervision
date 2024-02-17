#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include "manage.h"
// Structure pour stocker les informations de chaque ligne du CSV
typedef struct {
    int id;
    float temp;
    int etat_porte;
    int presence_personnel;
    int autorisation_badge;
    int key;
    int alarme;
} RoomData;


// Déclaration de la variable globale temps_formatte
char temps_formatte[20];

// Déclaration des fonctions
int lire_fichier_csv(const char *nom_fichier, RoomData *donnees, int *nb_lignes);
void enregistrer_log(const char *nom_fichier, const char *format, ...);
void modifier_etat_porte(const char *fichier_entree, int numero_salle, int nouvelle_etat_porte);
void modifier_alarme(const char *fichier_entree, int numero_salle, int nouvelle_valeur);

int main() {
    const char *fichier_csv = "sensor_data.csv";
    RoomData donnees[10];  // Supposons que vous ayez au plus 10 salles
    int nb_lignes;

    while (1) {
        if (lire_fichier_csv(fichier_csv, donnees, &nb_lignes) == -1) {
            fprintf(stderr, "Erreur lors de la lecture du fichier CSV.\n");
            return EXIT_FAILURE;
        }

        // Parcourir toutes les salles
        for (int i = 0; i < nb_lignes; ++i) {
            int salle_a_surveiller = donnees[i].id;
            int temps_d_attente = 60;

            if (donnees[i].etat_porte == 1) {
                printf("La porte de la salle %d est ouverte. Attendez %d secondes...\n", salle_a_surveiller, temps_d_attente);
                sleep(temps_d_attente);

                // Fermer la porte après le délai d'attente
                modifier_etat_porte(fichier_csv, salle_a_surveiller, 0);

                // Enregistrement dans le fichier log.txt
                time_t now;
                time(&now);
                struct tm *tm_info = localtime(&now);
                char temps_formatte[20];
                strftime(temps_formatte, 20, "%Y-%m-%d %H:%M:%S", tm_info);

                enregistrer_log("surveille_Porte.txt", "[%s] La porte de la salle %d s'est fermée automatiquement après %d secondes\n",
                                temps_formatte, salle_a_surveiller, temps_d_attente);

                // Activation de l'alarme dans le fichier sensor_data.csv
                modifier_alarme(fichier_csv, salle_a_surveiller, 1);
            }
        }

        // Pause avant de relire les données du fichier
        sleep(1);
    }

    return EXIT_SUCCESS;
}


// Fonction pour lire les données du fichier CSV
int lire_fichier_csv(const char *nom_fichier, RoomData *donnees, int *nb_lignes) {
    FILE *fichier = fopen(nom_fichier, "r");
    if (fichier == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return -1;
    }

    // Ignorer la première ligne (entête)
    char ligne[256];
    if (fgets(ligne, sizeof(ligne), fichier) == NULL) {
        fclose(fichier);
        return -1;
    }

    int index = 0;
    while (fgets(ligne, sizeof(ligne), fichier) != NULL) {
        sscanf(ligne, "%d,%f,%d,%d,%d,%d,%d", 
               &donnees[index].id, 
               &donnees[index].temp, 
               &donnees[index].etat_porte, 
               &donnees[index].presence_personnel, 
               &donnees[index].autorisation_badge, 
               &donnees[index].key,
               &donnees[index].alarme);
        index++;
    }

    *nb_lignes = index;

    fclose(fichier);
    return 0;
}


// Fonction pour enregistrer des logs dans un fichier
void enregistrer_log(const char *nom_fichier, const char *format, ...) {
    FILE *log_file = fopen(nom_fichier, "a");
    if (log_file == NULL) {
        perror("Erreur lors de l'ouverture du fichier de log");
        return;
    }

    va_list args;
    va_start(args, format);

    vfprintf(log_file, format, args);

    va_end(args);
    fclose(log_file);
}

// Fonction pour modifier l'état de la porte dans le fichier CSV
void modifier_etat_porte(const char *fichier_entree, int numero_salle, int nouvelle_etat_porte) {
    FILE *file_in, *file_temp;
    char ligne[1000]; // Assumption: A line in the CSV file won't exceed 1000 characters

    // Ouverture du fichier en lecture
    file_in = fopen(fichier_entree, "r");
    if (file_in == NULL) {
        perror("Erreur lors de l'ouverture du fichier d'entrée");
        exit(EXIT_FAILURE);
    }

    // Création d'un fichier temporaire en écriture
    file_temp = fopen("temp.csv", "w");
    if (file_temp == NULL) {
        perror("Erreur lors de la création du fichier temporaire");
        fclose(file_in);
        exit(EXIT_FAILURE);
    }
    
    // Écrire l'en-tête du fichier CSV dans le fichier temporaire
    fprintf(file_temp, "%s,%s,%s,%s,%s,%s,%s\n", "id", "temp", "etat_porte", "presence_personnel", "autorisation_badge", "key", "alarme");

    // Lire et modifier chaque ligne du fichier d'entrée
    fgets(ligne, sizeof(ligne), file_in);
    while (fgets(ligne, sizeof(ligne), file_in) != NULL) {
        // Supprimer le caractère de nouvelle ligne s'il est présent
        ligne[strcspn(ligne, "\n")] = '\0';
        int salle, etat_porte, presence_personnel, autorisation_badge, key, alarme;
        float temperature;

        // Analyse des valeurs de la ligne
        sscanf(ligne, "%d,%f,%d,%d,%d,%d,%d", &salle, &temperature, &etat_porte, &presence_personnel, &autorisation_badge, &key, &alarme);

        // Modification de l'état de la porte pour la salle spécifiée
        if (salle == numero_salle) {
            fprintf(file_temp, "%d,%.1f,%d,%d,%d,%d,%d\n", salle, temperature, nouvelle_etat_porte, presence_personnel, autorisation_badge, key, alarme);
        } else {
            fprintf(file_temp, "%d,%.1f,%d,%d,%d,%d,%d\n", salle, temperature, etat_porte, presence_personnel, autorisation_badge, key, alarme);
        }
    }

    // Fermeture des fichiers
    fclose(file_in);
    fclose(file_temp);

    // Remplacement du fichier d'origine par le fichier temporaire
    if (remove(fichier_entree) == 0 && rename("temp.csv", fichier_entree) == 0) {
        // Enregistrement d'un log pour la fermeture automatique de la porte
        enregistrer_log("log.txt", "[%s] Porte de la salle %d fermée automatiquement.\n", temps_formatte, numero_salle);
    } else {
        perror("Erreur lors du remplacement du fichier d'origine");
    }
}

// Fonction pour enregistrer une alarme dans le fichier CSV
void enregistrer_alarme(const char *fichier_entree, int numero_salle, int nouvelle_valeur) {
    FILE *file_in, *file_temp;
    char ligne[1000]; // Assumption: A line in the CSV file won't exceed 1000 characters

    // Ouverture du fichier en lecture
    file_in = fopen(fichier_entree, "r");
    if (file_in == NULL) {
        perror("Erreur lors de l'ouverture du fichier d'entrée");
        exit(EXIT_FAILURE);
    }

    // Création d'un fichier temporaire en écriture
    file_temp = fopen("temp.csv", "w");
    if (file_temp == NULL) {
        perror("Erreur lors de la création du fichier temporaire");
        fclose(file_in);
        exit(EXIT_FAILURE);
    }

    // Écrire l'en-tête du fichier CSV dans le fichier temporaire
    fprintf(file_temp, "%s,%s,%s,%s,%s,%s,%s\n", "id", "temp", "etat_porte", "presence_personnel", "autorisation_badge", "key", "alarme");

    // Lire et modifier chaque ligne du fichier d'entrée
    fgets(ligne, sizeof(ligne), file_in);
    while (fgets(ligne, sizeof(ligne), file_in) != NULL) {
        // Supprimer le caractère de nouvelle ligne s'il est présent
        ligne[strcspn(ligne, "\n")] = '\0';
        int salle, etat_porte, presence_personnel, autorisation_badge, key, alarme;
        float temperature;

        // Analyse des valeurs de la ligne
        sscanf(ligne, "%d,%f,%d,%d,%d,%d,%d", &salle, &temperature, &etat_porte, &presence_personnel, &autorisation_badge, &key, &alarme);

        // Modification de l'alarme pour la salle spécifiée
        if (salle == numero_salle) {
            fprintf(file_temp, "%d,%.1f,%d,%d,%d,%d,%d\n", salle, temperature, etat_porte, presence_personnel, autorisation_badge, key, nouvelle_valeur);
        } else {
            fprintf(file_temp, "%d,%.1f,%d,%d,%d,%d,%d\n", salle, temperature, etat_porte, presence_personnel, autorisation_badge, key, alarme);
        }
    }

    // Fermeture des fichiers
    fclose(file_in);
    fclose(file_temp);

    // Remplacement du fichier d'origine par le fichier temporaire
    if (remove(fichier_entree) == 0 && rename("temp.csv", fichier_entree) == 0) {
        // Enregistrement d'un log pour l'activation de l'alarme
        enregistrer_log("log.txt", "[%s] Alarme activée dans la salle %d (température hors limites)\n", temps_formatte, numero_salle);
    } else {
        perror("Erreur lors du remplacement du fichier d'origine");
    }
}

void modifier_alarme(const char *fichier_entree, int numero_salle, int nouvelle_valeur) {
    FILE *file_in, *file_temp;
    char ligne[1000];

    // Ouvrir le fichier en lecture
    file_in = fopen(fichier_entree, "r");
    if (file_in == NULL) {
        perror("Erreur lors de l'ouverture du fichier d'entrée");
        exit(EXIT_FAILURE);
    }

    // Créer un fichier temporaire en écriture
    file_temp = fopen("temp.csv", "w");
    if (file_temp == NULL) {
        perror("Erreur lors de la création du fichier temporaire");
        fclose(file_in);
        exit(EXIT_FAILURE);
    }

    fprintf(file_temp, "%s,%s,%s,%s,%s,%s,%s\n", "id", "temp", "etat_porte", "presence_personnel", "autorisation_badge", "key", "alarme");
    fgets(ligne, sizeof(ligne), file_in);
    while (fgets(ligne, sizeof(ligne), file_in) != NULL) {
        ligne[strcspn(ligne, "\n")] = '\0'; // Supprimer le caractère de nouvelle ligne s'il est présent
        int salle, alarme;
        float temperature;
        int etat_porte, presence_personnel, autorisation_badge, key;

        // Analyser les valeurs de la ligne
        sscanf(ligne, "%d,%f,%d,%d,%d,%d,%d", &salle, &temperature, &etat_porte, &presence_personnel, &autorisation_badge, &key, &alarme);

        // Modifier l'alarme pour la salle spécifiée
        if (salle == numero_salle) {
            fprintf(file_temp, "%d,%.1f,%d,%d,%d,%d,%d\n", salle, temperature, etat_porte, presence_personnel, autorisation_badge, key, nouvelle_valeur);
        } else {
            fprintf(file_temp, "%d,%.1f,%d,%d,%d,%d,%d\n", salle, temperature, etat_porte, presence_personnel, autorisation_badge, key, alarme);
        }
    }

    // Fermer les fichiers
    fclose(file_in);
    fclose(file_temp);

    // Remplacer le fichier d'origine par le fichier temporaire
    if (remove(fichier_entree) == 0 && rename("temp.csv", fichier_entree) == 0) {
        printf("Modification de l'alarme réussie. Vérifiez le fichier %s\n", fichier_entree);
    } else {
        perror("Erreur lors du remplacement du fichier d'origine");
    }
}
