#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>

#define LEN	64

typedef struct link{
	int num;	
	struct link *next;
}Link;

typedef struct potato{
	int hops;
	Link *chain;
}Potato;

char host[64];
int num_players;
int id;
int **map;
void notifyport(int n,char nam[][64]);
Potato *pot;


void notifyport(int n,char nam[][64])
{
	int i,zen,to,waste;
	for(i=1;i<n+1;i++)
	{
		if(i==n)
			to=1;
		else
			to=i+1;
		
		zen= send(map[i][0],(char *)&(map[to][1]),sizeof(map[to][1]),0);
		if(zen!=sizeof(map[to][1]))
		{
			perror("send");
			exit(1);
		}
		zen = recv(map[i][0],&waste,sizeof(waste),0);
		zen= send(map[i][0],nam[to],64,0);
		
		
	}
}



main(int argc, char *argv[])
{
	id =0;	
	char buf[512],str[LEN];
	int s,set=1, p, fp, rc, len, port,l,hops;
	struct hostent *hp, *ihp;
	struct sockaddr_in sin, incoming;
	if ( argc != 4 ) 
	{
		fprintf(stderr, "Invalid number of parameters passed for %s", argv[0]);
		exit(1);
	}
	
	port=atoi(argv[1]);
	num_players=atoi(argv[2]);
	l=num_players;
	hops =atoi(argv[3]);
	
	char fumes[num_players+2][64];

	if ( hops <0 ) 
	{
		fprintf(stderr, "Invalid hop number passed \n");
		exit(1);
	}
	if ( num_players < 2 ) 
	{
		fprintf(stderr, "Invalid player number passed \n");
		exit(1);
	}	
	
	int x;
	
	
	gethostname(host, sizeof host);
	printf("Potato Master on host %s \n",host);
	printf("Players = %d \n",num_players);
	
	printf("Hops = %d \n", hops );
	hp = gethostbyname(host);
	if ( hp == NULL ) 
	{
    		fprintf(stderr, "%s: host not found (%s)\n", argv[0], host);
    		exit(1);
  	}
	s = socket(AF_INET, SOCK_STREAM, 0);
	if ( s < 0 ) 
	{
	    perror("socket:");
	    exit(s);
	}
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
	
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&set, sizeof(set)) < 0)
	    {
        printf("Error: setsockopt failed.\n");
        close(s);
        exit(EXIT_FAILURE);
	    }
	rc = bind(s, (struct sockaddr *)&sin, sizeof(sin));
	if ( rc < 0 ) 
	{
		perror("bind:");
		exit(rc);
	}

	rc = listen(s, num_players);
	if ( rc < 0 ) 
	{
	    perror("listen:");
	    exit(rc);
	}	
	int k;
	map=(int **)malloc((l+1)*sizeof(int));
	for(k=0;k<l+1;k++)
	{
		map[k]=(int*)malloc(2*sizeof(int));
	}
	
	while (1) 
	{
	    
	    len = sizeof(sin);
	
	    p = accept(s, (struct sockaddr *)&incoming, &len);
		
	    if ( p < 0 ) 
	    {
	      perror("bind:");
	      exit(rc);
	    }
	    id++;
		
	    l--;
	    map[id][0]=p;
	    
            ihp = gethostbyaddr((char *)&incoming.sin_addr,sizeof(struct in_addr), AF_INET);
		printf("player %d is on %s \n",(id-1),ihp->h_name);
    	    
	   memset(fumes[id],'\0',64);
	   strcpy(fumes[id],ihp->h_name);
		
	 	
	  
	    {
		int duf,horrible;
		len = recv(p,&duf, sizeof(duf) , 0);
		if ( len < 0 ) 
		{
			perror("recv");
			exit(1);
      	  	}
		
		map[id][1]=duf;
		
		len = send(map[id][0],(char *)&id,sizeof(id),0);
		len = send(map[id][0],(char *)&hops,sizeof(hops),0);
		len = recv(map[id][0],&horrible,sizeof(horrible),0);
		len = send(map[id][0],(char *)&num_players,sizeof(num_players),0);
	    }
 
    	   
	    if(l==0)
	    	break;
 	}
	
	notifyport(id,fumes);
	

	
	int nag,q,dummy;
	nag=1;
	len= send(map[1][0],(char *)&(nag),sizeof(nag),0);
	if(len!=sizeof(nag))
	{
		perror("send");
		exit(1);
	}
	
	
	len= recv(map[1][0],&dummy, sizeof(dummy) , 0); //1
	
	for(q=id;q>0;q--)
	{
		if(q==1)
		{
			len= send(map[q][0],(char *)&(nag),sizeof(nag),0);
			if(len!=sizeof(nag))
			{
				perror("send");
				exit(1);
			}
			continue;
		}
		
		nag=0;
		len= send(map[q][0],(char *)&(nag),sizeof(nag),0);
		if(len!=sizeof(nag))
		{
			perror("send");
			exit(1);
		}
		len = recv(map[q][0],&dummy, sizeof(dummy) , 0); //1
	}
	
	
	sleep(1);
	
	
		
	int v;
	srand( time(NULL) );
	v=rand();
	v=(v%id);
	v++;
	
	printf("All players present, sending potato to player %d \n",(v-1));
	if(hops ==0)
	{
		int kai=2,j;
		for(j=1; j<id+1 ; j++)
		{
			len= send(map[j][0],(char *)&(kai ),sizeof(kai),0);
		}
	
		close(s);
		for(j=1; j<id+1 ; j++)
		{
			close(map[j][0]);
		}
		exit(1);	
	}
	
	
	char chain[2*hops+10];
	memset(chain,'\0',sizeof(char)*(2*hops+10));	
	//sprintf(chain,"%d:",hops);
	strcat(chain,"");
	
	int pai;
	pai=atoi(chain);
	

	int pak=1,bliss;
	len= send(map[v][0],(char *)&(pak),sizeof(pak),0);
	len = recv(map[v][0],&bliss, sizeof(bliss) , 0);
	
	
	len= send(map[v][0],(char *)&(hops ),sizeof(hops),0);
	len = recv(map[v][0],&bliss, sizeof(bliss) , 0);
	
	len= send(map[v][0],(char *)chain,sizeof(chain),0);

	
	{
	}
	fd_set readfds;
	FD_ZERO(&readfds);
	int maxfd,j;
	maxfd=map[1][0];
	for(j=1; j<id+1 ; j++)
	{
		FD_SET(map[j][0],&readfds);
		
		if(map[j][0] > maxfd)
			maxfd= map[j][0];
	}
	
	int present,kai=2;
	int stratus=0;
	char pain[2*hops+10];
	memset(pain,'\0',sizeof(char)*(2*hops+10));
	
	select(maxfd+1,&readfds , NULL , NULL , NULL);
	
	int jadeja=0;
	for(j=1; j<id+1 ; j++)
	{
		if (FD_ISSET(map[j][0] , &readfds))
		{
			while(1)
			{
				len = recv(map[j][0] ,pain+jadeja, sizeof(pain) , 0);
				jadeja+=len;
				if(jadeja==(2*hops+10))
					break;
			}
			jadeja=0;
			break;
		}
	}
	
	
	
	printf("Trace of potato: \n");
	printf("%s \n",pain);
	
	for(j=1; j<id+1 ; j++)
	{
		len= send(map[j][0],(char *)&(kai ),sizeof(kai),0);
	}
	
	close(s);
	for(j=1; j<id+1 ; j++)
	{
		close(map[j][0]);
	}
	exit(0);

}
