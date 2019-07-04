#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "DTNmsg.h"
#include "ipv6info.h"
#include "blueinfo.h"
#include "msgprocess.h"

list_info *list = NULL;

int process_dtn_msg(int server,dtn_req_msg *msg,char *blueaddr)
{
    int res = 0;
    if(server < 0)
    {
        printf("func process_dtn_msg err:client < 0!\n");
        exit(-3);
    }
    if(msg == NULL || blueaddr == NULL)
    {
        printf("func pro_dtn_msg err:msg=NULL||addr=NULL!\n");
        exit(-4);
    }
    if(msg->type == 0)
    {
         res = send_msg(server,msg);
    }
    if(msg->type == 8)
    {
        return 0;
    }
    if(msg->type == 2)
    {//make friend
        strncpy(msg->dest_ipv6_addr,"fe80::961:7aed:d92a:7118",
                                    sizeof("fe80::961:7aed:d92a:7118"));
        res = process_make_friend(server,msg);
        if(res < 0)
        {
            printf("pro_dtn_msg:process_make_friend err!\n");
            exit(-6);
        }
    }
    if(msg->type == 3)
    {//add new friend
        res = process_add_new_friend(msg,blueaddr);
        if(res < 0)
        {
            printf("pro_dtn_msg:pro_add_new_friend err!\n");
            exit(-7);
        }
    }
    if(msg->type == 4)
    {//send msg:require of list
        res = process_send_reqlist(server,msg);
        if(res < 0)
        {
            printf("pro_dtn_msg:pro_send_reqlist err!\n");
            exit(-8);
        }
    }
    if(msg->type == 5)
    {//recv msg:recv list data
        res = process_recv_list(server,msg,&list);
        if(res < 0)
        {
            printf("pro_dtn_msg:recv_list err!res=%d\n",res);
            exit(-9);
        }
        if(list != NULL)
        {
            printf_list(list);
        }
    }
    if(msg->type == 6)
    {//send msg:require data
        //printf_list(list);
        res = process_req_data(server,msg,list,blueaddr);
        if(res < 0)
        {
            printf("pro_dtn_msg err: process_req_data err!\n");
            exit(-10);
        }
    } 
    if(msg->type == 7)
    {//recv msg:require data
        res = process_recv_data(server,msg);
        if(res < 0)
        {
            printf("pro_dtn_msg err: process_recv_data err!\n");
            exit(-7);
        }
    }   
    if(msg->type == 11)
    {//recv msg:require data
        res = process_node_quit(server,msg,blueaddr);
        if(res < 0)
        {
            printf("pro_dtn_msg err: process_quit_data err!\n");
            exit(-11);
        }
    }
    return res;
}



