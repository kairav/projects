#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>


int r_s;
int c_s;


typedef struct node
{
	char name[256];
	char pname[256];
	char* data;
	
	struct stat *attrib;	
	struct node* next;
	struct node* child;
	struct node* parent;
}node;

node* head_dir;
char filename[256];

int isvalidpath(const char *lath)
{
	printf("entered isvalid path \n");
	char* path;
	
	path=strdup(lath);
	printf("isv: path value is %s \n",path);
	node* temp;
	temp=head_dir->child;
	char *token;
	token= strtok(path,"/");
	printf("isv: token value is %s \n",token);
	if(token ==NULL){
		printf("11 \n");
		return 1; }
	if(temp==NULL)
	{
		printf("12 \n");
		return 0;
	}
	int q,flag=0;
	printf("13 \n");	
	while(token!=NULL || temp!=NULL)
	{
		q=0;
		printf("14 \n");
		while(temp!=NULL )
		{
			if(strcmp(temp->name,token)==0)
			{
				q=1;
				break;
			}
			temp=temp->next;
		}
		printf("15 \n");
		if(q==0)
		{
			printf("isv: end of isv \n");
			return 0;
		}
		printf("16 \n");
		token=strtok(NULL, "/");
		temp=temp->child;
		if(token == NULL)
			return 1;
		if(temp==NULL)
		{
			return 0;
		}
	}
	
	printf("isv: end of isv \n");
	return 1;
}



node* getnode(const char *lath)
{
	char* path;
	path=strdup(lath);
	node* temp;
	node* ptr;
	ptr=head_dir;
	temp=head_dir->child;	
	char *token;
	token=(char*)malloc(sizeof(char));
	token= strtok(path,"/");
	if(token ==NULL)
		return ptr;
	if(temp==NULL)
	{
		strcpy(filename,token);
		return ptr;
	}
	int q,flag=0;	
	while(token!=NULL || temp!=NULL)
	{
		strcpy(filename,token);
		q=0;
		while(temp!=NULL )
		{
			if(strcmp(temp->name,token)==0)
			{
				q=1;
				break;
			}
			temp=temp->next;
		}
		if(q==0)
		{
			return ptr;
		}
		token=strtok(NULL, "/");
		ptr=temp;		
		temp=temp->child;
		if(token == NULL)
			return ptr;
		if(temp==NULL)
		{
			strcpy(filename,token);
			return ptr;
		}
	}
	
	return ptr;
}

static int ram_getattr(const char *path, struct stat *stbuf)
{
	int res = 0,temp;
	printf("inside getattr \n");
	if(path==NULL)
	{
		printf("invalid path passed \n");
		return -EPERM;
	}
	printf("path passed is %s \n",path);
	if (strcmp(path, "/") == 0) 
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		return res;
	}
	if(head_dir->child==NULL)
		printf("ikkada NULL child babu \n");
	temp=isvalidpath(path);
	printf("value if isvalidpath returned is %d \n",temp);
	if(temp==0)
	{
		printf("getattr will return fail \n");
		res=-ENOENT;
		
	}
	if(temp==1)
	{
		node* ptr;
		ptr=getnode(path);
		stbuf->st_mode = ptr->attrib->st_mode;
		stbuf->st_nlink = ptr->attrib->st_nlink;
		stbuf->st_size = ptr->attrib->st_size;
		
		stbuf->st_uid = ptr->attrib->st_uid;
		stbuf->st_gid = ptr->attrib->st_gid;
		return res;
		
	}
	/*memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path, ram_path) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(ram_str);
	} else
		res = -ENOENT;
	*/
	return res;
}

static int ram_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
	/*(void) offset;
	(void) fi;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, ram_path + 1, NULL, 0);

	return 0; */
	printf("inside readdir \n");
	node *ptr,*temp;
	int r=0;
	printf("path passed is %s \n",path);
	int flag;
	flag= isvalidpath(path);
	
	if(flag == 1)
	{
		ptr= getnode(path);
		temp= ptr->child;
		filler(buf, ".", NULL, 0);
        	filler(buf, "..", NULL, 0);
		while(temp)
		{
			filler(buf,temp->name,NULL,0);
			temp=temp->next;
		}
	}
	else
		r= -ENOENT;
	printf("readdir returning the value %d \n",r);	
	return r;
	
}

