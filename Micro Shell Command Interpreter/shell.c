#include <stdio.h>
#include <stdlib.h>
#include "parse.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
extern char **environ;
char *home, *cwd, **envp, *host_n ;
int f_in,f_out;
int old_fd[2],new_fd[2];
pid_t pid[9999];
int proc=0;
int lol=0;
int i=0;
int bak_in,bak_out;
int prev_cmd=0;
int pip[100][2];

static void prCmd(Cmd c);
static void prPipe(Pipe p);
void readfrmush(void);

int stdin_back,stdout_back,stderr_back;
void erro_handling(int err_num,char *c);


void error_handling(int err_num,char *c)
{
	if(err_num==EACCES)
	{
		fprintf(stderr, "Permission denied\n");
		return;
	}
	else 
	{
		fprintf(stderr, "Command is not found %s\n",c);
		return;
	}
}

void changedir( char *dir)
{
	int t;
	t=chdir(dir);
	if(t==0)
		return;
	else
	{
		perror("change directory error \n");
		return;
	}
}

int checkvalidprio(int prio)
{
	if(prio<21 && prio > -21)
		return prio;
	else if (prio > 20)
		return 20;
	else if (prio < -20)
		return -20;
}

int isbuiltin(char *cm)
{
	int r=0;


	if (!strcmp(cm,"bg") || !strcmp(cm,"cd") || !strcmp(cm,"fg") || !strcmp(cm,"echo") || !strcmp(cm,"jobs") || !strcmp(cm,"kill") || !strcmp(cm,"logout") || !strcmp(cm,"nice") || !strcmp(cm,"pwd") || !strcmp(cm,"setenv") || !strcmp(cm,"unsetenv") || !strcmp(cm,"where"))
	r=1;
	//abc
	
	return r;
}


