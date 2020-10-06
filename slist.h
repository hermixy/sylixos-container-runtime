

/****************************************************************************
 * 长春工业大学 创新实验室 取个名好难团队
 * 文件名： slist.h
 * 内容描述：可边长字符串线性表 数据结构List_string 和其支持函数
 * 依赖的头文件：无
 * 文件历史：
 * 版本号      日期              作者              说明
 * 0.1      2020-05-20      naihuangbao     设计数据结构
 * 0.3      2020-05-21      naihuangbao     完善部分操作函数
 * 0.6      2020-06-05      naihuangbao     完成大部分操作方法
 * 0.8      2020-06-14      naihuangbao     内存优化
 * 0.9      2020-07-01      naihuangbao     弃用copy，内存优化
 * 0.95     2020-07-14      naihuangbao     内存优化
 * 1.0      2020-07-15      naihuangbao     内存优化，修复隐藏问题
 * 1.1      2020-08-10      naihuangbao     添加注释，修改部分不符合编码规范的变量名
 */
#ifndef _SLIST_H_
#define _SLIST_H_
#include "include.h"

/* List_string
    成员变量                                                                  说明
    short size                                                          表示list的当前有效长度，是长度，是data的index+1（在有1个数据的时候是1）
    short max_size                                                      表示当前申请的内存大小
    string[] data                                                       存储数据的地方
    string flag_data                                                    函数返回时携带的其他传递信息，如错误信息等

    成员函数:
    返回值 名字                         args                                说明                                        用例
    void  append            List_string *self, string x                 向list末尾添加变量                       list.append(&list,"in");
    bool  insert            List_string *self, string x ,int pos        向任意index插入变量，pos为其位置         list.insert(&list,"in",3);
    bool  del               List_string *self, int pos                  删除指定index的变量                      list.del(&list,3);
    List_string  copy       List_string *self                           独立复制一个相同的list                   List_string res = list.copy(&list);
    string  get             List_string *self, int pos                  从list中取出第pos位元素                  list.get(&list,3);
    bool  change            List_string *self, string x,int pos         修改pos位元素的值                        list.change(&list,"changed",3);
    int  find               List_string *self, string x ,int start_pos  在输入list中的指定位置开始查找指定元素，返回pos     list.find(&list,"in",0);
    void  connect           List_string *self, List_string *src         连接两个list，src是需要合并进self的list   list.connect(&list,&res);
    string list_to_string                                               list->string                             string x = list_to_string(&list);
    List_string string_to_list                                          string->list                              List_string res = string_to_list(x);
    void write_list_value        List_string *list                           list按行写成文件                          write_list_value(&list)


    声明方法：
    List_string new_list = new_list_string();   即可初始化一个list

*/

typedef struct List_String List_string;
void append_s(List_string *tmp,string x);
bool insert_s(List_string *tmp,string x,short pos);
bool del_s(List_string *tmp,int pos);
//List_string copy_s(List_string *tmp);  //这个方法可能会导致严重的内存溢出
string get_s(List_string *tmp,int pos);
bool change_s(List_string *tmp,string x,int pos);
int find_index_s(List_string *tmp,string x,int pos);
void connect_list(List_string *base,List_string *src);



typedef struct List_String{
    short size;         //线性表当前大小
    short max_size;     //线性表当前最大长度
    char** data;        //存储的数据
    string flag_data;   //用于特殊的返回值

    //下面是一些成员函数
    void(*append)(List_string *tmp,string x);
    bool(*insert)(List_string *tmp,string x,short pos);
    bool(*delete)(List_string *tmp,int pos);
    //List_string(*copy)(List_string *tmp);
    string(*get)(List_string *tmp,int pos);
    bool(*change)(List_string *tmp,string x,int pos);
    int (*find)(List_string *tmp,string x,int pos);
    void (*connect)(List_string *base,List_string *src);
}List_string;


/*
 * 初始化新的List_string
 * 返回初始化好的list
 */
