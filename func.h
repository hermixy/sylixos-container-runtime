

/****************************************************************************
 * 长春工业大学 创新实验室 取个名好难团队
 * 文件名：func.h
 * 内容描述：对容器运行时的执行做支持，几乎所有容器运行时的具体操作都是在这个头文件进行
 * 依赖的头文件：slist.h
 * 文件历史：
 * 版本号      日期              作者              说明
 * 0.1      2020-05-24      naihuangbao     设计include顺序并完成r_system函数
 * 0.2      2020-05-26      naihuangbao     完成remove_dir函数，递归删除目录
 * 0.5      2020-06-01      naihuangbao     完成copy_dir以及其依赖方法
 * 0.8      2020-06-18      naihuangbao     更新dolog方法方便记录日志和测试
 * 0.9      2020-07-03      naihuangbao     对copy_dir及其支持方法进行内存优化
 * 1.0      2020-07-10      naihuangbao     修复copy_dir方法的一个隐藏bug
 * 1.1      2020-08-20      naihuangbao     添加部分注释，优化代码风格使其更符合编码规范
 * 1.2      2020-08-25      naihuangbao     将runtime.h中的部分内容移入func.h
 */

#ifndef _FUNC_H_
#define _FUNC_H_

#include "include.h"
#include "slist.h"

void dolog(string state,string msg);

List_string r_system(char *cmd)
{
    //为返回值申请内存
    List_string result = new_list_string();
    //string buf={0};
    char buf[256] = {0};
    //string buf = (string)calloc(sizeof(char),256);
    //strcpy(buf,"\0");
    //通过popen方法执行系统命令并获取返回值
    FILE *fp = NULL;
    if( (fp = popen(cmd, "r")) == NULL ) {
        printf("popen error!\n");
        result.flag_data = "popen error!";
        return result;
    }
    //将返回值按行存储
    while (fgets(buf, sizeof(buf), fp)) {
        result.append(&result,buf);
    }

    pclose(fp);
    //free(buf);
    dolog("system",cmd);

    return result;
}

/*
 * 递归法删除非空目录
 */
int remove_dir(string path){
    //申请内存，读取目录
    DIR *s=NULL;
    char tmp[256];
    struct dirent *p=NULL;
    if((s=opendir(path))!=NULL){
        while((p=readdir(s))!=NULL){
            //拼接文件目录
            if(strcmp(p->d_name,"..") != 0 && strcmp(p->d_name,".") != 0){
                strcpy(tmp,"\0");
                strcat(tmp,path);
                strcat(tmp,"/");
                strcat(tmp,p->d_name);
                //分类操作
                if(p->d_type==4) //目录，递归
                    remove_dir(tmp);
                rmdir(tmp);
                if(p->d_type==8) //文件，删除
                    remove(tmp);
            }
        }
        rmdir(path);
    }
    closedir(s);
    return 0;
}

/*
 * 删除单个文件
 * 无论文件是否存在都会执行
 * 输入：
 *      string path     需要删除的文件路径
 */
void remove_file(string path){
    //申请内存 合成命令 执行命令
    string cmd;
    cmd = (string)calloc(sizeof(char),strlen(path)+6);
    strcpy(cmd,"rm ");
    strcat(cmd,path);
    system(cmd);
}

/*
 * 数目录中有几层( / 的个数)
 * string x     需要数个数的目录
 */
int count_splits(string x){
    int i;
    int len = strlen(x);
    int count = 0;
    for(i=0;i<len;i++){
        //按位查找 /
        char tc[2]={0};
        strncpy(tc,x,1);
        if(strcmp(tc,"/\0")==0)
            count++;
        //printf("%s ",tc);
        x++;
    }
    return count;
}

/*
 * 整理一个可以有序创建文件夹的list
 */
List_string make_mkdir_tree(List_string *list){
    //生命变量 申请内存
    int i;
    int count = 0;
    int sign = 0;
    List_string res = new_list_string();

    //一个一个的查找数量，然后按数量存放
    while (count < list->size){
        for(i=0;i<list->size;i++){
            int c = count_splits(list->data[i]);
            //printf("%d ",c);
            if(sign == c){
                string x = list->data[i];
                res.append(&res,x);
                count++;
                //printf("count:%d  ",count);
            }
        }
        sign++;
        //printf("\nsign: %d ",sign);
    }
    //printf("\n%d\n",list->size);
    return res;

}


