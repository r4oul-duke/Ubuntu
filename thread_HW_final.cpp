#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/param.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>

// nadefinuji typ pro mozny pripad zmeny typu cisel
#define TYPE int

typedef struct Task
{
    TYPE *array;
    int len;
} Task;


void insertion_sort( int random_numbers_array[], int half_of_the_array )
{
    for( int i = 1; i < half_of_the_array; i++)
    {
        int key = random_numbers_array[i]; // keep track of the element we are currently looking at
        int j = i - 1; // zajisti, ze se budeme divat vzdy na elementy key-1 doleva (od key)

        while ( j >= 0 && random_numbers_array[j] > key ) // dokud je j vetsi jak nula, tzn neni jeste na zacatku array a dokud je array[j] vetsi nez key
        {
            random_numbers_array[j + 1] = random_numbers_array[j]; // this is gonna do the shifting of elements over
            j--; 
        }
        random_numbers_array[j + 1] = key; // this is where we insert the value
    }
}

void bubble_sort( int random_numbers_array[], int array_length )
{
    bool swapped;
    int i = 0;
    do
    {
        swapped = false;
        for( int j = 0; j < (array_length - 1 - i); j++) // odecitame i, protoze i je vzdy behem jedne iterace sorted a neni treba se na nej znovu divat
        {
            if( random_numbers_array[j] > random_numbers_array[j + 1] ) // for descending order staci prehodit toto znamenko
            {
                int temp = random_numbers_array[j];
                random_numbers_array[j] = random_numbers_array[j + 1];
                random_numbers_array[j + 1] = temp;
                swapped = true;
            }
        }
        i++;
    } while(swapped);
}

void selection_sort( int array[], int array_length )
{
    for( int i = 0; i < (array_length - 1); i++)
    {
      int minimum_position = i; //stanovim si minimum, ze je je na prvnim indexu
      for(int j = i + 1; j < array_length; j++)
      {
        if(array[j] < array[minimum_position]) // pokud je nejaky nasledujici prvek rady mensi nez dosavadni minimum, dame ho jako nove minimum
        {
            minimum_position = j; // index s minimem je ted na indexu aktualni indexu j v zavislosti na pozici v loop
        }
        if( minimum_position != i ) //pokud se index minima nerovna indexu i, potom provedeme klasicky swap, tentokrat uz s konkretnimi cisly
        {
            int temp = array[i];
            array[i] = array[minimum_position];
            array[minimum_position ] = temp;
        }
      }
    }
}

int timeval_diff_to_ms( timeval *time_before, timeval *time_after)
{
    timeval result;
    timersub( time_after, time_before, &result ); //subtracts the time value in b from the time value in a, and places the result in the timeval pointed to by res. The result is normalized such that res->tv_usec has a value in the range 0 to 999,999.
    return 1000 * result.tv_sec + result.tv_usec / 1000; // tv_sec jsou seconds, tv_usec jsou microseconds
}


void* my_thread( void* arg )
{
    //"rozbaleni" parametru ze pointeru na strukturu
    Task *task = (Task *) arg;

    // zavolani algoritmu na trideni
    insertion_sort(task->array, task->len);
    
    //predani setrideneho pole do pointeru, ktery budeme vracet zpet do vlakna
    int* sorted_array = (int*)malloc(task->len*(sizeof(int)));
    sorted_array = task->array;

    return (void*) sorted_array;
}
   // merge dvou poli
void* merge_fun(int array_length, int half_of_the_array, int *sorted_array1, int *sorted_array2,int *merged_array)
{
    int j = 0;
    int k = 0;


    for(int i = 0; i < array_length; i++)
    {
        if(j < half_of_the_array && k < half_of_the_array)
        {
            if(sorted_array1[j] <= sorted_array2[k])
            {
                merged_array[i] = sorted_array1[j];
                j++;
            }
            else
            {
                merged_array[i] = sorted_array2[k];
                k++;
            }
        }
        else if(j == half_of_the_array)
            {
               merged_array[i] = sorted_array2[k];
               k++;
            }
        else if(k == half_of_the_array)
            {
                merged_array[i] = sorted_array1[j];
                j++;
            }
    }
    return NULL;
}

//vypsani dvou paralelne setridenych poli ve dvou vlaknech
void* print_merged_halves(int *sorted_array1, int *sorted_array2, int half_of_the_array)
{
    for(int i = 0; i < half_of_the_array; i++)
    {
    printf("%d\n", sorted_array1[i]);
    }
    printf("\n");
    for(int i = 0; i < half_of_the_array; i++)
    {
    printf("%d\n", sorted_array2[i]);
    }
    printf("\n");
    return NULL;
}
    

