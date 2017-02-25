#ifndef __SKIPLIST_H
#define __SKIPLIST_H
#include <functional>
#include <random>
#include <time.h>
#include "node.h"
#include "iterator.h"
#include <string>
#include <fstream>
#include <iostream>
#include <map>


/**
 * Skiplist interface
 */


template<class Key, class Value, size_t MAXHEIGHT, class Compare = std::less<Key>>
class SkipList {
private:
  DataNode<Key, Value>* pHead;
  DataNode<Key, Value>* pTail;
  
  IndexNode<Key, Value>* pTailIdx;
  IndexNode<Key, Value>* aHeadIdx[MAXHEIGHT];
  static const Compare& keyLess;

public:
  /**
   * Creates new empty skiplist
   */
  SkipList()
  {
      pHead   = new DataNode<Key, Value>(nullptr, nullptr);
      pTail   = new DataNode<Key, Value>(nullptr, nullptr);
      pHead->next(pTail);

      Node<Key, Value> *prev = pHead;
      pTailIdx = new IndexNode<Key, Value>(pTail, pTail);
      for (int i=0; i < MAXHEIGHT; i++) 
      {
          aHeadIdx[i] = new IndexNode<Key, Value>(prev, pHead);
          aHeadIdx[i]->next(pTailIdx);
          prev = aHeadIdx[i];
      }
  }

  /**
   * Disable copy constructor
   */
  SkipList(const SkipList& that) = delete;

  /*
   * Dump skip list to file in .dot format
   */
  void dump(std::string file_name)
  {
      std::ofstream fout;
      fout.open(file_name);
      fout << "digraph G {" << std::endl;
      std::map<Node<Key, Value>*, std::string> names;

      // data layer
      fout << "subgraph cluster_0 { style = filled; color = lightgrey;"\
           << "node[style=filled, color=white];"\
           << "label = \"Data Layer\";"
           << std::endl;
      names[pHead] = "pHead";


      DataNode<Key, Value>* dataNode = static_cast<DataNode<Key, Value>*>(&pHead->next());
      DataNode<Key, Value>* nextNode = nullptr;

      int nodeNum = 0;
      if (dataNode != pTail)
      {
          names[dataNode] = "dataNode_" + std::to_string(++nodeNum);
      }
      else
      {
          names[dataNode] = "pTail";
      }

      fout << names[pHead]\
          << "[ label = \""\
          << "<name>" << names[pHead]\
          << "| <next> next"\
          << "| <key>" << "nullptr"\
          << "| <value>" << "nullptr \""\
          << "shape = record]" << std::endl;

      fout << names[pHead] << ":next -> " << names[dataNode]\
          << ":name" << std::endl;


      while(dataNode != pTail)
      {
          nextNode = static_cast<DataNode<Key, Value>*>(&dataNode->next());
          if (nextNode != pTail)
          {
              names[nextNode] = "dataNode_" + std::to_string(++nodeNum);
          }
          else
          {
              names[nextNode] = "pTail";
          }
          fout << names[dataNode]\
              << "[ label = \""\
              << "<name>" << names[dataNode]\
              << "| <next> next"\
              << "| <key>" << dataNode->key() \
              << "| <value>" << dataNode->value()\
              << "\" shape = record]" << std::endl;
          fout << names[dataNode] << ":next -> "\
              << names[nextNode] << ":name" << std::endl;
          dataNode = nextNode;
      }
        
      names[pTail] = "pTail";
      fout << names[pTail]\
           << "[ label = \""\
           << "<name>" << names[pTail]\
           << "| <next> next"  \
           << "| <key> nullptr"\
           << "| <value> nullptr \""\
           << "shape = record]"
           << std::endl;

      fout << "}" << std::endl;

      IndexNode<Key, Value>* indexNode; 
      IndexNode<Key, Value>* nextIndexNode;
      names[pTailIdx] = "pTailIdx";

      for(int level = 0; level < MAXHEIGHT; ++level)
      {
          fout << "subgraph cluster_" << (level + 1) << " { style = filled; color = lightgrey;"\
              << "node[style=filled, color=white];"\
              << "label = \"Index Layer " << level <<"\";"\
              << std::endl;
          nodeNum = 0;
          indexNode = aHeadIdx[level];
          names[indexNode] = "aHeadIdx" + std::to_string(level);

          while(indexNode != pTailIdx)
          {
              nextIndexNode = static_cast<IndexNode<Key,\
                            Value>*>(&indexNode->next());
              if (nextIndexNode != pTailIdx)
              {
                  names[nextIndexNode] = "IndexNode_" + std::to_string(level) + 
                      "_" + std::to_string(++nodeNum);
              }

              fout << names[indexNode]\
                  << "[ label = \""\
                  << "<name>" << names[indexNode]\
                  << "| <next> next"\
                  << "| <down> down"\
                  << "| <root> root"\
                  << "\"shape = record]"
                  << std::endl;
              fout << names[indexNode] << ":next -> "\
                << names[nextIndexNode] << ":name" << std::endl;
              fout << names[indexNode] << ":down -> "\
                  << names[&indexNode->down()] << ":name" << std::endl;
              fout << names[indexNode] << ":root -> "\
                  << names[&indexNode->root()] << ":name" << std::endl;
              indexNode = nextIndexNode;
          }
          fout << "}" << std::endl;
      }


      fout << names[pTailIdx]\
                  << "[ label = \""\
                  << "<name>" << names[pTailIdx]\
                  << "| <next> next"\
                  << "| <down> down"\
                  << "| <root> root"\
                  << "\"shape = record]"
                  << std::endl;

      fout << "}" << std::endl;
      fout.close();
  }
  /**
   * Destructor
   */
  virtual ~SkipList() {
      // delete index layers
      Node<Key, Value>* curNode;
      Node<Key, Value>* nextNode;
      for (int i=0; i < MAXHEIGHT; i++) {
          curNode = aHeadIdx[i];
          while(curNode != pTailIdx)
          {
              nextNode = &curNode->next();
              delete curNode;
              curNode = nextNode;
          }
      }
      delete pTailIdx;


      curNode = pHead;
      while(curNode != pTail)
      {
          nextNode = &curNode->next();
          delete curNode;
          curNode = nextNode;
      }

      delete pTail;
  }

