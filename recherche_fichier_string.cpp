// ============================================================
//  TP3 - Recherche d'un mot dans un fichier texte
//  Version 1 : avec la bibliotheque standard (std::string, STL)
//  Mesure du temps avec <chrono> + statistiques via std::vector
// ============================================================
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <vector>
#include <chrono>
#include <cctype>

using Horloge = std::chrono::steady_clock;

// ------------------------------------------------------------
//  Utilitaires
// ------------------------------------------------------------

// Convertit une string en minuscules
std::string toLower(const std::string& s) {
    std::string res = s;
    std::transform(res.begin(), res.end(), res.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return res;
}

// Mesure le temps d'execution (en microsecondes) d'un appelable
template <typename Fonction>
double mesurerUs(Fonction f) {
    auto debut = Horloge::now();
    f();
    auto fin = Horloge::now();
    return std::chrono::duration<double, std::micro>(fin - debut).count();
}

// ------------------------------------------------------------
//  Lecture du fichier
// ------------------------------------------------------------

// Lit tout le contenu dans une string (via rdbuf)
bool lireFichier(const std::string& nomFichier, std::string& contenu) {
    std::ifstream fichier(nomFichier);
    if (!fichier) return false;

    std::stringstream buffer;
    buffer << fichier.rdbuf();
    contenu = buffer.str();
    return true;
}

// ------------------------------------------------------------
//  Recherche (insensible a la casse) avec std::string::find
// ------------------------------------------------------------

size_t rechercher(const std::string& contenu, const std::string& mot) {
    std::string contenuLower = toLower(contenu);
    std::string motLower     = toLower(mot);
    return contenuLower.find(motLower);
}

// ------------------------------------------------------------
//  Affichage du resultat
// ------------------------------------------------------------

void afficherResultat(const std::string& contenu, const std::string& mot, size_t pos) {
    if (pos != std::string::npos) {
        std::cout << "\"" << mot << "\" EXISTE dans le fichier.\n";
        std::cout << "Premiere occurrence a la position : " << pos << "\n";

        // Extrait du contexte
        size_t debut = (pos >= 30) ? pos - 30 : 0;
        size_t fin   = std::min(pos + mot.size() + 30, contenu.size());
        std::cout << "Contexte : ..." << contenu.substr(debut, fin - debut) << "...\n";
    } else {
        std::cout << "\"" << mot << "\" N'EXISTE PAS dans le fichier.\n";
    }
}

// ------------------------------------------------------------
//  Benchmark : repetition de la recherche, statistiques (vector)
// ------------------------------------------------------------

void benchmark(const std::string& contenu, const std::string& mot, int repetitions) {
    std::vector<double> temps;
    temps.reserve(repetitions);

    volatile size_t sink = 0;  // empeche l'optimiseur de supprimer la recherche
    for (int i = 0; i < repetitions; ++i) {
        temps.push_back(mesurerUs([&] { sink = rechercher(contenu, mot); }));
    }
    (void)sink;

    std::sort(temps.begin(), temps.end());
    double somme   = std::accumulate(temps.begin(), temps.end(), 0.0);
    double moyenne = somme / temps.size();
    double mediane = temps[temps.size() / 2];

    std::cout << "\n--- Benchmark (" << repetitions << " repetitions) ---\n";
    std::cout << "Min     : " << temps.front() << " us\n";
    std::cout << "Moyenne : " << moyenne       << " us\n";
    std::cout << "Mediane : " << mediane       << " us\n";
    std::cout << "Max     : " << temps.back()  << " us\n";
}

// ------------------------------------------------------------
//  Programme principal
// ------------------------------------------------------------

int main(int argc, char* argv[]) {
    // Nom du fichier .txt (argument ou saisie)
    std::string nomFichier;
    if (argc >= 2) {
        nomFichier = argv[1];
    } else {
        std::cout << "Nom du fichier .txt : ";
        std::getline(std::cin, nomFichier);
    }

    // Lecture (chronometree)
    std::string contenu;
    bool ok = false;
    double tLecture = mesurerUs([&] { ok = lireFichier(nomFichier, contenu); });
    if (!ok) {
        std::cerr << "Erreur : impossible d'ouvrir " << nomFichier << "\n";
        return 1;
    }

    // Mot a chercher
    std::string motCherche;
    std::cout << "Mot a chercher : ";
    std::getline(std::cin, motCherche);
    if (motCherche.empty()) {
        std::cerr << "Mot vide, rien a chercher.\n";
        return 1;
    }

    // Recherche (chronometree)
    size_t pos = std::string::npos;
    double tRecherche = mesurerUs([&] { pos = rechercher(contenu, motCherche); });

    afficherResultat(contenu, motCherche, pos);

    std::cout << "\n--- Temps d'execution ---\n";
    std::cout << "Taille du fichier : " << contenu.size() << " octets\n";
    std::cout << "Lecture           : " << tLecture   << " us\n";
    std::cout << "Recherche         : " << tRecherche << " us\n";

    benchmark(contenu, motCherche, 1000);
    return 0;
}
