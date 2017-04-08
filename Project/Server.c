#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>


#define PORT 6262 /* portul utilizat */

extern int errno; /* eroarea returnata de anumite apeluri */




int main()
{
  struct sockaddr_in server; /* structura folosita de server */
  fd_set readfds; /* multimea descriptorilor de citire */
  fd_set activefds; /* multimea descriptorilor activi */
  int sd, cd;  /* descriptori de socket */
  int general[1], captains[5];  /* vectori ce vor contine descriptori de clienti cu rolurile respective selectate */
  general[0] = -99;
  int x;
  for( x = 0; x < 5; x++)
    captains[x] = -99;   /* initializarea vectorilor */
  int ok=1;
  if((sd=socket(AF_INET, SOCK_STREAM, 0))== -1)
    {
      perror("[server] Eroarea la socket() !\n");
      return errno;
    }  
  /* setare optiune SO_REUSEADDR */
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok));
  /* pregatire structuri de date */
  bzero(&server, sizeof(server));
  /* completarea structurii folosita de server */
  server.sin_family=AF_INET;
  server.sin_addr.s_addr= htonl(INADDR_ANY);
  server.sin_port= htons(PORT);
  /* atasarea socket-ului */
  if(bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr))== -1)
    {
      perror("[server]Eroare la bind() !\n");
      return errno;
    }
