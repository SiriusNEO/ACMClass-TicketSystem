//
// Created by SiriusNEO on 2021/5/18.
//

#ifndef CODE_BST_HPP
#define CODE_BST_HPP

namespace Sirius {
    template<class keyType>
    class BinarySearchTree { //key - > int
    private:
        struct Node {
            keyType key;
            int val;
            Node *leftSon, *rightSon;
            Node(const keyType& _key, int _val):key(_key), val(_val), leftSon(nullptr), rightSon(nullptr){}
            ~Node(){delete leftSon, delete rightSon;}
        };
        Node* root;
    public:
        BinarySearchTree():root(nullptr){}
        ~BinarySearchTree(){delete root;}
        void insert(const keyType& key, int val) {
            Node* newNode = new Node(key, val);
            if (root == nullptr) {
                root = newNode;
                return;
            }
            Node *now = root;
            while (true) {
                if (newNode->key < now->key) {
                    if (now->leftSon == nullptr) {now->leftSon = newNode;return;}
                    else now = now->leftSon;
                }
                else if (newNode->key > now->key) {
                    if (now->rightSon == nullptr) {now->rightSon = newNode;return;}
                    else now = now->rightSon;
                }
                else {
                    delete newNode;
                    now->val = val;
                    return;
                }
            }
        }

        int find(const keyType& key) {
            Node* now = root;
            while (now != nullptr) {
                if (key < now->key) now = now->leftSon;
                else if (key > now->key) now = now->rightSon;
                else return now->val;
            }
            return -1;
        }

        void del(const keyType& key) {
            Node* now = root;
            while (now != nullptr) {
                if (key < now->key) now = now->leftSon;
                else if (key > now->key) now = now->rightSon;
                else {now->val = -1; return;};
            }
        }
        void clear() {delete root, root = nullptr;}
    };
}
#endif //CODE_BST_HPP
