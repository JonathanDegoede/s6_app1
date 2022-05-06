#include <cstdio>
#include <thread>
#include <mutex>
#include <queue>
#include <cstdlib>      // rand
#include <chrono>
#include <condition_variable>

// 2.1 Quest-ce qui cause la consommation abusive du processeur? Le consommateur est dans une while loop infinie sans verifications ni condition darret
// 2.2 ajout d'une pause de 10ms. Diminution ? std::this_thread::sleep_for(std::chrono::milliseconds(10)); , oui on est a genre 0.3 %
// 2.3 proposer et implementer un mechanisme pour eliminer les verifications inutiles et les delais : condition variable

namespace {
    std::queue<int> queue_;
    std::mutex      mutex_;
    std::condition_variable cv;
    bool shouldStop = false;
}

void add_to_queue(int v)
{
    // Fournit un accès synchronisé à queue_ pour l'ajout de valeurs.
    
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(v);
}

void prod()
{
    // Produit 100 nombres aléatoires de 1000 à 2000 et les ajoute
    // à une file d'attente (queue_) pour traitement.
    // À la fin, transmet "0", ce qui indique que le travail est terminé.

    for (int i = 0; i < 100; ++i)
    {
        int r = rand() % 1001 + 1000;
        add_to_queue(r);
        cv.notify_one();
    }

    add_to_queue(0);
}

void cons()
{   
    while (!shouldStop)
    	{
		std::unique_lock<std::mutex> lock(mutex_);
		cv.wait(lock, []{
			return !queue_.empty();
		});
		// On doit toujours vérifier si un objet std::queue n'est pas vide
		// avant de retirer un élément.
		    int v = queue_.front(); // Copie le premier élément de la queue.
		    
		    if(v == 0){
		    	shouldStop = true;
		    }
		    
		    queue_.pop();           // Retire le premier élément.

		    printf("Reçu: %d\n", v);
    	}
}

int main(int argc, char** argv)
{
    std::thread t_prod(prod);
    std::thread t_cons(cons);

    t_prod.join();
    t_cons.join();

    return 0;
}

