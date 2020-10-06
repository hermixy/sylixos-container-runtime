

/****************************************************************************
 * 长春工业大学 创新实验室 取个名好难团队
 * 文件名： runtime.h
 * 内容描述：容器运行时的主要内容，从存储结构到容器操作
 * 依赖的头文件：func.h
 * 文件历史：
 * 版本号      日期              作者              说明
 * 0.1      2020-05-25      naihuangbao     设计数据结构和程序基本框架
 * 0.2      2020-05-27      naihuangbao     完善部分管理方法
 * 0.3      2020-06-05      naihuangbao     完成运行时核心方法get_pod_cid 和部分管理方法
 * 0.35     2020-06-06      naihuangbao     对get_pod_cid方法将进行部分修复和优化
 * 0.4      2020-06-10      naihuangbao     完善赛题要求的所有部分管理方法
 * 0.5      2020-06-18      naihuangbao     完成对头文件进行小规模重构，使得程序的可扩展性增强
 * 0.6      2020-06-23      naihuangbao     对连接容器的功能进行进一步优化，可以判定tid是否存在
 * 0.7      2020-07-05      naihuangbao     增加数据文件以保存程序数据
 * 0.8      2020-07-06      naihuangbao     改进get_pod_cid使其安全性提高，将copy中的文件识别定义为适用于sylixos
 * 0.85     2020-07-08      naihuangbao     修复部分bug
 * 0.9      2020-07-14      naihuangbao     增加功能暂停和继续容器并修复bug
 * 0.92     2020-07-14      naihuangbao     优化get_pod_cid，使其变得更加安全可靠
 * 0.95     2020-07-14      naihuangbao     进行多处内存优化
 * 1.0      2020-07-16      naihuangbao     新增修改容器路径的功能，对get_path进行部分问题的修复
 * 1.05     2020-07-16      naihuangbao     修复多处bug。新增功能change_path
 * 1.1      2020-08-04      naihuangbao     添加部分注释，优化代码风格使其更符合编码规范，修复get_pod_cid中的一个隐藏bug
 * 1.15     2020-08-09      naihuangbao     修复了get_pod_cid方法中的一个隐藏的迭代溢出的问题
 * 1.5      2020-08-23      naihuangbao     完整实现了备份和恢复功能
 */

#ifndef _RUNTIME_H_
#define _RUNTIME_H_
#include <dirent.h>
#include "include.h"    //集中引用
#include "define.h"     //宏预定义
#include "func.h"       //执行函数，如r_system等


//容器基本信息的数据结构以及线性表，以及线性表的管理方法
typedef struct Pod_data Pod;    //存储container基本信息
typedef struct Pod_List Pod_list;   //pod线性表
Pod new_pod(const string name, const string path); //新建pod
Pod_list new_pod_list();    //新建pod_list
bool del_pod_from_pod_list(Pod_list *tmp,int16 pos);  //删除pod
Pod get_pod(Pod_list *tmp,int pos);     //取得pod
void append_pod(Pod_list *tmp,Pod *pod);    //添加pod
//以下是3种查找方式
int find_pod_index_by_name(Pod_list *list,string name,int start_pos);
int find_pod_index_by_id(Pod_list *list,string id,int start_pos);
int find_pod_index_by_path(Pod_list *list,string path,int start_pos);



/*
 * 结构体Pod用于存储容器的基础信息
 */
typedef struct Pod_data{
    //容器基础属性
    char name[30];      //name属性
    char id[5];         //容器id
    char path[260] ;    //容器路径

    //扩展属性预留位置
    string config_path; //容器配置路径
    string lib_path;    //容器资源路径
    string flag_data;   //特殊返回值
    string config_opt;  //容器的其他属性

    //暂停相关支持
    int is_pause;       //容器是否处于暂停状态   0：未暂停   1：已暂停
    int cid_flag;       //用于在get_pod_cid中判断容器的状态

    //备份相关
    char backups_path[100];     //备份文件路径
    char backups_time[40];      //备份文件时间
    char backups_unzip_path[260]; //备份时容器目录

    //重要预留，对已经设计好准备更新的内容进行预留
    //预留位置，用于存储rid，使用rid代替name和path验证进行操作
    //rid分大小写，在容器创建时自动随机生成
    string rid;

    //预留位置，用于判断容器目录文件是否被删除
    //容器文件被删除后，如果存在备份可以使用备份进行恢复
    int alive;

}Pod;

/*
 * 新增一个container的基本信息记录
 * 输入：
 *     name : 容器name属性
 *     path : 容器路径
 * 输出：
 *     创建好的容器结构体
 */
Pod new_pod(const string name, const string path){
    //新增了属性is_pause，判断容器是否为停止状态
    Pod pod;
    string arg1 = (string)calloc(sizeof(char),strlen(name));
    strcpy(arg1,name);

    string arg2 = (string)calloc(sizeof(char),strlen(path));
    strcpy(arg2,path);

    //将Pod初始化
    strcpy(pod.name, arg1);
    strcpy(pod.path, arg2);
    strcpy(pod.id,"");
    pod.is_pause = False;
    strcpy(pod.backups_path,"\0");
    strcpy(pod.backups_time,"\0");
    strcpy(pod.backups_unzip_path,"\0");
    return pod;
}

/*
 * Pod_list
 * 实现了pod的顺序表，仅存储
 */

typedef struct Pod_List{
    int16 size;     //当前长度
    int16 max_size; //当前最大长度（可变）
    string flag_data;   //用于特殊返回值
    Pod** data;     //实际存储的数据位置

    bool (*del_pod_from_pod_list)(Pod_list *tmp,int16 pos);     //从线性表中删除指定pos的pod
    Pod (*get)(Pod_list *tmp,int pos);  //取得指定pos的pod
    void (*append)(Pod_list *tmp,Pod *pod); //尾置新的pod

    //三种索引方式
    int (*find_index_by_name)(Pod_list *list,string name,int start_pos);
    int (*find_index_by_id)(Pod_list *list,string id,int start_pos);
    int (*find_index_by_path)(Pod_list *list,string path,int start_pos);
}Pod_list;


/*
 * 初始化pod_list
 */
Pod_list new_pod_list(){
    //成员变量初始化
    Pod_list list;
    list.size = 0;
    list.max_size = 50;
    list.flag_data = NULL;

    //为data申请内存空间
    Pod **data;
    data = (Pod**)malloc(sizeof(Pod*)*list.max_size);
    list.data = data;

    //成员函数初始化
    list.del_pod_from_pod_list = *del_pod_from_pod_list;
    list.get = *get_pod;
    list.append = *append_pod;
    list.find_index_by_name = *find_pod_index_by_name;
    list.find_index_by_id = *find_pod_index_by_id;
    list.find_index_by_path = *find_pod_index_by_path;

    return list;
}


Pod get_pod(Pod_list *tmp,int pos){
    //判定输入的pos是否合法
    if(tmp->size <= pos || pos < 0){
        Pod pod;
        pod.flag_data = "pos error";
        return pod;
    }
    Pod* pod_tmp = tmp->data[pos];
    return *pod_tmp;
}