void execbuiltin(Cmd c)
{
	int t;
	if(!strcmp(c->args[0],"cd"))
	{
		if(c->nargs > 2)
		{
			
			write(2, "Invalid number of arguments for cd\n",24 );

			return;
		}		
		if(c->args[1]==NULL || c->nargs == 1)
		{
			home = getenv("HOME");
			changedir(home);
		}
		else if(c->args[1][0]=='/')
		{
			changedir(c->args[1]);
			return;
		}
		else
		{
			/*printf("entered the last case of cd \n");
			if ((cwd=getcwd(NULL, 64)) != NULL);
           			//fprintf(stdout, "Current working dir: %s\n", cwd);
       			else
          			 perror("getcwd() error");*/
			//cwd=getcwd(NULL, 64);
       			//printf("%s\n",cwd);
			//strcat(cwd,c->args[1]);
			//printf("cwd is %s",cwd);
			//changedir(cwd);
			
			changedir(c->args[1]);
			return;
		}
	}
	else if(!strcmp(c->args[0],"echo"))
	{
		int i;
		for(i=1;c->args[i]!=NULL;i++)
		{
			
			printf("%s ",c->args[i]);			
		
		}
		printf("\n");
	}
	else if(!strcmp(c->args[0],"pwd"))
	{
		cwd=getcwd(NULL, 64);
       		printf("%s\n",cwd);
	}
	else if(!strcmp(c->args[0],"logout"))
	{
		exit(0);
	}
	else if(!strcmp(c->args[0],"nice"))
	{
		int prio,ret;		
		if(c->args[1]==NULL)
		{
			prio=4;
			ret=nice(prio);
			if(ret==-1)
			{
				printf("error changing process priority");
			}
		}
		else if(c->args[2]==NULL)
		{
			if (c->args[1][0]=='-')
			{
				prio=atoi(c->args[1]+1);
				prio=prio*(-1);
				prio=checkvalidprio(prio);
				ret=nice(prio);
				if(ret==-1)
				{
					printf("error changing process priority");
				}	
			}
			else 
			{
				if (c->args[1][0]=='+')
				{
					prio=atoi(c->args[1]+1);
					prio=checkvalidprio(prio);
					ret=nice(prio);
					if(ret==-1)
					{
						printf("error changing process priority");
					}

				}
				else
				{
					prio=atoi(c->args[1]);
					prio=checkvalidprio(prio);
					ret=nice(prio);
					if(ret==-1)
					{
						printf("error changing process priority");
					}
				}
			}
			
		}
		else if(c->args[2]!=NULL)
		{
			
			if (c->args[1][0]=='+')
				{
					prio=atoi(c->args[1]+1);
					prio=checkvalidprio(prio);
					
				}
				else
				{
					prio=atoi(c->args[1]);
					prio=checkvalidprio(prio);
					
				}
			pid_t gid;
			int status;
			gid=fork();
			if(gid==0)
			{
				ret=nice(prio);
				if(ret==-1)
				{
					printf("error changing process priority");
				}
				if(execvp(c->args[2],c->args+2)<0)
				{
					
					error_handling(errno,c->args[2]);
					_exit(errno);
				}

			}
			else
			{
				waitpid(gid,&status,0);
				
			}
		}
		
	}
	else if(!strcmp(c->args[0],"setenv"))
	{ 
		
		if(c->args[1]==NULL)
		{
			char **env;
			env=environ;
			while (*env!=NULL)
 			{
   			 //printf("%d \n",index);
    			 printf("%s\n", *env);
				++env;
			}
		}
		else
		{
			if(c->args[2]==NULL)
			{
				if( (setenv(c->args[1],NULL,1))== -1){
			
					printf("error setting the environment variables \n");}

			}
			else
			{
				if( (setenv(c->args[1],c->args[2],1))== -1){


					printf("error setting the environment variables \n");}

			}
		}
	}

	else if(!strcmp(c->args[0],"unsetenv"))
	{
		
		
		if(c->args[0]==NULL)
			return;
		
		unsetenv(c->args[1]);
	
	       	
		return;		
	}
	else if(!strcmp(c->args[0],"where"))
	{
		char *path,**token,i=0;
		path=(char *) calloc(2048, sizeof(char));
		if(isbuiltin(c->args[1])==1)
		{
			printf("%s is a builtin command \n", c->args[1]);
			return ;
		}
		
		pid_t mid;
		mid=fork();
		if(mid==0)
		{
			strcat(c->args[0],"is");
			if(execvp(c->args[0],c->args)<0)
			{
				error_handling(errno,c->args[0]);
				_exit(errno);
			}

		}
		else
		{
			waitpid(mid,NULL,0);
		}
	}
}


void closefd(Cmd c )
{
 if(c)
 {
	if( c->out == Tout | c->out ==Tapp | c->out ==ToutErr | c->out == TappErr  ){
		
		dup2(stdout_back, STDOUT_FILENO);
	
		close(stdout_back);		

		close(f_out);
	}
	if(c->out== ToutErr | c->out== TappErr)
	{
		dup2(stderr_back, STDERR_FILENO);
	
		close(stderr_back);

	}
	if (c->in==Tin)
	{
		dup2(stdin_back, STDIN_FILENO);
		
		close(f_in);
		
		close(stdin_back); 
	}
 }
}

void executeCmd(Cmd c)
{
	if (c==NULL)
		return;

		
	if(isbuiltin(c->args[0]))
	{	
		execbuiltin(c);
		_exit(EXIT_SUCCESS);				
	}
	else
	{
				
		if(execvp(c->args[0],c->args)<0)
		{
			error_handling(errno,c->args[0]);
			_exit(errno);
		}
		
		
	}
	
	
	
}

