//
// Created by kun
//

#ifndef BTREE_BPTREE_HPP
#define BTREE_BPTREE_HPP

#include <iostream>
#include <fstream>
#include <cstring>
#include "hash_map.h"

//long long

//debug
//cout<<
int bug_num=0;
int split_num=0;
int delete_num=0;

    template <class Key, class Value, class Compare = std::less<Key>>
    class Bptree{
    private:
        typedef std::pair<Key, int> key_offset;

    public:
       class basic_info;
       class Node;
        Compare cmp;
        using key_type=Key;
        using value_type=Value;
        using bpt_node_type=Node;

        class Diskmanager {
        private:
            class list;
            //using data_type=sjtu::B
            // using data_type = typename sjtu::Bptree<key_type, value_type, Compare>::Node;
            // using basicinfo= typename sjtu::bptree<key_type,value_type,Compare>::basic_info;
            using data_type=bpt_node_type;
            using map_back = typename std::pair<bool, typename list::node *>;

            FILE *f1= nullptr;
            FILE *f_value= nullptr;

            class recycle_pool{//动态数组不好存进外存诶
            public://1--base
                int free_off1[25001] = {0};
                int free_off2[25001] = {0};
                int free_num1 = 0;
                int free_num2 = 0;
                const  int capacity = 25000;

                recycle_pool() {
                    memset(&(free_off1),0,sizeof(free_off1));
                    memset(&(free_off2),0,sizeof(free_off2));
                }

                ~recycle_pool() = default;

                //如果超过上限，将浪费一定外存
                void push_back_node( int offset_) {
                    if (free_num1 < capacity ) {
                        free_off1[++free_num1] = offset_;
                    }
                }

                void push_back_value(int offset_) {
                    if (free_num2 < capacity )
                        free_off2[++free_num2] = offset_;
                }

                int pop_back_node() {
                    return free_off1[free_num1--];
                }

                int pop_back_value() {
                    return free_off2[free_num2--];
                }
            };

            class list {
            public:
                class node {
                    //这里面的data是指针，所以,一旦delete，就会将内存里这个点消灭，要注意！！
                public:
                    node *front_node = nullptr;
                    node *next_node = nullptr;
                    data_type *data = nullptr;
                    //这里面其实是data所在文件的offset
                    int node_offset = -1;

                    node(node *front_, node *next_, data_type *data_, int offset_) {
                        front_node = front_;
                         next_node = next_;
                        node_offset = offset_;
                        data = data_;
                    }

                    node(node *front_= nullptr, node *next_= nullptr) {
                        front_node = front_;
                        next_node = next_;
                    }

                    ~node() {
                        if (data != nullptr) {
                            delete data;
                        }
                        data= nullptr;
                    }

                };

                int capacity = 0;
                int data_num = 0;
                node *head_node = nullptr;
                node *tail_node = nullptr;
                Diskmanager *themanager= nullptr;

                void write_(int off_, data_type *data_) {
                    fseek(themanager->f1, off_, SEEK_SET);
                    fwrite(data_, sizeof(data_type), 1, themanager->f1);//???
                }

                void pop_back() {

                   // std::cout<<data_num<<'\n';

                    node *mid = tail_node->front_node;
                    write_(mid->node_offset, mid->data);
                    mid->front_node->next_node = tail_node;
                    tail_node->front_node = mid->front_node;
                    themanager->the_map->erase(mid->node_offset);
                    data_num--;///!!!!!!!!!!!!!!!!!!!!!!!!!!!可能错了//todo
                    delete mid;//这时顺便把data给delete掉了,所以，记得不要double delete
                }

                list() = delete;

                explicit list(int capacity_, Diskmanager *the_manager_):capacity(capacity_),themanager(the_manager_) {
                    head_node = new node(nullptr, nullptr);
                    tail_node = new node(nullptr, nullptr);
                    head_node->next_node = tail_node;
                    tail_node->front_node = head_node;
                }

                ~list() {
                    //需要打开文件吗？？？
                    //如果在delete list 之前关闭了文件，就得打开
                   //如果在关闭文件之前delete list 则不用可能也不能打开文件
                    node *mid = head_node;
                    node *mid2= nullptr;
                    while (mid != nullptr) {
                        if (mid->node_offset != -1) {
                            write_(mid->node_offset, mid->data);
                        }
                        mid2 = mid;
                        mid = mid->next_node;
                        delete mid2;
                    }
                }

                node *push_front(int off_, data_type *data_) {
                    //offset可能来自内存池
                    //std::cout<<data_num<<'\n';
                    node *now_node = new node(head_node, head_node->next_node, data_, off_);
                    head_node->next_node->front_node = now_node;
                    head_node->next_node = now_node;
                    ++data_num;
                    if (data_num == capacity)pop_back();
                    return now_node;
                }

                void updata_node(node *now_node) {
                    if (now_node == head_node->next_node)return;
                    now_node->front_node->next_node = now_node->next_node;
                    now_node->next_node->front_node = now_node->front_node;
                    now_node->next_node = head_node->next_node;
                    now_node->front_node = head_node;
                    head_node->next_node->front_node = now_node;
                    head_node->next_node = now_node;
                }

                //when erase do we need to write_ the delete_node?
                //应该不需要，erase之后，他的offset会被占用
                void erase(node *delete_node) {
                    delete_node->next_node->front_node = delete_node->front_node;
                    delete_node->front_node->next_node = delete_node->next_node;
                    data_num--;
                    themanager->recyclePool->push_back_node(delete_node->node_offset);
                    delete delete_node;
                }

                //用于换root
                void erase_2(node *delete_node) {
                    delete_node->next_node->front_node = delete_node->front_node;
                    delete_node->front_node->next_node = delete_node->next_node;
                    data_num--;
                    // themanager->recyclePool->push_back_node(delete_node->node_offset);
                    delete_node->data = nullptr;//防止把现在的root给delete掉
                    delete delete_node;
                }

                //需要write_back吗？
                void clear() {
                    node *mid1 = head_node;
                    node *mid2;
                    while (mid1 != nullptr) {
                        mid2 = mid1;
                        mid1 = mid1->next_node;
                        write_(mid2->node_offset, mid2->data);//????
                        delete mid2;
                    }

                    head_node = new node(nullptr, nullptr);
                    tail_node = new node(nullptr, nullptr);
                    tail_node->front_node = head_node;
                    head_node->next_node = tail_node;
                    data_num = 0;
                }
            };

        public:
            list* cache= nullptr;
            hash_map<int, typename list::node *>* the_map= nullptr;
            recycle_pool* recyclePool= nullptr;
            // Bptree

            Bptree<key_type, value_type, Compare> *the_tree = nullptr;

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
            //this_node_offset

            Diskmanager() = delete;

            Diskmanager(Bptree<key_type, value_type, Compare> *the_tree_, int capacity_,const char *filename1_,
                    const    char *filename2_):the_tree(the_tree_)  {
                cache=new list(capacity_,this);
                the_map=new hash_map<int,typename list::node*>(capacity_);
                recyclePool=new recycle_pool;
                //the_tree = the_tree_;
                the_tree->root = new data_type;
                f1 = fopen(filename1_, "rb+");
                f_value = fopen(filename2_, "rb+");
                //新建
                if (!f1) {
                    f1 = fopen(filename1_, "wb+");
                    f_value = fopen(filename2_, "wb+");
                    strcpy((the_tree_->basicInfo.file_name1), filename1_);
                    strcpy((the_tree_->basicInfo.file_name2), filename2_);
                    fseek(f1, 0, SEEK_SET);
                    fwrite(&(the_tree_->basicInfo), sizeof(the_tree_->basicInfo), 1, f1);
                    fseek(f1, 0, SEEK_END);
                    fwrite((recyclePool), sizeof(recycle_pool), 1, f1);
                    fseek(f1, 0, SEEK_END);
                    the_tree_->root->this_node_off = ftell(f1);
                    the_tree_->root->is_leaf = true;
                    the_tree_->basicInfo.root_offset = ftell(f1);
                   // the_tree_->basicInfo.head_leaf_offset = ftell(f1);
                    fwrite((the_tree->root), the_tree->node_size, 1, f1);
                    fseek(f1, 0, SEEK_SET);
                    fwrite(&(the_tree_->basicInfo), sizeof(the_tree_->basicInfo), 1, f1);
                    //todo
                } else {
                    fseek(f1, 0, SEEK_SET);
                    fread(&(the_tree_->basicInfo), sizeof(the_tree_->basicInfo), 1, f1);
                    fseek(f1, sizeof(the_tree_->basicInfo), SEEK_SET);
                    fread((recyclePool), sizeof(recycle_pool), 1, f1);
                    fseek(f1, (the_tree_->basicInfo.root_offset), SEEK_SET);
                    fread(the_tree_->root, the_tree_->node_size, 1, f1);
                }
            }

            //写入basic_info\recycle_pool\root\

            ~Diskmanager() {
                fseek(f1, (the_tree->basicInfo.root_offset), SEEK_SET);
                fwrite((the_tree->root), the_tree->node_size, 1, f1);

                delete the_tree->root;
                the_tree->root= nullptr;

                fseek(f1, 0, SEEK_SET);
                fwrite(&(the_tree->basicInfo), sizeof(the_tree->basicInfo), 1, f1);
                fseek(f1, sizeof(the_tree->basicInfo), SEEK_SET);
                fwrite((recyclePool), sizeof(recycle_pool), 1, f1);
                delete (cache);
                delete (the_map);
                delete recyclePool;
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

            void write_(int off_, data_type *data_){
                fseek(f1,off_,SEEK_SET);
                fwrite(data_,the_tree->node_size,1,f1);
            }


            int write_value(const value_type &value_) {
               // std::cout<<value_<<'\n';
                int off_;
                if (recyclePool->free_num2>0){
                    off_=recyclePool->pop_back_value();
                    fseek(f_value,off_,SEEK_SET);
                } else{
                fseek(f_value, 0, SEEK_END);
                 off_ =ftell(f_value);}
                fwrite(&(value_), the_tree->value_size, 1, f_value);
               // std::cout<<off_<<'\n';
                return off_;
            }

            //更新value
            void write_value(const value_type &value_, int offset_) {
                fseek(f_value, offset_, SEEK_SET);
                fwrite(&(value_), the_tree->value_size, 1, f_value);
            }

            //by Sirius
            template<class T>
            void write_info(const T& info_, int offset_) {
                fseek(f_value, offset_, SEEK_SET);
                fwrite(&(info_), sizeof(T), 1, f_value);
            }

            //用引用传递而不是return应该可以提高效率
            void read_value(int off_, value_type &value_) {
                fseek(f_value, off_, SEEK_SET);
                fread(&value_,the_tree->value_size,1,f_value);
               // fwrite(&(value_), the_tree->value_size, 1, f_value);
            }

            void erase_value(int off_) {
                //cache.erase()
                recyclePool->push_back_value(off_);
            }


            //还要写一个更新root的函数
            //offset在这个函数里面得到  //这个函数里面要更新缓存池 //这个应该是写进新节点
            int write_node( bpt_node_type &bpt_node_)
            {
                int off_;
                if (recyclePool->free_num1 > 0)
                {
                    off_=recyclePool->pop_back_node();
                    fseek(f1, off_, SEEK_SET);

                } else {
                    fseek(f1, 0, SEEK_END);
                    off_ = ftell(f1);
                }
                bpt_node_.this_node_off = off_;

             //   std::cout<<off_<<'\n';

                fwrite(&(bpt_node_), the_tree->node_size, 1, f1);
                typename list::node *list_node_ = cache->push_front(off_, &bpt_node_);
                the_map->insert(off_, list_node_);
                //
                return off_;
            }

            //把新的root写进外存
            //?????
            int write_node_root( bpt_node_type &bpt_node_) {
                int off_;
                if (recyclePool->free_num1 > 0) {
                  //  off_ = recyclePool.free_off1[recyclePool.free_num1--];
                  off_=recyclePool->pop_back_node();
                    fseek(f1, off_, SEEK_SET);
                   // fseek(f1, 0, SEEK_END);
                   // off_ = ftell(f1);
                } else {
                    fseek(f1, 0, SEEK_END);
                    off_ = ftell(f1);
                }
                bpt_node_.this_node_off = off_;
                fwrite(&(bpt_node_), the_tree->node_size, 1, f1);
                //typename  list::node* list_node_=cache.push_front(off_,bpt_node_);
                //the_map.insert(off_,list_node_);
                //
                return off_;
            }


            //更新节点，应该只用于更新root时，将原来的root写进缓存
            void write_node(int off_,  bpt_node_type &bpt_node_) {
                typename list::node *list_node_ = cache->push_front(off_, &bpt_node_);
                the_map->insert(off_, list_node_);
            }

            bpt_node_type *read_node(int off_)
            {
                map_back mapBack = the_map->find(off_);
                if (mapBack.first) {
                    typename list::node *list_node = mapBack.second;
                    cache->updata_node(list_node);
                    return list_node->data;
                }
                bpt_node_type *bptNode = new bpt_node_type;
                fseek(f1, off_, SEEK_SET);
                fread(bptNode, the_tree->node_size, 1, f1);
                typename list::node *list_node = cache->push_front(off_, bptNode);
                the_map->insert(off_, list_node);
                return bptNode;
            }

            //应该用到这个函数时，只会发生在合并节点或者删干净的情况
            //反正被删的点，一定出现在缓存中了  吧
            //不一定
            void erase_node(int off_)
            {
                typename list::node *list_node_;
                //if (the_map.find(off_).first) {
                list_node_ = the_map->find(off_).second;
                //  } else{
                //    list_node_
                //}
                the_map->erase(off_);
                cache->erase(list_node_);
            }

            //将新root从缓存池里面弄出来
            void erase_node_2(int off_) {
                typename list::node *list_node_ = the_map->find(off_).second;
                the_map->erase(off_);
                cache->erase_2(list_node_);
            }


            int tell_off_1() {
                if (recyclePool->free_num1 > 0) {
                    return recyclePool->free_off1[recyclePool->free_num1];
                } else {
                    fseek(f1, 0, SEEK_END);
                    return ftell(f1);
                }
            }

            //虽然我觉得没用
            int tell_off_2() {
                if (recyclePool->free_num2 > 0) {
                    return recyclePool->free_off2[recyclePool->free_num2];
                } else {
                    fseek(f_value, 0, SEEK_END);
                    return ftell(f_value);
                }
            }

            void clear() {
                if (the_tree->root!= nullptr) {
                    delete the_tree->root;
                }
                the_tree->root=new Node;
                cache->clear();
                the_map->clear();
                recyclePool->free_num1 = 0;
                recyclePool->free_num2 = 0;
                //delete the_tree->root;
                fclose(f1);
                fclose(f_value);
                f1 = fopen(the_tree->basicInfo.file_name1, "wb+");
                f_value = fopen(the_tree->basicInfo.file_name2, "wb+");
                // strcpy((the_tree_->basicInfo.file_name1),filename1_);
                //strcpy((the_tree_->basicInfo.file_name2),filename2_);
                the_tree->basicInfo.values_num=0;
                fseek(f1, 0, SEEK_SET);
                fwrite(&(the_tree->basicInfo), sizeof(the_tree->basicInfo), 1, f1);
                fseek(f1, 0, SEEK_END);
                fwrite((recyclePool), sizeof(recycle_pool), 1, f1);
                fseek(f1, 0, SEEK_END);
                the_tree->root->this_node_off = ftell(f1);
                the_tree->root->is_leaf = true;
                the_tree->basicInfo.root_offset = ftell(f1);
             //   the_tree->basicInfo.head_leaf_offset = ftell(f1);
                fwrite((the_tree->root), the_tree->node_size, 1, f1);
                fseek(f1, 0, SEEK_SET);
                fwrite(&(the_tree->basicInfo), sizeof(the_tree->basicInfo), 1, f1);
            }

        };
        static const int MAX_SIZ = 1700;
        static const int MIN_SIZ = MAX_SIZ / 2;
        typedef std::pair<Node *, int > node_index;
        static const int Node_size = sizeof(Node);

        Node *root = nullptr;
        Diskmanager* the_manager= nullptr;
        basic_info basicInfo;

    public:
        class basic_info{
        public:
            int root_offset=-1;
            //int head_leaf_offset=-1;
            int values_num=0;
            char file_name1[25]={0};
            char file_name2[25]={0};
            basic_info(){
                memset(file_name1,0,sizeof(file_name1));
                memset(file_name2,0,sizeof(file_name2));
            }
        };

        static const int basic_siz = sizeof(basic_info);

        class Node{
        public:
            key_offset little_node[MAX_SIZ + 1];
            std::pair<Node *, int> father;
            int this_node_off=0;
            int r_node_off=-1;
            bool is_leaf=false;
            int siz=0;

            Node(): this_node_off(0), is_leaf(false), siz(0){
                father.first = nullptr;
                father.second = 0;
                memset(little_node, 0, sizeof(little_node));
                r_node_off=-1;
            }

            ~Node()= default;
        };

        const static int node_size=sizeof(Node);
        const static int value_size=sizeof(Value);


        //v
        void search_to_leaf_node(const Key& key_,Node * & now_node)
        {

          //  std::cout<<" search_to_leaf_node"<<'\n';

            while (!now_node->is_leaf)
            {
                int now_node_siz_ = now_node->siz;
                int i;
                for ( i = 1; i <= now_node_siz_; i++)
                {
                    if(cmp(key_, now_node->little_node[i].first)){
                        Node *mid_node =the_manager->read_node(now_node->little_node[i-1].second);
                        mid_node->father.first = now_node;mid_node->father.second = i - 1;
                        now_node = mid_node;
                        break;
                    }
                }
                if (i==now_node_siz_+1){
                    Node *mid_node =the_manager->read_node(now_node->little_node[now_node_siz_].second);
                    mid_node->father.first = now_node;mid_node->father.second = now_node_siz_;
                    now_node = mid_node;
                }
            }
        }

        //v
        node_index search_node(const Key &key)
        {

           // std::cout<<" search_node"<<'\n';

            Node *now_node = root;
            node_index new_pos;
            search_to_leaf_node(key,now_node);
            for (int i = 1; i <= now_node->siz; ++i){
                if (!(cmp(now_node->little_node[i].first, key))&&!(cmp(key, now_node->little_node[i].first)))
                {
                    new_pos.first = now_node;
                    new_pos.second = i;
                    return new_pos;
                }
                if(cmp(key, now_node->little_node[i].first)){break;}
            }
            new_pos.first = nullptr;
            new_pos.second = 0;
            return new_pos;
        }

        //v
        node_index search_for_insert (const Key &key){

           // std::cout<<" search_for_insert"<<'\n';

            Node *now_node = root;
            node_index new_pos;
           search_to_leaf_node(key,now_node);
            int now_siz = now_node->siz;

            for (int i=1;i<=now_siz;i++)
            {
                if (!(cmp(key, now_node->little_node[i].first))&&!(cmp(now_node->little_node[i].first,key))){
                    new_pos.first = nullptr;
                    new_pos.second = 0;
                    return new_pos;
                }
                if (cmp(key, now_node->little_node[i].first))
                {
                    new_pos.first = now_node;
                    new_pos.second = i-1;
                    return new_pos;
                }
            }
            new_pos.first = now_node;
            new_pos.second = now_siz;
            return new_pos;

        }


        //v
        void modify_father_key( node_index &father, Key &key){

            //std::cout<<" modify_father_key"<<'\n';

           // if (father)
            if (father.second == 0 && father.first != root){
                modify_father_key(father.first->father, key);
                return;
            }
            father.first->little_node[father.second].first = key;
        }


        //v
        void split_leaf(Node *now_node){

            int new_offset;
           Node* new_node=new Node;
           new_offset=the_manager->write_node(*new_node);

            new_node->is_leaf = true;
            new_node->siz = now_node->siz - MIN_SIZ;
            now_node->siz = MIN_SIZ;
            for (int i = 1; i <= new_node->siz; ++i){
                new_node->little_node[i] = now_node->little_node[MIN_SIZ + i];
            }
            new_node->r_node_off=now_node->r_node_off;
            now_node->r_node_off=new_node->this_node_off;
            Node* father_node=now_node->father.first;int brother_index=now_node->father.second+1;
//            if (now_node!=root){
//            if (brother_index<=father_node->siz){
//                int bro_off=father_node->little_node[brother_index].second;
//                new_node->r_node_off=bro_off;}
//            }


            if (now_node != root){
                insert_inner(now_node->father, new_offset, now_node->little_node[MIN_SIZ + 1].first);
            } else {
                the_manager->write_node(root->this_node_off,*root);
                int  new_root_off;
                root = new Node;
                new_root_off=the_manager->write_node_root(*root);
                root->is_leaf = false;
                root->siz = 1;
                root->little_node[0].second = now_node->this_node_off;
                root->little_node[1].second = new_offset;
                root->little_node[1].first = now_node->little_node[MIN_SIZ + 1].first;
                basicInfo.root_offset = new_root_off;
            }
        }

        //v
        void split_inner(Node *now_node)
        {

           // std::cout<<" split_inner"<<'\n';

            int new_offset;Node *new_node=new Node ;
            new_offset=the_manager->write_node(*new_node);
            new_node->is_leaf = false;
            new_node->siz = now_node->siz - MIN_SIZ - 1;
            now_node->siz = MIN_SIZ;
            new_node->little_node[0].second = now_node->little_node[MIN_SIZ + 1].second;
            for (int i = 1; i <= new_node->siz; ++i){
                new_node->little_node[i] = now_node->little_node[MIN_SIZ + 1 + i];
            }
            new_node->r_node_off=now_node->r_node_off;
            now_node->r_node_off=new_node->this_node_off;

            if (now_node != root) {
                insert_inner(now_node->father, new_offset, now_node->little_node[MIN_SIZ + 1].first);
            } else {

                Node* mid=new Node;
                mid->little_node[0].second = now_node->this_node_off;
                mid->little_node[1].second = new_offset;
                mid->little_node[1].first = now_node->little_node[MIN_SIZ + 1].first;
                int new_root_offset;
                the_manager->write_(root->this_node_off,root);
                delete root;
                root=mid;
                //the_manager->write_node(root->this_node_off,*root);
                //root = new Node;
                new_root_offset=the_manager->write_node_root(*root);
            //    the_manager->erase_node_2(new_root_offset);
                root->is_leaf = false;
                root->siz = 1;
                basicInfo.root_offset = new_root_offset;
            }
        }

        //v
        void insert_inner( node_index father_node,int off_, Key key) {

            //std::cout<<"insert_inner"<<'\n';

            Node *now_node = father_node.first;
            now_node->siz++;
            int cur_pos = father_node.second + 1;
            for (int i = now_node->siz; i >= cur_pos+1; i--){
                now_node->little_node[i] = now_node->little_node[i - 1];
            }
            now_node->little_node[cur_pos].first = key;now_node->little_node[cur_pos].second = off_;
            if (now_node->siz == MAX_SIZ)
                split_inner(now_node);
        }

        //v
        void leaf_borrow_from_r(Node* now_node,Node* bro_node,Node* father_node,int now_pos){

            //std::cout<<"leaf_borrow_from_r"<<'\n';

            now_node->little_node[++now_node->siz] = bro_node->little_node[1];

            --bro_node->siz;
            int bro_siz = bro_node->siz;
            for(int i = 1; i <= bro_siz; ++i){
                bro_node->little_node[i].first = bro_node->little_node[i + 1].first;
                bro_node->little_node[i].second = bro_node->little_node[i + 1].second;
            }
            node_index parent;
            parent.first = father_node;
            parent.second = 1;
            modify_father_key(parent, bro_node->little_node[1].first);
        }

        void leaf_borrow_from_l(Node* now_node,Node* bro_node,Node* father_node,int now_pos)
        {

           // std::cout<<"leaf_borrow_from_l"<<'\n';

            now_node->siz++;
            for(int i = now_node->siz; i > 1; --i){
                now_node->little_node[i] = now_node->little_node[i - 1];
            }
            now_node->little_node[1] = bro_node->little_node[bro_node->siz--];
            modify_father_key(now_node->father, now_node->little_node[1].first);
        }

        //v
        void leaf_merge_r(Node* now_node,Node* bro_node,Node* father_node,int now_pos){
            //std::cout<<"leaf_merge_r"<<'\n';
            for(int i = 1; i <= bro_node->siz; ++i){
                now_node->little_node[now_node->siz + i] = bro_node->little_node[i];
            }
            now_node->siz += bro_node->siz;
           now_node->r_node_off=bro_node->r_node_off;
            the_manager->erase_node(bro_node->this_node_off);
            node_index parent;
            parent.first = father_node;
            parent.second = 1;
            if (father_node == root && root->siz == 1) {
                the_manager->write_node(root->this_node_off,*root);
                the_manager->erase_node(root->this_node_off);
                root = now_node;
                the_manager->erase_node_2(now_node->this_node_off);
                basicInfo.root_offset=now_node->this_node_off;
            } else {
                erase_mid(parent);
                return;
            }
        }

        void leaf_merge_l(Node* now_node,Node* bro_node,Node* father_node,int now_pos)
        {
            for(int i = 1; i <= now_node->siz; ++i){
                bro_node->little_node[bro_node->siz + i] = now_node->little_node[i];
            }
            bro_node->siz += now_node->siz;
            bro_node->r_node_off=now_node->r_node_off;
            the_manager->erase_node(now_node->this_node_off);
            node_index parent;
            parent.first = father_node;
            parent.second = now_pos;
            if (father_node == root && root->siz == 1) {
                the_manager->write_node(root->this_node_off,*root);
                the_manager->erase_node(root->this_node_off);
                root = bro_node;
                the_manager->erase_node_2(bro_node->this_node_off);
                basicInfo.root_offset = bro_node->this_node_off;
            } else {
                erase_mid(parent);
            }
        }

        //记得更新right
        void merge_leaf(Node *now_node){
            Node *father_node = now_node->father.first;
            int  now_pos = now_node->father.second;
            if (now_pos == 0)
            {
                int bro_pos = father_node->little_node[1].second;
                Node *bro_node = the_manager->read_node(bro_pos);
                if(bro_node->siz > MIN_SIZ){
                 leaf_borrow_from_r(now_node,bro_node,father_node,now_pos);
                 return;
                } else {
                  leaf_merge_r(now_node,bro_node,father_node,now_pos);
                  return;
                }
            } else{
                int bro_pos = father_node->little_node[now_pos - 1].second;
                Node *bro_node = the_manager->read_node(bro_pos);
                if(bro_node->siz > MIN_SIZ){
                  leaf_borrow_from_l(now_node,bro_node,father_node,now_pos);
                    return;
                } else {
                    leaf_merge_l(now_node,bro_node,father_node,now_pos);
                }
            }
        }

        void mid_borrow_from_r(Node* now_node,Node* bro_node,Node* father_node,int now_pos)
        {
            now_node->little_node[++now_node->siz].second = bro_node->little_node[0].second;
            now_node->little_node[now_node->siz].first = father_node->little_node[1].first;
            bro_node->little_node[0].second = bro_node->little_node[1].second;
            int bro_siz = --bro_node->siz;
            for(int i = 1; i <= bro_siz; ++i){
                bro_node->little_node[i] = bro_node->little_node[i + 1];
            }
            node_index parent;
            parent.first = father_node;parent.second = 0;
            modify_father_key(parent, bro_node->little_node[1].first);
        }

        void mid_merge_r(Node* now_node,Node* bro_node,Node* father_node,int now_pos){
            now_node->siz++;
            now_node->little_node[now_node->siz].second = bro_node->little_node[0].second;
            now_node->little_node[now_node->siz].first = father_node->little_node[1].first;
            for(int i = 1; i <= bro_node->siz; ++i){
                now_node->little_node[now_node->siz + i] = bro_node->little_node[i];
            }
            now_node->siz+=bro_node->siz;

            now_node->r_node_off=bro_node->r_node_off;
            the_manager->erase_node(bro_node->this_node_off);
            node_index parent;
            parent.first = father_node;
            parent.second = 1;
            if (father_node == root && root->siz == 1) {
                the_manager->write_node(root->this_node_off,*root);
                the_manager->erase_node(root->this_node_off);
                root = now_node;
                the_manager->erase_node_2(now_node->this_node_off);
                basicInfo.root_offset = now_node->this_node_off;
            } else {
                erase_mid(parent);
                return;
            }
        }

        void mid_borrow_from_l(Node* now_node,Node* bro_node,Node* father_node,int now_index)
        {
            for(int i = ++now_node->siz; i > 1; --i){
                now_node->little_node[i] = now_node->little_node[i - 1];
            }
            now_node->little_node[0].second = bro_node->little_node[bro_node->siz].second;
            now_node->little_node[1].first = father_node->little_node[now_index].first;
            modify_father_key(now_node->father, bro_node->little_node[bro_node->siz--].first);
        }

        void mid_merge_l(Node* now_node,Node* bro_node,Node* father_node,int now_index){
            bro_node->siz++;
            bro_node->little_node[bro_node->siz].second = now_node->little_node[0].second;
            bro_node->little_node[bro_node->siz].first = father_node->little_node[now_index].first;
            for(int i = 1; i <= now_node->siz; ++i){
                bro_node->little_node[bro_node->siz + i] = now_node->little_node[i];
            }
            bro_node->siz+=now_node->siz;
            bro_node->r_node_off=now_node->r_node_off;
            the_manager->erase_node(now_node->this_node_off);
            node_index parent;
            parent.first = father_node;
            parent.second = now_index;
            if (father_node == root && root->siz == 1) {
                the_manager->write_node(root->this_node_off,*root);
                the_manager->erase_node(root->this_node_off);
                root = bro_node;
                the_manager->erase_node_2(bro_node->this_node_off);
                basicInfo.root_offset = bro_node->this_node_off;
            } else{
                erase_mid(parent);
                return;
            }
        }

        void merge_mid(Node *now_node){
            Node *father_node = now_node->father.first;
            int now_index = now_node->father.second;
            if (now_index == 0){
                int  bro_pos = father_node->little_node[1].second;
                Node *bro_node = the_manager->read_node(bro_pos);
                if(bro_node->siz > MIN_SIZ)
                {
                   mid_borrow_from_r(now_node,bro_node,father_node,now_index);
                    return;
                } else {
                  mid_merge_r(now_node,bro_node,father_node,now_index);
                    return;
                }
            } else{
                int bro_pos = father_node->little_node[now_index - 1].second;
                Node *bro_node = the_manager->read_node(bro_pos);
                if(bro_node->siz > MIN_SIZ){
                  mid_borrow_from_l(now_node,bro_node,father_node,now_index);
                  return;
                } else {
                    mid_merge_l(now_node,bro_node,father_node,now_index);
                }
            }
        }

        void erase_mid( node_index &pos)
        {
            Node *now_node = pos.first;
            int  now_pos = pos.second;
            int sz = --now_node->siz;
            if(now_pos != 0){
                for(int i = now_pos; i <= sz; ++i){
                    now_node->little_node[i] = now_node->little_node[i + 1];
                }
            } else {
                //得更新father_key?
                //应该进不去这个函数部分
                now_node->little_node[0] = now_node->little_node[1];
                for(int i = 1; i <= sz; ++i){
                    now_node->little_node[i] = now_node->little_node[i + 1];
                }
                node_index father;
                father.first=now_node->father.first;
                father.second=0;
                modify_father_key(father,now_node->little_node[0].first);
            }
            if (now_node == root) {
                return;
            }
            if(sz < MIN_SIZ){
                merge_mid(now_node);
            }
        }



    public:

        //debug


        explicit Bptree(const char *file_name1 = "data1", const char *file_name2 = "data2") {
            the_manager=new Diskmanager(this,2000,file_name1,file_name2);
        }

        ~Bptree() {
            delete (the_manager);
        }

        int get_maxn(){
            return MAX_SIZ;
        }

        int get_min(){
            return MIN_SIZ;
        }

        // Clear the BTree
        void clear()
        {
            the_manager->clear();
        }

        int size() {
            return basicInfo.values_num;
        }

        bool insert(const Key &key, const Value &value)
        {
            //bug_num++;
            //std::cout<<bug_num<<'\n';

            if (basicInfo.values_num == 0){
                root->is_leaf = true;
                root->siz++;
                root->little_node[1].first = key;
                root->little_node[1].second=the_manager->write_value(value);
                basicInfo.values_num++;
                return true;
            }
            node_index now_node_off = search_for_insert(key);
            if (now_node_off.first == nullptr){
                return false;
            }
            basicInfo.values_num++;
            Node *now_node = now_node_off.first;
            int now_pos = now_node_off.second + 1;
            for (int i = ++now_node->siz; i > now_pos; i--){
                now_node->little_node[i] = now_node->little_node[i - 1];
            }
            now_node->little_node[now_pos].first = key;
            now_node->little_node[now_pos].second=the_manager->write_value(value);

            if (now_node->siz >= MAX_SIZ) {
                split_leaf(now_node);
               // split_num++;
                //std::cout<<"split"<<'\n';
            }
            return true;
        }

        bool modify(const Key &key, const Value &value) {
            node_index p = search_node(key);
            if (p.first != nullptr)
            {
                the_manager->write_value(value,p.first->little_node[p.second].second);
                return true;
            }
            return false;
        }

        template<class T>
        bool modify_info(const Key &key, const T& info, size_t offset) {
            node_index p = search_node(key);
            if (p.first == nullptr) return false;
            the_manager->write_info(info, p.first->little_node[p.second].second+offset);
            return true;
        }

        std::pair<Value, bool> find(const Key &key)
        {
            node_index parent = search_node(key);
            Value val;
            if (parent.first != nullptr){
                the_manager->read_value(parent.first->little_node[parent.second].second,val);
                return std::make_pair(val, true);
                //return pair<bool,Value>(true,val);
            }
            return std::make_pair(val, false);
           // return pair<bool,Value>(false,Value());
        }

        bool exist(const Key &key) {
            node_index parent = search_node(key);
            return parent.first != nullptr;
        }

        bool end(){
            return false;
        }

        bool erase(const Key &key) {
            if (basicInfo.values_num == 0) return false;
            node_index pos = search_node(key);
            if (pos.first == nullptr){
                return false;
            }
            --basicInfo.values_num;
            Node *cur_node = pos.first;
            int cur_pos = pos.second;
            int sz = --cur_node->siz;

           // basic->free_pos2[++basic->free_num2] = cur_node->info[cur_pos].second;
            the_manager->erase_value( cur_node->little_node[cur_pos].second);
            for(int i = cur_pos; i <= sz; ++i){
                cur_node->little_node[i] = cur_node->little_node[i + 1];
            }
            if(cur_node == root){
                return true;
            }
            if(cur_pos == 1) {
                modify_father_key(cur_node->father, cur_node->little_node[1].first);
            }
            if(sz < MIN_SIZ){
                merge_leaf(cur_node);
            }
            return true;
        }

        void range_find(const Key &key_low, const Key& key_high, value_type* ret, int& retCnt){
          //  Key low=key_low,high=key_high;

         // std::cout<<key_low<<'\n';

            node_index now_node_index=search_node(key_low);
            Node* now_node=now_node_index.first;  int index=now_node_index.second;
            if (now_node== nullptr){
                now_node=root;
                search_to_leaf_node(key_low,now_node);
                index=1;
            }

            //std::cout<<index<<'\n';
            retCnt = 0;
            while (now_node->little_node[index].first<=key_high)
            {
                if (now_node->little_node[index].first>=key_low)
                {
                    the_manager->read_value(now_node->little_node[index].second, *(ret+retCnt));
                    retCnt++;
                }
                if (index<now_node->siz)index++;
                else
                {
                    if (now_node->r_node_off==-1)return;
                    now_node=the_manager->read_node(now_node->r_node_off);
                    index=1;
                }
            }
        }

//        void debug_(){
//            for (int i=1;i<=10;i++) {
//                std::cout<<the_manager->write_value(i);
//            }
//        }

       // std::vector<key>
    };



#endif //BTREE_BPTREE_HPP


    //裂开
    //千古风流八咏楼，江山留与后人愁。水通南国三千里，气压江城十四州。