int process_connect(int server,char* blue_addr )
{//the node initiative to connect first to send data
 //first check if the dest node is his friend
 //if not make a friend
 //
    //check_node_list(NULL);
    printf("process connect...\n");
    if(server < 0 || blue_addr == NULL)
    {
        printf("pro_connect:server < 0||dest_ipv6 = NULL!\n");
        return -1;
    }
    char dest_ipv6_addr[IPv6_ADDR_LEN] = {0};
    dtn_req_msg msg;
    /*strncpy(msg.dest_ipv6_addr,"fe80::961:7aed:d92a:7119",sizeof("fe80::961:7aed:d92a:7119"));
    strncpy(msg.source_ipv6_addr,"fe80::961:7aed:d92a:7118",sizeof("fe80::961:7aed:d92a:7118"));
    int res = 0;
    msg.type = 0;
    */
    //res = process_dtn_msg(server,&msg,blue_addr);
    /*
    int fd = check_node_list_blue(blue_addr,dest_ipv6_addr);
    if(fd != 1 && fd > 0)
    {//not a friend and check_node_list success
     //send make friend msg
        dtn_req_msg msg;
        res = init_dtn2_msg(&msg);
        if(res < 0)
        {
            printf("process_connect err:init_dtn7_msg err res = %d!\n",res);
            return -2;
        }
        res = process_dtn_msg(server,&msg,blue_addr);
        if(res < 0)
        {
            printf("pro_connect:pro_dtn_msg res=%d!\n",res);
            return -3;
        }
        //send make friend msg success! 
        //recv server ipv6_addr and add new friend
        res = init_dtn3_msg(server,&msg,fd);
        if(res < 0)
        {
            printf("pro_connect:init_dtn3_msg err res=%d!\n",res);
            return -4;
        }
        res = process_dtn_msg(server,&msg,blue_addr);
        if(res < 0)
        {
            printf("pro_conn:pro_dtn_msg err res = %d!\n",res);
        }
        //add new friend success
    }
    res = init_dtn_msg(&msg);
    if(res < 0)
    {
        printf("pro_conn:init_dtn_msg err!res=%d\n",res);
        return -5;
    }
    */
    //send require list msg
    int res = init_dtn4_msg(&msg,blue_addr);
    //res = init_dtn2_msg(&msg);

    if(res < 0)
    {
        printf("process_connect:init_dtn2_msg err!\n");
        return -4;
    }
    res = process_dtn_msg(server,&msg,blue_addr);
    //printf("process client success!\n");
    //return 0;

    //recv server list
    res = init_dtn_msg(&msg);
    if(res < 0)
    {
        printf("pro_conn:init_dtn_msg err!res=%d\n",res);
        return -5;
    }
    res = init_dtn5_msg(server,&msg);
    if(res < 0)
    {
        printf("pro_conn:init_dtn5_msg err:res=%d\n",res);
        return -6;
    }
    res = process_dtn_msg(server,&msg,blue_addr);
    if(res < 0)
    {
        printf("peo_conn:pro_dtn_msg err!res=%d\n",res);
        return -7;
    }
    //send msg: require data 
    res = init_dtn_msg(&msg);
    if(res < 0)
    {
        printf("pro_conn:init_dtn_msg err!res=%d\n",res);
        return -5;
    }
    msg.type = 6;
    res = process_dtn_msg(server,&msg,blue_addr);
    if(res < 0)
    {
        printf("pro_conn:pro_dtn_msg err!res=%d\n",res);
        return -6;
    }    
    if(res == 1)
    {
        return 0;
    }
    //recv data 
    res = init_dtn_msg(&msg);
    if(res < 0)
    {
        printf("pro_conn:init_dtn_msg err!res=%d\n",res);
        return -5;
    }
    res = myrecv(server,(char*)(&msg),sizeof(dtn_req_msg));
    if(res < 0)
    {
        printf("pro_conn:myrecv dtn7 msg err!\n");
        return -6;
    }
    res = process_dtn_msg(server,&msg,blue_addr);
    if(res < 0)
    {
        printf("pro_conn:pro_dtn_msg err!res=%d\n",res);
        return -6;
    }
    //quit
    res = init_dtn_msg(&msg);
    if(res < 0)
    {
        printf("pro_conn:init_dtn11_msg err!res=%d\n",res);
        return -5;
    }
    msg.type = 11;
    res = process_dtn_msg(server,&msg,blue_addr);
    if(res < 0)
    {
        printf("pro_conn:pro_dtn11_msg err!res=%d\n",res);
        return -6;
    }
    return 0;
} 

int main()
{
    struct sockaddr_rc addr = {0};
    struct sockaddr_in server;
    int s,status;
    //char blue_addr[18] = "90:00:4E:A7:10:55";//dest blue addr
    //char ipv6_addr[25] = {"fe80::961:7aed:d92a:7118"};
    char blue_addr[18] = "00:27:13:C9:42:6E";//dest blue addr
    //char ipv6_addr[25] = {"fe80::961:7aed:d92a:7118"};
    //allocate socket
    s = socket(AF_BLUETOOTH,SOCK_STREAM,BTPROTO_RFCOMM);
     
    //set the connection parameters(who to connect to)
    addr.rc_family = AF_BLUETOOTH;
    //loc_addr.rc_bdaddr = *BDADDR_ANY;
    str2ba(blue_addr,&addr.rc_bdaddr);
    addr.rc_channel = 1;

    //connect 
    status = connect(s,(struct sockaddr*)&addr,sizeof(addr));
    printf("status:%d\n",status);
    //send msg
    if( 0 == status)
    {  
        int res = process_connect(s,blue_addr);
        if(res < 0)
        {
            printf("peocess_connect err!\n");
            close(s);
            return -1;
        }
    }
    if(status < 0)
    {
        printf("connect err!\n");
    }
    //close connection
    close(s);
    return 0;
}
