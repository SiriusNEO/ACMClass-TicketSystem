//
// Created by SiriusNEO on 2021/4/29.
//

#ifndef TICKETSYSTEM_2021_MAIN_LIST_HPP
#define TICKETSYSTEM_2021_MAIN_LIST_HPP

namespace Sirius {
    template<class T>
    class LinkList {
    private:
        struct Node {
            T data;
            Node *nxt;
            Node():data(), nxt(nullptr) {}
            explicit Node(const T& _data):data(_data), nxt(nullptr){}
        };
        int siz;
        Node* head;
    public:
        LinkList():head(), siz(0){head = new Node();}
        ~LinkList(){while(siz)del(1);delete head;}
        void push_front(const T& _data) {
            Node *newNode = new Node(_data);
            newNode->nxt = head->nxt, head->nxt = newNode;
            siz++;
        }
        int insert(int pos, const T& _data) {
            Node *newNode = new Node(_data), *nowNode = head;
            int ret = pos+1;
            while (nowNode->nxt && pos--) nowNode = nowNode->nxt;
            newNode->nxt = nowNode->nxt, nowNode->nxt = newNode;
            siz++;
            return ret;
        }
        void del(int pos) {
            Node *nowNode = head;
            while (nowNode->nxt && --pos) nowNode = nowNode->nxt;
            Node *targetNode = nowNode->nxt;
            nowNode->nxt = nowNode->nxt->nxt;
            delete targetNode;
            siz--;
        }
        int find(const T& _data) {
            Node* nowNode = head->nxt;
            int ret = 0;
            while (nowNode) {
                ++ret;
                if (nowNode->data == _data) return ret;
                nowNode = nowNode->nxt;
            }
            return -1;
        }
        int size(){return siz;}
    };
}

#endif //TICKETSYSTEM_2021_MAIN_LIST_HPP
