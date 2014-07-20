#ifndef TYPES_H_
#define TYPES_H_

#define NUMINTS (1000)
#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define MAX_NUMBER_LIST 10
#define MAX_FILE_SIZE 1024
#define MAX_PATH_LEN 1024

typedef enum {false, true} bool;

struct data {
    char name[MAX_PATH_LEN];
    char content[MAX_FILE_SIZE];
    int count;
};

struct file_node
{
    char *name;
    char *location;
    struct file_node *next;
};

struct hash_table
{
    int capacity;
    int num_ele;
    struct file_node **list;
};

struct data *heap;
int heapsize;

/* Min heap functions */

void createHeap ();
void insert (struct data *el);
void increment (char *name);
char *pop ();

/* Hash table functions */

bool create_ht();
int hash_function(char *name);
struct data* fill_data (char *name, char *content);
struct file_node* fill_node (char *name, char *content);
bool cache_insert_file (char *name, char *content);
char* cache_lookup_file (char *name);
void delete_ht ();
void print();

/* Memory mapping structure */

int *map;

#endif
