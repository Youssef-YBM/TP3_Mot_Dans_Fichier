// ============================================================
//  TP3 - Recherche d'un mot dans un fichier texte
//  Version 2 : "from scratch" (algorithme naif ecrit a la main)
//  Mesure du temps avec <chrono> + statistiques via std::vector
// ============================================================
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <numeric>
#include <vector>
#include <chrono>
#include <cctype>

using Horloge = std::chrono::steady_clock;

// ------------------------------------------------------------
//  Utilitaires
// ------------------------------------------------------------

// Mesure le temps d'execution (en microsecondes) d'un appelable
template <typename Fonction>
double mesurerUs(Fonction f) {
    auto debut = Horloge::now();
    f();
    auto fin = Horloge::now();
    return std::chrono::duration<double, std::micro>(fin - debut).count();
}

// ------------------------------------------------------------
//  Lecture du fichier (caractere par caractere)
// ------------------------------------------------------------

bool lireFichier(const std::string& nomFichier, std::string& contenu) {
    std::ifstream fichier(nomFichier);
    if (!fichier) return false;

    char c;
    while (fichier.get(c)) {
        contenu += c;
    }
    return true;
}

// ------------------------------------------------------------
//  Recherche from scratch : sous-chaine insensible a la casse
//  Retourne l'index de la premiere occurrence, ou -1 si absente
// ------------------------------------------------------------

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

// ------------------------------------------------------------
//  Affichage du resultat
// ------------------------------------------------------------

void afficherResultat(const std::string& mot, int pos) {
    if (pos != -1) {
        std::cout << "\"" << mot << "\" EXISTE dans le fichier.\n";
        std::cout << "Position : " << pos << "\n";
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

    volatile int sink = 0;  // empeche l'optimiseur de supprimer la recherche
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
        std::cerr << "Mot vide.\n";
        return 1;
    }

    // Recherche (chronometree)
    int pos = -1;
    double tRecherche = mesurerUs([&] { pos = rechercher(contenu, motCherche); });

    afficherResultat(motCherche, pos);

    std::cout << "\n--- Temps d'execution ---\n";
    std::cout << "Taille du fichier : " << contenu.size() << " octets\n";
    std::cout << "Lecture           : " << tLecture   << " us\n";
    std::cout << "Recherche         : " << tRecherche << " us\n";

    benchmark(contenu, motCherche, 1000);
    return 0;
}
