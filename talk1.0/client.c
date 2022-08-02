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
#include<unistd.h>
#include<pthread.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include"Msg.h"
int loged=1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *thread_read(void *arg)//子线程读
    {
        pthread_t id=pthread_self();
        pthread_detach(id);
        long cfd=(long)arg;
        Msg *pmsg=(Msg*)malloc(sizeof(Msg));

        while(1)
            {
                memset(pmsg,0,sizeof(Msg));
                int r_n=read(cfd,pmsg,sizeof(Msg));
                if(r_n==0)
                    {
                        printf("serever is exit!\n");
                        pthread_exit(NULL);
                    }
               switch (pmsg->action)
                {
                    case 1://注册
                        {
                            system("clear");
                            pthread_mutex_lock(&mutex);
                            if(pmsg->flags==2)
                                {
                                    printf("---------------------------------------------------------\n");
                                    printf("注册成功!\n");
                                    printf("欢迎您，新用户\n");
                                    printf("---------------------------------------------------------\n");
                                }
                            else if(pmsg->flags==0)
                                {
                                    printf("---------------------------------------------------------\n");
                                    printf("注册失败!\n\n");
                                    printf("你之前已经注册过了\n");
                                    printf("请直接登陆\n\n");
                                    printf("---------------------------------------------------------\n");
                                }
                            pthread_mutex_unlock(&mutex);
                            break;
                        }
                    case 2://登陆
                        {
                            pthread_mutex_lock(&mutex);
                            system("clear");
                            if(pmsg->flags==3)
                                {
                                    loged=3;
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                    printf("$欢迎回家，尊贵的vip用户!\n\n");//在这里加上用户名
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                    
                                }
                            else if(pmsg->flags==2)
                                {
                                    loged=2;
                                    printf("---------------------------------------------------------\n\n");
                                    printf("登陆成功!\n\n");
                                    printf("欢迎您\n\n");//在这里加上用户名
                                    printf("---------------------------------------------------------\n\n");
                                }
                            else if(pmsg->flags==0)
                                {
                                    loged=0;
                                    printf("---------------------------------------------------------\n\n");
                                    printf("登陆失败!\n\n");
                                    printf("用户名不存在\n\n");//在这里加上用户名
                                    printf("请重新登陆\n\n");
                                    printf("---------------------------------------------------------\n\n");
                                }
                            else if(pmsg->flags==-1)
                                {
                                    loged=0;
                                    printf("---------------------------------------------------------\n\n");
                                    printf("登陆失败!\n\n");
                                    printf("密码输入错误\n\n");//在这里加上用户名
                                    printf("请重新登陆并修改密码\n\n");
                                    printf("---------------------------------------------------------\n\n");
                                }
                            else if(pmsg->flags==-2)
                                {
                                    loged=0;
                                    printf("---------------------------------------------------------\n\n");
                                    printf("登陆失败!\n\n");
                                    printf("你已经登陆过了\n\n");
                                    printf("不可以重复登陆!\n\n");
                                    printf("---------------------------------------------------------\n\n");
                                }
                            pthread_mutex_unlock(&mutex);
                            break;
                        }
                    case 3://私聊
                        {
                            pthread_mutex_lock(&mutex);
                            if(pmsg->flags==5)
                                {
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                    printf("$您有一条来自%s新消息$\n\n",pmsg->name);
                                    printf("$%s$\n\n",pmsg->msg);
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                }
                            else if(pmsg->flags==4)
                                {
                                    printf("*******************************************\n\n");
                                    printf("一条来自%s新消息$\n\n",pmsg->name);
                                    printf("%s$\n\n",pmsg->msg);
                                    printf("*******************************************\n\n");
                                }
                            else if(pmsg->flags==3)
                                {
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                    printf("$消息发送成功$\n\n");
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                }
                            else if(pmsg->flags==2)
                                {

                                    printf("*******************************************\n\n");
                                    printf("消息发送成功\n\n");
                                    printf("*******************************************\n\n");
                                }
                             else if(pmsg->flags==-2)
                                {
                                    printf("*******************************************\n\n");
                                    printf("你已经被禁言\n\n");
                                    printf("*******************************************\n\n");
                                }
                             else if(pmsg->flags==0)
                                {
                                    printf("*******************************************\n\n");
                                    printf("好友不在线\n\n");
                                    printf("*******************************************\n\n");
                                }
                             else if(pmsg->flags==-1)
                                {
                                    printf("*******************************************\n\n");
                                    printf("你输入的好友名有误\n\n");
                                    printf("*******************************************\n\n");
                                }
                            pthread_mutex_unlock(&mutex);
                            break;
                        }
                    case 4://群聊
                        {
                            pthread_mutex_lock(&mutex);
                            if(pmsg->flags==3)
                                {
                                    printf("\t群聊信息发送成功!\n\n");
                                    printf("\t所有在线好友都收到了这条消息!\n\n");
                                }
                            else if(pmsg->flags==2)
                                {
                                    printf("\t->你收到了一条来自%s的群发信息!\n\n",pmsg->name);
                                    printf("\t->%s\n",pmsg->msg);
                                }
                            else if(pmsg->flags==-1)
                                {
                                    printf("->你被禁言,无法群法消息!\n\n");
                                }
                            pthread_mutex_unlock(&mutex);
                            break;
                        }
                    case 5://申请成为vip
                        {
                            pthread_mutex_lock(&mutex);
                            if(pmsg->flags==2)
                                {
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                    printf("$恭喜你，尊贵的vip用户$\n\n");
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                }
                            else if(pmsg->flags==0)
                                {
                                    printf("抱歉哦，您还未进行注册,注册后在来成为尊贵的vip用户吧!\n");
                                }
                            pthread_mutex_unlock(&mutex);
                            break;
                        }
                    case 6://禁言
                        {
                            pthread_mutex_lock(&mutex);
                            if(pmsg->flags==3)
                                {
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                    printf("$恭喜你，尊贵的vip用户$\n\n");
                                    printf("$成功将他禁言$\n\n");
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                }
                            else if(pmsg->flags==2)
                                {
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                    printf("$抱歉!尊贵的vip用户$\n\n");
                                    printf("$您没有禁言此vip用户的权限$\n\n");
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                }
                            else if(pmsg->flags==0)
                                {
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                    printf("$尊贵的vip用户!$\n\n");
                                    printf("$此用户本来就是禁言状态!$\n\n");
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                }
                            else if(pmsg->flags==-1)
                                {
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                    printf("$抱歉!尊贵的vip用户$\n\n");
                                    printf("$此用户不存在!$\n\n");
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                }
                            else if(pmsg->flags==-2)
                                {
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                    printf("$抱歉!尊贵的vip用户$\n\n");
                                    printf("$你不可以把自己禁言哦!$\n\n");
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                }
                            pthread_mutex_unlock(&mutex);
                            break;
                        }
                    case 7://解除禁言
                        {
                            pthread_mutex_lock(&mutex);
                            if(pmsg->flags==3)
                                {
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                    printf("$恭喜你，尊贵的vip用户$\n\n");
                                    printf("$成功将他解除禁言$\n\n");
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                }
                            else if(pmsg->flags==2)
                                {
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                    printf("$抱歉!尊贵的vip用户$\n\n");
                                    printf("$您没有解除禁言此vip用户的权限$\n\n");
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                }
                            else if(pmsg->flags==0)
                                {
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                    printf("$尊贵的vip用户!$\n\n");
                                    printf("$此用户本来就是解除禁言状态!$\n\n");
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                }
                            else if(pmsg->flags==-1)
                                {
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                    printf("$抱歉!尊贵的vip用户$\n\n");
                                    printf("$此用户不存在!$\n\n");
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                }
                            else if(pmsg->flags==-2)
                                {
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                    printf("$抱歉!尊贵的vip用户$\n\n");
                                    printf("$你不可以把自己解除禁言哦!$\n\n");
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                }
                            pthread_mutex_unlock(&mutex);
                            break;
                        }
                    case 8://踢人
                        {
                            pthread_mutex_lock(&mutex);
                            if(pmsg->flags==3)
                                {
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                    printf("$恭喜你，尊贵的vip用户$\n\n");
                                    printf("$成功将他踢下线$\n\n");
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                }
                            else if(pmsg->flags==2)
                                {
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                    printf("$抱歉!尊贵的vip用户$\n\n");
                                    printf("$您没有把此vip用户踢下线的权限$\n\n");
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                }
                            else if(pmsg->flags==0)
                                {
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                    printf("$尊贵的vip用户!$\n\n");
                                    printf("$此用户不在线!$\n\n");
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                }
                            else if(pmsg->flags==-1)
                                {
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                    printf("$抱歉!尊贵的vip用户$\n\n");
                                    printf("$此用户未注册!$\n\n");
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                }
                            else if(pmsg->flags==-2)
                                {
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                    printf("$抱歉!尊贵的vip用户$\n\n");
                                    printf("$你不可以把自己踢下线哦!$\n\n");
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                }
                            else if(pmsg->flags==-3)
                                {
                                    printf("*******************************************\n\n");
                                    printf("你已经被%s踢下线!\n\n",pmsg->name);
                                    printf("正在退出,请稍候...\n\n");
                                    printf("*******************************************\n\n");
                                    sleep(1);
                                    exit(1);
                                }
                            pthread_mutex_unlock(&mutex);
                            break;
                        }
                }
            }
    }