  /**
   * Assign new value for the key. If a such key already has
   * assosiation then old value returns, otherwise nullptri
   *
   * @param key key to be assigned with value
   * @param value to be added
   * @return old value for the given key or nullptr
   */

    virtual Value* Put(const Key& key, const Value& value) const 
    {
        IndexNode<Key, Value>* update_list[MAXHEIGHT];
        DataNode<Key, Value>* curDataNode = getBeforeEqual(key, update_list);
        DataNode<Key, Value>* next = static_cast<DataNode<Key, Value>*>(&curDataNode->next());

        if (next != pTail &&  keysEqual(next->key(), key))
        {
            Value* prev_value = const_cast<Value*>(&next->value());
            next->value(&value);
            return prev_value;
        }

        DataNode<Key, Value>* dataNode = new DataNode<Key, Value>(&key, &value);
        InsertToList(dataNode, curDataNode, update_list);


        return nullptr;
  };


  /**
   * Put value only if there is no assosiation with key in
   * the list and returns nullptr
   *
   * If there is an established assosiation with the key already
   * method doesn't nothing and returns existing value
   *
   * @param key key to be assigned with value
   * @param value to be added
   * @return existing value for the given key or nullptr
   */
  virtual Value* PutIfAbsent(const Key& key, const Value& value) {
    IndexNode<Key, Value>* update_list[MAXHEIGHT];
    DataNode<Key, Value>* curDataNode = getBeforeEqual(key, update_list);
    DataNode<Key, Value>* next = static_cast<DataNode<Key, Value>*>(&curDataNode->next());

    if (next != pTail && keysEqual(next->key(),key))
    {
        return const_cast<Value*>(&next->value());
    }

    DataNode<Key, Value>* dataNode = new DataNode<Key, Value>(&key, &value);
    InsertToList(dataNode, curDataNode, update_list);
    return nullptr;
  };

  /**
   * Returns value assigned for the given key or nullptr
   * if there is no established assosiation with the given key
   *
   * @param key to find
   * @return value assosiated with given key or nullptr
   */
  virtual Value* Get(const Key& key) const {
    IndexNode<Key, Value>* update_list[MAXHEIGHT];
    DataNode<Key, Value>* curDataNode = getBeforeEqual(key, update_list);
    DataNode<Key, Value>* next = static_cast<DataNode<Key, Value>*>(&curDataNode->next());

    if (next != pTail && keysEqual(next->key(), key))
    {
        return const_cast<Value*>(&next->value());
    }
    return nullptr;
  };

  /**
   * Remove given key from the skpiplist and returns value
   * it has or nullptr in case if key wasn't assosiated with
   * any value
   *
   * @param key to be added
   * @return value for the removed key or nullptr
   */
  virtual Value* Delete(const Key& key) {
    IndexNode<Key, Value>* update_list[MAXHEIGHT];
    DataNode<Key, Value>* curDataNode = getBeforeEqual(key, update_list);
    DataNode<Key, Value>* deleteNode = static_cast<DataNode<Key, Value>*>(&curDataNode->next());
    // key doesn't exist
    if (deleteNode == pTail || (! keysEqual(deleteNode->key(), key)))
    {
        return nullptr;
    }


    Node<Key, Value>* next;
    for(int i = 0; i < MAXHEIGHT; ++i)
    {
        next = &update_list[i]->next();
        if (next != pTailIdx && keysEqual(next->key(), key))
        {
            update_list[i]->next(static_cast<IndexNode<Key, Value>*>(&next->next()));
            delete next;
        }
        else
        {
            break;
        }
    }


    Value* value = const_cast<Value*>(&deleteNode->value());
    curDataNode->next(static_cast<DataNode<Key, Value>*>(&deleteNode->next()));
    delete deleteNode;

    return value;
  };

  /**
   * Same as Get
   */
  virtual const Value* operator[](const Key& key) {
      return Get(key);
  };