/*
 * 得到不包含目标路径父路径的相对路径
 * eg:
 * printf("%s\n",get_relative_path("/root/apps","/root/apps/src/main.c"))
 * 输出: /src/main.c
 */
string get_relative_path(string input_path,string new_path){
    int x1 = strlen(input_path);
    return new_path+x1;
}

bool get_path(string path,List_string *dir, List_string *file){
    //申请内存，申请变量
    DIR *s = NULL;
    char tmp[300]={0};
    struct  dirent *p=NULL;
    if((s=opendir(path))!=NULL) {
        while((p=readdir(s))!=NULL){
            //判断文件类型然后分类存放
            if(strcmp(p->d_name,"..") != 0 && strcmp(p->d_name,".") != 0){
                //拼接目录
                strcpy(tmp,"\0");
                strcat(tmp,path);
                strcat(tmp,"/");
                strcat(tmp,p->d_name);
                printf("path: %s, counts: %d\n",tmp,file->size+dir->size);
                if(p->d_type==4){
                    //目录，linux下是4
                    get_path(tmp,dir,file);
                    bool res = dir->find(dir,tmp,0);
                    if(res < 0){
                        dir->append(dir,tmp);
                        //printf("append finish\n");
                    }
                }
                if (p->d_type==8){
                    //文件，linux下是8
                    bool res = file->find(file,tmp,0);
                    if(res < 0){
                        file->append(file,tmp);
                        //printf("append finish\n");
                    }
                    continue;
                }
            }
        }
        closedir(s);
    }else{
        return -1;
    }
    return 0;
}

/*
 * 调用系统命令执行复制文件
 * args:
 * string src   源路径
 * string dst   目标路径
 */
void system_do_copy(string src,string dst){
    string cmd=NULL;
    int size = strlen(src)+strlen(dst)+8;
    printf("%d  ",size);
    cmd = (string)calloc(sizeof(char),size);
    strcat(cmd,"cp ");
    strcat(cmd,src);
    strcat(cmd," ");
    strcat(cmd,dst);
    printf("%s\n",cmd);
    system(cmd);
}
/*
 * 调用系统命令执行新建文件夹操作
 * args:
 * string path  路径
 */
void system_mkdir(string path){
    string cmd=NULL;
    int size = strlen(path)+8;
    printf("%d  ",size);
    cmd = (string)calloc(sizeof(char),size);
    strcat(cmd,"mkdir ");
    strcat(cmd,path);
    dolog("system",cmd);
    system(cmd);
}

/*
 * 判断文件是否已经存在，用于
 * args:
 * string path  文件路径
 *
 * return:
 * 0    文件存在
 * -1    文件不存在
 */
int is_file_found(string path){
    return access(path,F_OK);
}

/*
 * 使用源文件(目录)路径，复制源路径，目标源路径合成目标文件(目录)路径
 * args:
 * string file_path 源文件(目录)路径
 * string src_path  复制源路径
 * string dst_path  目标源路径
 * char res[4100]       最后返回的字符串,注意，这里不可以传入string，一定要传入char[4100]
 * 传入char[4100]是为了保存最长路径
 */
void get_full_dst_path(string file_path,string src_path,string dst_path,string res){
    // printf("filepath: %s    dstpath: %s\n",file_path,dst_path);

    int int_flag = 0;
    string relative_path = get_relative_path(src_path,file_path);
    char flag1[2] = {0};
    strncpy(flag1,relative_path,1);
    if(strcmp(flag1,"/") == 0)
        int_flag ++;

    int len = strlen(dst_path);
    char flag2[2] = {0};
    strncpy(flag2,dst_path+len-1,1); //取最后一位
    if(strcmp(flag2,"/") == 0)
        int_flag ++;

    strcpy(res,dst_path);
    //printf("1");
    if(int_flag == 0){
        strcat(res,"/");
        strcat(res,relative_path);
    }else if(int_flag == 1){
        strcat(res,relative_path);
    }else{
        strcat(res,relative_path+1);
    }
    //printf("%s\n",res);
}


