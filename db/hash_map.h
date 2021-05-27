//
// Created by Lenovo on 2021/5/12.
//

#ifndef B_PLUS_TREE_HASH_MAP_H
#define B_PLUS_TREE_HASH_MAP_H
#include<functional>

template<class key_type,class value_type,class my_hash=std::hash<key_type>>
class hash_map{
private:
    class node {
    public:
        key_type key;
        value_type value;
        node* next= nullptr;
        node()=delete;
        node(key_type key_,value_type value_,node* next_= nullptr):next(next_),key(key_),value(value_){
        }
        ~node()=default;
    };

    class link_list{
    public:
        long long node_num=0;
        node* head= nullptr;
        link_list()=default;
        ~link_list(){
            node* mid;
            for(int i=1;i<=node_num;i++)
            {
                mid=head;
                head=head->next;
                delete mid;
            }
            head= nullptr;
            node_num=0;
        }

        void insert_node(const key_type& key_,const value_type& value_)
        {
            node_num++;
            head=new node(key_,value_,head);
        }

        node* find_node(const key_type& key_)
        {
            if (node_num!=0){
                node* mid=head;
                while (mid!= nullptr)
                {
                    if (mid->key==key_)return mid;
                    mid=mid->next;
                }
            }
            return nullptr;
        }

        void erase_node(const key_type& key_)
        {
            if (node_num==0)return;

            if (head->key==key_){
                node_num--;
                node* mid=head;
                head=head->next;
                delete mid;
                return;
            }

            node* front_node= head;node* now_node=head->next;
            while (now_node!= nullptr)
            {
                if (now_node->key==key_)
                {
                    front_node->next=now_node->next;
                    node_num--;
                    delete now_node;
                }
                front_node=now_node;
                now_node=now_node->next;
            }
        }


    };

    long long contain_num=0;
    link_list* the_list= nullptr;
    my_hash myHash;
public:
    long long get_index(const key_type& key){
        return myHash(key)%contain_num;
    }

    hash_map()=delete;

    explicit hash_map(long long contain_num_){
        contain_num=contain_num_;
        the_list=new link_list[contain_num];
    }

    ~hash_map(){
        delete [] the_list;
    }

     std::pair<bool,value_type> find(const key_type& key_){
        long long index=get_index(key_);
        node* the_node=the_list[index].find_node(key_);
         if (the_node== nullptr)return std::make_pair(false,value_type());
         return std::make_pair(true,the_node->value);
    }

    void insert(const key_type& key_,const value_type& value_){
        long long index=get_index(key_);
        the_list[index].insert_node(key_,value_);
    }

    void erase(const key_type& key_){
        long long index=get_index(key_);
        the_list[index].erase_node(key_);
    }

    void clear(){
        delete[] the_list;
        the_list=new link_list[contain_num];
    }

};


#endif //B_PLUS_TREE_HASH_MAP_H
