// ============================================================
//  TP3 - Recherche d'une sous-chaine : liste de tous les mots
//  du fichier qui la contiennent (ex. "comp" -> compteur, compte...)
//  Deux versions : STL (std::string::find) et from scratch
//  Temps mesures avec <chrono>, resultats stockes dans std::vector
// ============================================================
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <cctype>

using Horloge = std::chrono::steady_clock;

// ------------------------------------------------------------
//  Utilitaires
// ------------------------------------------------------------

std::string toLower(const std::string& s) {
    std::string res = s;
    std::transform(res.begin(), res.end(), res.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return res;
}

template <typename Fonction>
double mesurerUs(Fonction f) {
    auto debut = Horloge::now();
    f();
    auto fin = Horloge::now();
    return std::chrono::duration<double, std::micro>(fin - debut).count();
}

// ------------------------------------------------------------
//  Lecture et decoupage du texte en mots
// ------------------------------------------------------------

bool lireFichier(const std::string& nomFichier, std::string& contenu) {
    std::ifstream fichier(nomFichier);
    if (!fichier) return false;
    std::stringstream buffer;
    buffer << fichier.rdbuf();
    contenu = buffer.str();
    return true;
}

// Un mot = suite de lettres/chiffres/'_' ; tout le reste est separateur
std::vector<std::string> extraireMots(const std::string& contenu) {
    std::vector<std::string> mots;
    std::string courant;
    for (char c : contenu) {
        if (std::isalnum((unsigned char)c) || c == '_') {
            courant += c;
        } else if (!courant.empty()) {
            mots.push_back(courant);
            courant.clear();
        }
    }
    if (!courant.empty()) mots.push_back(courant);
    return mots;
}

// ------------------------------------------------------------
//  Version 1 : STL (std::string::find)
// ------------------------------------------------------------

std::vector<std::string> motsContenantSTL(const std::vector<std::string>& mots,
                                          const std::string& sousChaine) {
    std::vector<std::string> resultat;
    std::string motif = toLower(sousChaine);
    for (const std::string& mot : mots) {
        if (toLower(mot).find(motif) != std::string::npos)
            resultat.push_back(mot);
    }
    return resultat;
}

// ------------------------------------------------------------
//  Version 2 : from scratch (comparaison caractere par caractere)
// ------------------------------------------------------------

bool contientScratch(const std::string& texte, const std::string& motif) {
    int n = (int)texte.size();
    int m = (int)motif.size();
    if (m == 0 || m > n) return false;

    for (int i = 0; i <= n - m; ++i) {
        int j = 0;
        while (j < m &&
               std::tolower((unsigned char)texte[i + j]) ==
               std::tolower((unsigned char)motif[j])) {
            ++j;
        }
        if (j == m) return true;
    }
    return false;
}

std::vector<std::string> motsContenantScratch(const std::vector<std::string>& mots,
                                              const std::string& sousChaine) {
    std::vector<std::string> resultat;
    for (const std::string& mot : mots) {
        if (contientScratch(mot, sousChaine))
            resultat.push_back(mot);
    }
    return resultat;
}

// ------------------------------------------------------------
//  Affichage : mots distincts (minuscules) avec leur nombre d'occurrences
// ------------------------------------------------------------

void afficherResultat(const std::vector<std::string>& trouves, const std::string& sousChaine) {
    std::cout << "\nSous-chaine \"" << sousChaine << "\" : "
              << trouves.size() << " mot(s) trouve(s) au total.\n";
    if (trouves.empty()) return;

    std::vector<std::string> tries;
    for (const std::string& m : trouves) tries.push_back(toLower(m));
    std::sort(tries.begin(), tries.end());

    std::cout << "Mots distincts :\n";
    for (size_t i = 0; i < tries.size();) {
        size_t j = i;
        while (j < tries.size() && tries[j] == tries[i]) ++j;
        std::cout << "  - " << tries[i] << " (x" << (j - i) << ")\n";
        i = j;
    }
}

// ------------------------------------------------------------
//  Benchmark : repetition, statistiques avec std::vector
// ------------------------------------------------------------

template <typename Fonction>
void benchmark(const std::string& etiquette, Fonction f, int repetitions) {
    std::vector<double> temps;
    temps.reserve(repetitions);

    volatile size_t sink = 0;  // empeche l'optimiseur de supprimer l'appel
    for (int i = 0; i < repetitions; ++i)
        temps.push_back(mesurerUs([&] { sink = f().size(); }));
    (void)sink;

    std::sort(temps.begin(), temps.end());
    double moyenne = std::accumulate(temps.begin(), temps.end(), 0.0) / temps.size();

    std::cout << etiquette << " : min " << temps.front()
              << " us | mediane " << temps[temps.size() / 2]
              << " us | moyenne " << moyenne
              << " us | max " << temps.back() << " us\n";
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

    std::string contenu;
    if (!lireFichier(nomFichier, contenu)) {
        std::cerr << "Erreur : impossible d'ouvrir " << nomFichier << "\n";
        return 1;
    }

    // Decoupage en mots (chronometre)
    std::vector<std::string> mots;
    double tDecoupe = mesurerUs([&] { mots = extraireMots(contenu); });
    std::cout << "Fichier : " << contenu.size() << " octets, "
              << mots.size() << " mots (decoupage : " << tDecoupe << " us)\n";

    std::string sousChaine;
    std::cout << "Sous-chaine a chercher : ";
    std::getline(std::cin, sousChaine);
    if (sousChaine.empty()) {
        std::cerr << "Sous-chaine vide.\n";
        return 1;
    }

    // Une execution de chaque version, chronometree
    std::vector<std::string> resSTL, resScratch;
    double tSTL     = mesurerUs([&] { resSTL     = motsContenantSTL(mots, sousChaine); });
    double tScratch = mesurerUs([&] { resScratch = motsContenantScratch(mots, sousChaine); });

    afficherResultat(resSTL, sousChaine);

    std::cout << "\n--- Verification ---\n";
    std::cout << "Les deux versions donnent le meme resultat : "
              << (resSTL == resScratch ? "OUI" : "NON") << "\n";

    std::cout << "\n--- Temps d'execution (1 passage) ---\n";
    std::cout << "STL     : " << tSTL     << " us\n";
    std::cout << "Scratch : " << tScratch << " us\n";

    const int repetitions = 1000;
    std::cout << "\n--- Benchmark (" << repetitions << " repetitions) ---\n";
    benchmark("STL    ", [&] { return motsContenantSTL(mots, sousChaine); },     repetitions);
    benchmark("Scratch", [&] { return motsContenantScratch(mots, sousChaine); }, repetitions);

    return 0;
}