List_string new_list_string(){
    List_string tmp;
    tmp.max_size = 50;
    tmp.size = 0;
    char** data_string;
    data_string = (string*)malloc(sizeof(string)*(tmp.max_size));
    tmp.flag_data = "None";
    tmp.data = data_string;
    tmp.append = *append_s;
    tmp.insert = *insert_s;
    tmp.delete = *del_s;
    //tmp.copy = *copy_s;
    tmp.get = *get_s;
    tmp.change = *change_s;
    tmp.find = *find_index_s;
    tmp.connect = *connect_list;
    return tmp;
}

/*
 * 向输入的list中后置一个字符串
 * 输入：
 *      List_string *tmp    操作的list
 *      string x            插入的字符串
 */
void append_s(List_string *tmp,string x){
    int i;

    //判断线性表是否表满，如果表满则延长表
    if(tmp->max_size <= tmp->size-1){
        tmp->max_size += 20;

        //申请新的内存空间
        string* data_string;
        data_string = (string*)malloc(sizeof(string)*(tmp->max_size+1));

        //拷贝数据
        for(i=0;i<tmp->size;i++){
            string x = (string)calloc(sizeof(char),strlen(tmp->data[i])+1);
            strcpy(x,tmp->data[i]);
            data_string[i] = x;
            free(tmp->data[i]);
        }

        //更替新老数据
        free(tmp->data);
        tmp->data = data_string;
    }

    //向表尾插入输入的字符串
    string data_tmp = (string)malloc(strlen(x)+1);
    strcpy(data_tmp,x);
    tmp->data[tmp->size] = data_tmp;
    tmp->size += 1;
}


/*
 * 向指定的pos插入输入的字符串
 * 输入：
 *      List_string *tmp    操作的线性表
 *      string x            插入的字符串
 *      short pos           需要插入的位置pos
 */
bool insert_s(List_string *tmp,string input,short pos){

    //判断输入的pos是否合法
    if (tmp->size+1 < pos || pos < 0){
        return Error;
    }
    int i;

    //判断是否表满，如果表满则增加表长
    if(tmp->max_size <= tmp->size-1)
        tmp->max_size += 20;

    //申请新的内存空间
    string* data_string;
    data_string = (string*)malloc(sizeof(string)*(tmp->max_size));

    //拷贝前段数据
    for (i=0;i<pos;i++){
        //data_string[i] = tmp->data[i];
        string tmpx = (string)calloc(sizeof(char),strlen(tmp->data[i])+1);
        strcpy(tmpx,tmp->data[i]);
        data_string[i] = tmpx;
        free(tmp->data[i]);
    }

    //将数据插入表
    data_string[pos] = input;
    tmp->size += 1;

    //拷贝后段数据
    for(i=pos+1;i<tmp->size;i++){
        //data_string[i] = tmp->data[i-1];
        string x1 = (string)calloc(sizeof(char),strlen(tmp->data[i-1])+1);
        strcpy(x1,tmp->data[i-1]);
        data_string[i] = x1;
        free(tmp->data[i-1]);

    }

    //更替数据
    free(tmp->data);
    tmp->data = data_string;
    return True;

}

/*
 * 从表中删除指定位置的元素
 * 输入：
 *      List_string *tmp    当前操作的线性表
 *      int pos             需要删除的元素的index
 * 输出：
 *      0   pos不合法
 *      1   删除成功
 */
bool del_s(List_string *tmp,int pos){
    //判断pos是否合法
    if (tmp->size < pos || pos < 0){
        return False;
    }

    //申请新的内存空间
    string* data_string;
    data_string = (string*)malloc(sizeof(string)*(tmp->max_size));

    //拷贝数据（不拷贝指定pos）
    int i;
    for(i=0;i<pos;i++){
        data_string[i] = tmp->data[i];
    }
    for(i=pos+1;i<tmp->size;i++){
        data_string[i-1] = tmp->data[i];
    }

    //更替数据，然后修正线性表长度
    free(tmp->data);
    tmp->data = data_string;
    tmp->size -= 1;
    return True;
}