int main(int argc, char const *argv[])
{
    if(argc!=3)
        {
            printf("please input serever ip and port!\n");
        }
    int r_n;
    long sockfd;
    struct sockaddr_in s_addr;
    pthread_t id;
    pthread_mutex_init(&mutex,NULL);
    Msg *pmsg=(Msg*)malloc(sizeof(Msg));

    if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
        {
            perror("socket");
            exit(1);
        }
    //printf("socket success!\n");
    s_addr.sin_family=AF_INET;
    s_addr.sin_port=htons(atoi(argv[2]));//指定服务器端口号
    s_addr.sin_addr.s_addr=inet_addr(argv[1]);//指定服务器ip地址

    if(connect(sockfd,(struct sockaddr*)&s_addr,sizeof(struct sockaddr))!=0)
        {
            perror("connect");
            exit(1);
        }
    //printf("connect success!\n");
    //system("clear");
    pthread_create(&id,NULL,thread_read,(void*)sockfd);//父线程发
    int cmd;
    int cmd2;
    printf("欢迎来到菜鸡聊天室\n");
    while(1)
        {
            sleep(2);
            system("clear");
            printf("---------------------------------------------------------\n\n");
            printf("\t    选项:(请输入文字前面的选项)\n\n");
            printf("\t        1.注册(reg):\n\n");
            printf("\t        2.登陆(log):\n\n");
            printf("\t        3.退出(exit):\n\n");
            printf("---------------------------------------------------------\n\n");
            printf("请输入选项\n");
            scanf("%d",&cmd);
            system("clear");
            if(cmd==1)//注册
                {
                    memset(pmsg,0,sizeof(Msg));
                    pmsg->action=1;
                    pmsg->flags=1;
                    printf("---------------------------------------------------------\n\n");
                    printf("\t您好!\n\n");
                    printf("\t\t欢迎来到注册界面!\n\n");
                    printf("\t\t请稍后!\n\n");
                    printf("---------------------------------------------------------\n\n");
                    printf("请输入昵称!\n");
                    scanf("%s",pmsg->name);
                    printf("请输入您的账号密码!\n");
                    scanf("%s",pmsg->passwd);
                    system("clear");
                    printf("---------------------------------------------------------\n\n");
                    printf("\t\t正在注册!\n");
                    printf("\t\t请稍后!\n");
                    printf("---------------------------------------------------------\n\n");
                    sleep(1);
                    if(write(sockfd,pmsg,sizeof(Msg))<0)
                        {
                            perror("write data error");
                            exit(1);
                        }
                }
            if(cmd==2)//登陆
                {
                    memset(pmsg,0,sizeof(Msg));
                    pmsg->action=2;
                    pmsg->flags=1;
                    printf("---------------------------------------------------------\n");
                    printf("\t您好!\n");
                    printf("\t\t欢迎来到登陆界面!\n");
                    printf("---------------------------------------------------------\n");
                    printf("请输入昵称!\n");
                    scanf("%s",pmsg->name);
                    printf("请输入您的账号密码!\n");
                    scanf("%s",pmsg->passwd);
                    system("clear");
                    if(write(sockfd,pmsg,sizeof(Msg))<0)
                        {
                            perror("write data error");
                            exit(1);
                        }
                    while(1)
                    {//修改一下命令输入错误的情况
                        if(loged == 2)//普通用户登陆之后的操作
                            {
                                sleep(2);
                                system("clear");
                                printf("******************************************\n");
                                printf("\t    选项:(请输入文字前面的选项)\n\n");
                                printf("\t        1.私聊(chat):\n\n");
                                printf("\t        2.群聊(allchat):\n\n");
                                printf("\t        3.申请成为会员(apply_vip):\n\n");
                                printf("\t        4.查看聊天记录(read):\n\n");
                                printf("\t        5.返回注册界面(break):\n\n");
                                printf("******************************************\n\n");
                                scanf("%d",&cmd2);
                                if(cmd2==1)//私聊
                                    {
                                        system("clear");
                                        memset(pmsg,0,sizeof(Msg));
                                        pmsg->action=3;
                                        pmsg->flags=1;
                                        printf("******************************************\n\n");
                                        printf("\t你好!\n\n");
                                        printf("\t\t欢迎来到私聊界面!\n\n");
                                        printf("******************************************\n\n");
                                        printf("请输入好友的昵称!\n");
                                        scanf("%s",pmsg->friend);
                                        printf("你想要发送什么信息呢？\n");
                                        scanf("%s",pmsg->msg);
                                        if(write(sockfd,pmsg,sizeof(Msg))<0)
                                            {
                                                perror("write data error");
                                                exit(1);
                                            }
                                    }
                                if(cmd2==2)//群聊
                                        {
                                            system("clear");
                                            memset(pmsg,0,sizeof(Msg));
                                            pmsg->action=4;
                                            pmsg->flags=1;
                                            //printf("please input some friend's name!\n");//这里可以设置部分好友发送
                                            //scanf("%s",pmsg->friend);
                                            printf("******************************************\n\n");
                                            printf("\t你好!\n\n");
                                            printf("\t\t欢迎来到群聊界面!\n\n");
                                            printf("******************************************\n\n");
                                            printf("你想要发送什么信息呢？\n");
                                            scanf("%s",pmsg->msg);
                                            if(write(sockfd,pmsg,sizeof(Msg))<0)
                                                {
                                                    perror("write data error");
                                                    exit(1);
                                                }
                                        }
                                if(cmd2==3)//申请成为管理员
                                        {
                                            system("clear");
                                            memset(pmsg,0,sizeof(Msg));
                                            pmsg->action=5;
                                            pmsg->flags=1;
                                            pmsg->vip=1;
                                            int a;
                                            printf("******************************************\n\n");
                                            printf("\t你好!\n\n");
                                            printf("\t\t这里是vip申请界面!\n\n");
                                            printf("联系开发人员支付会员费用(李强:13674774635)\n\n");
                                            printf("请等待...\n\n");
                                            printf("******************************************\n\n");
                                            sleep(2);
                                            printf("是否支付成功？(1.是的，我已经支付       2.残忍拒绝)\n");
                                            scanf("%d",&a);
                                            if(a==1)
                                                {
                                                    system("clear");
                                                    printf("请输入昵称!\n");
                                                    scanf("%s",pmsg->name);
                                                    if(write(sockfd,pmsg,sizeof(Msg))<0)
                                                        {
                                                            perror("write data error");
                                                            exit(1);
                                                        }
                                                    loged=3;
                                                }
                                            else 
                                                {
                                                    system("clear");
                                                    printf("******************************************\n");
                                                    printf("\t申请失败，我们下次再见!\n\n");
                                                    printf("\t\t正在退出vip充值通道,请稍候...\n\n");
                                                    printf("******************************************\n");
                                                    sleep(2);
                                                }
                                        }
                                if(cmd2==4)//查看聊天记录
                                        {
                                            system("clear");
                                            memset(pmsg,0,sizeof(Msg));
                                            printf("******************************************\n\n");
                                            printf("\t你好!\n\n");
                                            printf("\t\t这里是你的聊天记录!\n\n");
                                            printf("******************************************\n\n");
                                            printf("请输入好友的昵称!\n");
                                            scanf("%s",pmsg->friend);
                                            if(write(sockfd,pmsg,sizeof(Msg))<0)
                                                {
                                                    perror("write data error");
                                                    exit(1);
                                                }
                                        }
                                    // else if(strcmp(cmd2,"io")==0)//文件传输
                                    //     {
                                    //         system("clear");
                                    //         memset(pmsg,0,sizeof(Msg));
                                    //         pmsg->action=9;
                                    //         pmsg->flags=1;
                                    //         printf("******************************************\n\n");
                                    //         printf("\t你好!\n\n");
                                    //         printf("\t\t欢迎来到文件传输界面!\n\n");
                                    //         printf("******************************************\n\n");
                                    //         printf("请输入好友的昵称!\n");
                                    //         scanf("%s",pmsg->friend);
                                    //         printf("你想要发送哪个文件？\n");
                                    //         printf("请输入文件名？\n");
                                    //         scanf("%s",pmsg->io_name);
                                    //         if(write(sockfd,pmsg,sizeof(Msg))<0)
                                    //             {
                                    //                 perror("write data error");
                                    //                 exit(1);
                                    //             }
                                    //     }
                                if(cmd2==5)
                                        {
                                            system("clear");
                                            printf("正在退出,请稍等...\n");
                                            sleep(1);
                                            loged=0;
                                            break;
                                        }
                            }
                        if(loged == 3)//vip用户登陆之后的操作
                            {
                                while(1)
                                {
                                    sleep(2);
                                    system("clear");
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                    printf("\t    $选项$:(请输入文字前面的选项)\n\n");
                                    printf("\t        $1.私聊$(chat):\n\n");
                                    printf("\t        $2.群聊$(allchat):\n\n");
                                    printf("\t        $3.禁言$(stop):\n\n");
                                    printf("\t        $4.解除禁言$(over_stop):\n\n");
                                    printf("\t        $5.踢人下线$(out):\n\n");
                                    printf("\t        $6.查看聊天记录(read):\n\n");
                                    printf("\t        $7.返回注册界面(break):\n\n");
                                    printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                    scanf("%d",&cmd2);
                                    if(cmd2==1)//vip 功能私聊
                                        {
                                            system("clear");
                                            memset(pmsg,0,sizeof(Msg));
                                            pmsg->action=3;
                                            pmsg->flags=1;
                                            printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                            printf("$您好，尊贵的vip用户!\n\n");
                                            printf("\t\t$欢迎来到私聊界面!\n");
                                            printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                            printf("$请输入好友的昵称!\n");
                                            scanf("%s",pmsg->friend);
                                            printf("$你想要发送什么信息呢？\n");
                                            scanf("%s",pmsg->msg);
                                            if(write(sockfd,pmsg,sizeof(Msg))<0)
                                                {
                                                    perror("write data error");
                                                    exit(1);
                                                }
                                        }
                                    if(cmd2==2)//vip 功能群聊
                                        {
                                            system("clear");
                                            memset(pmsg,0,sizeof(Msg));
                                            pmsg->action=4;
                                            pmsg->flags=1;
                                            //printf("please input some friend's name!\n");//这里可以设置部分好友发送
                                            //scanf("%s",pmsg->friend);
                                            printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                            printf("$您好，尊贵的vip用户!\n\n");
                                            printf("\t\t$欢迎来到群聊界面!\n");
                                            printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                            printf("你想要发送什么信息呢？\n");
                                            scanf("%s",pmsg->msg);
                                            if(write(sockfd,pmsg,sizeof(Msg))<0)
                                                {
                                                    perror("write data error");
                                                    exit(1);
                                                }
                                        }
                                    if(cmd2==3)//vip 功能　禁言
                                        {
                                            system("clear");
                                            memset(pmsg,0,sizeof(Msg));
                                            pmsg->action=6;
                                            pmsg->flags=1;
                                            pmsg->talk=1;
                                            printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                            printf("尊贵的vip用户，您想把谁禁言？\n\n");
                                            printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                            scanf("%s",pmsg->friend);
                                            if(write(sockfd,pmsg,sizeof(Msg))<0)
                                                {
                                                    perror("write data error");
                                                    exit(1);
                                                }
                                        }
                                    if(cmd2==4)//vip 功能　解除禁言
                                        {
                                            system("clear");
                                            memset(pmsg,0,sizeof(Msg));
                                            pmsg->action=7;
                                            pmsg->flags=1;
                                            pmsg->talk=0;
                                            printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                            printf("尊贵的vip用户，您想把谁解除禁言？\n\n");
                                            printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                            scanf("%s",pmsg->friend);
                                            if(write(sockfd,pmsg,sizeof(Msg))<0)
                                                {
                                                    perror("write data error");
                                                    exit(1);
                                                }
                                        }
                                    if(cmd2==5)//vip 功能　踢人下线
                                        {
                                            system("clear");
                                            memset(pmsg,0,sizeof(Msg));
                                            pmsg->action=8;
                                            pmsg->flags=1;
                                            printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                            printf("尊贵的vip用户，您想把谁踢下线？\n\n");
                                            printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                            scanf("%s",pmsg->friend);
                                            if(write(sockfd,pmsg,sizeof(Msg))<0)
                                                {
                                                    perror("write data error");
                                                    exit(1);
                                                }
                                        }
                                    if(cmd2==6)//查看聊天记录
                                        {
                                            system("clear");
                                            memset(pmsg,0,sizeof(Msg));
                                            printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                            printf("\t你好!\n\n");
                                            printf("\t\t这里是你的聊天记录!\n\n");
                                            printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n");
                                            printf("请输入好友的昵称!\n");
                                            scanf("%s",pmsg->friend);
                                            if(write(sockfd,pmsg,sizeof(Msg))<0)
                                                {
                                                    perror("write data error");
                                                    exit(1);
                                                }
                                        }
                                    if(cmd2==7)//返回上一级界面
                                        {
                                            system("clear");
                                            printf("正在退出,请稍等...\n");
                                            sleep(1);
                                            loged = 0;
                                            break;
                                        }
                                }
                            }
                        if(loged == 0)
                            {
                                loged = 1;
                                break;
                            }
                    }
                }
            if(cmd==3)//退出系统
                {
                    system("clear");
                    printf("\t\t相聚的时光总是如此短暂\n");
                    printf("\t\t我们下次再见!\n");
                    printf("\t\t请稍候，系统正在退出......\n");
                    sleep(2);
                    exit(1);
                }
        }
    return 0;
}
