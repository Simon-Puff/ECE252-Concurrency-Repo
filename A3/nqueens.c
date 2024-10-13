// from the Cilk manual: http://supertech.csail.mit.edu/cilk/manual-5.4.6.pdf
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
typedef struct package{
    int n;
    char* config;
} package_p;

int safe(char * config, int i, int j)
{
    int r, s;

    for (r = 0; r < i; r++)
    {
        s = config[r];
        if (j == s || i-r==j-s || i-r==s-j)
            return 0;
    }
    return 1;
}

int count = 0;
pthread_mutex_t count_mutex; // Mutex for protecting the count variable

void nqueens(char *config, int n, int i)
{
    char *new_config;
    int j;

    if (i==n)
    {
        pthread_mutex_lock(&count_mutex);
        count++;
        pthread_mutex_unlock(&count_mutex);
    }
    
    /* try each possible position for queen <i> */
    for (j=0; j<n; j++)
    {
        /* allocate a temporary array and copy the config into it */
        new_config = malloc((i+1)*sizeof(char));
        memcpy(new_config, config, i*sizeof(char));
        if (safe(new_config, i, j))
        {
            new_config[i] = j;
	        nqueens(new_config, n, i+1);
        }
        free(new_config);
    }
    return;
}

void * pthread_handle(void *config_p){
    package_p *pkg = (package_p *)config_p; // Cast to package_p type
    nqueens(pkg->config, pkg->n, 1);        // Start solving from the second row
    free(pkg->config);                      // Free the configuration array
    free(pkg);                              // Free the package struct
    return NULL;
}

int main(int argc, char *argv[])
{
    int n;
    char *config;

    if (argc < 2)
    {
        printf("%s: number of queens required\n", argv[0]);
        return 1;
    }

    n = atoi(argv[1]);
    config = malloc(n * sizeof(char));
    pthread_mutex_init(&count_mutex, NULL);
    pthread_t thread_ids[n];
    for(int i = 0; i < n; i ++){
        package_p *package = malloc(sizeof(package_p));
        package->config = malloc(n*sizeof(char));
        package->config[0] = i;
        package->n = n;
        //printf("running queens %d\n", n);
        pthread_create(&thread_ids[i], NULL, pthread_handle, (void*) package);
        //nqueens(new_config, n, 1);
        //printf("# solutions: %d\n", count);
    }

    for (int i = 0; i < n; i++){
        pthread_join(thread_ids[i], NULL);
    }
    printf("Total number of solutions: %d\n", count);
    pthread_mutex_destroy(&count_mutex);
    return 0;
}