void append_pod(Pod_list *tmp,Pod *pod){
    int i;
    //判定是否表满
    if(tmp->max_size == tmp->size-1){
        tmp->max_size += 20;    //延长表的大小
        Pod** data_pod;         //重新申请新的内存空间
        data_pod = (Pod**)malloc(sizeof(Pod*)*tmp->max_size);
        for(i=0;i<tmp->size;i++){   //迁移数据
            data_pod[i] = tmp->data[i];
        }
        tmp->data = data_pod;   //更新数据
    }

    //安全起见复制所有数据而不是直接饮用
    Pod *x;
    x = (Pod*)calloc(sizeof(Pod),1);
    strcpy(x->name,pod->name);
    strcpy(x->path,pod->path);
    strcpy(x->id,pod->id);
    strcpy(x->backups_path,pod->backups_path);
    strcpy(x->backups_unzip_path,pod->backups_unzip_path);
    strcpy(x->backups_time,pod->backups_time);
    x->is_pause = False;
    tmp->data[tmp->size] = x;
    tmp->size += 1;
}

/*
 * 删除节点
 * 注意！
 * 这个方法只是将节点从内存中移除，并非删除节点的文件和配置信息
 */
bool del_pod_from_pod_list(Pod_list *tmp,int16 pos){
    //判定pos是否合理
    if (tmp->size <= pos || pos < 0){
        return False;
    }

    //重新申请等长内存
    Pod** data_pod = NULL;
    data_pod = (Pod**)malloc(sizeof(Pod*)*tmp->max_size);
    if(data_pod == NULL)
        return Error;
    int i;

    //迁移数据
    for (i=0;i<pos;i++)
        data_pod[i] = tmp->data[i];
    for(i=pos+1;i<tmp->size;i++)
        data_pod[i-1] = tmp->data[i];
    free(tmp->data); //释放内存
    tmp->data = data_pod;   //更新存储
    tmp->size -= 1;     //更新大小

    return True;
}

/*
 * args:
 * Pod_list *list   pod_list自身
 * string name      需要索引的名字
 * int start_pos    开始索引的位置
 *
 * return:
 *  >=0 元素的index
 *  -1  未找到
 *  -2  错误，超出数组范围
 */
int find_pod_index_by_name(Pod_list *list,string name,int start_pos){
    if(list->size == 0 || list->size <= start_pos)
        return -2;
    int i;
    for(i=start_pos;i<list->size;i++){
        if(strcmp(list->data[i]->name,name) == 0)
            return i;
    }
    return -1;
}

/*
 * args:
 * Pod_list *list   pod_list自身
 * string id        需要索引的pod的id
 * int start_pos    开始索引的位置
 *
 * return:
 *  >=0 元素的index
 *  -1  未找到
 *  -2  错误，超出数组范围
 */
int find_pod_index_by_id(Pod_list *list,string id,int start_pos){
    if(list->size == 0 || list->size <= start_pos)
        return -2;
    int i;
    for(i=start_pos;i<list->size;i++){
        if(strcmp(list->data[i]->id,id)==0)
            return i;
    }
    return -1;
}


/*
 * args:
 * Pod_list *list   pod_list自身
 * string path      需要索引的pod的路径
 * int start_pos    开始索引的位置
 *
 * return:
 *  >=0 元素的index
 *  -1  未找到
 *  -2  错误，超出数组范围
 */
int find_pod_index_by_path(Pod_list *list,string path,int start_pos){
    //todo
    //增加/验证
    if(list->size == 0 || list->size <= start_pos)
        return -2;
    int i;
    for(i=start_pos;i<list->size;i++){
        if(strcmp(list->data[i]->path, path)==0)
            return i;
    }
    return -1;
}






typedef struct Node_data Node ;
List_string get_containers_notes();
List_string new_container(Pod *pod);
int remove_container(Pod *pod);
bool remove_pod_thorough(Pod_list *list,int16 pos);
List_string stop_container(Pod *pod);

int get_pod_cid(Pod_list *plist);  //项目核心方法，非重要更新或者修复问题请勿动
List_string run_container(string path);
List_string stop_container(Pod *pod);
int restart_container(Pod *pod);
int update_container(Pod *pod,string update_path);
bool copy_container(string src_path, string dst_path);
void append_to_node(Node *node,Pod *pod);
string get_remaining_disk_space();
void get_remaining_memory();


/*
 * Node节点信息
 * 这里包括了node节点的各种信息
 */
typedef struct Node_data{
    string ip_self;     //自身ip地址 保留属性，方便扩展
    string ip_master;   //master节点ip地址 保留属性
    int port_self;      //暴露的端口 保留属性
    int port_master;    //apiserver的通讯端口 保留属性
    string node_name;   //节点名称
    Pod_list pods;      //存放pod_list
    int run_now;        //当前运行的容器数量 暂时无用，已被get_pod_cid的返回值替代
    List_string history;    //操作历史记录 保留属性
    char total_memory[20];  //sylixos系统总内存 可能有bug
    char used_memory[20];   //sylixos系统使用了的内存
    char total_disk_space[20];  //sylixos中总存储空间大小
    char remaining_disk_space[20]; //可用存储空间大小
    void (*append)(Node *node,Pod *pod);
}Node;

/*
 * 新建一个node(初始化)   这个步骤只有在初始化节点的时候进行
 * args                     说明
 * string ip
 * int port
 * string node_name
 */
Node new_node(string ip_self,string ip_master,int port_self,int port_master, string node_name){
    //初始化各项成员变量
    Node node;
    node.ip_master=ip_master;
    node.ip_self=ip_self;
    node.port_self = port_self;
    node.port_master = port_master;
    node.node_name = node_name;
    node.pods = new_pod_list();
    //node.run_now = get_pod_cid(&node.pods);
    //node.total_memory;
    //node.remaining_memory;
    //get_remaining_memory();
    //node.remaining_disk_space = get_remaining_disk_space();
    //node.append = *append_to_node;
    dolog("new node","success");
    return node;
}

/*
 * 此方法会引起内存问题，暂时弃用
 */
void append_to_node(Node *node,Pod *pod){
    node->pods.append(&node->pods,pod);
}


/*
 * 获取当前总运行内存和已使用的运行内存
 * TODO
 */
