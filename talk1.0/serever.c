#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<errno.h>
#include<sys/un.h>
#include<string.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<signal.h>
#include<sys/wait.h>
#include<unistd.h>
#include <sqlite3.h>
#include<pthread.h>

#include"Msg.h"
int c1=0;
int c2=0;
int c3=0;
int c4=0;

sqlite3 *ppdb;
sqlite3 *ppdb2;
int sockfd;

pthread_mutex_t mutex ;

struct online
{
    int cfd;
    char name[20];
    struct online *next;
};
struct online*head=NULL;

void *insert_link(struct online*new_user)
    {
        new_user->next=head;
        head=new_user;
    }

int find_fd(char *friend)//私聊时查看好友是否在链表内（是否在线）
    {
        struct online *temp=head;
        while(temp!=NULL)
            {
                if(strcmp(temp->name,friend)==0)
                    {
                        return temp->cfd;
                    }
                temp=temp->next;
            }
        //return NULL;
    }

char*find_self(int cfd)//查看在链表内的自己的名称（是否在线）
    {
        struct online *p=head;
        while(p!=NULL)
            {
                if(p->cfd==cfd)
                    {
                        return p->name;
                    }
                p=p->next;
            }
        return NULL;
    }

int callback1(void *arg,int ncolumn,char**f_value,char**f_name)//检查姓名
{
    c1=1;
    return 0;
}
int callback2(void *arg,int ncolumn,char**f_value,char**f_name)//检查密码
{
    c2=1;
    return 0;
}
int callback3(void *arg,int ncolumn,char**f_value,char**f_name)//检查vip
{
    c3=1;
    return 0;
}
int callback4(void *arg,int ncolumn,char**f_value,char**f_name)//检查是否可以说话
{
    c4=1;
    return 0;
}

void serever_exit(int sig)//退出信号
    {
        if(sig==SIGINT)
            {
                shutdown(sockfd,SHUT_RDWR);
                printf("SEREVER EXIT!\n");
            }
        exit(1);
    }

