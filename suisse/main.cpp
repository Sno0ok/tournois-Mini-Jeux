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

// Trie "ordre" (les numéros des joueurs) du meilleur au moins bon. Tri à bulles.
// b passe devant a si : il a plus de victoires,
// ou autant de victoires mais plus de manches gagnées.
// Si tout est égal, on n'échange pas : l'ordre d'avant est gardé.
void trierClassement (vector<unsigned> & ordre, const vector<int> & victoires,
                      const vector<int> & manches){
    unsigned n = ordre.size();
    for (unsigned passe = 0; passe + 1 < n; passe++){
        for (unsigned k = 0; k + 1 < n - passe; k++){
            unsigned a = ordre[k];
            unsigned b = ordre[k + 1];
            bool bPasseDevant = victoires[b] > victoires[a]
                    || (victoires[b] == victoires[a] && manches[b] > manches[a]);
            if (bPasseDevant){
                ordre[k] = b;
                ordre[k + 1] = a;
            }
        }
    }
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

    unsigned n = joueurs.size();
    unsigned indice = 0;                 // prochaine action à utiliser
    vector<int> victoires (n, 0);        // victoires[i] = matchs gagnés par le joueur i
    vector<int> manches (n, 0);          // manches[i] = manches gagnées par le joueur i
    vector<bool> dejaExempte (n, false); // true si le joueur a déjà été exempté une fois
    // dejaJoue[a][b] vaut true si a et b se sont déjà affrontés
    vector<vector<bool>> dejaJoue (n, vector<bool>(n, false));

    // 2) Nombre de rondes : on double "puissance" jusqu'à dépasser n
    // ex : 4 joueurs -> 2 rondes, 5 à 8 joueurs -> 3 rondes
    unsigned nbRondes = 0;
    unsigned puissance = 1;
    while (puissance < n){
        puissance = puissance * 2;
        nbRondes++;
    }

    // ordre = numéros des joueurs, au départ dans l'ordre d'inscription : 0, 1, 2, ...
    vector<unsigned> ordre;
    for (unsigned i = 0; i < n; i++)
        ordre.push_back(i);

    // 3) Les rondes
    for (unsigned ronde = 0; ronde < nbRondes; ronde++){
        trierClassement(ordre, victoires, manches);   // on apparie selon le classement actuel
        vector<bool> apparie (n, false);               // true si le joueur a un adversaire cette ronde

        // Nombre impair : le moins bien classé qui n'a jamais été exempté
        // ne joue pas cette ronde et gagne 1 victoire (mais aucune manche)
        if (n % 2 == 1){
            for (unsigned k = n; k > 0; k--){
                unsigned p = ordre[k - 1];
                if (!dejaExempte[p]){
                    dejaExempte[p] = true;
                    apparie[p] = true;
                    victoires[p]++;
                    break;
                }
            }
        }

        // Appariement : on prend les joueurs du haut du classement vers le bas
        for (unsigned k = 0; k < n; k++){
            unsigned a = ordre[k];
            if (apparie[a])
                continue;                       // a a déjà un adversaire

            // on cherche le prochain joueur libre que a n'a pas encore affronté
            bool trouve = false;
            unsigned adversaire = 0;
            for (unsigned l = k + 1; l < n && !trouve; l++){
                unsigned b = ordre[l];
                if (!apparie[b] && !dejaJoue[a][b]){
                    adversaire = b;
                    trouve = true;
                }
            }
            // s'il n'y en a aucun, on accepte une revanche avec le prochain joueur libre
            for (unsigned l = k + 1; l < n && !trouve; l++){
                unsigned b = ordre[l];
                if (!apparie[b]){
                    adversaire = b;
                    trouve = true;
                }
            }
            if (!trouve)
                continue;                       // plus personne de libre

            apparie[a] = true;
            apparie[adversaire] = true;
            dejaJoue[a][adversaire] = true;
            dejaJoue[adversaire][a] = true;

            unsigned gagnant = match(a, adversaire, actions, indice, manches);
            victoires[gagnant]++;
        }
    }

    // 4) Classement final et affichage : uniquement le classement, rien d'autre
    trierClassement(ordre, victoires, manches);
    for (unsigned k = 0; k < n; k++)
        cout << k + 1 << ":" << joueurs[ordre[k]] << endl;

    return 0;
}