void get_remaining_memory(List_string *list,Node *node){
    //List_string list1 = r_system("free");  //正式测试需要去掉部分参数
    int i;
    char res1[20]=""; //总内存
    char res2[20]=""; //已使用内存
    for(i=0;i<list->size;i++){
        //printf("%s",list->data[i]);

        //申请并复制得到的系统信息第i行的前6位
        char tmp1[7] = {0};
        strncpy(tmp1,list->data[i],6);

        if(strcmp(tmp1,"kersys") == 0){
            //比较是不是 “kersys” 如果匹配成功则进行下面操作
            string temp = list->data[i]+6; //移动指针位置对准内存数量
            //printf("temp: %s",temp);


            int j;
            int flag = 1; //用于判定是否到了空格

            //获取总内存
            for(j=0;j<strlen(temp);j++){
                //申请长度为2的字符串，一个字符一个字符的提取
                char tmp[2] = {0};
                strncpy(tmp,temp+j,1);
                if(strcmp(tmp," ") == 0){
                    //循环判断是否到空格
                    if(flag == 1)
                        continue;
                    else
                        break;
                }
                flag = 0;
                //printf("tmp:%s   ",tmp);
                //结果拼接到res1返回值上
                strcat(res1,tmp);
            }

            temp+=j;  //移动指针位置
            //printf("temp: %s",temp);
            flag = 1; //重置flag

            //获取当前已使用内存 算法同上
            for(j=0;j<strlen(temp);j++){
                char tmp[2] = {0};
                strncpy(tmp,temp+j,1);
                if(strcmp(tmp," ") == 0){
                    if(flag == 1)
                        continue;
                    else
                        break;
                }
                flag = 0;
                //printf("tmp:%s   ",tmp);
                strcat(res2,tmp);
            }
            break;
        }
    }
    //printf("res1:%s\n",res1);
    //printf("res2:%s\n",res2);

    //将结果写入传入的node结构体指针中
    strcpy(node->total_memory,res1);
    strcpy(node->used_memory,res2);
    //printf("total: %s,used: %s\n",node->total_memory,node->remaining_memory);

    //进行日志记录
    string lg;
    lg = (string)calloc(sizeof(char),70);
    sprintf(lg,"success,total:%s, used:%s",res1,res2);
    dolog("get memory",lg);

    //return "If you see this message, the function is not finished";
}

/*
 * 获取总磁盘空间和剩余磁盘空间，算法与上面的获取内存相同
 */
string get_remaining_disk_space(List_string *list,Node *node){
    //List_string list1 = r_system("df /yaffs2/n1");  //正式测试需要去掉部分参数
    int i;
    char res1[20]=""; //总存储空间
    char res2[20]=""; //剩余存储空间
    for(i=0;i<list->size;i++){
        //printf("%s",list->data[i]);
        char t[11] = {0};
        strncpy(t,list->data[i],10);
        //printf("%s\n",t);
        if(strcmp(t,"/yaffs2/n1") == 0){
            //printf("223333");
            string temp = list->data[i]+11;
            //printf("temp: %s",temp);
            int j;

            int flag = 1;
            //获取总存储
            for(j=0;j<strlen(temp);j++){
                char tmp[2] = {0};
                strncpy(tmp,temp+j,1);
                if(strcmp(tmp," ") == 0){
                    if(flag == 1)
                        continue;
                    else
                        break;
                }
                flag = 0;
                //printf("tmp:%s   ",tmp);
                strcat(res1,tmp);
            }
            temp+=j;
            //printf("temp: %s",temp);
            flag = 1;
            //获取当前剩余存储空间
            for(j=0;j<strlen(temp);j++){
                char tmp[2] = {0};
                strncpy(tmp,temp+j,1);
                if(strcmp(tmp," ") == 0){
                    if(flag == 1)
                        continue;
                    else
                        break;
                }
                flag = 0;
                //printf("tmp:%s   ",tmp);
                strcat(res2,tmp);
            }
            break;
        }
    }
    //printf("res1:%s\n",res1);
    //printf("res2:%s\n",res2);
    //写入数据
    strcpy(node->total_disk_space,res1);
    strcpy(node->remaining_disk_space,res2);

    //记录日志
    string lg;
    lg = (string)calloc(sizeof(char),70);
    sprintf(lg,"success,total:%s, remaining:%s",res1,res2);
    dolog("get disk",lg);
    return "If you see this message, the function is not finished";
}

/*
 返回当前所有容器的运行情况
 */
List_string get_containers_notes(){
    dolog("get notes","success");
    //执行系统命令并且返回值
    return r_system("container -s");
}

/*
 新建一个容器
    args                        说明
    Pod *pod                    新建的容器
    注意：因为使用线程中断法，所以次方法的返回值无意义，如必须返回值，可以用管道
        将结果输出至缓存文件然后再读取缓存文件
 */
List_string new_container(Pod *pod){
    //申请内存并合成系统命令  container -c path name
    string cmd;
    cmd = (string)calloc(sizeof(char),strlen(pod->path)+strlen(pod->name)+16);
    strcpy(cmd,"container -c ");
    strcat(cmd,pod->path);
    strcat(cmd," ");
    strcat(cmd,pod->name);

    //记录日志
    string lg;
    lg = (string)calloc(sizeof(char),70);
    sprintf(lg,"name:%s, path:%s",pod->name,pod->path);
    dolog("new container",lg);

    //执行系统命令
    return r_system(cmd);
}

/*
 * 从文件系统中删除这个pod的全部信息，但是在pod_list中不删除
 * 预留这个方法的目的是为了以后更新，可以随时创建同配置容器
 */
int remove_container(Pod *pod){
    //检查容器是否已停止，如果没有则先行停止容器
    if(strcmp(pod->id , "") == 0)
        stop_container(pod);

    //记录日志
    string lg;
    lg = (string)calloc(sizeof(char),70);
    sprintf(lg,"name:%s, path:%s",pod->name,pod->path);
    dolog("del pod",lg);

    //删除文件并且返回是否成功（在func.h中）
    return remove_dir(pod->path);
}

/*
 * 彻底删除pod信息，包括文件和node_list
 */
bool remove_pod_thorough(Pod_list *list,int16 pos){
    //判断pos是否合理
    if(pos>=list->size || pos < 0)
        return Error;
    //string del_path = list->data[pos]->path;

    //先行调用删除方法删除目录
    int res = remove_container(list->data[pos]);
    if(res != 0){
        return False;
    }

    //从程序内存中也删除该容器信息
    list->del_pod_from_pod_list(list,pos);
    return True;
}


List_string get_containers_notes1();


/*
 * 本系统中的最核心方法
 * 程序内核
 * 通过分析系统返回值来对内存中的Pod_list进行状态变更
 * 同时可以识别未录入管理的容器，并将其录入管理
 * 输入： Pod_list *plist  当前操作的Pod线性表
 * 输出： 当前正在运行的容器数量（不包括系统内核）
 */
