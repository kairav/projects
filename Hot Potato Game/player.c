#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/select.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define LEN	256



int r_port;
int oport,pport;
int gen_random(void)
{
	int v;
	v=rand();
	v=v%2;
	return v;
		
}
main (int argc, char *argv[])
{
	int s, rc, len,s1,s2,yen;
	int own,other;
	char machine_name[LEN],buf[512];
	struct hostent *hp,*hp1,*hp2;
	struct sockaddr_in sin,sin1,sin2,dummy,yummy;
	socklen_t pen = sizeof(dummy);
	yen=sizeof(yummy);
	
	/* read host and port number from command line */
	if ( argc != 3 ) 
	{
		fprintf(stderr, "Usage: %s <host-name> <port-number>\n", argv[0]);
		exit(1);
	}
	
	hp = gethostbyname(argv[1]);
	if ( hp == NULL ) 
	{
		//fprintf(stderr, "%s: host not found (%s)\n", argv[0], machine_name);
		exit(1);
	}
	r_port = atoi(argv[2]);
	
	
	s = socket(AF_INET, SOCK_STREAM, 0);
	if ( s < 0 ) 
	{
		perror("socket:");
		exit(s);
	}
	
	
	sin.sin_family = AF_INET;
	sin.sin_port = htons(r_port);
	memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);

	
	rc = connect(s, (struct sockaddr *)&sin, sizeof(sin));
	if ( rc < 0 ) 
	{
		perror("connect:1");
		exit(rc);
	}
	
	gethostname(machine_name, sizeof machine_name);
	hp1 = gethostbyname(machine_name);
	
	s1= socket(AF_INET, SOCK_STREAM, 0);
	if ( s1 < 0 ) 
	{
		perror("socket:");
		exit(s1);
	}


	sin1.sin_family = AF_INET;
	sin1.sin_port = htons(0);
	memcpy(&sin1.sin_addr, hp1->h_addr_list[0], hp1->h_length);	

	rc = bind(s1, (struct sockaddr *)&sin1, sizeof(sin1));
	if ( rc < 0 ) 
	{
		perror("bind 3:");
		exit(rc);
	}

	if (getsockname(s1, (struct sockaddr *)&dummy, &pen) == -1)
    		perror("getsockname");
	else
	{
		oport=ntohs(dummy.sin_port);
    		//printf("port number %d\n", oport);
	}

	
	len = send(s,(char *)&oport,sizeof(oport),0);
	
	int id;
	
	len = recv(s,&id,sizeof(id),0);
	if ( len < 0 ) 
	{
		perror("recv");
		exit(1);
      	}
	printf("Connected as player %d \n",id-1 );


	int hops,horrible=1,players ;
	len = recv(s,&hops ,sizeof(hops ),0);
	if ( len < 0 ) 
	{
		perror("recv");
		exit(1);
      	}
	len = send(s,(char *)&horrible,sizeof(horrible),0);
	len = recv(s,&players ,sizeof(players ),0);
	
	

	char band[64];
	memset(band,'\0',64);

	len = recv(s,&pport,sizeof(pport),0);
	len = send(s,(char *)&horrible,sizeof(horrible),0);
	len = recv(s,band,64,0);
	husband[63]='\0';

	
	if ( len < 0 ) 
	{
		perror("recv");
		exit(1);
      	}
	int kj;
	
	if(strcmp(husband,"localhost")==0)
	hp2 = gethostbyname(machine_name);
	else 
	hp2 = gethostbyname(husband);
	
	if ( hp2 == NULL ) 
	{
	    	fprintf(stderr, "%s: host not found (%s)\n", argv[0], machine_name);
    		exit(1);
  	}	
	
	
	rc = listen(s1, 2);
	if ( rc < 0 ) 
	{
		perror("listen:");
		exit(rc);
	}

	s2= socket(AF_INET, SOCK_STREAM, 0);
	if ( s2 < 0 ) 
	{
		perror("socket:");
		exit(s);
	}
	sin2.sin_family = AF_INET;

	sin2.sin_port = htons(pport);
	memcpy(&sin2.sin_addr, hp2->h_addr_list[0], hp2->h_length);

	int nag,p,x,mummy=1,brahmi,fake;
	len = recv(s,&nag,sizeof(nag),0);
	if ( len < 0 ) 
	{
		perror("recv");
		exit(1);
      	}
	
	if(nag==1)
	{
		
		len = send(s,(char *)&mummy,sizeof(mummy),0);
		if(len!=sizeof(mummy))
		{
			perror("send");
		}

		x = accept(s1, (struct sockaddr *)&yummy, &pen);
		if ( x < 0 ) {
			perror("accept error :");
      			exit(rc);
		}

		other=x;
		len = recv(s,&fake,sizeof(fake),0);
		rc = connect(s2, (struct sockaddr *)&sin2, sizeof(sin2));
		if ( rc < 0 ) 
		{
			perror("connect:2");
			exit(rc);
		}

		
	}
	else
	{

		
		rc = connect(s2, (struct sockaddr *)&sin2, sizeof(sin2));
		if ( rc < 0 ) 
		{
			perror("connect:3");
			exit(rc);
		}
		len = send(s,(char *)&mummy,sizeof(mummy),0);
		x = accept(s1, (struct sockaddr *)&yummy, &pen);
		if ( p < 0 ) {
			perror("bind 2:");
      			exit(rc);
		}
		other=x;
	}
	
	fd_set readfds,tempfds;
	int maxfd,activ,current,v,pump=88,pimp;
	FD_ZERO(&readfds);
	maxfd=s;	
	FD_SET(s,&readfds);
	FD_SET(s2,&readfds);
	if(s2>maxfd)
		maxfd=s2;
	FD_SET(other,&readfds);
	if(other>maxfd)
		maxfd=other;
	

	int flag=0,to,pops,laps,t;
	int jadeja;
	char chain[2*hops+10],temp[2*hops+10],temp1[2*hops+10],temp3[2*hops+10];
	
	memset(chain,'\0',sizeof(char)*(2*hops+10));	
	memset(temp,'\0',sizeof(char)*(2*hops+10));	
	memset(temp1,'\0',sizeof(char)*(2*hops+10));	
	memset(temp3,'\0',sizeof(char)*(2*hops+10));

	char *token;

	char vp[2];
	char name[8];
	strcpy(vp,"");
	sprintf(vp,"%d",id-1);
	
	
	while(1)
	{	
		tempfds=readfds;

		activ=select(maxfd+1,&tempfds , NULL , NULL , NULL);

		if (FD_ISSET( s , &tempfds))
		{
			current=s;
			
		}
		else if(FD_ISSET( other , &tempfds))
		{
			current=other;

		}
		else if(FD_ISSET( s2, &tempfds))
		{
			current=s2;

		}
		len= recv(current,&flag,sizeof(flag),0);
		if(flag==2)
		{

			break;
		}
		else
		{
			
			len = send(current,(char *)&pump,sizeof(pump),0);
			
			len = recv(current,&pops,sizeof(pops),0);
			
			jadeja=0;

			len = send(current,(char *)&pump,sizeof(pump),0);
			
			while(1)
			{
				len = recv(current,chain+jadeja,sizeof(chain),0);
				jadeja+=len;
				if(jadeja==(2*hops+10))
					break;
			}
			jadeja=0;
			
			
			pops--;
			
			if(pops==0)
			{
				strcat(chain,vp);
				len = send(s,chain,sizeof(chain),0);
				printf("I'm it\n");
				continue;
			}
			else
			{
				int direc,otherid;
				
				strcat(chain,vp);
				strcat(chain,",");
				
				v=gen_random();
				if(v==0)
				{
					direc=1;
					to=other;

				}
				else{
					direc=2;
					to=s2;	

				}
				if(id==1 )
				{
					if(direc==2)
						otherid=id;
					else if (direc==1)
						otherid=players-1;
				}
				else if(id==players)
				{
					if(direc==1)
						otherid=id-2;
					else if(direc==2)
						 otherid=0;	 
				}
				else
				{
					if(direc==2)
						otherid=id;
					else if(direc==1)
						otherid=id-2;
				}
				printf("Sending potato to %d \n",otherid);
				pimp=1;

				len = send(to,(char *)&pimp,sizeof(pimp),0);

				len = recv(to, &pump, sizeof(pump) , 0);

				len = send(to,&pops,sizeof(pops),0);
				len = recv(to, &pump, sizeof(pump) , 0);

				len = send(to,chain,sizeof(chain),0);
				continue;
			}
		}  	
	}
		
	close(s1);
	close(s2);
	close(s);
	exit(0);
}
