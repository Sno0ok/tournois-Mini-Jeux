#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

using namespace std;

// Lit une ligne en sautant les commentaires (#) et la coupe au ':'
// ex : "Joueur:Alice" -> cle = "Joueur", valeur = "Alice"
void litUneString (string & cle, string & valeur){
    string uneChaine;
    while (true){
        getline (cin, uneChaine);
        if ((!cin) || (uneChaine.substr(0,1) != "#")) break;
    }
    // un fichier écrit sous Windows finit ses lignes par '\r' : on l'enlève
    if (!uneChaine.empty() && uneChaine.back() == '\r')
        uneChaine.pop_back();

    size_t pos = uneChaine.find(':');
    cle = uneChaine.substr(0, pos);         // avant le ':'
    valeur = uneChaine.substr(pos + 1);     // après le ':'
}

// Prend la prochaine action du paquet et vérifie qu'elle est entre min et max.
// Tous les jeux passent par ici, donc la vérification n'est écrite qu'une fois.
int lireAction (const vector<int> & actions, unsigned & indice, int min, int max){
    if (indice >= actions.size()){
        cerr << "Erreur : il manque des actions dans le fichier" << endl;
        exit(1);
    }
    int action = actions[indice];
    indice++;                               // la prochaine fois, on prendra l'action suivante
    if (action < min || action > max){
        cerr << "Erreur : action " << action << " invalide (attendu entre "
             << min << " et " << max << ")" << endl;
        exit(1);
    }
    return action;
}

// Manche 1 : pile ou face. 1 action : 0 = pile, 1 = face.
// Pile -> le joueur i gagne, face -> le joueur j gagne.
// Renvoie 0 si i gagne, 1 si j gagne.
int pileOuFace (const vector<int> & actions, unsigned & indice){
    return lireAction(actions, indice, 0, 1);
}

// Manche 2 : pierre-feuille-ciseaux. 2 actions : le coup de i, puis le coup de j.
// 0 = pierre, 1 = feuille, 2 = ciseaux. En cas d'égalité, on rejoue.
// Renvoie 0 si i gagne, 1 si j gagne.
int pierreFeuilleCiseaux (const vector<int> & actions, unsigned & indice){
    while (true){
        int coupI = lireAction(actions, indice, 0, 2);
        int coupJ = lireAction(actions, indice, 0, 2);

        if (coupI == coupJ)
            continue;                       // égalité : on rejoue

        bool iGagne = (coupI == 0 && coupJ == 2)    // pierre bat ciseaux
                   || (coupI == 1 && coupJ == 0)    // feuille bat pierre
                   || (coupI == 2 && coupJ == 1);   // ciseaux bat feuille
        if (iGagne)
            return 0;
        return 1;
    }
}

// Manche 3 : dés. 2 actions : le dé de i, puis le dé de j (de 1 à 6).
// Le plus grand gagne. En cas d'égalité, on relance.
// Renvoie 0 si i gagne, 1 si j gagne.
int des (const vector<int> & actions, unsigned & indice){
    while (true){
        int deI = lireAction(actions, indice, 1, 6);
        int deJ = lireAction(actions, indice, 1, 6);

        if (deI > deJ) return 0;
        if (deJ > deI) return 1;
        // égalité : on relance (la boucle recommence)
    }
}

// Un match en BO3 entre le joueur i et le joueur j.
// Le premier qui gagne 2 manches gagne le match.
// Ajoute les manches gagnées dans "manches" et renvoie le numéro du gagnant.
unsigned match (unsigned i, unsigned j, const vector<int> & actions,
                unsigned & indice, vector<int> & manches){
    int mancheI = 0;   // manches gagnées par i dans ce match
    int mancheJ = 0;   // manches gagnées par j dans ce match
    int numManche = 1; // 1, 2 ou 3 : choisit le jeu

    while (mancheI < 2 && mancheJ < 2){
        int resultat;
        if (numManche == 1)
            resultat = pileOuFace(actions, indice);
        else if (numManche == 2)
            resultat = pierreFeuilleCiseaux(actions, indice);
        else
            resultat = des(actions, indice);

        if (resultat == 0){
            mancheI++;
            manches[i]++;
        } else {
            mancheJ++;
            manches[j]++;
        }
        numManche++;
    }

    if (mancheI == 2)
        return i;
    return j;
}

// Renvoie le perdant d'un match entre i et j, quand on connaît le gagnant
unsigned perdant (unsigned gagnant, unsigned i, unsigned j){
    if (gagnant == i)
        return j;
    return i;
}

int main(){
    vector<string> joueurs;   // les noms des joueurs
    vector<int> actions;      // toutes les actions, dans l'ordre du fichier
    string cle, valeur;

    // 1) Lecture de tout le fichier
    while (true){
        litUneString(cle, valeur);
        if (!cin) break;                          // fin du fichier
        if (cle == "Joueur")
            joueurs.push_back(valeur);
        else if (cle == "Action"){
            try {
                actions.push_back(stoi(valeur));  // stoi : "1" -> 1
            } catch (...) {                       // stoi échoue si ce n'est pas un nombre
                cerr << "Erreur : action non numérique (" << valeur << ")" << endl;
                exit(1);
            }
        }
    }

    // Notre Double Bracket est prévu pour 4 joueurs exactement
    if (joueurs.size() != 4){
        cerr << "Erreur : il faut exactement 4 joueurs" << endl;
        return 1;
    }

    unsigned indice = 0;           // prochaine action à utiliser
    vector<int> manches (4, 0);    // demandé par match(), pas utilisé pour ce classement

    // 2) Tableau des gagnants (personne n'a encore perdu)
    // Match 1 : joueur 0 contre joueur 1
    unsigned g1 = match(0, 1, actions, indice, manches);
    unsigned p1 = perdant(g1, 0, 1);
    // Match 2 : joueur 2 contre joueur 3
    unsigned g2 = match(2, 3, actions, indice, manches);
    unsigned p2 = perdant(g2, 2, 3);
    // Match 3 : finale des gagnants
    unsigned g3 = match(g1, g2, actions, indice, manches);
    unsigned p3 = perdant(g3, g1, g2);

    // 3) Tableau des perdants (ceux qui ont perdu une fois)
    // Match 4 : les deux perdants du 1er tour. Le perdant a 2 défaites : il finit 4e
    unsigned g4 = match(p1, p2, actions, indice, manches);
    unsigned quatrieme = perdant(g4, p1, p2);
    // Match 5 : le perdant de la finale des gagnants contre le gagnant du match 4.
    // Le perdant a 2 défaites : il finit 3e
    unsigned g5 = match(p3, g4, actions, indice, manches);
    unsigned troisieme = perdant(g5, p3, g4);

    // 4) Grande finale : g3 (0 défaite) contre g5 (1 défaite)
    unsigned premier = match(g3, g5, actions, indice, manches);
    // Si g5 gagne, g3 n'a perdu qu'une fois : on rejoue un match décisif
    if (premier == g5)
        premier = match(g3, g5, actions, indice, manches);
    unsigned deuxieme = perdant(premier, g3, g5);

    // 5) Affichage : uniquement le classement, rien d'autre
    cout << "1:" << joueurs[premier] << endl;
    cout << "2:" << joueurs[deuxieme] << endl;
    cout << "3:" << joueurs[troisieme] << endl;
    cout << "4:" << joueurs[quatrieme] << endl;

    return 0;
}