int get_pod_cid(Pod_list *plist){
    //NotTODO
    //在这里需要加一个路径匹配以保证安全，低优先级(已完成)
    //需要新增功能，将正在运行但是并不在podlist中记录的container加入podlist(已完成)
    List_string slist = get_containers_notes();
    int i;
    int count=0;  //用于对当前运行中的容器进行计数

    for(i=0;i<slist.size;i++){
        //申请长度为10的字符串，为了判定系统命令返回值的开头是否为"CONTAINER"
        //front_9_的意义为前9个字符
        char front_9_[10]={0};
        strncpy(front_9_,slist.data[i],9);
        //printf("data:%s,,fount 9:%s\n",slist.data[i],front_9_);
        if(strcmp(front_9_,"CONTAINER") == 0){
            //确定有CONTAINER字样为首的一行，第15个字符即为容器id
            int j=14;
            char cid[5]={0};//多设置一点点大小，防止容器数量太多超过上限，现在上限9999
            char name[20] = "";
            while(True){
                //设置长度为2的缓存
                char tmp[2]={0};
                strncpy(tmp,slist.data[i]+j,1); //从第15位开始取(index=14)
                if(strcmp(tmp,":\0") != 0){
                    strcat(cid,tmp); //将缓存拼接到cid中
                    j++; //指针后移
                    strcpy(tmp,"\0");//重置tmp
                } else
                    break;
            }
            string tmp_name_string = slist.data[i]+j+2; //从这个位置开始是name 这个变量为整个字符串指针
            int t;  //循环变量
            int fg;
            for(t=0;t<strlen(tmp_name_string);t++){
                //一个字符一个字符依次拼接成完整的name
                char name_tmp_char[2]={0};  //这个变量意义为存储的单个字符
                strncpy(name_tmp_char,tmp_name_string+t,1);
                if(strcmp(name_tmp_char,"(\0") == 0 ){
                    fg = 0;
                    break;
                }else if(strcmp(name_tmp_char,"\n\0") == 0) {
                    fg = 1;
                    break;
                }
                //进行拼接
                strcat(name,name_tmp_char);
            }

            //移动指针位置，准备提取path
            tmp_name_string+=t+1;

            char path1[260]={0};
            sscanf(tmp_name_string,"%[^)\n]",path1);  //匹配路径 从头取到 ”)\n“

            char *path = path1+10;
            path[strlen(path)-11]='\0'; //切割获得的path

            printf("cid,name,path: %s,%s,%s\n",cid,name,path1);

            count += 1;
            //下面是将数据匹配
            int ct;
            int fgx=0;
            printf("----------------------------");
            for(ct=0;ct<plist->size;ct++){
                if(strcmp(plist->data[ct]->name,name) == 0){

                    /*
                     * 在name匹配成功的情况下进一步验证path
                     * path存在很多种情况，有没有/，分别验证
                     * 变量解释：x,y,z 三种情况的判定结果(源路径与提取路径完全一致，原路径带"/" 和提取路径带"/")
                     * 变量解释：tc,td 分别临时存储path的其他情况
                     * TODO
                     * 这里可以优化内存
                    */
                    int x = strcmp(plist->data[ct]->path,path);
                    char tc[260];
                    strcpy(tc,plist->data[ct]->path);
                    strcat(tc,"/");
                    int y = strcmp(tc,path);
                    char td[260];
                    strcpy(td,path);
                    strcat(td,"/");
                    int z = strcmp(td,plist->data[ct]->path);
                    //printf("df:%d,%s,%s\n",x,plist->data[ct]->path,path);

                    if(x == 0 || y == 0 || z == 0){
                        strcpy(plist->data[ct]->id , cid);
                        plist->data[ct]->cid_flag = 1; //2020年7月14日20:10:26新增，用于准确识别已停止容器
                        fgx = 1;
                        break;
                    }
                }else if(plist->data[ct]->cid_flag == 0)//2020年7月14日20:10:26新增，用于准确识别已停止容器
                    strcpy(plist->data[ct]->id , "");//2020年7月14日20:10:26新增，用于准确识别已停止容器
            }
            if(fgx == 0){
                Pod xcpc = new_pod(name,path);  //xcpc : 未在本程序中录入但是在运行中的容器
                strcpy(xcpc.id,cid);
                plist->append(plist,&xcpc);     //将容器也纳入管理
            }
        }
    }
    //2020年7月16日03:18:37 更新
    for(i=0;i<plist->size;i++){
        if(strcmp(plist->data[i]->id,"") == 0){
            plist->data[i]->is_pause = 0;
        }
        plist->data[i]->cid_flag = 0;   //时间2020年8月4日03:32:46新增，修复一些问题 重置属性
    }
    return count-1;
}



/*
 * 启动一个容器，后面要紧跟get_pod_cid方法以验证是否启动成功
 * 注意：因为使用线程中断法，所以次方法的返回值无意义，如必须返回值，可以用管道
        将结果输出至缓存文件然后再读取缓存文件
 */
List_string run_container(string path){
    //not_todo 逻辑可优化，低优先度
    //拼接命令 srtp -f path/config.ini &
    string cmd;
    cmd = (string)calloc(sizeof(char),strlen(path)+20);
    strcpy(cmd,"srtp -f ");
    strcat(cmd,path);
    strcat(cmd,"/config.ini &");

    //记录日志
    string lg;
    lg = (string)calloc(sizeof(char),70);
    sprintf(lg,"path:%s",path);
    dolog("run pod",lg);

    //执行命令
    return r_system(cmd);

}

/*
 * 停止/杀死一个容器
 */
List_string stop_container(Pod *pod){
    //nottodo
    //判定容器是否正在运行
    if(strcmp(pod->id,"") == 0){
        List_string res = new_list_string();
        res.flag_data = "error,this container is already stoped";
        return res;
    }

    //拼接命令 container -k cid
    string cmd;
    cmd = (string)calloc(sizeof(char),strlen(pod->id)+15);
    strcpy(cmd,"container -k ");
    strcat(cmd,pod->id);

    //记录日志
    string lg;
    lg = (string)calloc(sizeof(char),70);
    sprintf(lg,"name:%s, path:%s",pod->name,pod->path);
    dolog("stop pod",lg);

    //初始化cid
    strcpy(pod->id,"");

    //执行命令
    return r_system(cmd);
}

/*
 * 重启一个容器
 * 注意：因为使用线程中断法，所以次方法的返回值无意义，如必须返回值，可以用管道
        将结果输出至缓存文件然后再读取缓存文件
 */
int restart_container(Pod *pod){
    //notTODO
    //需要改进一下连接字符串的机制(已经不需要)
    //判定容器是否正在运行
    if(strcmp(pod->id,"") == 0){
        run_container(pod->path);
        return 0;
    }

    //记录日志
    string lg;
    lg = (string)calloc(sizeof(char),70);
    sprintf(lg,"name:%s, path:%s",pod->name,pod->path);
    dolog("restart pod",lg);

    //停止容器
    stop_container(pod);

    //启动容器
    run_container(pod->path);

    return 0;
}

/*
 * 更新容器
 */
int update_container(Pod *pod,string update_path){
    //notTODO
    //这里需要确定更新容器要不要停止，经过实验，可以不停止
    //现在是不停止

    //判定容器是否正在运行
    int flag = 0;
    if(strcmp(pod->id,"") == 0){
        //stop_container(pod);
        flag = 1;
    }

    //分别初始化存放目录和文件的线性表
    List_string dir = new_list_string();
    List_string file = new_list_string();

    //通过get_path方法将复制源路径的所有path都分类存放进上面初始化的线性表方便复制
    bool x = get_path(update_path,&dir,&file);
    if (x == -1){
        return -1; //空目录
    }

    //执行复制操作
    copy_dir(update_path,pod->path,&dir,&file);
    if (flag == 1){
        //run_container(pod->path);
    }

    //2020年7月14日20:18:28增加，释放内存
    int i;
    for(i=0;i<dir.size;i++){
        free(dir.data[i]);
    }
    free(dir.data);
    for(i=0;i<file.size;i++){
        free(file.data[i]);
    }
    free(file.data);
    return 0;
}


bool copy_container(string src_path, string dst_path){
    return copy_dir_full(src_path,dst_path);
}



/*
 * 将系统命令 ts 的返回值分别存放至name_list 和 tid_list 两个线性表
 * 这两个线性表中元素关系是一一对应的（根据下标一一对应）
 * runtime内部支持方法，建议不要外部调用
 */
