// including necessary library files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#define debug 0
// defining the time functionality
#define _XOPEN_SOURCE
#define __USE_XOPEN

// defining database functionalities
#define DB "database.csv"
// defining the readable and unreadable queries
#define TRY(a)  if (!(a)) {perror(#a);exit(1);}
#define TRY2(a) if((a)<0) {perror(#a);exit(1);}
#define FREE(a) if(a) {free(a);a=NULL;}
// defining a sort function using string compare(strcmp), pointers and a lambda expression
#define sort_by(sort) \
static int by_##sort (const void*p1, const void*p2) { \
    return strcmp ((*(const pdb_t*)p1)->sort, (*(const pdb_t*)p2)->sort); }

// build succesful

// defining a struct for the management system
typedef struct db
{
    char drug[56];
    char type[26];
    time_t dateOfExpiry;
    char manufacturer[56];
    int quantity;
    float price;
    bool isPrescription;
    struct db *next;
}
db_t,*pdb_t;

// typedef for sorting
typedef int (sort)(const void*, const void*);

// enum for commands
enum {ADD, PRINT, READLINE, READ, SORT, DESTROY};

// function definitions
// data access object(DAO)
static pdb_t dao (int cmd, FILE *f, pdb_t db, sort sortby);
// static char *time2str (time_t *time);
// static time_t str2time (char *date);

// qsort comparator callbacks
sort_by(drug);
sort_by(quantity);
sort_by(manufacturer);
sort_by(type);
static int by_date(pdb_t *p1, pdb_t *p2);

int main(int argc,char** argv){
    char buf[100];
    #if debug
    printf("%s\n",argv[1]);
    #endif
    // defining the database commands
    const char *commands[] = {"-a", "-p", "-s", "-m", "-t" , "-u", NULL};
    // setting up the db and its locations
    // line 45
    db_t db;
    db.next = NULL;
    pdb_t dbList;
    int i;
    FILE *f;

    // line 27
    TRY (f=fopen(DB, "a+"));

// printing usage menu if the CL argument count is less than 2 ie. if the program is just launched
    if (argc<2){
        usage: printf("[commands]\n"
        "-a add to database\n"
        "-d delete record\n"
        "-s sort records by date of expiry\n"
        "-m sort records by manufacturer\n"
        "-s sort records by type\n"
        "-p find last added record \n");
        fclose (f);
        return 0;
    }

// defining cases for command line inputs
    for (i=0;commands[i]&&strcmp(argv[1],commands[i]);i++);
    switch (i) {
        case ADD:
            printf("-a  Create a new entry.\n");
            printf("Name           :");if((scanf(" %s[^\n]",db.drug     ))<0)break;
            printf("Type           :");if((scanf(" %s[^\n]",db.type))<0)break;
            printf("Manufacturer   :");if((scanf(" %s[^\n]",db.manufacturer ))<0)break;
            // printf("Date of expiry :");if((scanf(" %s[^\n]",buf          ))<0)break;
            printf("Is prescription(1/0):");if((scanf(" %d[^\n]",&db.isPrescription))<0)break;
            printf("Price          :");if((scanf(" %f[^\n]",&db.price))<0)break;
            printf("Quantity       :");if((scanf(" %d[^\n]",&db.quantity))<0)break;
            // db.dateOfExpiry=strftime (buf);
            dao (ADD,f,&db,NULL);
            break;

        case PRINT:
            printf ("-p  Print the latest entry.\n");
            while (!feof(f)){
                //  dao (READLINE,f,&db,NULL);
                 dao (PRINT,f,&db,NULL);
            }
            break;
            
        default: {
            printf ("Unknown command: %s.\n",(strlen(argv[1])<10?argv[1]:NULL));
            goto usage;
        }
    }

    fclose (f);
    return 0;

}

static pdb_t dao (int cmd, FILE *f, pdb_t in_db, sort sortby) {
pdb_t *pdb=NULL,rec=NULL,hd=NULL;
    int i=0, ret;
    char buf[100];
    switch (cmd) {
        case ADD:
            fprintf (f,"\"%s\",",in_db->drug);
            fprintf (f,"\"%s\",",in_db->type);
            fprintf (f,"\"%s\",",in_db->manufacturer);
            // fprintf (f,"\"%s\",",time2str(&in_db->dateOfExpiry));
            fprintf(f, "\"%d\",", in_db->isPrescription);
            fprintf (f,"\"%f\",",in_db->price);
            fprintf (f,"\"%d\"\n",in_db->quantity);
            break;

        case PRINT:
            for (;in_db;i++) {
                printf ("Drug           : %s\n",     *in_db->drug);
                printf ("Type           : %s\n",     *in_db->type);
                printf ("Manufacturer   : %s\n",     *in_db->manufacturer);
                // printf ("Date of expiry : %s\n",     time2str(&in_db->dateOfExpiry));
                printf ("Price          : %f\n",     in_db->price);
                printf ("Quantity       : %s\n",     in_db->quantity);

                if (!((i+1)%3)) {
                    printf ("Press Enter to continue.\n");
                    ret = scanf ("%*[^\n]");
                    // ret=-1;
                    if (ret<0) return rec; /* handle EOF */
                    else getchar();
                }
                    in_db=in_db->next;
            }
            break;

            case READLINE:
                if((fscanf(f," \"%[^\"]\",",in_db->drug     ))<0)break;
                if((fscanf(f," \"%[^\"]\",",in_db->type))<0)break;
                if((fscanf(f," \"%[^\"]\",",in_db->manufacturer ))<0)break;
                // if((fscanf(f," \"%[^\"]\",",buf              ))<0)break;
                if((fscanf(f," \"%[^\"]\" ",in_db->price      ))<0)break;
                if((fscanf(f," \"%[^\"]\" ",in_db->quantity      ))<0)break;
                // in_db->dateOfExpiry=str2time (buf);
                break;

            case READ:
                while (!feof(f)) {
                    dao (READLINE,f,in_db,NULL);
                    TRY (rec=malloc(sizeof(db_t)));
                    *rec=*in_db; /* copy contents */
                    rec->next=hd;/* to linked list */
                    hd=rec;i++;
                }
                if (i<2) {
                    puts ("Empty database. Please create some entries.");
                    fclose (f);
                    exit (0);
                }
            break;

        case DESTROY: {
            while ((rec=in_db)) {
                in_db=in_db->next;
                FREE (rec);
            }
        }
    }
    return rec;
}
