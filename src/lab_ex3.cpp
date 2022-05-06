#include <iostream>
#include <fstream>
#include <cstring>

// 3.1 Lancer et taper 2 , quest-ce que vous obtenez ? On obtient une erreur disant qu'on ne peut pas ouvrir 2 et qu'on doit appuyer sur CTRL-D pour EOF.
// 3.2 CTRL-D signifie : CTRL_D is just a signal saying that this is the end of a text stream. You do not end a file with it, you end your input stream by typing it.
// 3.3 Nouvelle facon echo 4 | ./lab_ex3 : Using stdin (press CTRL-D for EOF). 8
// 3.4 echo 2 | ./lab_ex3 > ./times_2.txt Pourquoi on ne voit pas le msg dinfo using stdin : Pck cerr nest pas output dans le fichier par defaut. On peut utiliser 2> pour le forcer.
// 3.5 pq on fait un | et non > . Pck > sert a ecrire dans un fichier et | sert a passer le resultat de la premiere commande en entree a la seconde.
// 3.6 quarrive til si cat sans argument :  it waits for an input from your keyboard. il faut mettre un CTRL-D (eof) pour le terminer.
// 3.7 fichier contenant les valeurs a entrer
// Donner 2 facons de lancer le programmer. : cat entree.txt | ./lab_ex3, ./lab_ex3 entree.txt
// Memes resultats ? : oui
// 3.8 multiplier par 4 sans modifier le code? cat entree.txt | ./lab_ex3 | ./lab_ex3
// 3.9 prendre comme premier argument le facteur de multiplication
// 3.10 ajouter une valeur sans lediteu de texte echo 5343223 >> entree.txt 



int main(int argc, char** argv)
{
    std::ifstream file_in;

    if (argc >= 3 && (strcmp(argv[2], "-") != 0)) {
        file_in.open(argv[2]);
        if (file_in.is_open()) {
            std::cin.rdbuf(file_in.rdbuf());
            std::cerr << "Using " << argv[2] << "..." << std::endl;
        } else {
            std::cerr   << "Error: Cannot open '"
                        << argv[2] 
                        << "', using stdin (press CTRL-D for EOF)." 
                        << std::endl;
        }
    } else {
        std::cerr << "Using stdin (press CTRL-D for EOF)." << std::endl;
    }

    while (!std::cin.eof()) {

        std::string line;

        std::getline(std::cin, line);
        if (!line.empty()) {
            int val = atoi(line.c_str());
            int fact = atoi(argv[1]);
            std::cout << val * fact << std::endl;
        }
    }

    if (file_in.is_open()) {
        file_in.close();
    }

    return 0;
}