void get_tids(List_string *name_list, List_string *tid_list ){
    //nottodo
    //放到sylixos上的时候，要把下面那行去掉注释，然后去掉参数input(已去掉)

    //执行系统命令ts
    List_string input = r_system("ts");  //check 2020年8月4日04:21:36增加 弃用do_ts方法
    int i;
    for(i=4;i<input.size;i++){
        //开始分割，先去掉开始4行（无意义）
        string tmp = input.data[i];
        //tmp = (string)calloc(sizeof(char),strlen(input->data[i]));
        //printf("tmp: %s",tmp);

        //判断是否已经结束
        char is_finish[2];
        strncpy(is_finish,tmp,1);
        if(strcmp(is_finish,"\n") == 0) // 设置空行结束
            break;

        //提取名字，name长度上限暂定40
        char name[40]={0};
        while(1){
            char name_tmp_char[2]={0};  //name的单个字母缓存
            strncpy(name_tmp_char,tmp,1);
            //printf("%s\n",name);
            if(strcmp(name_tmp_char," \0") == 0){
                //读到空格就break
                tmp += 1;
                break;
            }
            strcat(name,name_tmp_char); //连接内容到name
            tmp +=1;
        }
        //printf("1,%s\n",tmp);

        while(1){ //去除中间的空格
            char space_tmp_char[2]={0};
            strncpy(space_tmp_char,tmp,1);
            if(strcmp(space_tmp_char," \0") != 0){
                //tmp += 1;
                break;
            }
            tmp +=1;
        }
        //printf("2,%s\n",tmp);

        //开始提取tid
        char tid[15]={0};
        while(1){
            //按位遍历tid
            char tid_tmp_char[2]={0};
            strncpy(tid_tmp_char,tmp,1);
            if(strcmp(tid_tmp_char," \0") == 0){
                //判断tid是否读到了空格
                //tmp += 1;
                break;
            }
            strcat(tid,tid_tmp_char); //将非空格位连接进tid
            tmp +=1;
        }
        //printf("name: %s   tid: %s\n",name,tid); //输出结果，仅测试
        //将取得的结果按位存放入输入的list指针
        name_list->append(name_list,name);
        tid_list->append(tid_list,tid);
    }

    //释放内存
    for(i=0;i<input.size;i++)
        free(input.data[i]);
    free(input.data);
}



/*
 * 连接容器
 * 参数表：
 *      Pod_list *list      当前使用的pod线性表
 *      string cid          所选需要连接的cid
 *      string tid          所选需要连接的tid
 * 返回值：
 *      -1      所选cid未在运行
 *      0       所选tid不存在
 *      1       成功
 */
int link_container(Pod_list* list,string cid,string tid){
    //按照cid查找相关的pod，返回pos
    int pos = find_pod_index_by_id(list,cid,0);
    if(pos<0){
        return -1; //所选cid没在运行中
    }

    //合成命令 container -a cid tid
    char cmd[40]={0};
    strcpy(cmd,"container -a ");
    strcat(cmd,cid);
    strcat(cmd," ");
    strcat(cmd,tid);

    //验证输入的线程是否在内核存在
    //先初始化两个线性表，一个存放name，一个存放tid，name和tid是一一对应的，即name_list.data[0] 对应 tid_list.data[0]
    List_string name_list = new_list_string();
    List_string tid_list = new_list_string();
    get_tids(&name_list,&tid_list); //将系统返回值分割存放到两个数组
    int i;
    for(i=0;i<tid_list.size;i++){   //验证所选tid是否存在
        if(strcmp(tid,tid_list.data[i]) == 0){
            system(cmd);  //如果存在则直接执行
            return 1; //成功
        }
    }
    return 0; //所选tid不存在
}




/*
 * 将内存中的Pod_list信息按行存储进Pod_list中
 */
void save_data(Node *node){
    //读取数据文件
    FILE *file = fopen(DATA_PATH,"wb");

    int i;
    char split_flag[16] = ":/:splitflag:/:";    //设置分隔符
    for(i=0;i<node->pods.size;i++){
        //为每行申请内存然后拼接
        string data = (string)calloc(sizeof(char),strlen(node->pods.data[i]->name)+
                        strlen(node->pods.data[i]->path)+strlen(node->pods.data[i]->backups_path)+
                        strlen(node->pods.data[i]->backups_unzip_path)+
                        strlen(node->pods.data[i]->backups_time)+strlen(split_flag)*4+20);
        strcpy(data,node->pods.data[i]->name);
        strcat(data,split_flag);
        strcat(data,node->pods.data[i]->path);
        strcat(data,split_flag);
        strcat(data,node->pods.data[i]->backups_path);
        strcat(data,split_flag);
        strcat(data,node->pods.data[i]->backups_unzip_path);
        strcat(data,split_flag);
        strcat(data,node->pods.data[i]->backups_time);
        strcat(data,"\n");
        printf("%s",data);
        //写入
        fputs(data,file);

        //释放内存
        free(data);
    }
    fclose(file);
    return;
}





/*
 * 初始化已存储数据的方法
 * 传入刚创建的*node
 * 找到数据文件返回1，无数据文件返回0，数据文件错误返回-1
 */
