//
// Created by SiriusNEO on 2021/5/13.
//

#ifndef TICKETSYSTEM_2021_MAIN_BLOCKLIST_H
#define TICKETSYSTEM_2021_MAIN_BLOCKLIST_H


#include <fstream>
#include <vector>
#include <string>
#include <iostream>

namespace Sirius {
    template<class keyType, int BLOCK_SIZE> //推荐: 1024
    class BlockList {
    private:
        struct Node {
            keyType key;
            int offset;
            bool delTag;
            Node() = default;
            Node(keyType _key, int _offset, bool _delTag):key(_key), offset(_offset), delTag(_delTag) {}
            bool operator < (const Node& obj) const {
                return key < obj.key;
            }
            bool operator <= (const Node& obj) const {
                return key <= obj.key;
            }
        };

        class Block {
        public:
            int next, nodeNum;
            Node nodes[BLOCK_SIZE];

            Block() : next(-1), nodeNum(0) {}

            void Print() {
                std::cout << "[Block] " << next << " " << nodeNum << '\n';
                for (int i = 0; i < nodeNum; ++i) {
                    std::cout << nodes[i].key << " " << nodes[i].offset << " " << nodes[i].delTag << '\n';
                }
                std::cout << '\n';
            }
        };

        int first;
        std::fstream indexFile;

        template<class T>
        int Save(T x) {
            indexFile.seekg(0, std::ios::end);
            int savePos = indexFile.tellp();
            indexFile.write(reinterpret_cast<char *>(&x), sizeof(T));
            return savePos;
        }

        template<class T>
        void SaveIn(T x, int offset) {
            indexFile.seekp(offset, std::ios::beg);
            indexFile.write(reinterpret_cast<char *>(&x), sizeof(T));
        }

        template<class T>
        T Load(int offset) {
            T ret = T();
            indexFile.seekg(offset, std::ios::beg);
            indexFile.read(reinterpret_cast<char *>(&ret), sizeof(T));
            return ret;
        }

        void AddBlock(int rank, const Node &node) {
            Block tmp;
            tmp.nodes[0] = node;
            tmp.nodeNum = 1;
            int pointer = first;
            if (rank == 0 || first == -1) {
                tmp.next = first;
                first = Save(tmp);
                SaveIn(first, 0);
                return;
            }
            while (--rank) pointer = Load<int>(pointer);
            tmp.next = Load<int>(pointer);
            SaveIn(Save(tmp), pointer);
        }

        int FindBlock(int rank) {
            int pointer = first;
            while (--rank) pointer = Load<int>(pointer);
            return pointer;
        }

        void SplitBlock(int rank, int pos, const Node &node) {
            AddBlock(rank, node);
            int pointer = FindBlock(rank), nextPointer = Load<int>(pointer);
            Block tmp = Load<Block>(pointer);
            Block nextTmp = Load<Block>(nextPointer);
            int tmpNum = tmp.nodeNum;
            for (int i = pos; i < tmpNum; ++i) {
                nextTmp.nodes[i - pos + 1] = tmp.nodes[i];
                tmp.nodeNum--, nextTmp.nodeNum++;
            }
            SaveIn(tmp, pointer);
            SaveIn(nextTmp, nextPointer);
        }

        void MergeBlock(int rank) {
            int pointer = FindBlock(rank);
            Block tmp = Load<Block>(pointer), nextTmp = Load<Block>(Load<int>(pointer));
            int tmpNum = tmp.nodeNum, nextTmpNum = tmp.nodeNum + nextTmp.nodeNum;
            for (int i = tmpNum; i < nextTmpNum; ++i) {
                tmp.nodes[i] = nextTmp.nodes[i - tmpNum];
                tmp.nodeNum++, nextTmp.nodeNum--;
            }
            tmp.next = nextTmp.next;
            SaveIn(tmp, pointer);
        }

