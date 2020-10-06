/****************************************************************************
 * 长春工业大学 创新实验室 取个名好难团队
 * 文件名：image.h
 * 内容描述：镜像管理相关支持
 * 依赖的头文件：func.h
 * 文件历史：
 * 版本号      日期            作者                说明
 * 0.1      2020-08-25      naihuangbao         设计了镜像管理的基础框架
 * 0.2beta  2020-08-26      naihuangbao         实现了部分管理方法
 * 0.2      2020-08-26      naihuangbao         修复部分0.2beta中的问题
 */

#include "func.h"

typedef struct Image_data Image;
typedef struct Image_List Image_list;
string random_image_id(Image_list *list, int len);



typedef struct Image_data {
    char id[LEN_ID];    //镜像id
    char unzip_path[260];   //解压所用路径（需要合成）
    char describe[100];     //镜像描述
    char create_time[50];   //创建时间
}Image;

Image new_image(Image_list *list, string unzip_path, string describe){
    Image res;
    strcpy(res.id,random_image_id(list,LEN_ID));
    strcpy(res.unzip_path,unzip_path);
    strcpy(res.describe,describe);

    time_t tp;
    struct tm *now;
    time (&tp);
    now=gmtime(&tp);
    string time_string = (string)calloc(sizeof(char),50);
    sprintf(time_string,"%d/%d/%d-%d:%d:%d",now->tm_year+1900,now->tm_mon,
            now->tm_mday,now->tm_hour,now->tm_min,now->tm_sec);

    strcpy(res.create_time,time_string);
    free(time_string);

    return res;
}

string make_image_path(Image *image){
    string path = (string)calloc(sizeof(char),strlen(image->id)+10+strlen(IMAGE_PATH));
    strcpy(path,IMAGE_PATH);
    strcat(path,image->id);
    strcat(path,".zip");
    return path;
}


typedef struct Image_List{
    int size;
    int max_size;
    Image **data;
}Image_list;

/*
 * 初始化image_list
 */
Image_list new_image_list(){
    Image_list res;
    res.max_size = 50;
    res.size = 0;
    Image** data = (Image**)calloc(sizeof(Image*),res.max_size);
    res.data = data;

    return res;
}

string random_image_id(Image_list *list, int len){
    //设置随机表
    string pst =  (string)calloc(1,64);
    strcpy(pst,"abcdefghijklmnopqrstuvwxyz1234567890_ABCDEFGHIJKLMNOPQRSTUVWXYZ");


    //申请内存
    string res = (string)calloc(sizeof(char), len);
    strcpy(res,"\0");
    int i;
    int j;

    while(1){
        //重置变量
        int flag = 0;
        //strcpy(res,BACKUP_PATH);
        //重置种子

        for(i=0;i<len-1;i++){
            int tmp = rand() % 63;
            char single_char_tmp[2] = {0};
            strncpy(single_char_tmp,pst+tmp,1);
            strcat(res,single_char_tmp);
        }

        //strcat(res,".zip");
        for(j=0;j<list->size;j++){
            if(strcmp(list->data[j]->id,res) == 0)
                flag = 1;
        }

        if(flag == 0)
            break;

    }

    free(pst);
    return res;
}

void append_image(Image_list *list, Image *image){
    int i;
    if(list->max_size == list->size-1){
        list->max_size += 20;
        Image** data = (Image**)calloc(sizeof(Image*),list->max_size);
        for(i=0;i<list->size;i++)
            data[i] = list->data[i];
        list->data = data;
    }

    Image *tmp = (Image*)calloc(sizeof(Image),1);
    strcpy(tmp->id,image->id);
    strcpy(tmp->unzip_path,image->unzip_path);
    strcpy(tmp->describe,image->describe);
    strcpy(tmp->create_time,image->create_time);

    list->data[list->size] = tmp;
    list->size += 1;

}


int remove_image(Image_list *list, string id){
    int i;
    int flag = 0;
    for(i=0;i<list->size;i++){
        if(strcmp(list->data[i]->id,id) == 0){
            flag = 1;
            break;
        }
    }

    if(flag == 0)
        return -1;  //无此镜像

    string path = (string)calloc(sizeof(char),strlen(list->data[i]->id)+10+strlen(IMAGE_PATH));
    strcpy(path,IMAGE_PATH);
    strcat(path,list->data[i]->id);
    strcat(path,".zip");

    remove_file(path);

    if(is_file_found(path) == 0)
        return 0;   //镜像文件被占用

    Image** data = (Image**)calloc(sizeof(Image*),list->max_size);

    int j;
    list->size -= 1;
    for(j=0;j<list->size;j++){
        if(i < j)
            data[j] = list->data[j];
        else if(i>j)
            data[j] = list->data[j+1];
    }

    list->data = data;
    free(path);
    return 1;
}