int init_pods_data(Node *node){
    int i=0;
    int j=0;
    char split_flag[16] = ":/:splitflag:/:";    //分隔符

    int x = access(DATA_PATH,F_OK);         //判断文件是否存在
    if(x == -1){    //如果不存在，新建一个数据文件
        char new_file[60]={0};
        strcpy(new_file,"touch ");
        strcat(new_file,DATA_PATH);
        system(new_file);
        return 0;
    }

    //读取数据文件
    FILE *f = fopen(DATA_PATH, "rb");
    while (!feof(f) && !ferror(f)) {
        //申请内存
        char *line;
        line = (string)calloc(sizeof(char),300);
        strcpy(line, "\n");
        //按行读取文件
        fgets(line, 300, f);
        //printf("%s",line);
        /*
        char name[30] = {0};
        char path[260] = {0};
        char backups_path[100] = {0};
        char backups_unzip_path[260] = {0};
        char backups_time[40] = {0};
        */
        string name = (string)calloc(sizeof(char),30);
        string path = (string)calloc(sizeof(char),260);
        string backups_path = (string)calloc(sizeof(char),100);
        string backups_unzip_path = (string)calloc(sizeof(char),260);
        string backups_time = (string)calloc(sizeof(char),40);

        strcpy(name,"\0");
        strcpy(path,"\0");
        strcpy(backups_path,"\0");
        strcpy(backups_unzip_path,"\0");
        strcpy(backups_time,"\0");

        char temp0[2]={0};

        //判定是否读到了空行（结尾）
        strncpy(temp0,line,1);
        if(strcmp(temp0,"\n\0") == 0 || j >= 300)
            return 0;

        //按位读取
        while(1){
            //判断是否读到了分隔符
            char tmp[16]={0};
            strncpy(tmp,line,15);
            if (strcmp(tmp,split_flag) == 0){
                line += 15;
                break;
            }

            //按位判断是否读到了结尾，如果没有就写入name
            char temp[2]={0};
            strncpy(temp,line,1);
            if(strcmp(temp,"\n\0") == 0 || j >= 300)
                return -1;
            strcat(name,temp);
            line+=1;
            j++;
        }


        //path
        while(1){
            char tmp[16]={0};
            strncpy(tmp,line,15);
            if (strcmp(tmp,split_flag) == 0){
                line += 15;

                break;

            }

            //算法同上，按位读取判断并写入
            char temp[2]={0};
            strncpy(temp,line,1);
            if(strcmp(temp,"\n\0") == 0 || strcmp(temp,"\0") == 0)
                return -1;
            strcat(path,temp);
            line+=1;
        }

        //backup_path
        while(1){
            char tmp[16]={0};
            strncpy(tmp,line,15);
            if (strcmp(tmp,split_flag) == 0){
                line += 15;
                break;
            }

            //算法同上，按位读取判断并写入
            char temp[2]={0};
            strncpy(temp,line,1);
            if(strcmp(temp,"\n\0") == 0 || strcmp(temp,"\0") == 0)
                return -1;

            strcat(backups_path,temp);
            line+=1;
        }

        //backups_unzip_path
        while(1){
            char tmp[16]={0};
            strncpy(tmp,line,15);
            if (strcmp(tmp,split_flag) == 0){
                line += 15;
                break;
            }

            //算法同上，按位读取判断并写入
            char temp[2]={0};
            strncpy(temp,line,1);
            if(strcmp(temp,"\n\0") == 0 || strcmp(temp,"\0") == 0)
                return -1;

            strcat(backups_unzip_path,temp);
            line+=1;
        }

        //backups_time
        while(1){
            //算法同上，按位读取判断并写入
            char temp[2]={0};
            strncpy(temp,line,1);
            if(strcmp(temp,"\n\0") == 0 || strcmp(temp,"\0") == 0)
                break;
            strcat(backups_time,temp);
            line+=1;
        }


        //新建一个pod结构体然后写入信息
        Pod x = new_pod(name,path);
        strcpy(x.backups_path,backups_path);
        strcpy(x.backups_unzip_path,backups_unzip_path);
        strcpy(x.backups_time,backups_time);
        node->pods.append(&node->pods,&x);
        //printf("init data:name:%s  path:%s\n",node->pods.data[i]->name,node->pods.data[i]->path);
        i++;
        free(name);
        free(path);
        free(backups_path);
        free(backups_unzip_path);
        free(backups_time);
    }
    fclose(f);
    return 0;

}



/*
 * 暂停容器
 * 输入： Pod *pod   需要操作的pod结构体指针
 * 输出：
 *      -1  容器未运行
 *      0   容器已经处于暂停状态
 *      1   成功
 */
int pause_container(Pod *pod){
    printf("stop,name:%s,path:%s,cid:%s\n",pod->name,pod->path,pod->id);

    if(strcmp(pod->id,"") == 0)
        return -1; //容器未运行
    if(pod->is_pause == True)
        return 0; //容器已暂停

    //合成命令 contaienr -p cid
    char cmd[20]={0};
    strcpy(cmd,"container -p ");
    strcat(cmd,pod->id);

    //记录日志
    string lg;
    lg = (string)calloc(sizeof(char),70);
    sprintf(lg,"name:%s, path:%s, cid:%s",pod->name,pod->path,pod->id);
    dolog("pause pod",lg);

    //执行系统命令
    system(cmd);

    //设置pod属性为已暂停
    pod->is_pause = True;

    return 1;
}


int continue_container(Pod *pod){
    //继续容器
    printf("continue,name:%s,path:%s,cid:%s\n",pod->name,pod->path,pod->id);
    if(strcmp(pod->id,"") == 0)
        return -1; //容器未运行

    if(pod->is_pause == False)
        return 0; //容器已在运行

    //合成命令 contaienr -r cid
    string cmd;
    cmd = (string)calloc(sizeof(char),strlen(pod->id)+15);
    strcpy(cmd,"container -r ");
    strcat(cmd,pod->id);

    //记录日志
    string lg;
    lg = (string)calloc(sizeof(char),70);
    sprintf(lg,"name:%s, path:%s, cid:%s",pod->name,pod->path,pod->id);
    dolog("continue pod",lg);

    //执行系统命令
    system(cmd);

    //设置pod属性
    pod->is_pause = False;
    return 1;
}


/*
 * 按cid查询容器状态信息
 * 输入：
 *      Pod_list *list  当前操作的pod线性表
 *      string cid      需要查找的cid
 * 输出：系统命令返回值+是否暂停的按行存储线性表
 */
List_string container_s_cid(Pod_list *list,string cid){
    //合成系统命令 container -s cid
    char cmd[25] = {0};
    strcpy(cmd,"container -s ");
    strcat(cmd,cid);

    //更新容器状态
    get_pod_cid(list);

    //执行系统命令并且获取返回值
    List_string res = r_system(cmd);

    //按照输入的cid查找容器，并将是否暂停写入尾行
    int x = find_pod_index_by_id(list,cid,0);
    if(x>=0){
        char cd[30] = {0};
        strcpy(cd,"is container pause: ");
        if(list->data[x]->is_pause == False)
            strcat(cd,"False");
        else
            strcat(cd,"True");
        res.append(&res,cd);
    }
    return res;
}

/*
 * 修改容器的name属性
 * 输入：
 *      Pod_list *list  当前操作的pod线性表指针
 *      string old_name 容器的原名字
 *      string path     容器的路径
 *      string new_path 容器的新名字
 *
 * 输出：是否成功
 *      -2  配置文件无法正确读取，可能是容器文件损坏
 *      -1  未找到与new_name 和 path 匹配的容器
 *      0   配置文件读取错误，可能是容器文件损坏
 *      1   修改成功
 */
int edit_pod_name(Pod_list *list,string old_name, string path, string new_name){
    //todo
    //重要
    int i;
    int flag1 = 0;   //是否读取找到name属性的标识符

    //在内存中匹配old_name
    for(i=0;i<list->size;i++){
        if(strcmp(list->data[i]->name,old_name) == 0 && strcmp(list->data[i]->path, path) == 0){

            //合成并验证配置文件是否存在
            char config_path[270] = {0};
            strcpy(config_path,path);
            char* t = &config_path[strlen(config_path)-1];
            if(strcmp(t,"/\0") == 0)
                strcat(config_path,"config.ini");
            else
                strcat(config_path,"/config.ini");
            int rcf = is_file_found(config_path);
            if(rcf == -1){
                return -2; // 找不到配置文件路径，可能是容器已损坏
            }

            //打开配置文件
            FILE *f = fopen(config_path,"r");
            int j=0;
            //新建一个List_string来按行存放文件信息
            List_string file_data = new_list_string();
            while (!feof(f) && !ferror(f)) {

                //按行读取配置文件
                char line[300]={0};
                //line = (string)calloc(sizeof(char),300);
                //strcpy(line, "\0");
                fgets(line, 299, f);

                if(j == 1 ){ //name属性在配置文件第二行
                    //判断是否能够找到name=字样
                    char flag[6] = {0};
                    strncpy(flag,line,5);
                    printf("%s\n",flag);
                    if(strcmp(flag,"name=\0") == 0){
                        flag1 = 1;
                        char d[30]={0};
                        strcpy(d,"name=");
                        strcat(d,new_name);
                        strcat(d,"\n");
                        file_data.append(&file_data,d);
                    }

                } else
                    file_data.append(&file_data,line);
                j++;

            }
            fclose(f);
            if(flag1 == 1){ //如果那么存在则按行覆写文件
                FILE *file = fopen(config_path,"w+");
                int c;
                for(c=0;c<file_data.size;c++){
                    printf("%s",get_s(&file_data,c));
                    fputs(get_s(&file_data,c),file);
                }
                fclose(file);
                strcpy(list->data[i]->name,new_name);
                return 1; // 成功
            }else{
                return 0; //配置文件错误
            }
            //todo
            //这里应该释放list_string的内存

        }
    }
    return -1; //未找到容器
}