int main(int argc, char **argv)
{
    if ( argc!= 2 ) 
    { 
        printf( "Specify number of elements.\n" ); 
        return 0; 
    }
    timeval time_before, time_after;

    int array_length = atoi(argv[1]);
    int half_of_the_array = array_length / 2;

    // -- 1.cast

    // memory alokace bez vlakna
    TYPE *random_numbers_arr_no_thread = (TYPE*)malloc(array_length*sizeof(TYPE));
    if ( !random_numbers_arr_no_thread )
    {
        printf("not enough memory to allocate");
        return 1;
    }

    // funkce s array
    // void (*sort_fctions[])(int[], int) = {insertion_sort, bubble_sort, selection_sort};

    // for(int i = 0; i < 3; i++) 
    // {
        //generating random numbers
        srand( time( NULL ));
        
        for(int i = 0; i < array_length; i++)
        {
        random_numbers_arr_no_thread[ i ] = rand() % ( array_length * 10 );
        printf("%d\n", random_numbers_arr_no_thread[i]);
        }

        gettimeofday(&time_before, NULL);
        
        // sort_fctions[i](random_numbers_arr_no_thread, array_length); //zavolani funkce pro trideni pomoc insertion sort
        insertion_sort( random_numbers_arr_no_thread, array_length );
        // bubble_sort( random_numbers_arr_no_thread, array_length );
        // selection_sort( rrandom_numbers_arr_no_thread, array_length );

        //print random numbers array
        for (int i = 0; i < array_length; i++) {
            printf("array[%d] = %d\n", i, random_numbers_arr_no_thread[i]); // loop pro vypsani serazenych cisel na obrazovku
        }
        printf("\n");

        gettimeofday(&time_after, NULL);
        printf("Sorting by insertion sort: %d [ms]\n",timeval_diff_to_ms(&time_before, &time_after));

    // }

    // -- 2.cast

    // 1)nahodne vygenerovani pole cisel
    srand( time( NULL ));
    TYPE *array = (TYPE*)malloc(array_length*sizeof(TYPE));
    for(int i = 0; i < array_length; i++)
    {
    array[ i ] = rand() % ( array_length * 10 );
    //vypsani pole cisel na obrazovku
    // printf("%d\n", array[i]);
    }

    //alokace dvou polovin pole
    TYPE *first_half = (TYPE*)malloc(half_of_the_array*sizeof(TYPE));
    TYPE *second_half = (TYPE*)malloc(half_of_the_array*sizeof(TYPE));

    //rozdeleni pole cisel na dve pole
    memcpy(first_half, array, half_of_the_array * sizeof(int));
    memcpy(second_half, array + half_of_the_array, half_of_the_array * sizeof(int));

    // vytvoreni struktury
    pthread_t thread1, thread2;

    //naplneni struktury
    Task my_thread1 = { first_half, half_of_the_array};
    Task my_thread2 = { second_half, half_of_the_array};
    
    // zacatek vlakna + zacatek mereni
    gettimeofday(&time_before, NULL);
    pthread_create( &thread1, NULL, my_thread, &my_thread1 );
    pthread_create( &thread2, NULL, my_thread, &my_thread2 );

    // navratovy pointer pro data z pole
    int* sorted_array1;
    int* sorted_array2;

    // cekani na ukonceni prace poli 
    pthread_join( thread1, (void**) &sorted_array1 );
    pthread_join( thread2, (void**) &sorted_array2 );

    //vypsani dvou setridenych polovin
    //print_merged_halves(sorted_array1, sorted_array2, half_of_the_array);
    
    // merge dvou poli
    int* merged_array = (int*)malloc(array_length*sizeof(int));
    merge_fun(array_length, half_of_the_array, sorted_array1, sorted_array2,merged_array);
    
    //konec mereni casu
    gettimeofday(&time_after, NULL);
    
    //vypis jednoho setrideneho pole po slouceni dvou predchozich 
    // for(int i = 0; i < array_length; i++)
    // {
    //     printf("%d\n", merged_array[i]);
    // }
        
    printf("Sorting by insertion sort: %d [ms]\n",timeval_diff_to_ms(&time_before, &time_after));

    free(array);
    free(first_half);
    free(second_half);
    free(random_numbers_arr_no_thread);
    // free(sorted_array1);
    // free(sorted_array2);
    return 0;
}