/*
 * 通过镜像文件创建容器
 * 要搭配edit_pod_name一起使用
 * 注意！是硬还原，会将输入的path目录清空后导入容器！
 */
int create_container_image(Image_list *list, string image_id, string path){
    int i;
    int flag = 0;
    for(i=0;i<list->size;i++){
        if(strcmp(list->data[i]->id,image_id) == 0){
            flag = 1;
            break;
        }
    }

    if(flag == 0)
        return -1;  //无此镜像

    string image_path = (string)calloc(sizeof(char),strlen(list->data[i]->id)+10+strlen(IMAGE_PATH));
    strcpy(image_path,make_image_path(list->data[i]));


    if(is_file_found(image_path) == -1) {
        //镜像文件丢失
        remove_image(list,image_id);
        return 0;
    }

    if(is_file_read(image_path) == -1)
        return -2;  //镜像文件被占用

    //删除所选的path目录
    remove_dir(path);


    string tmp_path1 = list->data[i]->unzip_path+strlen(list->data[i]->unzip_path)-1;
    int count = 0;
    flag = 0;
    while(count<strlen(list->data[i]->unzip_path)){
        char tmp[2]={0};
        strncpy(tmp,tmp_path1-count,1);
        if(strcmp(tmp,"/") == 0 && count != 0){
            flag = 1;
            break;
        }
        count++;
    }

    string real_path = (string)calloc(sizeof(char),strlen(list->data[i]->unzip_path)+10);
    if(flag == 0){
        strcpy(real_path, list->data[i]->unzip_path); //纯相对路径，而且只有一层
    }else{
        strncpy(real_path,list->data[i]->unzip_path,strlen(list->data[i]->unzip_path)-count);
    }

    string tmp_path = (string)calloc(sizeof(char),strlen(real_path)+strlen(TMP_PATH)+10);
    strcpy(tmp_path,TMP_PATH);
    int a = is_the_slash_at_the_end_of_string(tmp_path);
    int b = is_the_slash_at_the_head_of_string(real_path);
    if(a == 1 && b == 1){
        strcat(tmp_path,list->data[i]->unzip_path+1);
    }else if(a == 0 && b == 0){
        strcat(tmp_path,"/");
        strcat(tmp_path,list->data[i]->unzip_path);
    }else{
        strcat(tmp_path,list->data[i]->unzip_path);
    }
    printf("tmp_path: %s\n",tmp_path);

    free(real_path);

    //拼接解压指令并且执行 unzip 镜像文件目录 -d 缓存目录
    string cmd_unzip = (string)calloc(sizeof(char),strlen("unzip  d ")+strlen(image_path)+strlen(TMP_PATH));
    strcpy(cmd_unzip,"unzip ");
    strcat(cmd_unzip, image_path);
    strcat(cmd_unzip," -d ");
    strcat(cmd_unzip,TMP_PATH);
    printf("cmd_unzip in create_container_image: %s\n",cmd_unzip);
    system(cmd_unzip);


    int res = copy_dir_full(tmp_path,path);
    if(res == -1)
        return -3;  //解压失败


    return 1;

}





void save_images_list(Image_list *list){
    FILE *file = fopen(IMAGE_DATA_PATH,"w");
    char split_flag[16] = ":/:splitflag:/:";    //设置分隔符
    int i;
    for(i=0;i<list->size;i++){
        string line = (string)calloc(sizeof(char), strlen(list->data[i]->id)+
                        strlen(list->data[i]->unzip_path)+strlen(list->data[i]->create_time)+
                        strlen(list->data[i]->describe)+strlen(split_flag)*3+15);
        strcpy(line,list->data[i]->id);
        strcat(line,split_flag);
        strcat(line,list->data[i]->unzip_path);
        strcat(line,split_flag);
        strcat(line,list->data[i]->create_time);
        strcat(line,split_flag);
        strcat(line,list->data[i]->describe);
        strcat(line,"\n");

        printf("line:%s",line);
        fputs(line,file);

        free(line);
    }
    fclose(file);
    return;
}

