#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>

// Convertit une string en minuscules
std::string toLower(const std::string& s) {
    std::string res = s;
    std::transform(res.begin(), res.end(), res.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return res;
}

int main(int argc, char* argv[]) {
    // Nom du fichier .txt (argument ou saisie)
    std::string nomFichier;
    if (argc >= 2) {
        nomFichier = argv[1];
    } else {
        std::cout << "Nom du fichier .txt : ";
        std::getline(std::cin, nomFichier);
    }

    // Ouvrir le fichier
    std::ifstream fichier(nomFichier);
    if (!fichier) {
        std::cerr << "Erreur : impossible d'ouvrir " << nomFichier << "\n";
        return 1;
    }

    // Lire tout le contenu dans une string
    std::stringstream buffer;
    buffer << fichier.rdbuf();
    std::string contenu = buffer.str();
    fichier.close();

    // Demander le mot à chercher
    std::string motCherche;
    std::cout << "Mot a chercher : ";
    std::getline(std::cin, motCherche);

    if (motCherche.empty()) {
        std::cerr << "Mot vide, rien a chercher.\n";
        return 1;
    }

    // Recherche insensible a la casse
    std::string contenuLower = toLower(contenu);
    std::string motLower     = toLower(motCherche);

    size_t pos = contenuLower.find(motLower);

    if (pos != std::string::npos) {
        std::cout << "\"" << motCherche << "\" EXISTE dans le fichier.\n";
        std::cout << "Premiere occurrence a la position : " << pos << "\n";

        // Afficher un extrait du contexte
        size_t debut = (pos >= 30) ? pos - 30 : 0;
        size_t fin   = std::min(pos + motCherche.size() + 30, contenu.size());
        std::cout << "Contexte : ..."
                  << contenu.substr(debut, fin - debut)
                  << "...\n";
    } else {
        std::cout << "\"" << motCherche << "\" N'EXISTE PAS dans le fichier.\n";
    }

    return 0;
}