  /**
   * Return iterator onto very first key in the skiplist
       */
  virtual Iterator<Key, Value> cbegin() const {
    return Iterator<Key,Value>(static_cast<Node<Key, Value>*>(&pHead->next()));
  };

  /**
   * Returns iterator to the first key that is greater or equals to
   * the given key
   */
  virtual Iterator<Key, Value> cfind(const Key &min) const {
    IndexNode<Key, Value>* update_list[MAXHEIGHT];
    DataNode<Key, Value>* curDataNode = getBeforeEqual(min, update_list);
    return Iterator<Key,Value>(&curDataNode->next());
    
  };

  /**
   * Returns iterator on the skiplist tail
   */
  virtual Iterator<Key, Value> cend() const {
    return Iterator<Key,Value>(pTail);
  };
private:
    int randLevel() const 
    {
        // xorshift+ 
        static uint64_t x = time(NULL);
        x ^= x >> 12;
        x ^= x << 25;
        x ^= x >> 27;
        size_t y =  x * 0x2545F4914F6CDD1D;
    
        // find number of trailing zeros 64-bit
        // http://stackoverflow.com/questions/757059/position-of-least-significant-bit-that-is-set
        int r;            // result goes here
        static const char MultiplyDeBruijnBitPosition[64] = 
        {
            0, 1, 48, 2, 57, 49, 28, 3, 61, 58, 50, 42, 38, 29, 17, 4,
            62, 55, 59, 36, 53, 51, 43, 22, 45, 39, 33, 30, 24, 18, 12, 5,
            63, 47, 56, 27, 60, 41, 37, 16, 54, 35, 52, 21, 44, 32, 23, 11,
            46, 26, 40, 15, 34, 20, 31, 10, 25, 14, 19, 9, 13, 8, 7, 6
        };
        r = MultiplyDeBruijnBitPosition[((uint64_t)((y & -y) * 0x03F79D71B4CB0A89U)) >> 58];
        return r < MAXHEIGHT ? r : MAXHEIGHT;
    }

    virtual void InsertToList(DataNode<Key, Value>* dataNode,\
                DataNode<Key, Value>* prevDataNode,\
                IndexNode<Key, Value>* update_list[MAXHEIGHT]) const
    {
        InsertToDataLayer(dataNode, prevDataNode);

        int node_level = randLevel();
        Node<Key, Value>* prev = dataNode;
        IndexNode<Key, Value>* index_node;

        for(int i = 0; i < node_level; ++i)
        {
            index_node = new IndexNode<Key, Value>(prev, dataNode);
            InsertToIndexLayer(index_node, update_list[i]);
            prev = index_node;
        }
    }

    virtual void InsertToDataLayer(DataNode<Key, Value>* node,\
            DataNode<Key, Value>* prevLayerNode) const
    {
        node->next(static_cast<DataNode<Key, Value>*>(&prevLayerNode->next()));
        prevLayerNode->next(node);
    }

    virtual void InsertToIndexLayer(IndexNode<Key, Value>* node,\
            IndexNode<Key, Value>* prevLayerNode) const 
    {
        node->next(static_cast<IndexNode<Key, Value>*>(&prevLayerNode->next()));
        prevLayerNode->next(node);
    }

    virtual bool keysEqual(const Key& key1,  const Key& key2) const
    {
        return !(keyLess(key1, key2) || keyLess(key2, key1));
    }

    virtual DataNode<Key, Value>* getBeforeEqual(const Key& key,
            IndexNode<Key, Value>* update_list[MAXHEIGHT]) const
    {
        IndexNode<Key, Value>* curIndexNode = aHeadIdx[MAXHEIGHT - 1];
        Node<Key, Value>* next;
        for(int height = MAXHEIGHT - 1; height > 0; --height)
        {
            while ( (next = &curIndexNode->next()) != pTailIdx &&\
                            keyLess(next->key(), key))
            {
                curIndexNode = static_cast<IndexNode<Key, Value>*>(next);
            }
            update_list[height] = curIndexNode;
            curIndexNode = static_cast<IndexNode<Key, Value>*>(&curIndexNode->down());
        }

        while ((next = static_cast<IndexNode<Key, Value>*>(&curIndexNode->next())) != pTailIdx &&\
                            keyLess(next->key(), key))
        {
            curIndexNode = static_cast<IndexNode<Key, Value>*>(next);
        }
        update_list[0] = curIndexNode;

        // jump to data layer, curDataNode points to node, which next is
        // greater or equal to ours
        DataNode<Key, Value>* curDataNode = static_cast<DataNode<Key, Value>*>(&curIndexNode->down());

        while((next = &curDataNode->next()) != pTail && keyLess(next->key(), key))
        {
            curDataNode = static_cast<DataNode<Key, Value>*>(next);
        }
        return curDataNode;
    }
};

template<class Key, class Value, size_t MAXHEIGHT, class Compare>
const Compare& SkipList<Key, Value, MAXHEIGHT, Compare>::keyLess = Compare();


#endif // __SKIPLIST_H