void *thread_read(void *arg)//服务器接受客户端信息
    {
        pthread_t id=pthread_self();
        pthread_detach(id);
        long cfd=(long)arg;
        Msg *pmsg=(Msg*)malloc(sizeof(Msg));

        while(1)
            {
                pthread_mutex_init(&mutex,NULL);
                memset(pmsg,0,sizeof(Msg));
                int r_n=read(cfd,pmsg,sizeof(Msg));
                if(r_n==0)
                    {
                        printf("client had exited!\n");
                        pthread_exit(NULL);
                    }
                switch(pmsg->action)
                    {
                        case 1://注册
                            {
                                pthread_mutex_lock(&mutex);
                                char sql[2048] = {0};
                                memset(sql,0,sizeof(sql));
                                printf("3333333333\n");
                                sprintf(sql,"select *from stu where name is '%s'",pmsg->name);
                                int ret = sqlite3_exec(ppdb,sql,callback1, NULL,NULL);
                                if(ret != SQLITE_OK)
                                {
                                    printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                    return NULL; 
                                }
                                printf("444444444\n");
                                if(c1==1)//说明查询到相同名称，已经注册过了
                                    {
                                        c1=0;
                                        pmsg->flags=0;
                                        write(cfd,pmsg,sizeof(Msg));
                                    }
                                else//说明查询不到相同名称，开始注册写入
                                    {
                                        //int vip;
                                        //int a=0;
                                        char name[32] = {0};
                                        char passwd[32] = {0};
                                        memset(sql,0,sizeof(sql));
                                        sprintf(sql,"insert into stu (name,passwd)values('%s','%s');",pmsg->name,pmsg->passwd);
                                        int ret = sqlite3_exec(ppdb,sql,NULL,NULL,NULL);
                                        if(ret != SQLITE_OK)
                                            {
                                                printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                                return NULL; 
                                            }
                                        pmsg->flags=2;
                                        write(cfd,pmsg,sizeof(Msg));
                                    }
                                    pthread_mutex_unlock(&mutex);
                                    break;
                            }
                        case 2://登陆
                            {
                                pthread_mutex_lock(&mutex);
                                int log_flag=0;
                                struct online *p=head;
                                while(p!=NULL)
                                    {
                                        if(strcmp(p->name,pmsg->name)==0)
                                            {
                                                pmsg->flags=-2;
                                                log_flag=1;
                                                write(cfd,pmsg,sizeof(Msg));
                                                break;
                                            }
                                        p=p->next;
                                    }
                                if(log_flag==0)
                                    {
                                        char sql[2048] = {0};
                                        memset(sql,0,sizeof(sql));
                                        sprintf(sql,"select *from stu where name is '%s'",pmsg->name);
                                        int ret = sqlite3_exec(ppdb,sql,callback1, NULL,NULL);
                                        if(ret != SQLITE_OK)
                                        {
                                                printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                                return NULL; 
                                        }

                                        if(c1==1)//说明查询到相同名称，已经注册过了，开始登陆
                                            {
                                                c1=0;
                                                memset(sql,0,sizeof(sql));
                                                sprintf(sql,"select *from stu where passwd is '%s'",pmsg->passwd);//判断名称正确时密码是否正确
                                                int ret = sqlite3_exec(ppdb,sql,callback2, NULL,NULL);
                                                if(ret != SQLITE_OK)
                                                    {
                                                    printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                                    return NULL; 
                                                    }
                                                    
                                                if(c2==1)//说明名称正确而且密码正确，执行登陆操作
                                                    {
                                                        c2=0;
                                                        memset(sql,0,sizeof(sql));
                                                        sprintf(sql,"select *from stu where vip=1 and name ='%s' ",pmsg->name);
                                                        int ret = sqlite3_exec(ppdb,sql,callback3, NULL,NULL);//判断是否为vip用户
                                                        if(ret != SQLITE_OK)
                                                            {
                                                            printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                                            return NULL; 
                                                            }
                                                        if(c3==1)
                                                            {
                                                                c3=0;
                                                                struct online *new_user=(struct online*)malloc(sizeof(struct online));
                                                                strcpy(new_user->name,pmsg->name);
                                                                new_user->cfd=cfd;
                                                                insert_link(new_user);
                                                                pmsg->flags=3;//说明为vip用户注册
                                                                write(cfd,pmsg,sizeof(Msg));
                                                            }
                                                        else
                                                            {
                                                                struct online *new_user=(struct online*)malloc(sizeof(struct online));
                                                                strcpy(new_user->name,pmsg->name);
                                                                new_user->cfd=cfd;
                                                                insert_link(new_user);
                                                                pmsg->flags=2;
                                                                write(cfd,pmsg,sizeof(Msg));
                                                            }
                                                    }
                                                else
                                                    {
                                                        pmsg->flags=-1;
                                                        write(cfd,pmsg,sizeof(Msg));//说明名称正确，但是密码错误，返回flags==1
                                                    }
                                            }
                                        else//说明查询不到相同名称，还未注册
                                            {
                                                pmsg->flags=0;
                                                write(cfd,pmsg,sizeof(Msg));//
                                            }
                                    }
                                
                                pthread_mutex_unlock(&mutex);
                                break;
                            }
                        case 3://私聊
                            {
                                char *to_self = find_self(cfd);//找自己的名字
                                if(to_self == NULL)
                                    {
                                        printf("没找到\n");
                                        continue;
                                    }
                                char sql[2048] = {0};
                                strcpy(pmsg->name,to_self);//这时pmsg->name就是自己的名字
                                memset(sql,0,sizeof(sql));
                                sprintf(sql,"select *from stu where talk=1 and name ='%s' ",pmsg->name);//判断是否正常可以说话（１为正常）
                                int ret = sqlite3_exec(ppdb,sql,callback4, NULL,NULL);//判断talk
                                if(ret != SQLITE_OK)
                                    {
                                    printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                    return NULL; 
                                    }

                                if(c4==1)//说话状态位异常
                                    {
                                        c4=0;
                                        pmsg->flags=-2;//说明本来就是禁言状态
                                        write(cfd,pmsg,sizeof(Msg));
                                    }
                                else//说话状态位正常
                                    {
                                        char sql[2048] = {0};
                                        memset(sql,0,sizeof(sql));
                                        sprintf(sql,"select *from stu where name is '%s'",pmsg->friend);//查询朋友名称是否存在
                                        int ret = sqlite3_exec(ppdb,sql,callback1, NULL,NULL);
                                        if(ret != SQLITE_OK)
                                            {
                                                    printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                                    return NULL; 
                                            }

                                        if(c1==1)//说明查询到名称,注册过了
                                            {
                                                c1=0;
                                                char sql[2048] = {0};
                                                memset(sql,0,sizeof(sql));
                                                sprintf(sql,"select *from stu where vip=1 and name ='%s' ",pmsg->friend);//判断对象存在时，他是否为vip用户
                                                int ret = sqlite3_exec(ppdb,sql,callback3, NULL,NULL);
                                                if(ret != SQLITE_OK)
                                                    {
                                                    printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                                    return NULL; 
                                                    }
                                                if(c3==1)//是vip
                                                    {
                                                        c3=0;
                                                        int to_fd=find_fd(pmsg->friend);
                                                        if(to_fd == -1)
                                                            {
                                                                pmsg->flags = 0;//存在但是不在线
                                                                write(cfd, pmsg, sizeof(Msg));
                                                            }
                                                        else//说明朋友名称存在而且是vip用户而且在线
                                                            {
                                                                char sql[2048] = {0};
                                                                memset(sql,0,sizeof(sql));
                                                                sprintf(sql,"select *from stu where vip=1 and name ='%s' ",pmsg->name);
                                                                int ret = sqlite3_exec(ppdb,sql,callback3, NULL,NULL);//判断自己是否为vip用户
                                                                if(ret != SQLITE_OK)
                                                                    {
                                                                    printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                                                    return NULL; 
                                                                    }
                                                                if(c3==1)//自己是vip
                                                                    {
                                                                        c3=0;
                                                                        pmsg->flags=3;
                                                                        write(cfd,pmsg,sizeof(Msg));
                                                                        pmsg->flags=5;
                                                                        write(to_fd,pmsg,sizeof(Msg));
                                                                        char my_name[32] = {0};
                                                                        char friend_name[32] = {0};
                                                                        char chat_msg[50]={0};
                                                                        memset(sql,0,sizeof(sql));
                                                                        sprintf(sql,"insert into stu_chat (my_name,friend_name,chat_msg,datetime)values('%s','%s','%s',datetime('now','localtime'));",pmsg->name,pmsg->friend,pmsg->msg);
                                                                        int ret = sqlite3_exec(ppdb2,sql,NULL,NULL,NULL);
                                                                        if(ret != SQLITE_OK)
                                                                            {
                                                                            printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                                                            return NULL; 
                                                                            }
                                                                    }
                                                                else//自己不是vip
                                                                    {
                                                                        pmsg->flags=2;
                                                                        write(cfd,pmsg,sizeof(Msg));
                                                                        pmsg->flags=5;
                                                                        write(to_fd,pmsg,sizeof(Msg));
                                                                        char my_name[32] = {0};
                                                                        char friend_name[32] = {0};
                                                                        char chat_msg[50]={0};
                                                                        memset(sql,0,sizeof(sql));
                                                                        sprintf(sql,"insert into stu_chat (my_name,friend_name,chat_msg,datetime)values('%s','%s','%s',datetime('now','localtime'));",pmsg->name,pmsg->friend,pmsg->msg);
                                                                        int ret = sqlite3_exec(ppdb2,sql,NULL,NULL,NULL);
                                                                        if(ret != SQLITE_OK)
                                                                            {
                                                                            printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                                                            return NULL; 
                                                                            }
                                                                    } 
                                                            }
                                                    }
                                                else//存在不是vip
                                                    { 
                                                        int to_fd=find_fd(pmsg->friend);
                                                        if(to_fd == -1)
                                                            {
                                                                pmsg->flags = 0;//存在但是不在线
                                                                write(cfd, pmsg, sizeof(Msg));
                                                            }
                                                        else//说明朋友名称存在但不是vip用户而且在线
                                                            {
                                                                char sql[2048] = {0};
                                                                memset(sql,0,sizeof(sql));
                                                                sprintf(sql,"select *from stu where vip=1 and name ='%s' ",pmsg->name);
                                                                int ret = sqlite3_exec(ppdb,sql,callback3, NULL,NULL);//判断自己是否为vip用户
                                                                if(ret != SQLITE_OK)
                                                                    {
                                                                    printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                                                    return NULL; 
                                                                    }
                                                                if(c3==1)//自己是vip
                                                                    {
                                                                        c3=0;
                                                                        pmsg->flags=3;
                                                                        write(cfd,pmsg,sizeof(Msg));
                                                                        pmsg->flags=4;
                                                                        write(to_fd,pmsg,sizeof(Msg));
                                                                        char my_name[32] = {0};
                                                                        char friend_name[32] = {0};
                                                                        char chat_msg[50]={0};
                                                                        memset(sql,0,sizeof(sql));
                                                                        sprintf(sql,"insert into stu_chat (my_name,friend_name,chat_msg,datetime)values('%s','%s','%s',datetime('now','localtime'));",pmsg->name,pmsg->friend,pmsg->msg);
                                                                        int ret = sqlite3_exec(ppdb2,sql,NULL,NULL,NULL);
                                                                        if(ret != SQLITE_OK)
                                                                            {
                                                                            printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                                                            return NULL; 
                                                                            }
                                                                    }
                                                                else//自己不是vip
                                                                    {
                                                                        pmsg->flags=2;
                                                                        write(cfd,pmsg,sizeof(Msg));
                                                                        pmsg->flags=4;
                                                                        write(to_fd,pmsg,sizeof(Msg));
                                                                        char my_name[32] = {0};
                                                                        char friend_name[32] = {0};
                                                                        char chat_msg[50]={0};
                                                                        memset(sql,0,sizeof(sql));
                                                                        sprintf(sql,"insert into stu_chat (my_name,friend_name,chat_msg,datetime)values('%s','%s','%s',datetime('now','localtime'));",pmsg->name,pmsg->friend,pmsg->msg);
                                                                        int ret = sqlite3_exec(ppdb2,sql,NULL,NULL,NULL);
                                                                        if(ret != SQLITE_OK)
                                                                            {
                                                                            printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                                                            return NULL; 
                                                                            }
                                                                    }
                                                            }   
                                                    }
                                            }
                                        else//说明查询不到相同名称，对象没有注册过
                                            {
                                                pmsg->flags=-1;
                                                write(cfd,pmsg,sizeof(Msg));
                                            }
                                    }
                                pthread_mutex_unlock(&mutex);
                                break;  
                            }
                        case 4://群聊
                            {
                                char *to_self = find_self(cfd);//找自己的名字
                                if(to_self == NULL)
                                    {
                                        printf("没找到\n");
                                        continue;
                                    }
                                char sql[2048] = {0};
                                strcpy(pmsg->name,to_self);//这时pmsg->name就是自己的名字
                                memset(sql,0,sizeof(sql));
                                sprintf(sql,"select *from stu where talk=1 and name ='%s' ",pmsg->name);//判断自己是否正常可以说话（１为正常）
                                int ret = sqlite3_exec(ppdb,sql,callback4, NULL,NULL);//判断talk
                                if(ret != SQLITE_OK)
                                    {
                                        printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                        return NULL; 
                                    }

                                if(c4==1)//说话状态位异常
                                    {
                                        c4=0;
                                        pmsg->flags=-1;//说明自己本来就是禁言状态
                                        write(cfd,pmsg,sizeof(Msg));
                                    }
                                else//说话状态位正常
                                    {
                                        struct online *temp2=head;
                                        while(temp2!=NULL)
                                            {
                                                if(strcmp(temp2->name,pmsg->name)!=0)
                                                    {
                                                        memset(sql,0,sizeof(sql));
                                                        sprintf(sql,"insert into stu_chat (my_name,friend_name,chat_msg,datetime)values('%s','%s','%s',datetime('now','localtime'));",pmsg->name,temp2->name,pmsg->msg);
                                                        int ret = sqlite3_exec(ppdb2,sql,NULL,NULL,NULL);
                                                        if(ret != SQLITE_OK)
                                                            {
                                                                printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                                                return NULL; 
                                                            }
                                                        pmsg->flags=2;
                                                        write(temp2->cfd, pmsg, sizeof(Msg));
                                                    }
                                                temp2=temp2->next;
                                            }
                                        pmsg->flags=3;
                                        write(cfd, pmsg, sizeof(Msg));
                                    }
                                pthread_mutex_unlock(&mutex);
                                break;  
                            }
                        case 5://申请vip用户
                            {
                                char sql[2048] = {0};
                                pthread_mutex_lock(&mutex);
                                memset(sql,0,sizeof(sql));
                                sprintf(sql,"select *from stu where name is '%s'",pmsg->name);
                                int ret = sqlite3_exec(ppdb,sql,callback1, NULL,NULL);
                                if(ret != SQLITE_OK)
                                {
                                    printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                    return NULL; 
                                }

                                if(c1==1)//说明查询到相同名称，已经注册过了,修改为VIP
                                    {
                                        c1=0;
                                        int vip;//可以增加一个检查是否已经是vip用户的判断(检查vip是否为1)
                                        memset(sql,0,sizeof(sql));
                                        sprintf(sql, "update stu set vip = %d where name = '%s'", pmsg->vip, pmsg->name);
                                        int ret = sqlite3_exec(ppdb,sql,NULL,NULL,NULL);
                                        if(ret != SQLITE_OK)
                                            {
                                            printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                            return NULL; 
                                            }
                                        pmsg->flags=2;
                                        write(cfd,pmsg,sizeof(Msg));
                                        break;
                                    }
                                else
                                    {
                                        pmsg->flags=0;
                                        write(cfd,pmsg,sizeof(Msg));
                                    }
                                pthread_mutex_unlock(&mutex);
                                break;
                            }
                        case 6://vip 功能　禁言
                            {
                                pthread_mutex_lock(&mutex);
                                char *to_self = find_self(cfd);//找自己的名字
                                if(to_self == NULL)
                                    {
                                        printf("没找到\n");
                                        continue;
                                    }
                                char sql[2048] = {0};
                                strcpy(pmsg->name,to_self);//这时pmsg->name就是自己的名字
                                if(strcmp(pmsg->name,pmsg->friend)==0)
                                    {
                                        pmsg->flags=-2;
                                        write(cfd,pmsg,sizeof(Msg));
                                        printf("禁言\n");
                                        continue;
                                    }
                                memset(sql,0,sizeof(sql));
                                sprintf(sql,"select *from stu where name is '%s'",pmsg->friend);//查询名称是否存在
                                int ret = sqlite3_exec(ppdb,sql,callback1, NULL,NULL);
                                if(ret != SQLITE_OK)
                                    {
                                        printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                        return NULL; 
                                    }

                                if(c1==1)//说明查询到相同名称，禁言对象没有注册
                                    {
                                        c1=0;
                                        memset(sql,0,sizeof(sql));
                                        sprintf(sql,"select *from stu where vip=1 and name ='%s' ",pmsg->friend);//判断禁言对象存在时，他是否为vip用户
                                        int ret = sqlite3_exec(ppdb,sql,callback3, NULL,NULL);
                                        if(ret != SQLITE_OK)
                                            {
                                            printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                            return NULL; 
                                            }

                                        if(c3==1)
                                            {
                                                c3=0;
                                                pmsg->flags=2;//说明名称存在而且是vip用户
                                                write(cfd,pmsg,sizeof(Msg));
                                            }
                                        else
                                            {
                                                memset(sql,0,sizeof(sql));
                                                sprintf(sql,"select *from stu where talk=1 and name ='%s' ",pmsg->friend);//判断是否正常可以说话（１为正常）
                                                int ret = sqlite3_exec(ppdb,sql,callback4, NULL,NULL);//判断talk
                                                if(ret != SQLITE_OK)
                                                    {
                                                    printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                                    return NULL; 
                                                    }

                                                if(c4==1)
                                                    {
                                                        c4=0;
                                                        pmsg->flags=0;//说明本来就是禁言状态
                                                        write(cfd,pmsg,sizeof(Msg));
                                                    }
                                                else
                                                    {
                                                        memset(sql,0,sizeof(sql));
                                                        sprintf(sql, "update stu set talk = %d where name = '%s'", pmsg->talk, pmsg->friend);//修改禁言talk标志位=1
                                                        int ret = sqlite3_exec(ppdb,sql,NULL,NULL,NULL);
                                                        if(ret != SQLITE_OK)
                                                            {
                                                            printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                                            return NULL; 
                                                            }
                                                        pmsg->flags=3;//成功修改
                                                        write(cfd,pmsg,sizeof(Msg));  
                                                    }   
                                            }
                                    }
                                else//说明查询不到名称，还未注册
                                    {
                                        pmsg->flags=-1;
                                        write(cfd,pmsg,sizeof(Msg)); 
                                    }
                                pthread_mutex_unlock(&mutex);
                                break;
                            }
                        case 7://vip 功能　解除禁言
                            {
                                printf("1111%d\n",pmsg->talk);
                                pthread_mutex_lock(&mutex);
                                char *to_self = find_self(cfd);//找自己的名字
                                if(to_self == NULL)
                                    {
                                        printf("没找到\n");
                                        continue;
                                    }
                                char sql[2048] = {0};
                                strcpy(pmsg->name,to_self);//这时pmsg->name就是自己的名字

                                if(strcmp(pmsg->name,pmsg->friend)==0)
                                    {
                                        pmsg->flags=-2;
                                        write(cfd,pmsg,sizeof(Msg));
                                        printf("禁言\n");
                                        continue;
                                    }
                                memset(sql,0,sizeof(sql));
                                printf("开始\n");
                                printf("%s\n",pmsg->friend);
                                sprintf(sql,"select *from stu where name is '%s'",pmsg->friend);//查询名称是否存在
                                int ret = sqlite3_exec(ppdb,sql,callback1, NULL,NULL);
                                if(ret != SQLITE_OK)
                                    {
                                        printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                        return NULL; 
                                    }

                                if(c1==1)//说明存在
                                    {
                                        c1=0;
                                        printf("%s\n",pmsg->friend);
                                        memset(sql,0,sizeof(sql));
                                        sprintf(sql,"select *from stu where vip=1 and name ='%s' ",pmsg->friend);//判断禁言对象存在时，他是否为vip用户
                                        int ret = sqlite3_exec(ppdb,sql,callback3, NULL,NULL);
                                        if(ret != SQLITE_OK)
                                            {
                                            printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                            return NULL; 
                                            }

                                        if(c3==1)
                                            {
                                                c3=0;
                                                pmsg->flags=2;//说明名称存在而且是vip用户
                                                write(cfd,pmsg,sizeof(Msg));
                                            }
                                        else
                                            {
                                                memset(sql,0,sizeof(sql));
                                                sprintf(sql,"select *from stu where talk=1 and name ='%s' ",pmsg->friend);//判断是否正常可以说话（１为正常）
                                                printf("%s\n",pmsg->friend);
                                                int ret = sqlite3_exec(ppdb,sql,callback4, NULL,NULL);//判断talk
                                                if(ret != SQLITE_OK)
                                                    {
                                                    printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                                    return NULL; 
                                                    }

                                                if(c4==1)
                                                    {
                                                        c4=0;
                                                        memset(sql,0,sizeof(sql));
                                                        printf("11111111\n");
                                                        printf("%s\n",pmsg->friend);
                                                        printf("%d\n",pmsg->talk);
                                                        printf("%s\n",pmsg->friend);
                                                        sprintf(sql, "update stu set talk = %d where name = '%s'", pmsg->talk, pmsg->friend);//修改禁言talk标志位=0
                                                        int ret = sqlite3_exec(ppdb,sql,NULL,NULL,NULL);
                                                        if(ret != SQLITE_OK)
                                                            {
                                                            printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                                            return NULL; 
                                                            }
                                                        pmsg->flags=3;//成功修改
                                                        write(cfd,pmsg,sizeof(Msg));  
                                                        
                                                    }
                                                else
                                                    {
                                                        pmsg->flags=0;//说明本来就是解除禁言状态
                                                        write(cfd,pmsg,sizeof(Msg));
                                                    }   
                                            }
                                        
                                    }
                                else//说明查询不到相同名称
                                    {
                                        pmsg->flags=-1;
                                        write(cfd,pmsg,sizeof(Msg)); 
                                    }
                                pthread_mutex_unlock(&mutex);
                                break;
                            }
                        case 8://vip功能　踢人下线
                            {
                                pthread_mutex_lock(&mutex);
                                char *to_self = find_self(cfd);//找自己的名字
                                if(to_self == NULL)
                                    {
                                        printf("没找到\n");
                                        continue;
                                    }
                                char sql[2048] = {0};
                                strcpy(pmsg->name,to_self);//这时pmsg->name就是自己的名字

                                if(strcmp(pmsg->name,pmsg->friend)==0)
                                    {
                                        pmsg->flags=-2;//踢得人是自己
                                        write(cfd,pmsg,sizeof(Msg));
                                        printf("禁言\n");
                                        continue;
                                    }
                                memset(sql,0,sizeof(sql));
                                sprintf(sql,"select *from stu where name is '%s'",pmsg->friend);//查询名称是否存在数据库
                                int ret = sqlite3_exec(ppdb,sql,callback1, NULL,NULL);
                                if(ret != SQLITE_OK)
                                    {
                                        printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                        return NULL; 
                                    }

                                if(c1==1)//说明数据库查询到相同名称
                                    {
                                        c1=0;
                                        memset(sql,0,sizeof(sql));
                                        sprintf(sql,"select *from stu where vip=1 and name ='%s' ",pmsg->friend);//判断禁言对象存在时，他是否为vip用户
                                        int ret = sqlite3_exec(ppdb,sql,callback3, NULL,NULL);
                                        if(ret != SQLITE_OK)
                                            {
                                                printf("sqlite3_exec2: %s\n",sqlite3_errmsg(ppdb));
                                                return NULL; 
                                            }

                                        if(c3==1)//vip用户
                                            {
                                                c3=0;
                                                pmsg->flags=2;//说明名称存在而且是vip用户
                                                write(cfd,pmsg,sizeof(Msg));
                                            }
                                        else//非vip用户　判断是否在线，在线就踢出链表
                                            {
                                                int to_fd=find_fd(pmsg->friend);
                                                if(to_fd == -1)//不在线
                                                    {
                                                        pmsg->flags = 0;//存在但是不在线
                                                        write(cfd, pmsg, sizeof(Msg));
                                                    }
                                                else//在线踢人(有疑问，这里主要使用了无头链表的按值删除)
                                                    {
                                                        struct online*p=NULL;
                                                        p=head;
                                                        struct online*q=p->next;
                                                        if(strcmp(p->name,pmsg->friend)==0)//删除的结点在第一个
                                                            {
                                                                head=p->next;
                                                                free(p);
                                                            }
                                                        else    
                                                            {
                                                                while(q!=NULL)
                                                                    {
                                                                        if(strcmp(p->name,pmsg->friend)==0)
                                                                            {
                                                                                p->next=q->next;
                                                                                free(q);
                                                                            }
                                                                        p=q;
                                                                        q=q->next;
                                                                    }
                                                            }
                                                        p=NULL;
                                                        pmsg->flags=-3;
                                                        write(to_fd,pmsg,sizeof(Msg));
                                                        pmsg->flags=3;
                                                        write(cfd,pmsg,sizeof(Msg));
                                                    }
                                            }
                                    }
                                else//说明数据库查询不到名称
                                    {
                                        pmsg->flags=-1;
                                        write(cfd,pmsg,sizeof(Msg)); 
                                    }
                                pthread_mutex_unlock(&mutex);
                                break;
                            }
                    }
            }
    }