int ram_mkdir(const char *path, mode_t mode)
{
	node *ptr, *tmp, *new;
	ptr = getnode(path);
	
	if(c_s < sizeof(node) + sizeof(struct stat)) 
	{
		printf("RamDisk space is used up \n");
		return -EFBIG;
	}

	

	new = (node *) malloc(sizeof(node));
	new->attrib = (struct stat*) malloc(sizeof(struct stat));   
	c_s -= (sizeof(node) + sizeof(struct stat));
	
	strcpy(new->name, filename);
	strcpy(new->pname, ptr->name);
	new->attrib->st_mode = mode;
	new->attrib->st_size = 0;
	new->attrib->st_nlink = 2;
	ptr->attrib->st_nlink +=1;
	new->attrib->st_uid = getuid();
	new->attrib->st_gid = getgid(); 
	new->parent = ptr;
	new->child = NULL;
	new->next = NULL;
	new->attrib->st_mode = S_IFDIR | 0755;

	if (ptr->child == NULL)
	{
	        ptr->child = new;
        }
	else 
	{
		for (tmp = ptr->child; tmp->next; tmp = tmp->next);
		tmp->next = new;
	}
	return 0;
}


node* getprev(node* dup)
{
	node *par,*tmp;
	par = dup->parent;
	tmp = par->child;	
	while(tmp)
	{
		if(tmp->next == dup)
			return tmp;
		tmp = tmp->next;
	}	
	return tmp;	
}

int ram_unlink(const char *path)
{
	printf("entered the unlink function \n");	
	int flag;
	flag=isvalidpath(path);
	if(flag==0)
		return -ENOENT;
	node *ptr,*temp;
	ptr=getnode(path);
	temp= ptr->parent;
	printf("kairav: in the unlink function the path is %s and the node fetched is %s \n",path,ptr->name);
	if(temp->child == ptr)
	{
		if(ptr->next==NULL)
			temp->child=NULL;
		else
			temp->child = ptr->next;
	}
	else
	{
		node* prev;
		prev = getprev(ptr);
		printf("kairav: in the unlink function the path is %s and the prev node fetched is %s \n",path,prev->name);
		prev->next= ptr->next;
	}

	c_s += (sizeof(node) + sizeof(struct stat) );
	
	if(ptr->attrib->st_size>0)
	{
		c_s += 	ptr->attrib->st_size;
		free(ptr->data);
	}
	free(ptr->attrib);
	free(ptr);
	
	return 0;
}
void freetree(node* ptr)
{
	if(ptr == NULL)
		return;
	if(ptr->next==NULL && ptr->child == NULL)
	{
		printf("directory size is %d \n",(sizeof(node) + sizeof(struct stat)));
		c_s += (sizeof(node) + sizeof(struct stat) );
		if(ptr->attrib->st_size>0)
		{
			c_s += 	ptr->attrib->st_size;
			free(ptr->data);
		}
		printf("in freetree: the new available memory is %d \n",c_s);
		free(ptr->attrib);
		free(ptr);
		return;
	}
	freetree(ptr->next);
	freetree(ptr->child);
	printf("directory size is %u \n",(sizeof(node) + sizeof(struct stat)));
	c_s += (sizeof(node) + sizeof(struct stat) );
	if(ptr->attrib->st_size>0)
	{
		c_s += 	ptr->attrib->st_size;
		free(ptr->data);
	}
	printf("in freetree: the new available memory is %d \n",c_s);
	free(ptr->attrib);
	free(ptr);
	return;
}
int ram_rmdir(const char *path)
{
	printf("in the rmdir call \n");
	int flag;
	flag=isvalidpath(path);
	if(flag==0)
		return -ENOENT;
	node *ptr,*temp;
	ptr=getnode(path);
	temp= ptr->parent;
	printf("kairav: in the rmdir function the path is %s and the node fetched is %s \n",path,ptr->name);
	if(temp->child == ptr)
	{
		temp->child = ptr->next;
	}
	else
	{
		node* prev;
		prev = getprev(ptr);
		printf("kairav: in the rmdir function the path is %s and the prev node fetched is %s \n",path,prev->name);
		prev->next= ptr->next;
	}
	temp->attrib->st_nlink -= 1;
	freetree(ptr->child);
	c_s += (sizeof(node) + sizeof(struct stat) );
	
	if(ptr->attrib->st_size>0)
	{
		c_s += 	ptr->attrib->st_size;
		free(ptr->data);
	}
	free(ptr->attrib);
	free(ptr);
	return 0;
}
static int ram_open(const char *path, struct fuse_file_info *fi)
{
	/*if (strcmp(path, ram_path) != 0)
		return -ENOENT;

	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return 0;*/
	printf("entering open function for path %s \n",path);
	if(path == NULL || strcmp(path,"/") == 0 || '/'== (* (path+strlen(path)-1)) )
	{
		printf("path %s is invalid \n",path);
		return -ENOENT;
	}
	int status;
	status = isvalidpath(path);
	if(status == 1)
	{
		printf("path %s is valid \n",path);
		return 0;
	}
	else 
	{
		printf("path %s is invalid \n",path);
		return -ENOENT;
	}
	
}