/*
 * 这个方法可能会导致严重的内存错误,暂时停用
 */
List_string copy_s(List_string *tmp){
    List_string new_list = new_list_string();
    free(new_list.data);
    string* data_string;
    new_list.max_size = tmp->max_size;
    data_string = (string*)malloc(sizeof(string)*(tmp->max_size));
    new_list.data = data_string;
    int i;
    for(i=0;i<tmp->size;i++){
        string x;
        x = (string)malloc(strlen(tmp->data[i]));
        strcpy(x,tmp->data[i]);
        new_list.append(&new_list,x);
        free(x);
    }
    return new_list;
}


/*
 * 修改线性表中指定位置的元素
 * 输入：
 *      List_string *tmp    当前操作的线性表
 *      string x            需要替换上的数据
 *      int pos             需要更改的pos位置
 */
bool change_s(List_string *tmp,string input,int pos){
    //判断pos是否合法
    if (tmp->size < pos || pos < 0){
        return Error;
    }

    //申请新的内存空间，下面算法跟删除差不多
    string* data_string;
    data_string = (string*)malloc(sizeof(string)*(tmp->max_size));

    //拷贝前段数据
    int i;
    for(i=0;i<pos;i++){
        data_string[i] = tmp->data[i];
    }

    //修改指定pos的数据
    data_string[pos] = input;

    //拷贝后段数据
    for(i=pos;i<tmp->size;i++){
        data_string[i-1] = tmp->data[i];
    }

    //更替数据
    free(tmp->data);
    tmp->data = data_string;
    tmp->size -= 1;
    return True;
}


/*
 * 取得指定pos的字符串指针
 * 输入：
 *      List_string *tmp    当前操作的线性表
 *      int pos     需要取得的pos
 *
 * 输出：指定pos的字符串指针
 */
string get_s(List_string *tmp,int pos){
    if (tmp->size < pos || pos < 0){
        return Error_s;
    }
    return tmp->data[pos];
}

/*
 * 通过字符串来寻找pos
 * 输入：
 *      List_string *tmp    当前操作的线性表
 *      string x        需要查找的内容
 *      int start_pos   索引开始位置
 */
int find_index_s(List_string *tmp,string x,int start_pos){
    //判断是否表空，判断start_pos是否合法
    if(tmp->size == 0)
        return -3;
    if(start_pos > tmp->size || start_pos <0)
        return -2;
    int i;
    //在内存中查找指定的字符串
    for(i=start_pos;i<tmp->size;i++){
        if(strcmp(tmp->data[i],x) == 0)
            return i;
    }
    return -1;
}

/*
 * 将src添加到base后面
 * 好像···没啥用
 */
void connect_list(List_string *base,List_string *src){
    int i;
    for(i=0;i<src->size;i++) {
        base->append(base, src->get(src, i));
    }
}

/*
 * 释放list的内存
 * 不知为什么在sylixos下无法使用
 * 在windows和linux下都可以使用
 */
void free_list(List_string *list){
    int i;
    for(i=0;i<list->size;i++){
        free(list->data[i]);
    }
    //free(list->data);
    //free(list);
}


/*
 * list转换成string
 * 已通过测试
 */
string list_to_string(List_string *list){
    int size_string=12;
    int i;

    //设置分隔符
    char split_flag1[8] = "<split>";
    char split_flag2[9] = "</split>";
    char start_flag[8] = "<start>";
    char size_flag1[8] = "<size>";
    char size_flag2[9] = "</size>";
    char end_flag[9] = "</start>";
    for(i=0;i<list->size;i++){
        //确定字符串的长度方便申请内存
        size_string = size_string + strlen(split_flag1)+ strlen(split_flag2) ;
        size_string = size_string + strlen(list->data[i]) ;
        size_string = size_string +9+9;
    }

    //为合成的字符串申请内存
    string res;
    res = (string)calloc(sizeof(char),size_string);

    //开始按行加分隔符开始合成字符串
    //下面是合成的例子：只有1行的list，内容为data
    //<start><size>4</size><split>data</split></start>
    strcpy(res,start_flag);
    for(i=0;i<list->size;i++){
        strcat(res,split_flag1);
        strcat(res,size_flag1);
        char x[10]={0};
        sprintf(x,"%d",strlen(list->data[i]));
        strcat(res,x);
        strcat(res,size_flag2);
        strcat(res,list->data[i]);
        strcat(res,split_flag2);
    }
    strcat(res,end_flag);
    return res;
}