        void AddNode(Node node) {
            int pointer = first, rank = 1;
            if (pointer == -1) {
                AddBlock(0, node);
                return;
            } else if (node < Load<Node>(pointer + 2 * sizeof(int))) AddBlock(0, node);
            else {
                while (Load<int>(pointer) != -1 && Load<Node>(Load<int>(pointer) + 2 * sizeof(int)) <= node)
                    pointer = Load<int>(pointer), ++rank;
                Block tmp = Load<Block>(pointer);
                int i = 0;
                for (; i < tmp.nodeNum && tmp.nodes[i] <= node; ++i);
                SplitBlock(rank, i, node);
            }
            pointer = first, rank = 1;
            while (Load<int>(pointer) != -1) {
                if (Load<int>(pointer + sizeof(int)) + Load<int>(Load<int>(pointer) + sizeof(int)) <= BLOCK_SIZE) {
                    MergeBlock(rank);
                    break;     //try
                }
                pointer = Load<int>(pointer), ++rank;
            }
        }

    public:
        BlockList(const char *fileName) {
            indexFile.open(fileName, std::ios::ate | std::ios::in | std::ios::out | std::ios::binary);
            if (!indexFile) {
                std::ofstream outFile(fileName);
                outFile.close();
                indexFile.open(fileName, std::ios::ate | std::ios::in | std::ios::out | std::ios::binary);
                first = -1;
                indexFile.seekp(0, std::ios::beg);
                indexFile.write(reinterpret_cast<char *>(&first), sizeof(int));
            } else {
                indexFile.seekg(0, std::ios::beg);
                indexFile.read(reinterpret_cast<char *>(&first), sizeof(int));
            }
        }

        ~BlockList() {
            indexFile.seekp(0, std::ios::beg);
            indexFile.write(reinterpret_cast<char *>(&first), sizeof(int));
            indexFile.close();
        }

        void Print() {
            int pointer = first;
            Block tmp;
            while (pointer != -1) {
                tmp = Load<Block>(pointer);
                tmp.Print();
                pointer = tmp.next;
            }
        }

        void insert(const keyType &key, int offset) {
            Node tmp(key, offset, false);
            AddNode(tmp);
        }

        bool del(const keyType &key) {
            int pointer = first;
            if (pointer != -1) {
                while (Load<int>(pointer) != -1 && Load<Node>(Load<int>(pointer) + 2 * sizeof(int)).key < key)
                    pointer = Load<int>(pointer);
            }
            Block tmp;
            while (pointer != -1 && tmp.nodes[0].key <= key) {
                tmp = Load<Block>(pointer);
                for (int i = 0; i < tmp.nodeNum; ++i)
                    if (tmp.nodes[i].key == key && !tmp.nodes[i].delTag) {
                        tmp.nodes[i].delTag = true;
                        SaveIn(tmp, pointer);
                        return true;
                    }
                pointer = Load<int>(pointer);
            }
            return false;
        }

        std::pair<int, bool> find(const keyType &key) { //保证单点
            int pointer = first;
            if (pointer != -1) {
                while (Load<int>(pointer) != -1 && Load<Node>(Load<int>(pointer) + 2 * sizeof(int)).key < key)
                    pointer = Load<int>(pointer);
            }
            Block tmp;
            while (pointer != -1 && tmp.nodes[0].key <= key) {
                tmp = Load<Block>(pointer);
                for (int i = 0; i < tmp.nodeNum; ++i)
                    if (tmp.nodes[i].key == key && !tmp.nodes[i].delTag) return std::make_pair(tmp.nodes[i].offset, true);
                pointer = Load<int>(pointer);
            }
            return std::make_pair(-1, false);
        }

        std::vector<int> rangeFind(const keyType &key1, const keyType &key2) {
            std::vector<int> returnList;
            int pointer = first;
            if (pointer != -1) {
                while (Load<int>(pointer) != -1 && Load<Node>(Load<int>(pointer) + 2 * sizeof(int)).key < key1)
                    pointer = Load<int>(pointer);
            }
            Block tmp;
            while (pointer != -1 && tmp.nodes[0].key <= key2) {
                tmp = Load<Block>(pointer);
                for (int i = 0; i < tmp.nodeNum; ++i)
                    if (tmp.nodes[i].key >= key1 && tmp.nodes[i].key <= key2 && !tmp.nodes[i].delTag)
                        returnList.push_back(tmp.nodes[i].offset);
                pointer = Load<int>(pointer);
            }
            return returnList;
        }
    };
}


#endif //TICKETSYSTEM_2021_MAIN_BLOCKLIST_H
