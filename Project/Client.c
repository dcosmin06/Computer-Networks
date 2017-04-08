#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>

/* cod de eroare returnat de apeluri */
extern int errno;

/* portul pt conectarea la server */
int port;


int main(int argc, char *argv[])
{
  int sd; /* descriptor de socket */
  struct sockaddr_in server; /* structura pt conectarea la server */
  char msg[100];  /* mesajul trimis */
  /* siruri folosite pt verificarea rolului ales de client */
  char sir1[10]="general\n", sir2[10]="capitan\n", sir3[10]="wrrrz\n";
  int ok; /* variabila setata cu =-1, 0, 1 in functie de rolul ales */ 
  
  if(argc != 3)
    {
      printf("[client] Sintaxa este: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }
  /* stabilirea portului */
  port = atoi(argv[2]);
  /* creare socket */
  if((sd=socket(AF_INET, SOCK_STREAM, 0))== -1)
   {
     perror("[client] Eroare la socket().\n");
     return errno;
   }
   
  /* completarea structurii folosite la conectarea la server */
  server.sin_family=AF_INET;   /* familia socket-ului */
  server.sin_addr.s_addr=inet_addr(argv[1]);  /* adresa IP a serverului */
  server.sin_port=htons(port); /* portul de conectare la server */
  
  /* conectarea la server */
  if(connect(sd, (struct sockaddr *) &server, sizeof(struct sockaddr))==-1)
    {
      perror("[client] Eroare la connect().\n");
      return errno;
    }
  /* pregatirea mesajului */
  bzero(msg, 100);
  printf("[client]Alegeti un rol: [general/capitan/wrrrz]");
  fflush(stdout);
  read(0, msg, 100); /* citire de la tastatura */
  if(strcmp(msg,sir1)==0)
     ok= -1;                /* clientul a ales rol de general */
  else if(strcmp(msg,sir2)==0)
     ok = 0;                /* clientul a ales rol de capitan */
       else if(strcmp(msg,sir3)==0)
          ok = 1;                /* clientul a ales rol de wrrrz */
  
  /* trimitere mesaj catre server */
  if(write(sd, msg, 100) <= 0)
   {
     perror("[client] Eroare la write catre server.\n");
     return errno;
   }
  /* citire raspuns dat de server */
  if(read(sd, msg, 100) < 0)
    {
      perror("[client]Eroare la read() de la server.\n");
      return errno;
    }
  if(strcmp(msg,"Da!\n") == 0) /* autentificare reusita cu rolul selectat */
    {
      if(ok == -1)  /* client general */
      {
        int life = 3; /* numarul de "vieti" ale generalului */
        while(life > 0)
          {
          printf("general-Asteptam alte evenimente..\n");
          if(read(sd, msg, 100) < 0)
            {
              perror("[client]Eroare la read() de la general.\n");
              return errno;
            }
          if(strcmp(msg, "Nici un capitan ramas!\n")==0) /* compararea mesajului trimis de server */
            {
              life--;
              printf("%s\n",msg);
              printf("%d vieti ramase..\n",life);
              if(life == 0)
                 break;
              char destroy[50];
              bzero(destroy,50);
              strcat(destroy,"Aparare cu succes!\n");
              if(write(sd, destroy, 50) < 0)
                {
                  perror("[client]Eroare la write() catre server.\n");
                  return errno;
                }
            }
          }
        printf("Pregatim autodistrugerea bazei stelare..\n");
        char destroy[50];
        bzero(destroy,50);
        strcat(destroy,"Initiati autodistrugerea!\n");
        if(write(sd, destroy, 50) < 0)
          {
            perror("[client]Eroare la write() catre server.\n");
            return errno;
          }
        /*sleep(5); */
        close(sd);
      }
      if(ok == 0) /* client capitan */
        {
            printf("capitan-Asteptam inamici..\n");
            if(read(sd, msg, 100) < 0)
             {
               perror("[capitan]Eroare la read.\n");
               return errno;
             }
            if(strcmp(msg,"Prepare!\n") == 0)
            {
              int r;
              srand(time(NULL));
              r = rand();            /* generarea unui numar random */
              printf("Rezultatul luptei: %d\n",r);
              if(r % 2 == 0)
                {
                 printf("Esec!\n");
                 close(sd);
                }
              else
                {
                  int SD;
                  printf("Victorie!\n");
                   if((SD=socket(AF_INET, SOCK_STREAM, 0))== -1)    /* reconectarea la server in caz de succes, folosind alt socket */
                        {
                         perror("[client] Eroare la socket().\n");
                         return errno;
                        }
                  if(connect(SD, (struct sockaddr *) &server, sizeof(struct sockaddr))==-1)
                     {
                       perror("[client] Eroare la connect().\n");
                       return errno;
                     }
                 /* pregatirea mesajului */
                 bzero(msg, 100);
                 read(0, msg, 100); /* citire de la tastatura */
                 if(write(SD, msg, 100) < 0)
                   {
                     perror("[client]Eroare la write de la capitan victorios.\n");
                     return errno;
                   }
                printf("capitan-Asteptam inamici..\n");
               if(read(sd, msg, 100) < 0)
                {
                 perror("[capitan]Eroare la read.\n");
                 return errno;
                }
               if(strcmp(msg,"Prepare!\n") == 0)
                {
                 int r;
                 srand(time(NULL));
                 r = rand();
                 printf("Rezultatul luptei: %d\n",r);
                 if(r % 2 == 0)
                  {
                   printf("Esec!\n");
                   close(sd);
                  }
                else
                  {
                     printf("Iau o pauza..\n");
                     close(sd);
                  }
                }
            }
        }
      if(ok == 1)
        {
          printf("wrrrz-Kill them all !\n");
          close(sd);
        }
    }
  else if(strcmp(msg, "Nu!\n") == 0)
     {
       if(ok == -1)
         {
           printf("Exista deja general..\n");
           close(sd);
         }
       if(ok == 0)
         {
           printf("Numar maxim de capitani atins..\n");
           close(sd);
         }
    }
}

}