// void *child_exit(int sig)
//     {
//         waitpid(-1,NULL,0);//阻塞等待,防止僵尸进程产生
//         printf("child is exit\n");
//     }

void* createsqlite()//创建用户信息数据库
        {
            int ret = sqlite3_open("stu.db",&ppdb);
            if(ret != SQLITE_OK)
                {
                printf("sqlite3 open: %s\n",sqlite3_errmsg(ppdb));
                return NULL;
                }
            char sql[2048] = {0};
            sprintf(sql,"create table if not exists stu(name text, passwd text,vip integer,talk integer);");
            ret = sqlite3_exec(ppdb,sql,NULL,NULL,NULL);
            if(ret != SQLITE_OK)
                {
                printf("sqlite3_exec: %s\n",sqlite3_errmsg(ppdb));
                return NULL;  
                }
            printf("111111111\n");
        }

void* createsqlite_chatting_records()//创建用户聊天记录数据库
        {
            int ret2 = sqlite3_open("stu_chat.db",&ppdb2);
            if(ret2 != SQLITE_OK)
                {
                printf("sqlite3 open: %s\n",sqlite3_errmsg(ppdb2));
                return NULL;
                }
            char sql[2048] = {0};
            sprintf(sql,"create table if not exists stu_chat(my_name text, friend_name text,chat_msg text,datetime numeric );");
            ret2 = sqlite3_exec(ppdb2,sql,NULL,NULL,NULL);
            if(ret2 != SQLITE_OK)
                {
                printf("sqlite3_exec: %s\n",sqlite3_errmsg(ppdb2));
                return NULL;  
                }
            printf("666666666\n");
        }