/*
 * string转换成list
 * 已通过测试
 */
List_string string_to_list(string src){
    List_string res = new_list_string();
    int i;

    //设置分隔符
    char split_flag1[8] = "<split>";
    char split_flag2[9] = "</split>";
    char start_flag[8] = "<start>";
    char size_flag1[8] = "<size>";
    char size_flag2[9] = "</size>";
    char end_flag[9] = "</start>";

    //判断开头是否是 <start>
    char f[20]={0};
    strncpy(f,src,7);
    if(strcmp(f,start_flag) == 0){
        src += 7; // 应该是7
    } else{
        res.flag_data = "error";
        return res;
    }

    //开始按行提取，先提取size
    int size=0;
    for(i=0;i<strlen(src);i++){
        strncpy(f,src,13);
        //printf("%s  \n",f);

        //判断是否读取到<split><size>
        if(strcmp(f,"<split><size>") == 0){
            src += 13;
            int j;
            char tmp[6] = {0};
            char flag[8] = {0};
            for(j=0;j<strlen(src);j++){

                strncpy(flag,src,7);
                //printf("flag:%s  ",flag);
                if(strcmp(flag,"</size>") == 0){
                    size = atoi(tmp);
                    src+=7;
                    break;
                }
                char t[2]={0};
                strncpy(t,src,1);
                strcat(tmp,t);
                src+=1;
            }
        }else if(strcmp(f,"</split><spli") == 0){

            src += 21;
            int j;
            char tmp[6] = {0};
            char flag[8] = {0};
            for(j=0;j<strlen(src);j++){

                strncpy(flag,src,7);
                //printf("flag:%s  ",flag);
                if(strcmp(flag,"</size>") == 0){
                    size = atoi(tmp);   //字符串转int
                    src+=7;
                    break;
                }
                char t[2]={0};
                strncpy(t,src,1);
                strcat(tmp,t);
                src+=1;
            }

        }else{
            res.flag_data = "empty";
            return res;
        }

        //申请内存
        string data = (string)calloc(sizeof(char),size+1);
        int m;
        //开始提取数据
        for(m=i;m<strlen(src)+200;m++){
            char flag[9] = {0};
            strncpy(flag,src,8);
            //printf("flag2:%s  ",flag);
            if(strcmp(flag,"</split>") == 0){
                //printf("data:%s\n",data);
                //将提取好的数据按位存放进数组
                res.append(&res,data);
                src+=8;
                break;
            }
            char t[2]={0};
            strncpy(t,src,1);
            strcat(data,t);
            src+=1;
        }
        //释放内存
        free(data);
        char e[9]={0};
        strncpy(e,src,8);
        //printf("src: %s\n",src);
        if(strcmp(e,"</start>") == 0)
            break;
    }

    return res;
}

/*
 * 将list的内容按行写入文件
 * 输入：
 *      List_string *list   需要操作的线性表
 *      string path         目标目录
 */
void write_list_value(List_string *list, string path){
    FILE *file = fopen(path,"w");
    int i;
    for(i=0;i<list->size;i++){
        string data = (string)calloc(sizeof(char),strlen(list->data[i])+13);
        strcat(data,list->data[i]);
        strcat(data,"\n");
        fputs(data,file);
        free(data);
    }
    fclose(file);
}


//按行打印线性表内容
void print_list(List_string *list){
    int i;
    for(i=0;i<list->size;i++){
        printf("%s\n",list->data[i]);
    }
}
#endif