void macroexec(Cmd c)
{
	//printf("entering the macro for i %d \n",i);
	int status;	
	prCmd(c);
	//printf("value of lol is %d \n",lol);
	proc++;
	pid[proc]=fork();
	if(pid[proc] ==0)
	{
		
		if(prev_cmd)
		{
				
			
			dup2(pip[i-1][0],0);
			
			close(pip[i-1][1]);
			close(pip[i-1][0]);
			
		}
		if( c->out==Tpipe || c->out==TpipeErr )
		{
		
			dup2(pip[i][1],1);
			close(pip[i][0]);
			
			if(lol==6 || c->out==TpipeErr)
			{
				dup2(pip[i][1],STDERR_FILENO);
			}	
			//there is next command 
			close(pip[i][1]);
		}
		executeCmd(c);
	}
	else
	{	
		waitpid(pid[proc],NULL,0);
		close(pip[i][1]);
		closefd(c);
	}
	prev_cmd=1;
	i++;
	//printf("the i value is %d \n",i);
	lol=0;
}
static void prPipe(Pipe p)
{
  	int j,m=0;
	pid_t cid;
  	Cmd c,e;
	prev_cmd=0;
	lol=0;
	proc=0;
	
  	if ( p == NULL )
    		return;

	c=p->head;
	if(c==NULL)
		return;
	
	if(c->next!=NULL)
	{
		//printf("there is more than one command");
		m=1;
	}
	
  	for(j=0;j<100;j++)
	{
		if(pipe(pip[j])==-1)
			printf("error creating the pipe");
	}
	if(m==0)
	{
		//there is only one command 
		prCmd(c);
		if(isbuiltin(c->args[0]))
		{
			//the only command is the buitin command 
			execbuiltin(c);
			closefd(c);
			lol=0;
		}
		else
		{
			cid=fork();
			if(cid==0)
			{
				executeCmd(c);
				_exit(EXIT_SUCCESS);
			}
			else
			{
				waitpid(cid,NULL,0);
				closefd(c);
				lol=0;
			}

		}
	}
	else if(m==1)
	{
		e=c;
		while(e!=NULL)
		{
			macroexec(e);
			e=e->next;
		}		
		//there are multiple commands
	}
  
	for(j=0;j<100;j++)
	{
		close(pip[j][0]);		
		close(pip[j][1]);
	}
  prPipe(p->next);
}






