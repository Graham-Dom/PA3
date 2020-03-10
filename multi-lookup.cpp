#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <thread>
#include <semaphore.h>
#include <vector>
#include <sys/time.h>
#include <iomanip> 
#include <fcntl.h>
#include "util.h"
#include "FileHandler.h"
using namespace std;

#define BUFF_SIZE 10

string buffer[BUFF_SIZE];

static bool producers_complete(false);
static int item_count = 0;
static int num_prods;
static int num_cons;


sem_t *mtx;
sem_t *spaces_avl;
sem_t *spaces_fld;
sem_t *outfile_mtx;
sem_t *plog_mtx; 


void parse(int threadnum, ofstream* producer_log, FileHandler *handler) {
	static int in = 0;

	string line;
	while ((line  = (*handler).getLine()) != "") {

		sem_wait(plog_mtx);
		*producer_log << threadnum << "\t" << line << endl;
		sem_post(plog_mtx);

		sem_wait(spaces_avl);
				
		sem_wait(mtx);
		/* ------ CS ------- */
		buffer[in] = line;
		item_count++;
		in = (in + 1) % BUFF_SIZE;
		/* ----- END ------- */
				
		sem_post(mtx);
				

		sem_post(spaces_fld);
		
	}
	producers_complete = true;
}


void convert(ofstream* conversion_log) {
	static int out = 0;
	string to_convert;
	char converted[30];
	while (!producers_complete || item_count)
	{	
		sem_wait(spaces_fld);
			
		sem_wait(mtx);
		/* ------ CS ------- */
		to_convert = buffer[out];
		item_count--;
		out = (out + 1) % BUFF_SIZE;
		/* ----- END ------- */
		sem_post(mtx);

		sem_post(spaces_avl);
			
		
		sem_wait(outfile_mtx);
		if (dnslookup(to_convert.c_str(), converted, 30) == -1) {
			cerr << "Unable to resolve domain name " << to_convert << endl;
			*conversion_log << to_convert << ", " << endl;
		} else 
			*conversion_log << to_convert << ", " << string(converted) << endl;

		sem_post(outfile_mtx);	
	}
}

void Sem_Open(sem_t* &sem, string name, int num){
	if ((sem = sem_open(name.c_str(), O_CREAT, 0644, num)) == SEM_FAILED) {
		perror("sem_open");
		exit(EXIT_FAILURE);
	}
}

vector<ifstream*> openFiles(int argc, char **argv){
	vector<ifstream*> files;
	for (int i = 5; i < argc; ++i) {
		ifstream file(argv[i]);
		
		if (!file) 
			cerr << "Couldn't open " << argv[i] << endl; 
		else
			files.push_back(new ifstream(argv[i]));
	}

	return files;
}

int main(int argc, char *argv[]){
	sem_unlink("/mtx");
	sem_unlink("/avail");
	sem_unlink("/plog_mtx");
	sem_unlink("/filled");
	sem_unlink("/outfile");


	struct timeval start, end;
	gettimeofday(&start, NULL);

	if (argc < 7) {
		cout << "Usage :\n" << "multi-lookup <# parsing threads> <# conversion threads> <parsing log> <converter log> [ <datafile> ...]" << endl;
		return 0;
	}

	num_prods = stoi(argv[1]);
	num_cons = stoi(argv[2]);

	if (num_prods < 1 || num_cons < 1) {
		cerr << "<# thread> must be > 0" << endl;
		exit(EXIT_FAILURE);
	}

	ofstream producer_log = ofstream(argv[3], ios::in | ios::out);
	if (!producer_log) {
		cerr << "Cannot open " << argv[3] << " - aborting..." << endl;
		exit(EXIT_FAILURE);
	}
	
	ofstream conversion_log = ofstream(argv[4], ios::in | ios::out);
	if (!conversion_log) {
		cerr << "Cannot open " << argv[4] << " - aborting..." << endl;
		exit(EXIT_FAILURE);
	}

	vector<ifstream*> files = openFiles(argc, argv);


	FileHandler handler(files);

	Sem_Open(mtx, "/mtx", 1);
	Sem_Open(outfile_mtx, "/outfile", 1);
	Sem_Open(plog_mtx, "/plog_mtx", 1);
	Sem_Open(spaces_avl, "/avail", BUFF_SIZE);
	Sem_Open(spaces_fld, "/filled", 0);

	vector<thread> producers;
	for (int i = 0; i < num_prods; ++i) {
		producers.push_back(thread(parse, i, &producer_log, &handler));
	}

	vector<thread> consumers;
	for (int i = 0; i < num_cons; ++i) {
		consumers.push_back(thread(convert, &conversion_log));
	}
	
	for (int i = 0; i < num_prods; ++i){
		producers[i].join();
	}

	for (int i = 0; i < num_cons; ++i){
		consumers[i].join();
	}

	sem_unlink("/mtx");
	sem_unlink("/avail");
	sem_unlink("/plog_mtx");
	sem_unlink("/filled");
	sem_unlink("/outfile");

	for (int i = 0; i < (int) files.size(); i++){
		delete files[i];
	}

	gettimeofday(&end, NULL);
	double time_taken = (end.tv_sec - start.tv_sec) * 1e6; 
    time_taken = (time_taken + (end.tv_usec - start.tv_usec)) * 1e-6; 

    cout << "Time taken by program is : " << fixed << time_taken << setprecision(6); 
    cout << " sec" << endl;

	return 0;
}
