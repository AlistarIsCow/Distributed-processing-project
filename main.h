#ifndef MAIN_H
#define MAIN_H

#include <mpi.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <algorithm>
#include <vector>
#include <mutex>
#include <iostream>

#define REQ 1
#define ACK 2
#define RELEASE 3

struct data
{
    int ts;    // timestamp (zegar lamporta)
    int rank;  // id w¹tku
    int mechs; // liczba potrzebnych mechaników
};

extern pthread_t threadKom; 
extern int ts, mechs, rank, size;
extern pthread_mutex_t ts_mutex, queue_mutex;
extern std::vector<data> queue;
extern bool got_RELEASE, got_all_ACK;

void sendPacket(data *pkt, int destination, int tag);

void sendIPacket(data *pkt, int destination, int tag);

bool queue_sorter(data const& lhs, data const& rhs);

#endif