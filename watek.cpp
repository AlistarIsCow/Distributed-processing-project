#include "main.h"
#include "watek.h"


void *startKomWatek(void *ptr)
{
    MPI_Status status;
    data pakiet, msg;
    int ACKs = 0;

    while(true){
        MPI_Recv(&pakiet, sizeof(data), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        switch (status.MPI_TAG){

        case REQ:
		pthread_mutex_lock(&ts_mutex);
		queue.push_back(pakiet);
		std::sort(queue.begin(), queue.end(), &queue_sorter);
		ts = std::max(ts, pakiet.ts) + 1;
		msg.ts = ts;
		msg.mechs = mechs;
		pthread_mutex_unlock(&ts_mutex);
		sendPacket(&msg, status.MPI_SOURCE, ACK);
        break;
        case RELEASE:
        	pthread_mutex_lock(&ts_mutex);
        	//pthread_mutex_lock(&queue_mutex);
        	for (int i = 0 ; i < queue.size(); i++){
			if (queue[i].rank == status.MPI_SOURCE){
				queue.erase(queue.begin()+i);
				break;
			}
			      	
        	}
        	ts = std::max(ts, pakiet.ts) + 1;
        	printf("[%d] Zegar: %d Dostalem RELEASE\n", rank, ts);
        	ts++;
        	pthread_mutex_unlock(&ts_mutex);
        	std::sort(queue.begin(), queue.end(), &queue_sorter);
        	got_RELEASE = true;
        	//pthread_mutex_unlock(&queue_mutex);
        	
        break;
        case ACK:
       	pthread_mutex_lock(&ts_mutex);
        	ACKs++;
        	ts = std::max(ts, pakiet.ts) + 1;
        	if(ACKs == size - 1){
        		ts++;
        		printf("[%d] Zegar: %d Dostalem wszystkie ACK \n", rank, ts);
        		ts++;
        		ACKs =  0;
        		got_all_ACK = true;
        	}
        	pthread_mutex_unlock(&ts_mutex);
        	
        break;
        }
    }
    
}