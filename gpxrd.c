/* gpxrd1.c is a refinement of gpxrd0, with a concentration on seconds and functions for parsing time. */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include "gpxrd.h"

//haversine courtesy of chatgpt.
#define EARTH_RADIUS_KM 6371.0
#define GARMFIRSTLINE 16 // the horrid hardcodes ... at least declared early so you can spot them.
#define GARMREPEATPATT 9

double to_radians(double degree)
{
    return degree * M_PI / 180.0;
}

double haversine(double lat1, double lon1, double lat2, double lon2)
{
    double dLat = to_radians(lat2 - lat1);
    double dLon = to_radians(lon2 - lon1);

    lat1 = to_radians(lat1);
    lat2 = to_radians(lat2);

    double a = sin(dLat / 2) * sin(dLat / 2) +
        sin(dLon / 2) * sin(dLon / 2) * cos(lat1) * cos(lat2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return EARTH_RADIUS_KM * c;
}

void parsetime(char *time, hmst_t *hmst)
{
    // ignores date part (i.e. up tot T character.
    int j;
    int hrlen, minlen, seclen, thoulen;
    char *t0, *t1, *t2, *t3, *t4;
    char hr[12]={'\0'};
    char min[12]={'\0'};
    char sec[12]={'\0'};
    char thou[12]={'\0'};
    t0=strchr(time, 'T');
    t1=strchr(time, ':');
    t2=strchr(t1+1, ':');
    t3=strchr(t2+1, '.');
    t4=strchr(t3+1, 'Z');
    hrlen=(int)(t1-t0);
    for(j=0;j<hrlen-1;j++)
        hr[j]=t0[j+1];
    hmst->h=atoi(hr);
    minlen=(int)(t2-t1);
    for(j=0;j<minlen-1;j++)
        min[j]=t1[j+1];
    hmst->m=atoi(min);
    seclen=(int)(t3-t2);
    for(j=0;j<seclen-1;j++)
        sec[j]=t2[j+1];
    hmst->s=atoi(sec);
    thoulen=(int)(t4-t3);
    for(j=0;j<thoulen-1;j++)
        thou[j]=t3[j+1];
    hmst->th=atoi(thou);
}

w_c *crea_wc(unsigned initsz)
{
    w_c *wc=malloc(sizeof(w_c));
    wc->lp1=initsz;
    wc->t=STRG;
    wc->w=malloc(wc->lp1*sizeof(char));
    return wc;
}

void reall_wc(w_c **wc, unsigned *cbuf)
{
    w_c *twc=*wc;
    unsigned tcbuf=*cbuf;
    tcbuf += CBUF;
    twc->lp1=tcbuf;
    twc->w=realloc(twc->w, tcbuf*sizeof(char));
    *wc=twc; /* realloc can often change the ptr */
    *cbuf=tcbuf;
    return;
}

void norm_wc(w_c **wc)
{
    w_c *twc=*wc;
    twc->w=realloc(twc->w, twc->lp1*sizeof(char));
    *wc=twc; /* realloc can often change the ptr */
    return;
}

void free_wc(w_c **wc)
{
    w_c *twc=*wc;
    free(twc->w);
    free(twc);
    return;
}

aw_c *crea_awc(unsigned initsz)
{
    int i;
    aw_c *awc=malloc(sizeof(aw_c));
    awc->ab=initsz;
    awc->al=awc->ab;
    awc->aw=malloc(awc->ab*sizeof(w_c*));
    for(i=0;i<awc->ab;++i) 
        awc->aw[i]=crea_wc(CBUF);
    return awc;
}

void reall_awc(aw_c **awc, unsigned buf)
{
    int i;
    aw_c *tawc=*awc;
    tawc->ab += buf;
    tawc->al=tawc->ab;
    tawc->aw=realloc(tawc->aw, tawc->ab*sizeof(aw_c*));
    for(i=tawc->ab-buf;i<tawc->ab;++i)
        tawc->aw[i]=crea_wc(CBUF);
    *awc=tawc;
    return;
}

void norm_awc(aw_c **awc)
{
    int i;
    aw_c *tawc=*awc;
    /* free the individual w_c's */
    for(i=tawc->al;i<tawc->ab;++i) 
        free_wc(tawc->aw+i);
    /* now release the pointers to those freed w_c's */
    tawc->aw=realloc(tawc->aw, tawc->al*sizeof(aw_c*));
    *awc=tawc;
    return;
}

void free_awc(aw_c **awc)
{
    int i;
    aw_c *tawc=*awc;
    for(i=0;i<tawc->al;++i) 
        free_wc(tawc->aw+i);
    free(tawc->aw); /* unbelieveable: I left this out, couldn't find where I leaking the memory! */
    free(tawc);
    return;
}

aaw_c *crea_aawc(unsigned initsz)
{
    int i;
    unsigned lbuf=initsz;
    aaw_c *aawc=malloc(sizeof(aaw_c));
    aawc->numl=0;
    aawc->aaw=malloc(lbuf*sizeof(aw_c*));
    for(i=0;i<initsz;++i) 
        aawc->aaw[i]=crea_awc(WABUF);
    return aawc;
}

void free_aawc(aaw_c **aw)
{
    int i;
    aaw_c *taw=*aw;
    for(i=0;i<taw->numl;++i) /* tried to release 1 more, no go */
        free_awc(taw->aaw+i);
    free(taw->aaw);
    free(taw);
}

void prtaawcdbg(aaw_c *aawc)
{
    int i, j, k;
    for(i=0;i<aawc->numl;++i) {
        printf("l.%u(%u): ", i, aawc->aaw[i]->al); 
        for(j=0;j<aawc->aaw[i]->al;++j) {
            printf("w_%u: ", j); 
            if(aawc->aaw[i]->aw[j]->t == NUMS) {
                printf("NUM! "); 
                continue;
            } else if(aawc->aaw[i]->aw[j]->t == PNI) {
                printf("PNI! "); 
                continue;
            } else if(aawc->aaw[i]->aw[j]->t == STCP) {
                printf("STCP! "); 
                continue;
            }
            for(k=0;k<aawc->aaw[i]->aw[j]->lp1-1; k++)
                putchar(aawc->aaw[i]->aw[j]->w[k]);
            printf("/%u ", aawc->aaw[i]->aw[j]->lp1-1); 
        }
        printf("\n"); 
    }
}

void prtaawcdata(aaw_c *aawc) /* print line and word details, but not the words themselves */
{
    int i, j;
    for(i=0;i<aawc->numl;++i) {
        printf("L%u(%uw):", i, aawc->aaw[i]->al); 
        for(j=0;j<aawc->aaw[i]->al;++j) {
            printf("l%ut", aawc->aaw[i]->aw[j]->lp1-1);
            switch(aawc->aaw[i]->aw[j]->t) {
                case NUMS: printf("N "); break;
                case PNI: printf("I "); break;
                case STRG: printf("S "); break;
                case STCP: printf("C "); break; /* closing punctuation */
                case SCST: printf("Z "); break; /* starting capital */
                case SCCP: printf("Y "); break; /* starting capital and closing punctuation */
                case ALLC: printf("A "); break; /* horrid! all capitals */
            }
        }
    }
    printf("\n"); 
    printf("L is a line, l is length of word, S is normal string, C closing punct, Z, starting cap, Y Starting cap and closing punct.\n"); 
}

void prtaawcplain(aaw_c *aawc) /* print line and word details, but not the words themselves */
{
    int i, j;
    for(i=0;i<aawc->numl;++i) {
        printf("L%u(%uw):", i, aawc->aaw[i]->al); 
        for(j=0;j<aawc->aaw[i]->al;++j)
            printf((j!=aawc->aaw[i]->al-1)?"%s ":"%s\n", aawc->aaw[i]->aw[j]->w);
    }
}

void prtaawcplain2(aaw_c *aawc) /* print line and word details, but not the words themselves */
{
    // departing from prtaawcplain, do a poor man's tabling
    int i;
    char w0c0 /* first word, first char */, w0c1 /* obvious*/;
    int typidx=-1; // they're be just one type tag, find out which line, it'll be running or whatever, it'll be word 1.
    for(i=0;i<aawc->numl;++i) {
        w0c0 = aawc->aaw[i]->aw[0]->w[0];
        w0c1 = aawc->aaw[i]->aw[0]->w[1];
        if((w0c0=='t') && (w0c1=='r') && aawc->aaw[i]->al>1) {
            printf("%s %s ", aawc->aaw[i]->aw[2]->w, aawc->aaw[i]->aw[4]->w);
        } else if((w0c0=='e') && (w0c1=='l')) {
            printf("%s ", aawc->aaw[i]->aw[1]->w);
        } else if((w0c0=='t') && (w0c1=='i')) {
            printf("%s\n", aawc->aaw[i]->aw[1]->w);
        } else if((w0c0=='t') && (w0c1=='y')) {
            typidx = i;
        }
    }
    if(typidx!=-1)
        printf("Activity was a type \"%s\"\n", aawc->aaw[typidx]->aw[1]->w);
    else
        printf("No activity type found.\n"); 
}

void prtaawcpla2(aaw_c *aawc) /* print line and word details, but not the words themselves */
{
    int i;
    double lon, lat;
    float ele;
    char *time;
    printf("%24s%14s%14s%14s\n", "TIME", "LON", "LAT", "ELE"); 
    for(i=9;i<aawc->numl-3;i+=4) {
        // printf("L%u(%uw):", i, aawc->aaw[i]->al); 
        // for(j=0;j<aawc->aaw[i]->al;++j)
        lon=strtod(aawc->aaw[i]->aw[2]->w, NULL);
        lat=strtod(aawc->aaw[i]->aw[4]->w, NULL);
        ele=atof(aawc->aaw[i+1]->aw[1]->w);
        time=aawc->aaw[i+2]->aw[1]->w;
        printf("%24s%14.6f%14.6f%14.6f\n", time, lon, lat, ele);
    }
}

void prtaawcpla3(aaw_c *aawc) /* garmin connect running gpx */
{
    int i, k=0;
    double lon, lat;
    float ele;
    char *time;
    printf("%24s%14s%14s%14s\n", "TIME", "LON", "LAT", "ELE"); 
    for(i=16;i<aawc->numl-8;i+=9) {
        // printf("L%u(%uw):", i, aawc->aaw[i]->al); 
        // for(j=0;j<aawc->aaw[i]->al;++j)
        lon=strtod(aawc->aaw[i]->aw[2]->w, NULL);
        lat=strtod(aawc->aaw[i]->aw[4]->w, NULL);
        ele=atof(aawc->aaw[i+1]->aw[1]->w);
        time=aawc->aaw[i+2]->aw[1]->w;
        printf("%i: %24s%14.6f%14.6f%14.6f\n", k++, time, lon, lat, ele);
    }
    printf("total lines=%i\n", k); 
}

void prtaawcpla300(aaw_c *aawc) /* garmin connect running gpx, further refinement. */
{
    int i, k=0;
    double lon, lat, lon2, lat2;
    double dlon, dlat;
    float ele, ele2, dele;
    hmst_t *hmst=calloc(1, sizeof(hmst_t));
    hmst_t *hmst2=calloc(1, sizeof(hmst_t));
    char *timestr;

    float allsecs, allsecs2; // all seconds
                             // header:
    printf("%24s%12s%12s%12s%12s%12s%14s%14s%14s\n", "TIME", "HR", "MIN", "SEC", "THOU", "ASECS", "LON", "LAT", "ELE"); 
    // first trkpt: absolute, i.e starting point.
    printf("First trkpt: absolute, i.e starting point:\n");
    i=GARMFIRSTLINE;
    // printf("L%u(%uw):", i, aawc->aaw[i]->al); 
    // for(j=0;j<aawc->aaw[i]->al;++j)
    lon=strtod(aawc->aaw[i]->aw[2]->w, NULL);
    lat=strtod(aawc->aaw[i]->aw[4]->w, NULL);
    ele=atof(aawc->aaw[i+1]->aw[1]->w);

    timestr=aawc->aaw[i+2]->aw[1]->w;
    parsetime(aawc->aaw[i+2]->aw[1]->w, hmst);
    allsecs = 3600*hmst->h + 60*hmst->m + hmst->s + (double)hmst->th/1000.;
    printf("%i: %24s%12i%12i%12i%12i%14.6f%14.6f%14.6f%14.6f\n", k++, timestr, hmst->h, hmst->m, hmst->s, hmst->th, allsecs, lon, lat, ele);

    // the rest shall all be differences:
    printf("From now on, cumulative differences:\n"); 
    for(i=GARMFIRSTLINE+GARMREPEATPATT;i<aawc->numl-GARMREPEATPATT+1;i+=GARMREPEATPATT) {
        // printf("L%u(%uw):", i, aawc->aaw[i]->al); 
        // for(j=0;j<aawc->aaw[i]->al;++j)
        lon2=strtod(aawc->aaw[i]->aw[2]->w, NULL);
        lat2=strtod(aawc->aaw[i]->aw[4]->w, NULL);
        ele2=atof(aawc->aaw[i+1]->aw[1]->w);
        dlon=lon2-lon;
        dlat=lat2-lat;
        dele=ele2-ele;
        // time=aawc->aaw[i+2]->aw[1]->w;
        timestr=aawc->aaw[i+2]->aw[1]->w;
        parsetime(aawc->aaw[i+2]->aw[1]->w, hmst2);
        allsecs2=3600*hmst2->h + 60*hmst2->m + hmst2->s + (double)hmst2->th/1000.;

        printf("%i: %24s%12i%12i%12i%12i%14.6f%14.6f%14.6f%14.6f\n", k++, timestr, hmst2->h-hmst->h, hmst2->m -hmst->m, hmst2->s-hmst->s, hmst2->th-hmst->th, allsecs2-allsecs, dlon, dlat, dele);

        lon=lon2;
        lat=lat2;
        ele=ele2;
        hmst->h=hmst2->h;
        hmst->m=hmst2->m;
        hmst->s=hmst2->s;
        hmst->th=hmst2->th;
        allsecs=allsecs2;
    }
    printf("total lines=%i\n", k); 
    free(hmst);
    free(hmst2);
}

aaw_c *processinpf(char *fname)
{
    /* declarations */
    FILE *fp=fopen(fname,"r");
    int i;
    size_t couc /*count chars per line */, couw=0 /* count words */;
    int c, oldc='\0';
    boole inword=0;
    unsigned lbuf=LBUF /* buffer for number of lines */, cbuf=CBUF /* char buffer for size of w_c's: reused for every word */;
    aaw_c *aawc=crea_aawc(lbuf); /* array of words per line */

    while( (c=fgetc(fp)) != EOF) {
        if( (c== '\n') | (c == ' ') | (c == '<') | (c == '>') | (c == '"') ) {
            if( inword==1) { /* cue word-ending procedure */
                aawc->aaw[aawc->numl]->aw[couw]->w[couc++]='\0';
                aawc->aaw[aawc->numl]->aw[couw]->lp1=couc;
                SETCPTYPE(oldc, aawc->aaw[aawc->numl]->aw[couw]->t);
                norm_wc(aawc->aaw[aawc->numl]->aw+couw);
                couw++; /* verified: this has to be here */
            }
            if(c=='\n') { /* cue line-ending procedure */
                if(aawc->numl ==lbuf-1) {
                    lbuf += LBUF;
                    aawc->aaw=realloc(aawc->aaw, lbuf*sizeof(aw_c*));
                    for(i=lbuf-LBUF; i<lbuf; ++i)
                        aawc->aaw[i]=crea_awc(WABUF);
                }
                aawc->aaw[aawc->numl]->al=couw;
                norm_awc(aawc->aaw+aawc->numl);
                aawc->numl++;
                couw=0;
            }
            inword=0;
        } else if(inword==0) { /* a normal character opens word */
            if(couw ==aawc->aaw[aawc->numl]->ab-1) /* new word opening */
                reall_awc(aawc->aaw+aawc->numl, WABUF);
            couc=0;
            cbuf=CBUF;
            aawc->aaw[aawc->numl]->aw[couw]->w[couc++]=c;
            GETLCTYPE(c, aawc->aaw[aawc->numl]->aw[couw]->t); /* MACRO: the firt character gives a clue */
            inword=1;
        } else if(inword) { /* simply store */
            if(couc == cbuf-1)
                reall_wc(aawc->aaw[aawc->numl]->aw+couw, &cbuf);
            aawc->aaw[aawc->numl]->aw[couw]->w[couc++]=c;
            /* if word is a candidate for a NUM or PNI (i.e. via its first character), make sure it continues to obey rules: a MACRO */
            IWMODTYPEIF(c, aawc->aaw[aawc->numl]->aw[couw]->t);
        }
        oldc=c;
    } /* end of big for statement */
    fclose(fp);

    /* normalization stage */
    for(i=aawc->numl; i<lbuf; ++i)
        free_awc(aawc->aaw+i);
    aawc->aaw=realloc(aawc->aaw, aawc->numl*sizeof(aw_c*));

    return aawc;
}

int main(int argc, char *argv[])
{
    /* argument accounting */
    if(argc!=2) {
        printf("Error. Pls supply argument (name of text file).\n");
        exit(EXIT_FAILURE);
    }
#ifdef DBG2
    printf("typeszs: aaw_c: %zu aw_c: %zu w_c: %zu\n", sizeof(aaw_c), sizeof(aw_c), sizeof(w_c));
#endif

    aaw_c *aawc=processinpf(argv[1]);
#ifdef DBG
    prtaawcdbg(aawc);
#else
    // prtaawcdata(aawc); // just the metadata
    prtaawcplain2(aawc); // printout original text as well as you can.
    // prtaawcpla300(aawc); // printout original text as well as you can.
#endif
    // printf("Numlines: %zu\n", aawc->numl); 

    free_aawc(&aawc);

    return 0;
}
