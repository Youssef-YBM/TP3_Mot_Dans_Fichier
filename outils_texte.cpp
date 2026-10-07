// ============================================================
//  TP3 - Outils sur un fichier texte
//    1) Plus longue sous-chaine du mot saisi presente dans le fichier
//    2) Recherche puis remplacement de toutes les occurrences
//  Deux versions pour chaque outil : STL (find) et from scratch
//  Temps mesures avec <chrono>, statistiques avec std::vector
//  Recherche insensible a la casse.
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
const size_t NPOS = std::string::npos;

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

bool lireFichier(const std::string& nomFichier, std::string& contenu) {
    std::ifstream fichier(nomFichier);
    if (!fichier) return false;
    std::stringstream buffer;
    buffer << fichier.rdbuf();
    contenu = buffer.str();
    return true;
}

bool ecrireFichier(const std::string& nomFichier, const std::string& contenu) {
    std::ofstream fichier(nomFichier);
    if (!fichier) return false;
    fichier << contenu;
    return true;
}

// Repete f() 'repetitions' fois et affiche min / mediane / moyenne / max
template <typename Fonction>
void benchmark(const std::string& etiquette, Fonction f, int repetitions) {
    std::vector<double> temps;
    temps.reserve(repetitions);
    for (int i = 0; i < repetitions; ++i)
        temps.push_back(mesurerUs(f));

    std::sort(temps.begin(), temps.end());
    double moyenne = std::accumulate(temps.begin(), temps.end(), 0.0) / temps.size();
    std::cout << "  " << etiquette << " : min " << temps.front()
              << " | mediane " << temps[temps.size() / 2]
              << " | moyenne " << moyenne
              << " | max " << temps.back() << " us\n";
}

// ------------------------------------------------------------
//  Recherche d'une sous-chaine a partir de 'debut'
//  (une version STL, une version from scratch)
//  Le texte est deja en minuscules pour la version STL.
// ------------------------------------------------------------

size_t trouverSTL(const std::string& texteLower, const std::string& motifLower, size_t debut = 0) {
    return texteLower.find(motifLower, debut);
}

size_t trouverScratch(const std::string& texte, const std::string& motif, size_t debut = 0) {
    size_t n = texte.size(), m = motif.size();
    if (m == 0 || m > n) return NPOS;

    for (size_t i = debut; i + m <= n; ++i) {
        size_t j = 0;
        while (j < m &&
               std::tolower((unsigned char)texte[i + j]) ==
               std::tolower((unsigned char)motif[j])) {
            ++j;
        }
        if (j == m) return i;
    }
    return NPOS;
}

// ============================================================
//  OUTIL 1 : plus longue sous-chaine du mot presente dans le texte
// ============================================================

struct ResultatLongue {
    std::string sousChaine;   // la plus longue sous-chaine du mot trouvee dans le fichier
    size_t position = NPOS;   // sa premiere position dans le fichier
};

// Principe : si une sous-chaine de longueur L existe dans le texte,
// il en existe aussi une de longueur L-1 (propriete monotone).
// On cherche donc la plus grande longueur L par dichotomie ;
// pour chaque L on teste toutes les sous-chaines du mot de taille L.
template <typename Trouver>
ResultatLongue plusLongueGenerique(const std::string& mot, Trouver trouver) {
    ResultatLongue meilleur;
    int bas = 1, haut = (int)mot.size();

    while (bas <= haut) {
        int L = (bas + haut) / 2;
        bool trouve = false;
        for (size_t s = 0; s + L <= mot.size() && !trouve; ++s) {
            std::string candidat = mot.substr(s, L);
            size_t pos = trouver(candidat);
            if (pos != NPOS) {
                trouve = true;
                meilleur.sousChaine = candidat;
                meilleur.position   = pos;
            }
        }
        if (trouve) bas = L + 1; else haut = L - 1;
    }
    return meilleur;
}

ResultatLongue plusLongueSTL(const std::string& texteLower, const std::string& mot) {
    return plusLongueGenerique(mot, [&](const std::string& c) {
        return trouverSTL(texteLower, toLower(c));
    });
}

ResultatLongue plusLongueScratch(const std::string& texte, const std::string& mot) {
    return plusLongueGenerique(mot, [&](const std::string& c) {
        return trouverScratch(texte, c);
    });
}

