#pragma once//为了避免同一个头文件被包含(include)多次

struct message
{
    int action;//标志位
    int vip;//是否为vip
    int flags;//标志位
    int talk;//是否可以说话
    
    char name[20];//姓名
    char passwd[20];//密码
    char friend[20];//好友姓名
    char msg[100];//消息内容
    char io_name[20];//文件名
};
typedef struct message Msg;

zzz