//还在调试
//
// 还用不了，仍在调试
//
//64 76 112 121 163
#ifndef B_PLUS_TREE_DISK_MANAGER_H
#define B_PLUS_TREE_DISK_MANAGER_H
#include <cstring>
#include <string>
#include "hash_map.h"
#include "bpt_2.h"

//template<class data_type,class basicinfo>
template <class key_type,class value_type,class bpt_node_type,class Compare=std::less<key_type>>
class Diskmanager
{
private:

    using data_type=typename sjtu::bptree<key_type,value_type,Compare>::node;
    using basicinfo= typename sjtu::bptree<key_type,value_type,Compare>::basic_info;

     FILE * f1;
     FILE * f_value;

    class recycle_pool{//动态数组不好存进外存诶
    public://1--base
        long long  free_off1[50000]={0};
        long long  free_off2[50000]={0};
        int free_num1=0;
        int free_num2=0;
        int capacity=50000;
        recycle_pool()=default;
        ~recycle_pool()=default;

        /*void double_space()
        {
            int mid_num=capacity;
            long long mid[capacity+1];
            for (int i=1;i<=capacity;i++)mid[i]=free_off[i];
            delete [] free_off;
            capacity*=2;
            free_off=new long long [capacity+1];
            for (int i=1;i<=mid_num;i++)free_off[i]=mid[i];
        }*/
        //如果超过上限，将浪费一定外存
        void push_back_node(long long offset_)
        {
            if (free_num1<capacity-1)
            free_off1[++free_num1]=offset_;
        }

        void push_back_value(long long offset_)
        {
            if (free_num2<capacity-1)
                free_off2[++free_num2]=offset_;
        }

        long long pop_back_node()
        {
             return free_off1[free_num1--];
        }

        long long pop_back_value()
        {
            return free_off2[free_num2--];
        }
    };

    class list
    {
    public:
        class node{
            //这里面的data是指针，所以,一旦delete，就会将内存里这个点消灭，要注意！！
        public:
            node* front_node= nullptr;
            node* next_node= nullptr;
            data_type* data= nullptr;
            //这里面其实是data所在文件的offset
            long long node_offset=-1;
            node()=delete;
             node(node* front_,node* next_,data_type* data_,long long offset_){
                front_node=front_;next_node=next_;node_offset=offset_;
               // data=new data_type(data_);
               data=data_;
            }

            node(node* front_,node* next_){
                 front_node=front_;next_node=next_;
             }

             ~node(){
                 if (data!= nullptr) {
                     delete data;
                 }
             }

        };
        long long capacity=0;
        long long data_num=0;
        node* head_node= nullptr;
        node* tail_node= nullptr;
        Diskmanager* themanager;

        void write_(long long off_,data_type* data_)
        {
            fseek(themanager->f1,off_,SEEK_SET);
            fwrite(data_,sizeof(data_type),1,themanager->f1);
        }

        void pop_back(){
            node* mid=tail_node->front_node;
            write_(mid->node_offset,mid);
            mid->front_node->next_node=tail_node;
            tail_node->front_node=mid->front_node;
            themanager->the_map.erase(mid->node_offset);
            data_num--;///!!!!!!!!!!!!!!!!!!!!!!!!!!!可能错了//todo
            delete mid;//这时顺便把data给delete掉了,所以，记得不要double delete
        }

        list()=delete;
        explicit list(long long capacity_,Diskmanager* the_manager_){
            capacity=capacity_;themanager=the_manager_;
            head_node=new node(nullptr, nullptr);tail_node=new node(nullptr, nullptr);
            head_node->next_node=tail_node;
            tail_node->front_node=head_node;
        }

        ~list(){
            //需要打开文件吗？？？
            //如果在delete list 之前关闭了文件，就得打开
            //如果在关闭文件之前delete list 则不用可能也不能打开文件
            node* mid=head_node;node* mid2;
            while (mid!= nullptr)
            {
                if (mid->node_offset!=-1){
                    write_(mid->node_offset,mid);
                }
                mid2=mid;
                mid=mid->next_node;
                delete mid2;
            }
        }

        node* push_front(long long off_, data_type* data_)
        {
            //offset可能来自内存池
            node* now_node=new node(head_node,head_node->next_node,data_,off_);
            head_node->next_node->front_node=now_node;
            head_node->next_node=now_node;
            ++data_num;
            if (data_num==capacity)pop_back();
            return now_node;
        }

        void updata(node* now_node)
        {
            if (now_node==head_node->next_node)return;

            now_node->front_node->next_node=now_node->next_node;
            now_node->next_node->front_node=now_node->front_node;

            now_node->next_node=head_node->next_node;
            now_node->front_node=head_node;
            head_node->next_node->front_node=now_node;
            head_node->next_node=now_node;
        }

        //when erase do we need to write_ the delete_node?
        //应该不需要，erase之后，他的offset会被占用
        void erase(node* delete_node){
            delete_node->next_node->front_node=delete_node->front_node;
            delete_node->front_node->next_node=delete_node->next_node;
            data_num--;
            themanager->recyclePool.push_back_node(delete_node->node_offset);
            delete delete_node;
        }

