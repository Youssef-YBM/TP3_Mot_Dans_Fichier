// ============================================================
//  TP3 - Benchmark comparatif des deux versions de recherche
//    A : std::string::find (apres mise en minuscules)
//    B : algorithme naif "from scratch"
//  Mesures avec <chrono>, resultats stockes dans std::vector
//  Usage : benchmark texte.txt   (sortie : tableau + CSV)
// ============================================================
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <cctype>

using Horloge = std::chrono::steady_clock;

// ------------------------------------------------------------
//  Les deux algorithmes (meme logique que les programmes TP)
// ------------------------------------------------------------

std::string toLower(const std::string& s) {
    std::string res = s;
    std::transform(res.begin(), res.end(), res.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return res;
}

long rechercherString(const std::string& contenu, const std::string& mot) {
    size_t pos = toLower(contenu).find(toLower(mot));
    return pos == std::string::npos ? -1 : (long)pos;
}

long rechercherScratch(const std::string& texte, const std::string& motif) {
    if (motif.empty() || motif.size() > texte.size()) return -1;
    int n = (int)texte.size(), m = (int)motif.size();
    for (int i = 0; i <= n - m; ++i) {
        int j = 0;
        while (j < m && std::tolower((unsigned char)texte[i + j]) ==
                        std::tolower((unsigned char)motif[j])) ++j;
        if (j == m) return i;
    }
    return -1;
}

// ------------------------------------------------------------
//  Outils de mesure
// ------------------------------------------------------------

struct Stats { double min, moyenne, mediane, ecartType, max; };

Stats calculerStats(std::vector<double> t) {
    std::sort(t.begin(), t.end());
    double moy = std::accumulate(t.begin(), t.end(), 0.0) / t.size();
    double var = std::accumulate(t.begin(), t.end(), 0.0,
                     [moy](double a, double x){ return a + (x - moy) * (x - moy); }) / t.size();
    return { t.front(), moy, t[t.size() / 2], std::sqrt(var), t.back() };
}

template <typename Fonction>
Stats mesurer(Fonction f, int repetitions) {
    std::vector<double> temps;
    temps.reserve(repetitions);
    volatile long sink = 0;
    for (int i = 0; i < repetitions; ++i) {
        auto d = Horloge::now();
        sink = f();
        auto e = Horloge::now();
        temps.push_back(std::chrono::duration<double, std::micro>(e - d).count());
    }
    (void)sink;
    return calculerStats(temps);
}

bool lireFichier(const std::string& nom, std::string& contenu) {
    std::ifstream f(nom);
    if (!f) return false;
    std::stringstream b; b << f.rdbuf();
    contenu = b.str();
    return true;
}

// Repete le texte 'facteur' fois pour simuler de gros fichiers
std::string repeter(const std::string& s, int facteur) {
    std::string r;
    r.reserve(s.size() * facteur);
    for (int i = 0; i < facteur; ++i) r += s;
    return r;
}

// ------------------------------------------------------------
//  Programme principal
// ------------------------------------------------------------

int main(int argc, char* argv[]) {
    std::string base;
    std::string nom = (argc >= 2) ? argv[1] : "texte.txt";
    if (!lireFichier(nom, base)) {
        std::cerr << "Erreur : impossible d'ouvrir " << nom << "\n";
        return 1;
    }

    // Mots : trouve au debut, trouve a la fin, absent
    // (le mot "absent" est le pire cas : parcours complet du texte)
    const std::vector<std::pair<std::string, std::string>> mots = {
        {"debut",  "Bonjour"},
        {"fin",    "Fin du fichier de test"},
        {"absent", "zzzxyz"}
    };
    const std::vector<int> facteurs = {1, 10, 100, 1000};
    const int repetitions = 200;

    // Absent du texte replique : le mot "fin" est trouve des la 1ere copie
    // pour tous les facteurs ; seul "absent" force le parcours complet.

    std::cout << "Fichier : " << nom << " (" << base.size() << " octets), "
              << repetitions << " repetitions par mesure\n\n";
    std::cout << "CSV\nversion,mot,taille_octets,min_us,moyenne_us,mediane_us,ecart_type_us,max_us\n";
    std::cout << std::fixed << std::setprecision(2);

    for (int facteur : facteurs) {
        std::string texte = repeter(base, facteur);
        for (const auto& [etiquette, mot] : mots) {
            Stats a = mesurer([&] { return rechercherString (texte, mot); }, repetitions);
            Stats b = mesurer([&] { return rechercherScratch(texte, mot); }, repetitions);
            auto ligne = [&](const char* v, const Stats& s) {
                std::cout << v << "," << etiquette << "," << texte.size() << ","
                          << s.min << "," << s.moyenne << "," << s.mediane << ","
                          << s.ecartType << "," << s.max << "\n";
            };
            ligne("string",  a);
            ligne("scratch", b);
        }
    }
    return 0;
}
