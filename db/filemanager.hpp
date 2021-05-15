//
// Created by SiriusNEO on 2021/5/13.
//

#ifndef TICKETSYSTEM_2021_MAIN_FILEMANAGER_HPP
#define TICKETSYSTEM_2021_MAIN_FILEMANAGER_HPP

#include "blocklist.h"

namespace Sirius {
    template <class keyType, class valueType> //均为定长类型
    class FileManager {
        public:
            int siz;
            BlockList<keyType, 1024> dataStructure;
            std::fstream dataFile;
        public:
            FileManager(const char* dataFileName) : dataStructure(("i"+std::string(dataFileName)).c_str()) {
                dataFile.open(dataFileName, std::ios::ate | std::ios::in | std::ios::out | std::ios::binary);
                if (!dataFile) {
                    std::ofstream outFile(dataFileName);
                    outFile.close();
                    siz = 0;
                    dataFile.open(dataFileName, std::ios::ate | std::ios::in | std::ios::out | std::ios::binary);
                    dataFile.write(reinterpret_cast<char *>(&siz), sizeof(int));
                }
                else {
                    dataFile.seekg(0, std::ios::beg);
                    dataFile.read(reinterpret_cast<char *>(&siz), sizeof(int));
                }
            }
            ~FileManager() {
                dataFile.seekp(0, std::ios::beg);
                dataFile.write(reinterpret_cast<char *>(&siz), sizeof(int));
                dataFile.close();
            }

            int size() const {return siz;}

            void insert(const keyType& key, valueType val) {
                ++siz;
                dataFile.seekp(0, std::ios::end);
                int offset = dataFile.tellp();
                dataFile.write(reinterpret_cast<char *>(&val), sizeof(valueType));
                dataStructure.insert(key, offset);
            }

            bool del(const keyType& key) { //单点删除
                --siz;
                return dataStructure.del(key);
            }

            bool modify(const keyType& key, valueType val) { //单点修改，返回是否修改成功
                auto index = dataStructure.find(key);
                if (!index.second) return false;
                dataFile.seekp(index.first, std::ios::beg);
                dataFile.write(reinterpret_cast<char *>(&val), sizeof(valueType));
                return true;
            }

            std::pair<valueType, bool> find(const keyType& key) {
                auto index = dataStructure.find(key);
                valueType tmp;
                if (!index.second) return std::make_pair(tmp, false);
                dataFile.seekg(index.first, std::ios::beg);
                dataFile.read(reinterpret_cast<char *>(&tmp), sizeof(valueType));
                return std::make_pair(tmp, true);
            }

            std::vector<valueType> rangeFind(const keyType& key1, const keyType& key2) {
                auto index = dataStructure.rangeFind(key1, key2);
                std::vector<valueType> ret;
                for (auto i : index) {
                    valueType tmp;
                    dataFile.seekg(i, std::ios::beg);
                    dataFile.read(reinterpret_cast<char *>(&tmp), sizeof(valueType));
                    ret.push_back(tmp);
                }
                return ret;
            }
    };
}

#endif //TICKETSYSTEM_2021_MAIN_FILEMANAGER_HPP