        //需要write_back吗？
        void clear()
        {
            node* mid1=head_node;node* mid2;
            while (mid1!= nullptr)
            {
                mid2=mid1;
                mid1=mid1->next_node;
                write_(mid2->node_offset,mid2);//????
                delete mid2;
            }

            head_node=new node(nullptr, nullptr);
            tail_node=new node(nullptr, nullptr);
            tail_node->front_node=head_node;
            head_node->next_node=tail_node;
            data_num=0;
        }

    };

public:
    list cache;
    hash_map<long long,typename list:: node*>the_map;
    recycle_pool recyclePool;
    sjtu::bptree<key_type,value_type,Compare>* the_tree= nullptr;

    /*
     *   class basic_info{
            long long root_offset=-1;
            long long head_leaf_offset=-1;
            int values_num=0;
            char file_name1[25]{};
            char file_name2[25]{};
            basic_info(){
                memset(file_name1,0,sizeof(file_name1));
                memset(file_name2,0,sizeof(file_name2));
            }
        };*/

    Diskmanager()=delete;

    Diskmanager(sjtu::bptree<key_type,value_type,Compare>*the_tree_,long long capacity_,char* filename1_,char* filename2_){
        the_tree=the_tree_;
        f1=fopen(filename1_,"rb+");
        f_value=fopen(filename2_,"rb+");
        //新建
        if (!f1){
            f1=fopen(filename1_,"wb+");
            f_value=fopen(filename2_,"wb+");
            strcpy((the_tree_->basicInfo.file_name1),filename1_);
            strcpy((the_tree_->basicInfo.file_name2),filename2_);
            fseek(f1,0,SEEK_SET);
            fwrite(&(the_tree_->basicInfo),sizeof(the_tree_->basicInfo),1,f1);
            fseek(f1,0,SEEK_END);
            fwrite(&(recyclePool),sizeof(recyclePool),1,f1);
            fseek(f1,0,SEEK_END);
            the_tree_->root->this_node_offset=ftell(f1);
            the_tree_->root->is_leaf=true;
            the_tree_->basicInfo.root_offset=ftell(f1);
            the_tree_->basicInfo.head_leaf_offset=ftell(f1);
            fwrite((the_tree->root),the_tree->node_size,1,f1);
            fseek(f1,0,SEEK_SET);
            fwrite(&(the_tree_->basicInfo),sizeof(the_tree_->basicInfo),1,f1);
            //todo
        } else
        {
            fseek(f1,0,SEEK_SET);
            fread(&(the_tree_->basicInfo),sizeof(the_tree_->basicInfo),1,f1);
            fseek(f1,sizeof(the_tree_->basicInfo),SEEK_SET);
            fread(&(recyclePool),sizeof(recycle_pool),1,f1);
            fseek(f1,(the_tree_->basicInfo.root_offset),SEEK_SET);
            fread(the_tree_->root,the_tree_->node_size,1,f1);
        }
    }

    //写入basic_info\recycle_pool\root\

   ~Diskmanager(){
        fseek(f1,(the_tree->basicInfo.root_offset),SEEK_SET);
        fwrite((the_tree->root),the_tree->node_size,1,f1);
        fseek(f1,0,SEEK_SET);
        fwrite(&(the_tree->basicInfo),sizeof(the_tree->basicInfo),1,f1);
        fseek(f1,sizeof(the_tree->basicInfo),SEEK_SET);
        fwrite(&(recyclePool),sizeof(recyclePool),1,f1);
        delete cache;
        delete the_map;
        fclose(f1);
        fclose(f_value);
       // fseek()
    }

    //我得提供点什么函数呢？
    //write()//将node或者value写进外存
    //read()//将node或者value读进内存
    //ri了老狗，B你马的加树

    //或许，value也应该设置一个缓存池
    //将新value写进外存
    long long write_value(const value_type& value_){
        long long off_=fseek(f_value,0,SEEK_END);
        fwrite(&(value_),the_tree->value_size,1,f_value);
        return off_;
    }
    //更新value
    void write_value(const value_type& value_,long long offset_)
    {
        fseek(f_value,offset_,SEEK_SET);
        fwrite(&(value_),the_tree->value_size,1,f_value);
    }

    //用引用传递而不是return应该可以提高效率
    void read_value(long long off_,value_type& value_){
        fseek(f_value,off_,SEEK_SET);
        fwrite(&(value_),the_tree->value_size,1,f_value);
    }

    //还要写一个更新root的函数

    //offset在这个函数里面得到  //这个函数里面要更新缓存池 //这个应该是写进新节点
    long long write_node(const bpt_node_type& bpt_node_){
        long long off_;
        if (recyclePool.free_num1>0){
            off_=recyclePool.free_off1[recyclePool.free_num1--];
            fseek(f1,off_,SEEK_SET);
        } else
        {
            fseek(f1,0,SEEK_END);
            off_=ftell(f1);
        }
        bpt_node_.this_node_offset=off_;
        fwrite(&(bpt_node_),the_tree->node_size,1,f1);
       typename  list::node* list_node_=cache.push_front(off_,bpt_node_);
       the_map.insert(off_,list_node_);
        //
        return off_;
    }
    //更新节点，应该只用于更新root时，将原来的root写进缓存
    void write_node(long long off_,const bpt_node_type& bpt_node_)
    {
        typename  list::node* list_node_=cache.push_front(off_,bpt_node_);
        the_map.insert(off_,list_node_);
    }
    
    long long tell_off_1(){
        if (recyclePool.free_num1>0){
            return recyclePool.free_off1[recyclePool.free_num1--];
        } else
        {
            fseek(f1,0,SEEK_END);
            return ftell(f1);
        }
    }

};

#endif //B_PLUS_TREE_DISK_MANAGER_H
