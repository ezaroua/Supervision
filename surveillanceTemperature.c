#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h> 

char temps_formatte[20]; // Variable globale pour stocker la date formatée

typedef struct {
    int id;
    float temp;
    int etat_porte;
    int presence_personnel;
    int autorisation_badge;
    int key;
    int alarme;
} RoomData;

void enregistrer_log(const char *nom_fichier, const char *format, ...);
int lire_fichier_csv(const char *nom_fichier, RoomData *donnees, int *nb_lignes);
void modifier_alarme(const char *fichier_entree, int numero_salle, int nouvelle_valeur);

void enregistrer_log(const char *nom_fichier, const char *format, ...) {
    va_list args;
    va_start(args, format);

    FILE *file = fopen(nom_fichier, "a");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier de log");
        va_end(args);
        exit(EXIT_FAILURE);
    }

    // Utilisation de la variable globale pour la date formatée
    fprintf(file, "[%s] ", temps_formatte);
    vfprintf(file, format, args);
    fprintf(file, "\n");

    fclose(file);
    va_end(args);
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

void verifier_temperature(const char *fichier_csv) {
    RoomData donnees[10];  // Supposons que vous ayez au plus 10 salles
    int nb_lignes;

    // Obtenir la date formatée une seule fois avant d'entrer dans la boucle
    time_t temps_actuel;
    struct tm *temps_info;

    // Boucle principale
    while (1) {
        // Mettre à jour la date uniquement si nécessaire
        temps_actuel = time(NULL);
        temps_info = localtime(&temps_actuel);
        strftime(temps_formatte, sizeof(temps_formatte), "%Y-%m-%d %H:%M:%S", temps_info);

        if (lire_fichier_csv(fichier_csv, donnees, &nb_lignes) == -1) {
            fprintf(stderr, "Erreur lors de la lecture du fichier CSV.\n");
            exit(EXIT_FAILURE);
        }

        // Parcourir toutes les salles
        for (int i = 0; i < nb_lignes; ++i) {
            int salle_a_surveiller = donnees[i].id;
            float temperature = donnees[i].temp;

            if (temperature < 0 || temperature > 4) {
                // Température hors limites, enregistrer une ligne dans le fichier log.txt
                enregistrer_log("surveille_temperature.txt", "[%s] Température hors limites dans la salle %d (%.2f°C)\n",
                                temps_formatte, salle_a_surveiller, temperature);

                // Activation de l'alarme dans le fichier sensor_data.csv
                modifier_alarme(fichier_csv, salle_a_surveiller, 1);
            }
        }

        // Pause avant de relire les données du fichier
        sleep(1);
    }
}

int main() {
    const char *fichier_csv = "sensor_data.csv";
    verifier_temperature(fichier_csv);

    return EXIT_SUCCESS;
}