/* serverul asculta daca vin clienti */
  if(listen(sd,5)== -1)
   {
     perror("[server]Eroare la listen().\n");
     return errno;
   }
  struct sockaddr_in from; /* structura folosita pentru clienti */
  int len; /* lungimea structurii sockaddr_in */
  int fd; /* descriptor folosit pt parcurgerea listelor */
  int nfds; /* valoarea maxima a descriptorilor */
  struct timeval tv; /* structura folosita pentru select() */
  /* completarea multimii descriptorilor activi */
  FD_ZERO(&activefds); /* initiaizarea multimii */
  FD_SET(sd, &activefds); /* includem in multime socket-ul folosit la listen() */
  tv.tv_sec = 1;
  tv.tv_usec = 500000;  /* se va astepta 1.5 secunde */
  nfds = sd; /* initializarea valorii maxime */
  printf("[server]Asteptam la intrarea %d in baza stelara...\n", PORT);
  fflush(stdout);
  
  /*servirea concurenta a clientilor */
  while(1)
    {
      /* ajustarea multimii de descriptori de citire */
      bcopy((char *) &activefds, (char *) &readfds, sizeof(readfds));
      /* apelam select() */
      if(select(nfds+1, &readfds, NULL, NULL, &tv) < 0)
        {
          perror("[server]Eroare la select().\n");
          return errno;
        }
      /* verificam daca socket-ul sd poate primi clienti */
      if(FD_ISSET(sd, &readfds))
        {
          len=sizeof(from);
          bzero(&from, sizeof(from)); /* pregatirea structurii pt client */
          /* acceptam un client sosit */
          cd = accept(sd, (struct sockaddr *) &from, &len);
          if(cd < 0)
            {
              perror("[server]Eroare la accept().\n");
              continue;
            }
          if(nfds < cd)
            nfds = cd;  /* ajustarea valorii maxime */
          /* includem si acest socket in lista activefds */
          FD_SET(cd, &activefds);
          printf("[server]S-a conectat utilizatorul cu descriptorul %d.\n",cd);
          fflush(stdout);
       }
     /* verificam daca este pregatit vreun socket client */
       int var;    
       for(fd=0; fd <= nfds; fd++) /* parcurgere multime de descriptori */
        {
          if(fd != sd && FD_ISSET(fd, &readfds))
            {
              if((var=WarSpace(fd,general,captains))==0)
                {
                  close(fd);
                  FD_CLR(fd,&activefds);
                }
              else if(var == 1){
                  general[0]=fd;
                  FD_CLR(fd,&activefds);
                  }
                  else if(var>=2){
                  captains[var-2]=fd;
                  FD_CLR(fd,&activefds);
                  }
              if(var == -1)
                {
                  close(fd);
                  FD_CLR(fd, &activefds);
                }
              else if(var == -999)
                {
                  close(fd);
                  FD_CLR(fd, &activefds);
                  printf("Baza stelara se va autodistruge.\n");
                  close(sd);
                  return 0;
                }
            }
        }
    }
}


 /* functie care citeste rolul ales de client si continua corespunzator */
 int WarSpace(int fd, int general[1], int captains[5])
   {
     int val;
     char buffer[100]; /*mesajul */
     int bytes;        /* nr de octeti cititi/scrisi */
     char msg[100];    /* mesaj primit de la client */
     char rasp[100];   /* mesaj de raspuns pentru client */
     char s1[10]="general\n",s2[10]="capitan\n",s3[10]="wrrrz\n";
     bzero(rasp,100);
     bytes = read(fd, msg, sizeof(buffer));
     if(bytes < 0)
       {
         perror("Eroare la read() de la client.\n");
         return 0;
       }
     printf("[server]Mesajul a fost primit..%s\n", msg);
     if(strcmp(msg,s1) == 0)
       {
         if(general[0] == -99) /* nici un client cu rol de general */
            {
             val=1;
             /* pregatim mesaj de raspuns */
             strcat(rasp,"Da!\n");
             printf("[server]Trimitem un mesaj inapoi..\n");
             if(bytes && write(fd, rasp, bytes) < 0)
               {
                 perror("[server]Eroare la write catre client-general.\n");
                 return 0;
               }
            }
         else
          {
            val=0;
            /* pregatim mesaj de raspuns */
            strcat(rasp,"Nu!\n");
            printf("[server]Trimitem un mesaj inapoi..\n");
            if( write(fd, rasp, bytes) < 0)
              {
                perror("[server]Eroare la write catre client-general.\n");
                return 0;
              }
          }
      }
     else if(strcmp(msg,s2) == 0)
       {
         int j; /* folosit la parcurgerea vectorului captains */
         if(general[0] == -99)
           {
             val = 0;
             strcat(rasp,"Nici un general conectat!\n");
             printf("[server]Trimitem mesaj inapoi..\n");
             if(bytes && write(fd, rasp, bytes) < 0)
               {
                 perror("[server]Eroare la write catre client-capitan.\n");
                 return 0;
               }
             return val;
           }
         for(j = 0; j < 5; j++)
            {
              if(captains[j] == -99)
                 {
                   val = 2+j;
                   /* pregatim mesaj de raspuns */
                   strcat(rasp,"Da!\n");
                   printf("[server]Trimitem un mesaj inapoi..\n");
                   if(bytes && write(fd, rasp, bytes) < 0)
                     {
                       perror("[server]Eroare la write catre client-capitan.\n");
                       return 0;
                     }
                   break;
                  }
              else if(j==4) {
                   val = 0;
                /* pregatim mesaj de raspuns */
                   strcat(rasp,"Nu!\n");
                   printf("[server]Trimitem mesajul inapoi..\n");
                   if(bytes && write(fd, rasp, bytes) < 0)
                     {
                       perror("[server]Eroare la write catre client-capitan.\n");
                       return 0;
                     }
                   break;
                }
            }
       }
     if(strcmp(msg,s3) == 0)
       {
         int i;
         int flag=0;
         if(general[0] == -99)
           {
             val = 0;
             strcat(rasp,"Nici un general conectat!\n");
             printf("[server]Trimitem mesaj inapoi..\n");
             if(bytes && write(fd, rasp, bytes) < 0)
               {
                 perror("[server]Eroare la write catre client-capitan.\n");
                 return 0;
               }
             return val;
           }    
         for(i = 0; i < 5; i++)
           {
             if(captains[i] > 0)
               {
                 flag=1;
                 val = -1;
                 /* anuntam clientul capitan ca va fi trimis la lupta */
                 strcat(rasp,"Prepare!\n");
                 printf("Anuntam capitanul..\n");
                 if(bytes && write(captains[i], rasp, bytes) < 0)
                   {
                     perror("[server]Eroare la avertizarea capitanului.\n");
                     return 0;
                   }
                 captains[i]=-99;
                 close(captains[i]);
                 /* anuntam clientul wrrrz */
                 bzero(rasp,100);
                 strcat(rasp,"Da!\n");
                 printf("Anuntam wrrrz-ul..\n");
                 if(bytes && write(fd, rasp, bytes) < 0)
                   {
                     perror("[server]Eroare la avertizarea warrrz-ului.\n");
                     return 0;
                   }
                 break;
               }
            }
        if(flag == 0)
          {
            val = -1;
            strcat(rasp, "Nici un capitan ramas!\n");
            printf("Anuntam generalul..\n");
            int desc = general[0];
            if(bytes && write(desc, rasp, bytes) < 0)
              {
                perror("[server]Eroare la avertizarea generalului.\n");
                return 0;
              }
            char gMsj[50];
            bzero(gMsj,50);
            if(read(desc,gMsj,50) == -1)
              {
                perror("[server]Eroare la read de la general.\n");
                return errno;
              }
            if(strcmp(gMsj,"Initiati autodistrugerea!\n") == 0)
              val = -999;
          }
      }
      
 return val;
 }
