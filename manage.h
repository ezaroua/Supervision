
#ifndef MANAGER_H
#define MANAGER_H

typedef struct {
    int salle;
    float temperature;
    int Door_Status;
    int Staff_Presence;
    int authorization_badge;
    int key;
    int alarm;
} Room;


Room readFile(int salle);
void modifier_etat_porte(const char *fichier_entree, int numero_salle, int nouvel_etat_porte);
char *OpenDoors(int salle, int key) ;
char *CloseDoors(int salle,int key);
char *manageAlarms();


#endif

