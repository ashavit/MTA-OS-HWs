#include<stdio.h>
#include<stdlib.h>
#include <sys/wait.h>     // for wait()
#include <unistd.h>       // for sleep()
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <semaphore.h>
#include <pthread.h>

#undef isblank  // might be a problem for the function isblank() so we undef it
//-------------------------------------------
// Time delays - All numbers in tenth-of-second
#define TRY_TIME (5 * (1 + (int)pthread_self() % 2))
#define STORE_GET_TIME (3 * (1 + (int)pthread_self() % 3))
#define STORE_RETURN_TIME (1 + (int)pthread_self() % 3)
#define PAY_TIME 3
#define DELAY_FACTOR 1 //multiply the delay for all

//---------------------------------
// *** SEMAPHORES
#define SEM_FITTING 3
#define SEM_STORE 0
#define SEM_PAY 2
//---------------------------------
// semaphores as global variables
sem_t* sem_fitting;
sem_t* sem_store;
sem_t* sem_pay;

//---------------------------------
struct try_n_buy
{
	char name[10];
	char model[10];
	int size;
	int buy;
};
//---------------------------------

void* do_shopper(void *ptr);
int read_next_try(char* buf, struct try_n_buy* try);
void enter_fitting(char* name);
void exit_fitting(char* name);
void go_try(struct try_n_buy try);
void get_from_store(struct try_n_buy try);
void return_to_store(struct try_n_buy try);
void go_pay(struct try_n_buy try);
int tenth_sleep(int n);
void open_all_sem(void);
void close_all_sem(void);

//===========================================================


//---------- MAIN ----------------
int main(int argc, char* argv[])
{
	FILE* fp;
	int ret;
	size_t char_count=80; // this is ignored actually

	char* shoppers[100] = { NULL };
	pthread_t threads[100];
	int thread_count = 0;

	if (argc != 2) {
		fprintf(stderr, "Wrong number of Arguments: Missing file Name\n\n");
		exit(-1);
	}

	puts("Shoe-Store, Tzvi Melamed id 01234567");
	setbuf(stdout, NULL);

	fp = fopen(argv[1], "r");
	if (fp==NULL)
	{
		perror("failed to open file") ;
		exit(EXIT_FAILURE) ;
	}
	open_all_sem();

	while ((ret = getline(&shoppers[thread_count], &char_count, fp)) != -1) {
		printf("Shopper: read line: %s", shoppers[thread_count]);
		if (pthread_create( &threads[thread_count], NULL, do_shopper, (void*) shoppers[thread_count]))
	  {
			perror("Failed to create a shopper thread");
	    exit(EXIT_FAILURE);
	  }
		thread_count++;
	}

	// wait for all children
	for (int i = 0; i < thread_count; ++i)
	{
		printf("parent waiting for (%d)\n", (int)threads[i]);
		pthread_join(threads[i], NULL);
	}
	printf ("last customer left... closing the shop.\nbyebye\n");
	close_all_sem();
	return 0;
}

/*
=================================================================
  	do_shopper
-------------------------------------------------------------------
Each line has the following format:
NAME [model size take/dont-take]+
      \------- single try ----/ ^ plus sign designates possibly many tries
where: 1: take, 0: dont-take
e.g.
Avi assics 40 0 assics 42 0 addidas 41 1

Shopping process:

a) go to fitting room  (3 fitting rooms)
b) for each "try": go_try()
	1) get shoe from storage room
	2) try it (print a message)
	3) if dont-take, return to storage room
c) if take/buy: go to cashier (pay)
*/
//=================================================================
void* do_shopper(void *ptr)
{
	char *buf = (char*)ptr;
	struct try_n_buy try;

	//read name
	sscanf(buf, "%s",try.name);
	printf("(%d %s) Shopper starting\n", (int)pthread_self(), try.name);
	int i=0;
	while (isblank(buf[i])) i++; // ignore trailig blanks.
	while (!isblank(buf[i])) i++; // ignore NAME
	while (isblank(buf[i])) i++; // ignore blanks

	enter_fitting(try.name);
	while(read_next_try(&buf[i], &try))
	{
	 	go_try(try);
	 	if(try.buy) break;
	}
	exit_fitting(try.name);
	if (try.buy) go_pay(try);


	pthread_exit(NULL);
}