/*
 * 修改容器的hostname属性
 * 输入：
 *      Pod_list *list  当前操作的pod线性表指针
 *      string old_name 容器的原名字
 *      string path     容器的路径
 *      string new_path 容器的新hostname
 *
 * 输出：是否成功
 *      -2  配置文件无法正确读取，可能是容器文件损坏
 *      -1  未找到与old_name 和 path 匹配的容器
 *      0   配置文件读取错误，可能是容器文件损坏
 *      1   修改成功
 *
 * 算法同上edit_pod_name，完全一致，就是将name换成了hostname（在配置文件第三行），故不作多余注释
 */
int edit_pod_hostname(Pod_list *list,string old_name, string path, string new_name){
    int i;
    int flag1 = 0;
    for(i=0;i<list->size;i++){
        if(strcmp(list->data[i]->name,old_name) == 0 && strcmp(list->data[i]->path, path) == 0){
            char config_path[270] = {0};
            strcpy(config_path,path);
            char* t = &config_path[strlen(config_path)-1];
            if(strcmp(t,"/\0") == 0)
                strcat(config_path,"config.ini");
            else
                strcat(config_path,"/config.ini");


            int rcf = is_file_found(config_path);
            if(rcf == -1){
                return -2; // 找不到配置文件路径，可能是容器已损坏
            }
            FILE *f = fopen(config_path,"r");
            int j=0;
            List_string file_data = new_list_string();
            while (!feof(f) && !ferror(f)) {
                char line[300]={0};
                //line = (string)calloc(sizeof(char),300);
                //strcpy(line, "\0");
                fgets(line, 299, f);

                if(j == 2 ){
                    char flag[10] = {0};
                    strncpy(flag,line,9);
                    //printf("%s\n",flag);
                    if(strcmp(flag,"hostname=\0") == 0){
                        flag1 = 1;
                        char d[30]={0};
                        strcpy(d,"hostname=");
                        strcat(d,new_name);
                        strcat(d,"\n");
                        file_data.append(&file_data,d);
                    }

                } else
                    file_data.append(&file_data,line);
                j++;

            }
            fclose(f);
            if(flag1 == 1){
                FILE *file = fopen(config_path,"w+");
                int c;
                for(c=0;c<file_data.size;c++){
                    printf("%s",get_s(&file_data,c));
                    fputs(get_s(&file_data,c),file);
                }
                fclose(file);
                //strcpy(list->data[i]->name,new_name);
                return 1;
            }else{
                return 0; //配置文件错误
            }

        }
    }
    return -1; //未找到容器
}


/*
 * 执行container -s并且返回结果（字符串）
 */
string show_container(){
    //执行系统命令并按行获取返回值
    List_string res = r_system("container -s");
    int i;

    //为字符串申请内存空间
    string rs = (string)calloc(sizeof(char),4000);
    strcpy(rs,"");

    //拼接字符串
    for(i=0;i<res.size;i++){
        strcat(rs,res.data[i]);
    }

    //释放内存
    for(i=0;i<res.size;i++)
        free(res.data[i]);
    free(res.data);

    //记录日志
    dolog("in show","finish");
    return rs;
}


//2020年7月16日02:38:33 更新
/*
 * 更改容器的路径
 * 注意：更改容器路径后，原先的备份文件对应的路径仍为原路径！
 * 输入：
 *      Pod_list *list  当前操作的容器列表
 *      string name     容器的name属性
 *      string path     容器源路径
 *      string new_path 容器的新路径
 * 输出：
 *      -3  找不到容器目录
 *      -2  复制后无法找到相应路径（复制失败，new_path不合法），保留原容器不变
 *      -1  未找到容器
 *      0   容器正在运行
 *      1   成功
 */
int change_path(Pod_list *list, string name, string path, string new_path){
    int i;
    int flag = 0;   //设置判断标志

    //按行查找匹配的容器
    for(i=0;i<list->size;i++){
        if(strcmp(name,list->data[i]->name) == 0 && strcmp(path,list->data[i]->path) == 0){
            flag = 1;
            break;
        }
    }

    //判断容器状态
    if(flag == 0){
        return -1; //未找到容器
    }
    if(strcmp(list->data[i]->id,"") != 0){
        return 0; //容器正在运行
    }

    //清空目标路径
    remove_dir(new_path);

    //复制目录到目标目录
    int cp_res = copy_container(path,new_path);
    if(cp_res == -1)
        return -3; // Pod目录不存在

    //合成新的配置文件目录并验证目录是否存在
    string config_path_1 = (string)calloc(sizeof(char),128);
    string config_path_2 = (string)calloc(sizeof(char),128);
    strcpy(config_path_1,new_path);
    strcat(config_path_1,"/config.ini");
    strcpy(config_path_2,new_path);
    strcat(config_path_2,"config.ini");

    int a1 = is_file_found(config_path_1);
    int a2 = is_file_found(config_path_2);

    if(a1 == a2){
        return -2; // new_path 不存在
    }

    //移除容器的旧目录和内存信息，然后新建容器信息存储到Pod_list
    Pod p = new_pod(name,new_path);
    strcpy(p.backups_path, list->data[i]->backups_path);
    strcpy(p.backups_time, list->data[i]->backups_time);
    strcpy(p.backups_unzip_path, list->data[i]->backups_unzip_path);

    remove_pod_thorough(list,i);

    append_pod(list,&p);

    //释放内存
    free(config_path_1);
    free(config_path_2);

    return 1; //成功
}



string random_zip_name(Pod_list *list, int len){
    //设置随机表
    string pst =  (string)calloc(1,64);
    strcpy(pst,"abcdefghijklmnopqrstuvwxyz1234567890_ABCDEFGHIJKLMNOPQRSTUVWXYZ");



    //申请内存
    string res = (string)calloc(sizeof(char), len+strlen(BACKUP_PATH)+5);
    int i;
    int j;


    while(1){
        //重置变量
        int flag = 0;
        strcpy(res,BACKUP_PATH);
        //重置种子

        for(i=0;i<len-5;i++){
            int tmp = rand() % 64;
            char single_char_tmp[2] = {0};
            strncpy(single_char_tmp,pst+tmp,1);
            strcat(res,single_char_tmp);
        }

        strcat(res,".zip");
        for(j=0;j<list->size;j++){
            if(strcmp(list->data[j]->backups_path,res) == 0)
                flag = 1;
        }

        if(flag == 0)
            break;

    }
    free(pst);
    return res;
}