void outilPlusLongue(const std::string& contenu) {
    std::string mot;
    std::cout << "Mot a analyser : ";
    std::getline(std::cin, mot);
    if (mot.empty()) { std::cerr << "Mot vide.\n"; return; }

    std::string contenuLower = toLower(contenu);

    ResultatLongue rSTL, rScratch;
    double tSTL     = mesurerUs([&] { rSTL     = plusLongueSTL(contenuLower, mot); });
    double tScratch = mesurerUs([&] { rScratch = plusLongueScratch(contenu, mot); });

    if (rSTL.sousChaine.empty()) {
        std::cout << "Aucune lettre de \"" << mot << "\" n'apparait dans le fichier.\n";
    } else {
        size_t pos = rSTL.position;
        size_t L = rSTL.sousChaine.size();
        size_t debut = (pos >= 30) ? pos - 30 : 0;
        size_t fin   = std::min(pos + L + 30, contenu.size());

        std::cout << "\nPlus longue sous-chaine de \"" << mot << "\" presente dans le fichier :\n";
        std::cout << "  \"" << rSTL.sousChaine << "\" (longueur " << L << ")\n";
        std::cout << "  Premiere position : " << pos << "\n";
        std::cout << "  Contexte : ..." << contenu.substr(debut, fin - debut) << "...\n";
        if (L == mot.size()) std::cout << "  (le mot entier existe dans le fichier)\n";
    }

    std::cout << "\nLes deux versions concordent : "
              << (rSTL.sousChaine.size() == rScratch.sousChaine.size() ? "OUI" : "NON") << "\n";
    std::cout << "\n--- Temps d'execution (1 passage) ---\n";
    std::cout << "  STL     : " << tSTL     << " us\n";
    std::cout << "  Scratch : " << tScratch << " us\n";

    const int repetitions = 5;
    std::cout << "\n--- Benchmark (" << repetitions << " repetitions) ---\n";
    benchmark("STL    ", [&] { plusLongueSTL(contenuLower, mot); },   repetitions);
    benchmark("Scratch", [&] { plusLongueScratch(contenu, mot); },    repetitions);
}

// ============================================================
//  OUTIL 2 : remplacer toutes les occurrences
// ============================================================

// Retourne le texte modifie et le nombre de remplacements
struct ResultatRemplacement {
    std::string texte;
    size_t nombre = 0;
};

ResultatRemplacement remplacerSTL(const std::string& contenu, const std::string& ancien,
                                  const std::string& nouveau) {
    ResultatRemplacement r;
    std::string contenuLower = toLower(contenu);
    std::string ancienLower  = toLower(ancien);

    size_t dernier = 0, pos;
    while ((pos = contenuLower.find(ancienLower, dernier)) != NPOS) {
        r.texte.append(contenu, dernier, pos - dernier);
        r.texte += nouveau;
        dernier = pos + ancien.size();
        ++r.nombre;
    }
    r.texte.append(contenu, dernier, NPOS);
    return r;
}

ResultatRemplacement remplacerScratch(const std::string& contenu, const std::string& ancien,
                                      const std::string& nouveau) {
    ResultatRemplacement r;
    size_t dernier = 0, pos;
    while ((pos = trouverScratch(contenu, ancien, dernier)) != NPOS) {
        for (size_t k = dernier; k < pos; ++k) r.texte += contenu[k];
        r.texte += nouveau;
        dernier = pos + ancien.size();
        ++r.nombre;
    }
    for (size_t k = dernier; k < contenu.size(); ++k) r.texte += contenu[k];
    return r;
}

void outilRemplacer(const std::string& contenu, const std::string& nomFichier) {
    std::string ancien, nouveau;
    std::cout << "Mot / sous-chaine a remplacer : ";
    std::getline(std::cin, ancien);
    if (ancien.empty()) { std::cerr << "Chaine vide.\n"; return; }
    std::cout << "Remplacer par : ";
    std::getline(std::cin, nouveau);

    ResultatRemplacement rSTL, rScratch;
    double tSTL     = mesurerUs([&] { rSTL     = remplacerSTL(contenu, ancien, nouveau); });
    double tScratch = mesurerUs([&] { rScratch = remplacerScratch(contenu, ancien, nouveau); });

    std::cout << "\n" << rSTL.nombre << " occurrence(s) de \"" << ancien
              << "\" remplacee(s) par \"" << nouveau << "\".\n";
    std::cout << "Les deux versions donnent le meme resultat : "
              << (rSTL.texte == rScratch.texte ? "OUI" : "NON") << "\n";

    std::cout << "\n--- Temps d'execution (1 passage) ---\n";
    std::cout << "  STL     : " << tSTL     << " us\n";
    std::cout << "  Scratch : " << tScratch << " us\n";

    const int repetitions = 5;
    std::cout << "\n--- Benchmark (" << repetitions << " repetitions) ---\n";
    benchmark("STL    ", [&] { remplacerSTL(contenu, ancien, nouveau); },     repetitions);
    benchmark("Scratch", [&] { remplacerScratch(contenu, ancien, nouveau); }, repetitions);

    if (rSTL.nombre == 0) return;

    // Par defaut, on ne touche pas au fichier original
    std::string choix;
    std::cout << "\nEcraser le fichier original ? (o/N) : ";
    std::getline(std::cin, choix);
    std::string sortie = (choix == "o" || choix == "O") ? nomFichier : nomFichier + ".modifie.txt";

    if (ecrireFichier(sortie, rSTL.texte))
        std::cout << "Resultat ecrit dans : " << sortie << "\n";
    else
        std::cerr << "Erreur : impossible d'ecrire " << sortie << "\n";
}

// ------------------------------------------------------------
//  Programme principal : menu
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
    std::cout << "Fichier charge : " << contenu.size() << " octets\n";

    std::cout << "\n1) Plus longue sous-chaine d'un mot presente dans le fichier\n"
              << "2) Rechercher et remplacer toutes les occurrences\n"
              << "Choix : ";
    std::string choix;
    std::getline(std::cin, choix);

    if (choix == "1")      outilPlusLongue(contenu);
    else if (choix == "2") outilRemplacer(contenu, nomFichier);
    else { std::cerr << "Choix invalide.\n"; return 1; }

    return 0;
}
