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

#define BUFF_SIZE 10

std::string buffer[BUFF_SIZE];

static bool producers_complete(false);
static int item_count = 0;
static int num_prods;
static int num_cons;


sem_t *mtx;
sem_t *spaces_avl;
sem_t *spaces_fld;
sem_t *outfile_mtx;
sem_t *plog_mtx; 


void parse(int threadnum, std::ofstream* producer_log, FileHandler *handler);
void convert(std::ofstream* conversion_log);
void Sem_Open(sem_t* &sem, std::string name, int num);
std::vector<std::ifstream*> openFiles(int argc, char **argv);
int main(int argc, char *argv[]);