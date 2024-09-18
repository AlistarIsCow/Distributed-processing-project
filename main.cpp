#include "main.h"
#include "watek.h"


pthread_t threadKom; 
int ts = 0, mechs = 0, size, rank;
pthread_mutex_t ts_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
std::vector<data> queue;
bool got_RELEASE = false, got_all_ACK = false;

bool queue_sorter(data const& lhs, data const& rhs)
{
    if (lhs.ts != rhs.ts){
        return lhs.ts < rhs.ts;
    }
    return lhs.rank < rhs.rank;
}

void sendPacket(data *pkt, int destination, int tag)
{
    MPI_Send(pkt, sizeof(data), MPI_BYTE, destination, tag, MPI_COMM_WORLD);

}

void sendIPacket(data *pkt, int destination, int tag)
{
    MPI_Request request;
    MPI_Isend(pkt, sizeof(data), MPI_BYTE, destination, tag, MPI_COMM_WORLD, &request);
}


int main(int argc, char **argv)
{
    //MPI_Init(&argc, &argv);  
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    pthread_create(&threadKom, NULL, startKomWatek, 0);
    MPI_Status status;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 3)
    {
        MPI_Finalize();
        printf("Za malo argumentow\n");
        exit(0);
    }
    
    ts = rank * 10;
    int K = atoi(argv[1]); // liczba dokow
    int M = atoi(argv[2]); // liczba mechanikow
    data message, dummy, tmp_msg;
    message.rank = rank;
    int time, tmp_mechs, tmp_docks;
    bool is_enough, wait_release;
     
    srand(rank);
    
    while (true)
    {
        //mechs = rand() % M + 1; // liczba potrzebnych mechanikow
        mechs = 5; // liczba potrzebnych mechanikow
        time = rand() % 5 + 3;  // czas potrzebny do zepsucia sie statku
        //time = 5;  // czas potrzebny do zepsucia siê statku
        tmp_mechs = 0;
        tmp_docks = 0;
        is_enough = false;
        wait_release = false;
        message.ts = ts;
        message.mechs = mechs;
        sleep(time);
        
        pthread_mutex_lock(&ts_mutex);
        ts++;
        message.ts = ts;
        printf("[%d] Zegar: %d Zepsulem sie, zadam mechanikow %d i doku, wysylam REQ\n", rank, ts, mechs);
        for (int i = 0; i < size; i++)
        {
            if (i!=rank) {sendPacket(&message, i, REQ);}     
        }
        queue.push_back(message);
        pthread_mutex_unlock(&ts_mutex);
        
	while(!got_all_ACK){
	}
	got_all_ACK = false;
        std::sort(queue.begin(), queue.end(), &queue_sorter);
        while (!is_enough){
	    printf("[%d] Zegar: %d ",rank, ts);
	    for (int i = 0; i < queue.size(); i++){
	        printf("%d.%d ", queue[i].rank, queue[i].ts);
	    }
	    printf("\n");
            for (int i = 0; i < queue.size(); i++){
                //pthread_mutex_lock(&queue_mutex);
                std::sort(queue.begin(), queue.end(), &queue_sorter);
                got_RELEASE = false;
                if (queue[i].rank != rank){
                    tmp_docks++;
                    tmp_mechs += queue[i].mechs;
                    //pthread_mutex_unlock(&queue_mutex);
                }
                else if (queue[i].rank == rank){
                    if (tmp_docks <= K-1){
                        ts++;
                        printf("[%d] Zegar: %d Dostalem dok\n", rank, ts);
                        if (tmp_mechs <= M - mechs){
                            ts++;
                            printf("[%d] Zegar: %d Dostalem mechanikow\n", rank, ts);
                            printf("[%d] Zegar: %d Przede mna jest %d mechanikow\n", rank, ts, tmp_mechs);
                            is_enough = true;
                        }
                        else{
                            //pthread_mutex_unlock(&queue_mutex);
                            printf("[%d] Zegar: %d Nie dostalem mechanikow, czekam na RELEASE, \n", rank, ts);
                            printf("[%d] Zegar: %d Jestem %d w kolejce, \n", rank, ts, i+1);
                            printf("[%d] Zegar: %d Przede mna jest %d mechanikow\n", rank, ts, tmp_mechs);
                            while (!got_RELEASE){                          
                            }
                            ts++;
                            tmp_mechs = 0;
                            tmp_docks = 0;
                            break;
                        }
                    }
                    else{
                        //pthread_mutex_unlock(&queue_mutex);
                        printf("[%d] Zegar: %d Nie dostalem doku, czekam na RELEASE\n", rank, ts);
                        printf("[%d] Zegar: %d Jestem %d w kolejce, \n", rank, ts, i+1);
                        printf("[%d] Zegar: %d Przede mna jest %d mechanikow\n", rank, ts, tmp_mechs);
                        while (!got_RELEASE){                          
                        }
                        ts++;
                        tmp_mechs = 0;
                        tmp_docks = 0;
                        break;
                    }
                }
            }
        }
        //pthread_mutex_unlock(&queue_mutex);
        ts++;
        printf("[%d] Zegar: %d Naprawiam sie\n", rank, ts);
        sleep(time);
        
        printf("[%d] Zegar: %d Naprawilem siê, rozsylam RELEASE\n", rank, ts);
        pthread_mutex_lock(&ts_mutex);
	message.ts = ts;
        for (int i = 0; i < size; i++){
           if(rank!=i){
           	sendPacket(&message, i, RELEASE);
           }
        }
        pthread_mutex_unlock(&ts_mutex);
        
        //pthread_mutex_lock(&queue_mutex);
        for (int i = 0 ; i < queue.size(); i++){
	    if (queue[i].rank == rank){
	    	queue.erase(queue.begin()+i);
	    	break;
	    }       	
        }
        std::sort(queue.begin(), queue.end(), &queue_sorter);
        //pthread_mutex_unlock(&queue_mutex);
        ts++;
        printf("[%d] Zegar: %d Zwolnilem zasoby\n", rank, ts);
    }

    MPI_Finalize();
    return 0;
}