int ram_truncate(const char *path, off_t offset)
{
	//printf("inside truncate  and the offset is  %d \n",offset);	
	if(path==NULL)
	{
		return -ENOENT;
	}
	if( strcmp(path, "/")==0 )
	{
		
	}
	else
	{
		node* ptr;
		int status;
		status = isvalidpath(path);
		if(status ==0)
			return -ENOENT;
		ptr= getnode(path);
		size_t len = ptr->attrib->st_size;
		//printf("data size is %u \n",len);
		//printf("1 \n");
		if(offset == 0)
		{
			free(ptr->data);
			ptr->attrib->st_size = 0;
			c_s += len;
			return 0;
		}
		if(offset > len)
		{
			if(c_s < (offset - len))
				return -ENOMEM;
		}
			//ptr->data=(char *)realloc(ptr->data,(sizeof(char)*(offset+size)));
			printf("2 \n");			
			char *copy;
			copy = (char *)malloc(sizeof(char)*len);
			memcpy(copy,ptr->data, len);
			ptr->data= (char *)realloc(ptr->data,(sizeof(char)* offset));
			//printf("3 \n");		
			memset(ptr->data, 0, offset);
		if(offset > len)
		{
			strcpy(ptr->data,copy);
		}
		else
		{
			memcpy(ptr->data,copy, offset);
			ptr->data[offset-1]='\0';
		}
		//printf("4 \n");
		ptr->attrib->st_size = offset;
		c_s += len;
		c_s -= offset ;		
		
	}
	return 0;	
}


/* reference from fuseexmp_fh.c example given in the fuse package*/
static int ram_opendir(const char *path, struct fuse_file_info *fi)
{
	printf("inside open dir\n");
	int r=0,stat;
	
	if(path==NULL)
	{
		return -ENOENT;
	}
	
	if( strcmp(path, "/")==0 )
	{
		printf("opendir returning valid path \n");
		return r; 
	}
	if('/'== (* (path+strlen(path)-1)))
	{
		return -ENOENT;
	}
	stat=isvalidpath(path);
	if(stat==1)
	{
		printf("opendir returning valid path \n");
		return r;
	}	
	else 
	{
		printf("opendir returning invalid path \n");
		return -ENOENT;
	}
}

static int ram_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	/*size_t len;
	(void) fi;
	if(strcmp(path, ram_path) != 0)
		return -ENOENT;

	len = strlen(ram_str);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, ram_str + offset, size);
	} else
		size = 0;

	return size;*/

	if(path == NULL)
		return -ENOENT;
	if(path == NULL || strcmp(path,"/") == 0 || '/'== (* (path+strlen(path)-1)) )
	{
		printf("invalid path type 1 in read \n");
		return -ENOENT;
	}
	int status;
	node *ptr;
	status= isvalidpath(path);
	size_t len;
	
	if(status == 1)
	{
		ptr= getnode(path);
		printf("in read function for the path %s got the pointer %s \n",path,ptr->name);
		len=ptr->attrib->st_size;
		if(offset < len)
		{
			if (offset + size > len)
				size = len - offset;
			memcpy(buf,ptr->data + offset , size) ;
		}
		else
			size =0;		
	}
	else
	{
		return -ENOENT;
	}
	
	return size;
	
}

