/* an exercise in getline: this is the original example from the man pages
 * note that getline() does look quite convenient, because the line's memory need not
 * be explicitly allocated, as the c lib does this.
 * note the newline is kept.
 */
#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define GBUF 2
#define CONDREALLOC(x, b, c, a, t); \
    if((x)>=((b)-1)) { \
        (b) += (c); \
        (a)=realloc((a), (b)*sizeof(t)); \
        for(i=((b)-(c));i<(b);++i) \
            ((a)[i]) = NULL; \
    }

typedef struct /* larr_t: line array type */
{
    char **l;
    int lbf;
    int lsz;
} larr_t;

int main(int argc, char *argv[])
{
    FILE *stream;
    // char *line = NULL;
    size_t len = 0;
    ssize_t nread=0;
    int i, j;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    stream = fopen(argv[1], "r");
    if (stream == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int lbuf=GBUF;

    /* note we alloctae and initialize in this way, but freeing will be different:
     * it will later have to be done individually for each pointer via  for loop */
    char **aol=malloc(lbuf*sizeof(char*)); // does need this, yes, though later it won't be free'd

    /* basically here it's like creating a series of "char *tmp;" statements,
     * none equally NULL .. so valgrind will complain */
    for(i=0;i<lbuf;++i) 
        aol[i]=NULL;

    // test to see if initialised.
    // for(i=0;i<lbuf;++i) 
    //     printf("%p ", aol+i); 
    // printf("\n"); 
    int asz=0;
    unsigned char getout=0;
    // these three are character string markers.
    int ty0, ty1, tyidx; // type of activity markers
    int da0, da1, da2, daidx;
    int la0, la1, llidx;
    int lo0, lo1;

    while ((nread = getline(aol+asz, &len, stream)) != -1) {
        CONDREALLOC(asz, lbuf, GBUF, aol, char*);
        // printf("Retrieved line of length (returned val method): %zu\n", nread);
        // printf("Retrieved line of length (strlen method): %zu\n", strlen(line));
        // fwrite(line, nread, 1, stdout);
        // printf("%s (%zu - %zu)", aol[asz], nread, len);
        //printf("%*s\n", (int)nread-2, aol[asz]);
        // eat whitespace or <'s
        j=0;
        while( (aol[asz][j] == ' ') || (aol[asz][j] == '\t') || (aol[asz][j] == '<') )
           j++; 
        // check first chars
        if((aol[asz][j] == 't') && (aol[asz][j+1]== 'y') ) {
            while(aol[asz][j] != '>')
                j++; 
            ty0=j+1;
            while(aol[asz][j] != '<')
                j++; 
            ty1=j;
            // printf("%i: %*s\n", (int)nread, (int)nread-4, aol[asz]);
            printf("%.*s ", ty1-ty0, &aol[asz][ty0]);
            tyidx=asz;
        } else if((aol[asz][j] == 't') && (aol[asz][j+1]== 'i') && (aol[asz][j+2] == 'm') && (aol[asz][j+3]== 'e') ) {
            daidx=asz;
            while(aol[asz][j] != '>')
                j++; 
            da0=j+1;
            while(aol[asz][j] != 'T')
                j++; 
            da1=j;
            while(aol[asz][j] != 'Z')
                j++; 
            da2=j;
            // printf("%i: %*s\n", (int)nread, (int)nread-4, aol[asz]);
            printf("%.*s\n", da1-da0, &aol[asz][da0]);
            printf("%.*s\n", da2-da1-1, &aol[asz][da1+1]);
        } else if((aol[asz][j] == 't') && (aol[asz][j+1]== 'r') && (aol[asz][j+2] == 'k') && (aol[asz][j+3]== 'p') ) {
            llidx=asz;
            while(aol[asz][j] != '"')
                j++; 
            j++;
            la0=j;
            while(aol[asz][j] != '"')
                j++; 
            la1=j;
            // pick up long
            j++; // push up one
            while(aol[asz][j] != '"')
                j++; 
            j++;
            lo0=j;
            while(aol[asz][j] != '"')
                j++; 
            lo1=j;
            // printf("%i: %*s\n", (int)nread, (int)nread-4, aol[asz]);
            printf("lat: %.*s ", la1-la0, &aol[asz][la0]);
            // printf("%i: %*s\n", (int)nread, (int)nread-4, aol[asz]);
            printf("long: %.*s\n", lo1-lo0, &aol[asz][lo0]);
            // printf("la/lo: %i %i %i %i\n", la0, la1, lo0, lo1); 
            getout=1;
        }
        asz++;
        if(getout) {
            printf("%s ", argv[1]); 
            printf("%.*s ", da1-da0, &aol[daidx][da0]);
            printf("%.*s ", da2-da1-1, &aol[daidx][da1+1]);
            printf("%.*s ", la1-la0, &aol[llidx][la0]);
            printf("%.*s ", lo1-lo0, &aol[llidx][lo0]);
            printf("%.*s\n", ty1-ty0, &aol[tyidx][ty0]);
            // printf("%i: %*s\n", (int)nread, (int)nread-4, aol[asz]);
            break;
        }
    }
    // printf("lbuf:%i asz:%i\n", lbuf, asz);

    /* now to see if normalise works */
    for(i=asz;i<lbuf;++i)
        free(aol[i]);
    aol=realloc(aol, asz*sizeof(char*)); // normalize

    /* and now to free */
    for(i=0;i<asz;++i)
        free(aol[i]);
    free(aol);

    /* Ok that's it */
    fclose(stream);
    exit(EXIT_SUCCESS);
}