//=================================================================
int read_next_try(char* buf, struct try_n_buy* try)
{
	int ret;
	int i=0;
	int k=0;

	ret = sscanf(buf, "%s %d %d", try->model, &try->size, &try->buy);
	if (ret != 3)
	{
		return 0; // failed to read 3 fields
	}

	//advance buf to ignore the fields we just scanned
	while (!isblank(buf[i])) i++; // ignore model
	while (isblank(buf[i])) i++; // ignore blanks.
	while (!isblank(buf[i])) i++; // ignore size field
	while (isblank(buf[i])) i++; // ignore blanks.
	while (!isblank(buf[i]))i++; // ignore buy field
	while (isblank(buf[i])) i++; // ignore trailig blanks.
	// copy the rest of the string
	while(buf[i]!=0)
		buf[k++]=buf[i++];
	buf[k]=0;
	return 1; // true, i.e. successful
}

//=================================================================
void enter_fitting(char* name)
{
	sem_wait(sem_fitting);
	printf("(%d %s) entered fitting room\n", (int)pthread_self(), name);
}

//=================================================================
void exit_fitting(char* name)
{
	printf("(%d %s) exited fitting room\n", (int)pthread_self(), name);
	sem_post(sem_fitting);
}
//=================================================================
void go_try(struct try_n_buy try)
{
	get_from_store(try);
	printf("(%d %s) trying model %s size %d\n", (int)pthread_self(), try.name, try.model, try.size);
	tenth_sleep(TRY_TIME);
	if (try.buy == 0) return_to_store(try);
}

//=================================================================
void get_from_store(struct try_n_buy try)
{
	int ret;
	if ((ret = sem_trywait(sem_store)) < 0)
	{
		printf("(%d %s) sem_trywait for Store failed... going to wait...\n",
				(int)pthread_self(), try.name);
		sem_wait(sem_store);
	}
	printf("(%d %s) Store: get model %s size %d\n", (int)pthread_self(), try.name, try.model, try.size);
	tenth_sleep(STORE_GET_TIME);
	printf("(%d %s) Store: Get completed.\n", (int)pthread_self(), try.name);
	sem_post(sem_store);
}

//=================================================================
void return_to_store(struct try_n_buy try)
{
	int ret;
	if ((ret = sem_trywait(sem_store)) < 0)
	{
		printf("(%d %s) sem_trywait for Store failed... going to wait...\n",
				(int)pthread_self(), try.name);
		sem_wait(sem_store);
	}
	printf("(%d %s) Store: Returning model %s size %d\n", (int)pthread_self(), try.name, try.model, try.size);
	tenth_sleep(STORE_RETURN_TIME);
	printf("(%d %s) Store: Return completed.\n", (int)pthread_self(), try.name);
	sem_post(sem_store);
}

//=================================================================
void go_pay(struct try_n_buy try)
{
	sem_wait(sem_pay);
	printf("(%d %s) Cashier: Start payment...\n", (int)pthread_self(), try.name);
	tenth_sleep(PAY_TIME);
	printf("(%d %s) Cashier: Done  payment...\n", (int)pthread_self(), try.name);
	sem_post(sem_pay);
}

//=================================================================
int tenth_sleep(int n)
{
	return usleep(n*100000 * DELAY_FACTOR);
}

//=================================================================
void open_all_sem(void)
{
	if (sem_unlink("/sem_fitting")==0)
		fprintf(stderr, "successul unlink of /sem_fitting\n");
	sem_fitting = sem_open("/sem_fitting", O_CREAT, S_IRWXU, SEM_FITTING);
	if (sem_fitting == SEM_FAILED)
	{
		perror("failed to open semaphore /sem_fitting\n");
		exit(EXIT_FAILURE);
	}

	if (sem_unlink("/sem_pay")==0)
		fprintf(stderr, "successul unlink of /sem_pay\n");
	sem_pay = sem_open("/sem_pay", O_CREAT, S_IRWXU, SEM_PAY);
	if (sem_pay == SEM_FAILED)
	{
		perror("failed to open semaphore /sem_pay\n");
		exit(EXIT_FAILURE);
	}

	if (sem_unlink("/sem_store")==0)
		fprintf(stderr, "successul unlink of /sem_store\n");
	sem_store = sem_open("/sem_store", O_CREAT, S_IRWXU, SEM_STORE);
	if (sem_store == SEM_FAILED)
	{
		perror("failed to open semaphore /sem_store\n");
		exit(EXIT_FAILURE);
	}
}

void close_all_sem(void)
{
	// close the semaphores
	sem_close(sem_fitting);
	sem_close(sem_store);
	sem_close(sem_pay);
}
