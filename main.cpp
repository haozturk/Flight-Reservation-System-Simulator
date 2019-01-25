#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <vector>
#include "Seat.h"
#include <fstream>
#include <sstream>

using namespace std;


void *clientRunner(void *param); //Client thread runner function prototype
void *serverRunner(void *param); //Server thread runner function prototype

int CLIENT_NUM; //Number of client threads
int SERVER_NUM; //Number of server threads

//Vector which holds pointers to the Seat objects
vector<Seat*> seatVec;

//if a client books a seat, its index becomes true
bool isBooked[100];

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;

ofstream myfile;


string sharedMem[100];


int main(int argc, char* argv[]){

	if(argc != 2){
		cout << "Usage: ./executable <number of seats>" << endl;
	}
	else{
		//Error checking for the argument
		if(atoi(argv[1])<50 || atoi(argv[1]) >100){
			cout << "The argument should be between 50 and 100" << endl;
		}
		else{

			myfile.open ("output.txt");
			Seat* s;

			srand(time(nullptr)); //take the current time as seed for random number generator


			int seatNum = atoi(argv[1]);
			CLIENT_NUM = seatNum;
			SERVER_NUM = CLIENT_NUM;

			//Fill the seat vector and initilize the names of the seats
			for(int i=0;i<seatNum;i++){
				string seatName = "Seat" + to_string(i+1);
				s = new Seat(seatName);
				seatVec.push_back(s);
			}


			//Initially no client books a seat.
			for(int i=0;i<CLIENT_NUM;i++)
				isBooked[i] = false;

			cout << "Number of total seats: " << atoi(argv[1]) << endl;
			myfile << "Number of total seats: " << atoi(argv[1]) << endl;

			//Create arrays which will hold server and client objects
			pthread_t clients[CLIENT_NUM];
			pthread_t servers[SERVER_NUM];


			for(int i=0;i<CLIENT_NUM;i++){

				int* clientId = new int(i); //allocate memory for each client id.
				int* serverId = new int(i); //allocate memory for each server id.

				//Create client and server threads by giving their id's as argument
				pthread_create(&clients[i],0,clientRunner,clientId); 
				pthread_create(&servers[i],0,serverRunner,serverId);
			}

			//Wait for the client and server threads
			for(int i=0;i<CLIENT_NUM;i++){
				pthread_join(clients[i],NULL);
				pthread_join(servers[i],NULL);
			}
	
			if(seatVec.size() == 0){
				cout << "All seats are reserved" << endl;
				myfile << "All seats are reserved" << endl;
			}

			myfile.close();

			for(int i=0;i<seatVec.size();i++){
				delete seatVec[i];
			}
		}
	}

		return 0;
}

void *clientRunner(void *param){


	//sleep for a random time between 50 and 200 ms.
	double t = (rand() % 151) + 50;
	sleep(t/1000);

	//While a client selects a seat, another client cannot select
	pthread_mutex_lock(&m);

	//Select a random number between 0 and available seat size
	int randSeat = rand() % seatVec.size(); //indexed from 0
	//Write the selected number to the shared memory
	sharedMem[*((int *)param)] = seatVec[randSeat]->name;
	//Erase the selected seat
	seatVec.erase(seatVec.begin() + randSeat);
	//Client selected the seat, Now server can start execution
	isBooked[*((int *)param)] = true;


	pthread_mutex_unlock(&m);
	pthread_exit(0);

}

/////// I WAS THINKING THE AVAILABILITY ISSUES
void *serverRunner(void *param){

	//Do busy waiting, when client books a seat stop busy waiting
	while(!isBooked[*((int *)param)]);

	pthread_mutex_lock(&m2);
	
	//Write to the output file
	myfile << "Client" << (*((int *)param) + 1) << " reserves " << sharedMem[*((int *)param)] << endl;
	cout << "Client" << (*((int *)param) + 1) << " reserves " << sharedMem[*((int *)param)] << endl; 


	pthread_mutex_unlock(&m2);

	pthread_exit(0);
}