int ram_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("entered the create function \n");
	node *ptr, *tmp, *new;
	printf("available space is %d \n",c_s);
	if(c_s < sizeof(node) + sizeof(struct stat)) 
	{
		printf("RamDisk space is used up \n");
		return -ENOMEM;
	}

	ptr=getnode(path);
	printf("Kairav: inside the create the path is %s and the filename is %s \n",path,filename);
	
	new = (node *) malloc(sizeof(node));
	new->attrib = (struct stat*) malloc(sizeof(struct stat));   
	c_s -= (sizeof(node) + sizeof(struct stat));
	
	strcpy(new->name, filename);
	strcpy(new->pname, ptr->name);
	
	new->attrib->st_size = 0;
	new->attrib->st_nlink = 1;
	new->data = NULL;
	new->attrib->st_uid = getuid();
	new->attrib->st_gid = getgid(); 
	new->parent = ptr;
	new->child = NULL;
	new->next = NULL;
	new->attrib->st_mode = S_IFREG | 0444;
	if (ptr->child == NULL)
	{
	        ptr->child = new;
        }
	else 
	{
		for (tmp = ptr->child; tmp->next; tmp = tmp->next);
		tmp->next = new;
	}
	return 0;
}

int ram_write(const char *path, const char *buf, size_t size, off_t offset,
	     struct fuse_file_info *fi)
{
	printf("in write function \n");
	printf("write path is %s and the memory available is %d \n",path,c_s);
	if( size > c_s)
	{
		printf("not enough memory available \n");
		return -ENOMEM;
	}	
	node* ptr;
	ptr=getnode(path);
	printf("kairav: in write function the node returned is %s and the path is %s \n",ptr->name,path);
	size_t len=ptr->attrib->st_size;
	
	printf("write path is %s and the memory available is %d \n",path,c_s);
	
	if(len==0)
	{
		ptr->data=(char *)malloc(sizeof(char)*size);
		c_s -= size;
		if(size)
			memcpy(ptr->data, buf, size);
		ptr->attrib->st_size= size;
	}
	else if (len>0)
	{
		if (offset > len)
		{
			offset = len;
		}
			
		ptr->data=(char *)realloc(ptr->data,(sizeof(char)*(offset+size)));
		c_s -= (offset + size);
		c_s += len;
		memcpy(ptr->data+offset, buf, size);
		ptr->attrib->st_size= offset+size;
	}
	return size;	
}

int ram_rename(const char *path, const char *newpath)
{
	
}
static struct fuse_operations ram_oper = {
	.getattr	= ram_getattr,
	.readdir	= ram_readdir,
	.open		= ram_open,
	.read		= ram_read,
	.mkdir 		= ram_mkdir,
	.opendir 	= ram_opendir,
	.write		= ram_write,
	.create		= ram_create,
	.unlink		= ram_unlink,
	.rmdir		= ram_rmdir,
	.truncate	= ram_truncate
	.rename 	= ram_rename,
};

int main(int argc, char *argv[]) 
{
	printf("1\n");
	int ram_size;
	char* f_argv[2];
	ram_size=atoi(argv[2]);
	r_s= ram_size*1024*1024;
	c_s=ram_size*1024*1024;
	printf("3\n");
	printf("r_S and c_s values are %d and %d \n",r_s,c_s);
	if(ram_size < 0)
	{
		printf("Invalid size of ram disk \n");
		exit(0);
	}
	
	int dead,dead_1,dead_2;
	head_dir=(node*)malloc(sizeof(node));
	strcpy(head_dir->name,"/");
	strcpy(head_dir->pname,"broot");
	printf("7\n");
	head_dir->next=NULL;
	head_dir->child=NULL;
	head_dir->parent=head_dir;
	head_dir->attrib=(struct stat *) malloc(sizeof(struct stat));
	head_dir->attrib->st_nlink = 2;
	head_dir->attrib->st_size=0;
    head_dir->attrib->st_mode = S_IFDIR | 0755;
		
	printf("head_dir created with name %s \n",head_dir->name);
	f_argv[0]=argv[0];
	for(dead=0;dead<2;dead++)
	{
		dead_1=3;
		dead_2=5;
		dead_2=dead_1>>3;
	}
	f_argv[1]=argv[1];
	
	
	return fuse_main(argc-1,argv,&ram_oper,NULL);
}
