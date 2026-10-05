#include <iostream>
#include <fstream>
#include <string>
#include <cctype>

// Version from scratch : recherche une sous-chaine insensible a la casse
// Retourne l'index de la premiere occurrence, ou -1 si absente
int rechercher(const std::string& texte, const std::string& motif) {
    if (motif.empty()) return -1;
    if (motif.size() > texte.size()) return -1;

    int n = (int)texte.size();
    int m = (int)motif.size();

    for (int i = 0; i <= n - m; ++i) {
        int j = 0;
        while (j < m &&
               std::tolower((unsigned char)texte[i + j]) ==
               std::tolower((unsigned char)motif[j])) {
            ++j;
        }
        if (j == m) return i;  // motif trouve a la position i
    }
    return -1;
}

int main(int argc, char* argv[]) {
    std::string nomFichier;
    if (argc >= 2) {
        nomFichier = argv[1];
    } else {
        std::cout << "Nom du fichier .txt : ";
        std::getline(std::cin, nomFichier);
    }

    std::ifstream fichier(nomFichier);
    if (!fichier) {
        std::cerr << "Erreur : impossible d'ouvrir " << nomFichier << "\n";
        return 1;
    }

    // Lire tout le fichier caractere par caractere (from scratch)
    std::string contenu;
    char c;
    while (fichier.get(c)) {
        contenu += c;
    }
    fichier.close();

    std::string motCherche;
    std::cout << "Mot a chercher : ";
    std::getline(std::cin, motCherche);

    if (motCherche.empty()) {
        std::cerr << "Mot vide.\n";
        return 1;
    }

    int pos = rechercher(contenu, motCherche);

    if (pos != -1) {
        std::cout << "\"" << motCherche << "\" EXISTE dans le fichier.\n";
        std::cout << "Position : " << pos << "\n";
    } else {
        std::cout << "\"" << motCherche << "\" N'EXISTE PAS dans le fichier.\n";
    }

    return 0;
}