static void prCmd(Cmd c)
{
	
	//file descriptors
	
	if( !strcmp(c->args[0], "end") || !strcmp(c->args[0], "logout"))
      		exit(0);
	
if ( c ) 
 {
	    //printf("%s%s ", c->exec == Tamp ? "BG " : "", c->args[0]);

		if ( c->in == Tin )
		{
			lol=11;

			stdin_back = dup(fileno(stdin));
			
      			//printf("<(%s) ", c->infile);
			f_in=open(c->infile,O_RDONLY);
			
			dup2(f_in,0);

		}

	if ( c->out != Tnil )
	{
	      switch ( c->out ) 
		{
      			case Tout:
			//printf(">(%s) ", c->outfile);
			stdout_back = dup(fileno(stdout));

			f_out=open(c->outfile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
			if(f_out==-1){
				printf("error with the output file I/O handling");
				return;
			}			
			dup2(f_out,1);
			close(f_out);
			lol=1;
			
			//close(f_out);
			
			break;
		      case Tapp:
			
			stdout_back = dup(fileno(stdout));

			
			f_out=open(c->outfile, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
			if(f_out==-1){
				printf("error witht the output file I/O handling");
				return;
			}			
			dup2(f_out,1);
			close(f_out);
			lol=2;
			
			//close(f_out);
			
			break;
	              case ToutErr:
			
			stderr_back = dup( STDERR_FILENO);
				
			stdout_back = dup(fileno(stdout));

			//printf(">&(%s) ", c->outfile);
			f_out=open(c->outfile, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
			if(f_out==-1){
				printf("error witht the output file I/O handling");
				return;
			}			
			dup2(f_out,1);
			dup2(f_out,2);
			close(f_out);
			lol=3;
			
			//close(f_out);
			
			break;
     		      case TappErr:
			
			stderr_back = dup( STDERR_FILENO);
			
			stdout_back = dup(fileno(stdout));

			//printf(">>&(%s) ", c->outfile);
			
	 		f_out=open(c->outfile, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
			
			if(f_out==-1){
				printf("error with the output file I/O handling");
				return;
			}			
			dup2(f_out,1);
			dup2(f_out,2);
			close(f_out);
			lol=4;
			
			//close(f_out);
			
			break;
     	              case Tpipe:
			//printf("| ");
			//printf("case tpipe is reached for i value %d \n",i);
			lol=5;
			break;
     		      case TpipeErr:
			//printf("|& ");
			lol=6;
			break;
      		      default:
			fprintf(stderr, "Shouldn't get here\n");
			exit(-1);
     		 }
	}
	else
	{
		lol=7;
	}

    
    
  
 }
}



void readfrmush(void)
{
	int fin_bkup,fout_bkup,ferr_bkup;	
	char *ushp,*cop;
	ushp=NULL,cop=NULL;
	ushp=(char *) calloc(1024, sizeof(char));
	cop=(char *) calloc(1024, sizeof(char));
	ushp=getenv("HOME");
	strcpy(cop,ushp);
	strcat(cop,"/.ushrc");
	int ud=open(cop, O_RDONLY);
	if(ud==-1)
	{
		fprintf(stderr,"Error opening the ush file \n");
		return;
	}
	fin_bkup=dup(fileno(stdin));
	/*fout_bkup=dup(fileno(stdout));
	ferr_bkup=dup(fileno(stderr));*/
	dup2(ud,0);
	close(ud);
	Pipe p;
	while(1)
	{
		p=parse();
		
		
		if(p==NULL || p->head==NULL ){
		
			break;
		}
		if ( !strcmp(p->head->args[0], "end") || !strcmp(p->head->args[0], "logout")){
			
			break;
		}		
					
		prPipe(p);
		/*dup2(fin_bkup,0);
		dup2(fout_bkup,1);	
		dup2(ferr_bkup,2);*/
	}
	dup2(fin_bkup,0);
	//close(0);
	close(ud);
	
	return;
}

void handle_signal(int signum)
{
    if (signum == SIGTERM)
    {
        //do nothing
    }
    if (signum == SIGINT)
    {
        write(1, "\n", 1);
	//hello
        write(1, host_n, strlen(host_n));
        //do nothing
    }
    if (signum == SIGTSTP)
    {
        write(1, "\n", 1);
        write(1, host_n, strlen(host_n));
    }
}

void signl_h(void)
{	
	//SIGHUP
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM , handle_signal);
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, handle_signal);
	signal(SIGTSTP, handle_signal);
	
}

int main(int argc, char *argv[],char *envp[])
{
		
	
	cwd=(char *) calloc(2048, sizeof(char));
	home=(char *) calloc(2048, sizeof(char));
	host_n=	(char *) calloc(300, sizeof(char));
	cwd=NULL;
	home=NULL;
	//host_n=NULL;
	Pipe p;
	host_n=getenv("HOSTNAME");
	if(host_n == NULL)
	{
		//printf("entered the worst condition \n");
		host_n="csh% ";
	}
	else 
	strcat(host_n," %");
	//printf("%s",host_n);
	signl_h();

	readfrmush(); //parse ush and execute the commands
	//fflush(stdout);
	fflush(stdin);
		
	
	while(1)
	{

		fflush(NULL);
		
		host_n=getenv("HOSTNAME");
		if(host_n == NULL)
		{
			host_n="csh% ";
		}
		else
		strcat(host_n,"%");
		if( isatty(STDIN_FILENO) )
		printf("%s", host_n);
		fflush(NULL);
		fflush(stdin);
		p = parse();
		prPipe(p);
		freePipe(p);
		
	}
  
	
}