/*
 * 复制整个目录包括子目录的所有文件，并且保留文件结构
 * 将src下复制到dst下
 * args:
 * string src_path      复制源路径
 * string dst_path      复制目标路径
 * List_string *dir     目录数组，用于创建目录
 * List_string *file    文件数组，用于创建文件
 */
bool copy_dir(string src_path,string dst_path,List_string *dir_i,List_string *file){
    int i;
    List_string dir = make_mkdir_tree(dir_i);
    system_mkdir(dst_path);
    for(i=0;i<dir.size;i++){
        int size1 = strlen(dir.get(&dir,i))-strlen(src_path)+strlen(dst_path)+5;
        string tmp1;
        //printf("%d   ",size1);
        tmp1 = (string)malloc(sizeof(char)*size1);
        get_full_dst_path(dir.get(&dir,i),src_path,dst_path,tmp1);
        system_mkdir(tmp1);
        //printf("mkdir : %s\n",tmp1);
    }
    for(i=0;i<file->size;i++){
        string tmp2;
        int size2 = strlen(file->get(file,i))-strlen(src_path)+strlen(dst_path)+5;
        //printf("%s",file->data[i]);
        tmp2 = (string)malloc(sizeof(char)*size2);
        get_full_dst_path(file->get(file,i),src_path,dst_path,tmp2);
        //printf("copy files : %s\n",tmp2);
        /*
        bool x = is_file_found(tmp2); // 判断目标位置是否存在该文件，有则覆盖，避免系统询问时卡住
        if(x == True)
            remove_file(tmp2);
        */
        remove_file(tmp2);
        system_do_copy(file->get(file,i),tmp2);
    }


    for(i=0;i<dir.size;i++){//2020年7月14日20:16:08增加，用于释放内存
        free(dir.data[i]);
    }
    free(dir.data);
    return 1;
}

string int_to_string(string dst,int src){
    sprintf(dst,"%d",src);
    return dst;
}


void dolog(string state, string msg){
    //printf("start log");
    time_t tp;
    struct tm *now;
    time (&tp);
    now=gmtime(&tp);

    string a;
    a = (string)calloc(sizeof(char),strlen(state)+strlen(msg)+30);
    sprintf(a,"%d/%d/%d, %d:%d:%d\t",now->tm_year+1900,now->tm_mon,
            now->tm_mday,now->tm_hour,now->tm_min,now->tm_sec);

    strcat(a,state);
    strcat(a,"\t");
    strcat(a,msg);
    strcat(a,"\n");
    FILE *file = fopen(LOG_PATH,"a");
    fputs(a,file);
    fclose(file);
    //printf("%s\n",a);
    free(a);

}

/*
 * 判断字符串末位有没有  /  字符存在
 */
int is_the_slash_at_the_end_of_string(string input){
    char tmp[2]={0};
    strncpy(tmp,input+strlen(input)-1,1);
    if(strcmp(tmp,"/\0") == 0)
        return 1;
    return 0;
}


/*
 * 判断字符串首位有没有  /  字符存在
 */
int is_the_slash_at_the_head_of_string(string input){
    char tmp[2]={0};
    strncpy(tmp,input,1);
    if(strcmp(tmp,"/\0") == 0)
        return 1;
    return 0;
}


int is_file_read(string path){
    return access(path,R_OK);
}

bool copy_dir_full(string src_path, string dst_path){
    //算法与更新相似，就是目标路径不一样

    //分别初始化存放目录和文件的线性表
    List_string dir_list = new_list_string();
    List_string file_list = new_list_string();

    //通过get_path方法将复制源路径的所有path都分类存放进上面初始化的线性表方便复制
    bool x = get_path(src_path,&dir_list,&file_list);
    if (x == -1){
        printf("src:%s\n",src_path);
        return -1;//空目录
    }

    //执行复制操作
    copy_dir(src_path,dst_path,&dir_list,&file_list);


    //2020年7月14日20:18:28增加，释放内存
    int i;
    for(i=0;i<dir_list.size;i++){
        free(dir_list.data[i]);
    }
    free(dir_list.data);
    for(i=0;i<file_list.size;i++){
        free(file_list.data[i]);
    }
    free(file_list.data);
    return 1;
}


#endif