int backup_pod(Pod_list *list, string name, string path){
    //查找容器
    int i;
    int flag = 0;
    for(i=0;i<list->size;i++){
        if(strcmp(list->data[i]->name,name) == 0 && strcmp(list->data[i]->path,path) == 0){
            flag = 1;
            break;
        }
    }
    if(flag == 0)
        return -1; //找不到容器

    int path_size = 14;

    //如果已经有备份则覆盖
    if(strcmp(list->data[i]->backups_path,"\0")!=0){
        remove_file(list->data[i]->backups_path);
        strcpy(list->data[i]->backups_path,"\0");
        strcpy(list->data[i]->backups_time,"\0");
        strcpy(list->data[i]->backups_unzip_path,"\0");
    }

    //用zip压缩
    string backup_path = random_zip_name(list,path_size);
    string cmd = (string)calloc(sizeof(char),strlen(path)+strlen(backup_path)+20);
    strcpy(cmd,"zip -r ");
    strcat(cmd,backup_path);
    strcat(cmd," ");
    strcat(cmd,path);
    //system(cmd);

    printf("cmd in backup_pod:%s\n",cmd);

    //判断文件是否生成成功 windows测试中去除这项
    //if(is_file_found(backup_path) == -1)
    //    return 0;   //压缩失败


    strcpy(list->data[i]->backups_path,backup_path);

    time_t tp;
    struct tm *now;
    time (&tp);
    now=gmtime(&tp);

    string time_string = (string)calloc(sizeof(char),50);
    sprintf(time_string,"%d/%d/%d-%d:%d:%d",now->tm_year+1900,now->tm_mon,
            now->tm_mday,now->tm_hour,now->tm_min,now->tm_sec);
    strcpy(list->data[i]->backups_time,time_string);

    strcpy(list->data[i]->backups_unzip_path,list->data[i]->path);

    free(backup_path);
    free(time_string);

    printf("%s,%s\n",list->data[i]->backups_time,list->data[i]->backups_path);

    return 1; //压缩成功
}


/*
 * 还原备份
 * 注意：是硬还原，容器将完全还原到备份时的状态
 * 注意：如果备份后容器目录迁移，还原的时候会还原到原目录(删除后解压) 并且删除当前容器目录
 */
int reduction_pod_from_zip(Pod_list *list, string name, string path){
    int i;
    int flag = 0;
    for(i=0;i<list->size;i++){
        if(strcmp(list->data[i]->name,name) == 0 && strcmp(list->data[i]->path,path) == 0){
            flag = 1;
            break;
        }

    }
    if(flag == 0)
        return -1; //找不到容器

    if(strcmp(list->data[i]->backups_path,"") == 0)
        return -2;  //不存在备份


    //windows测试的时候去掉这个返回情况
    //if(is_file_found(list->data[i]->backups_path) == -1){
    //    strcpy(list->data[i]->backups_path,"\0");
    //    strcpy(list->data[i]->backups_time,"\0");
    //    return -3;  //备份文件丢失
    //}


    /*
    int count = 0;
    flag = 0;
    string tmp_path1 = list->data[i]->path+strlen(list->data[i]->path)-1;
    string real_path = (string)calloc(1,200);

    //提取最后一个/后的路径  eg：/root/con/con2  -> /root/con
    while(count<strlen(list->data[i]->path)){
        char tmp[2]={0};
        strncpy(tmp,tmp_path1-count,1);
        if(strcmp(tmp,"/") == 0 && count != 0){
            flag = 1;
            break;
        }
        count++;
    }
    if(flag == 0){
        strcpy(real_path, list->data[i]->path); //纯相对路径，而且只有一层
    }else{
        strncpy(real_path,list->data[i]->path,strlen(list->data[i]->path)-count);
    }
    printf("real_path:%s\n",real_path);
    */
    //删除原目录，采取硬还原，完全还原备份状态
    //不进行比较，如果目录不存在也不会报错
    //remove_dir(list->data[i]->backups_unzip_path);
    //remove_dir(list->data[i]->path);

    //合成命令 unzip backups_path -d /
    string cmd1 = (string)calloc(sizeof(char),strlen(TMP_PATH)+strlen(list->data[i]->backups_path)+20);
    strcpy(cmd1,"unzip ");
    strcat(cmd1,list->data[i]->backups_path);
    strcat(cmd1," -d /");
    //strcat(cmd1,real_path);
    //strcat(cmd1,"/");
    printf("cmd: %s\n",cmd1);

    //将容器路径回迁移
    strcpy(list->data[i]->path,list->data[i]->backups_unzip_path);

    //执行命令
    //system(cmd1);
    return 1;


}

string list_container_list(Pod_list *list){
    int size = 0;
    int i;
    for(i=0;i<list->size;i++){
        size+=strlen(list->data[i]->name);
        size+=strlen(list->data[i]->path);
        size+=strlen(list->data[i]->backups_path);
        size+=strlen(list->data[i]->backups_time);
        size+=strlen("name:  path:  backups_path:  backups_time:  ");
        size+=40;
    }
    string res = (string)calloc(sizeof(char),size);
    strcpy(res,"\0");

    for(i=0;i<list->size;i++){
        string tmp = (string)calloc(sizeof(char),
                                    strlen(list->data[i]->name)+
                                    strlen(list->data[i]->path)+
                                    strlen(list->data[i]->backups_path)+
                                    strlen(list->data[i]->backups_time)+
                                    strlen("name:  path:  backups_path:  backups_time:  ")
                                    +40);

        sprintf(tmp,"name: %s  path: %s  backups_path: %s  backups_time: %s\n",
                list->data[i]->name,list->data[i]->path,
                list->data[i]->backups_path,list->data[i]->backups_time);
        strcat(res,tmp);
        free(tmp);
    }
    return res;
}


int remove_backup(Pod_list *list, string name, string path){
    int i;
    int flag = 0;
    for(i=0;i<list->size;i++){
        if(strcmp(list->data[i]->name,name) == 0 && strcmp(list->data[i]->path,path) == 0){
            flag = 1;
            break;
        }

    }

    if(flag == 0)
        return -1; //找不到容器

    if(strcmp(list->data[i]->backups_path,"") == 0){
        return -2;  //备份不存在
    }

    //删除文件
    remove_file(list->data[i]->backups_path);

    //判断文件是否还存在
    if(is_file_found(list->data[i]->backups_path) == -1){
        strcpy(list->data[i]->backups_path,"\0");
        strcpy(list->data[i]->backups_time,"\0");
        strcpy(list->data[i]->backups_unzip_path,"\0");
        return 1;
    }

    return 0;   //删除失败，文件可能正在被占用
}

/*
 * 本地测试方法，用于模拟系统返回值，勿用
 */
List_string get_containers_notes1(){
    List_string res = new_list_string();
    res.append(&res,"container show >>\n");
    res.append(&res,"\n");
    res.append(&res,"CONTAINER     0: KERNEL\n");
    res.append(&res,"      NAME            FATHER      STAT  PID   GRP    MEMORY    UID   GID   USER\n");
    res.append(&res,"---------------- ---------------- ---- ----- ----- ---------- ----- ----- ------\n");
    res.append(&res,"total vprocess : 0\n");
    res.append(&res,"total memory   : 0KB\n");
    return res;
}

#endif