int main(int argc, char const *argv[])
{
    signal(SIGINT,serever_exit);

    createsqlite();
    createsqlite_chatting_records();

    socklen_t c_size; 
    long cfd;
    int r_n;
    int w_n;
    pthread_t id;

    struct sockaddr_in s_addr;
    struct sockaddr_in c_addr;
    
    if((sockfd=socket(AF_INET,SOCK_STREAM | SOCK_NONBLOCK,0))<0)
        {
            perror("socket");
            exit(1);
        }
    printf("socket  success!\n");
    s_addr.sin_family=AF_INET;
    s_addr.sin_port=htons(8888);//端口号
    //s_addr.sin_addr.s_addr=inet_addr("192.168.193.129");
    s_addr.sin_addr.s_addr=htons(INADDR_ANY);//绑定任意网卡
    int opt=1;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));//设置套接字可以重复使用套接字
    if(bind(sockfd,(struct sockaddr*)&s_addr,sizeof(struct sockaddr_in))!=0)
        {
            perror("bind");
            exit(1);
        }
    printf("bind success!\n");
    if(listen(sockfd,20)!=0)
        {
            perror("listen");
            exit(1);
        }
    printf("listen success!\n");

    // signal(SIGCHLD,child_exit);

    while(1)
    {
        c_size =sizeof(struct sockaddr_in);
        cfd=accept(sockfd,(struct sockaddr*)&c_addr,&c_size);
        if(cfd<0)
            {
                if(errno!=EAGAIN&&errno !=EWOULDBLOCK&&errno!=EINTR)//保存系统调用产生的错误值
                    {
                        perror("accept");
                        exit(1);
                    }
                continue;   
            }
        printf("info client:ip=%s,port=%d\n",inet_ntoa(c_addr.sin_addr),ntohs(c_addr.sin_port));
        
        if(pthread_create(&id,NULL,thread_read,(void*)cfd)<0)
            {
                perror("thread error");
                exit(1);
            }
    }
    return 0;
}     