int read_images_list(Image_list *list) {
    int i = 0;
    int j = 0;
    char split_flag[16] = ":/:splitflag:/:";    //分隔符

    int x = access(IMAGE_DATA_PATH, F_OK);         //判断文件是否存在
    if (x == -1) {    //如果不存在，新建一个数据文件
        char new_file[60] = {0};
        strcpy(new_file, "touch ");
        strcat(new_file, DATA_PATH);
        system(new_file);
        return 0;
    }

    FILE *f = fopen(IMAGE_DATA_PATH, "rb");
    while (!feof(f) && !ferror(f)) {
        //申请内存
        char *line;
        line = (string) calloc(sizeof(char), 300);
        strcpy(line, "\n");
        //按行读取文件
        fgets(line, 300, f);
        //printf("line:%s\n",line);
        char id[LEN_ID] = {0};
        char unzip_path[260] = {0};
        char creat_time[50] = {0};
        char describe[100] ={0};

        char temp0[2] = {0};
        //判定是否读到了空行（结尾）
        strncpy(temp0, line, 1);
        if (strcmp(temp0, "\n\0") == 0 || j >= 300)
            return 0;

        strncpy(id, line, LEN_ID-1);
        line += LEN_ID -1 + 15;

        //printf("id: %s\tline: %s\n", id, line);

        //unzip_path
        while (1) {
            char tmp[16]={0};
            strncpy(tmp,line,15);
            if (strcmp(tmp,split_flag) == 0){
                line += 15;
                break;
            }

            //算法同上，按位读取判断并写入
            char temp[2] = {0};
            strncpy(temp, line, 1);
            if(strcmp(temp,"\n\0") == 0 || strcmp(temp,"\0") == 0)
                return -1;
            strcat(unzip_path, temp);
            line += 1;
        }

        //create_time
        while (1) {
            char tmp[16]={0};
            strncpy(tmp,line,15);
            if (strcmp(tmp,split_flag) == 0){
                line += 15;
                break;
            }

            //算法同上，按位读取判断并写入
            char temp[2] = {0};
            strncpy(temp, line, 1);
            if(strcmp(temp,"\n\0") == 0 || strcmp(temp,"\0") == 0)
                return -1;
            strcat(creat_time, temp);
            line += 1;
        }

        //describe
        while (1) {
            //算法同上，按位读取判断并写入
            char temp[2] = {0};
            strncpy(temp, line, 1);
            if(strcmp(temp,"\n\0") == 0 || strcmp(temp,"\0") == 0)
                break;
            strcat(describe, temp);
            line += 1;
        }

        printf("id:%s\tunzip_path:%s\tcreate_time:%s\tdescribe:%s\n",id,unzip_path,creat_time,describe);
        Image tmp;
        strcpy(tmp.id,id);
        strcpy(tmp.unzip_path,unzip_path);
        strcpy(tmp.create_time,creat_time);
        strcpy(tmp.describe,describe);
        append_image(list,&tmp);
    }
    return 1;
}

string list_images(Image_list *list){
    int i;
    int size = 0;
    for(i=0;i<list->size;i++){
        size += strlen(list->data[i]->id);
        size += strlen(list->data[i]->unzip_path);
        size += strlen(list->data[i]->create_time);
        size += strlen(list->data[i]->describe);
        size += 50;
    }

    string res = (string)calloc(sizeof(char),size);
    strcpy(res,"\0");
    strcat(res,"Show image list:\n");

    for(i=0;i<list->size;i++){
        printf("id:%s\tunzip_path:%s\tcreate_time:%s\tdescribe:%s\n",
        list->data[i]->id,list->data[i]->unzip_path,
                list->data[i]->create_time,list->data[i]->describe);
        size = 0;
        size += strlen(list->data[i]->id);
        size += strlen(list->data[i]->unzip_path);
        size += strlen(list->data[i]->create_time);
        size += strlen(list->data[i]->describe);
        size += 45;
        printf("%d\t",size);
        string tmp = (string)calloc(sizeof(char),size);
        strcpy(tmp,"\0");
        /*
        strcpy(tmp,"name:");
        strcat(tmp,list->data[i]->id);
        strcat(tmp,"\tunzip_path:");
        strcat(tmp,list->data[i]->unzip_path);
        strcat(tmp,"\tcreate_time:");
        strcat(tmp,list->data[i]->create_time);
        strcat(tmp,"\tdescribe:");
        strcat(tmp,list->data[i]->describe);
        strcat(tmp,"\n");
        */
        sprintf(tmp,"id:%s\tunzip_path:%s\tcreate_time:%s\tdescribe:%s\n",
               list->data[i]->id,list->data[i]->unzip_path,
               list->data[i]->create_time,list->data[i]->describe);
        printf("tmp:%s\n",tmp);
        strcat(res,tmp);

        free(tmp);
    }

    return res;
}

