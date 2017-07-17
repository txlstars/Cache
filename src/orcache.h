/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Seantian
 *
 * Permission is hereby granted, delete of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/

/*****************************************************
 *Only Read Cache
 *for set(key, value) operator, key need not exit, 
 *if key has already exit, set fail 
 *****************************************************/

#ifndef _OR_CACHE_H_
#define _OR_CACHE_H_
#include <iostream>
#include <map>
#include <cstddef>
namespace orcache
{
	template<class K, class V> class ORCache
	{
        enum ORCacheRetCode
        {
            ORCacheRetCode_Succ		= 0,//操作成功
            ORCacheRetCode_Full		= -1,//提前告知Cache已满
            ORCacheRetCode_Exit		= -2,//set操作时key已经存在
			ORCacheRetCode_Non_Exit	= -3,//get操作时key不存在
        };

		template<class NK, class NV> struct Node
		{
			typedef Node* value_point;
			NK key;
			NV value;
			value_point gpre, gnext;//get list
			Node()
			{
				gpre = gnext = NULL;
			}
			Node(NK key, NV value):key(key), value(value)
			{
				gpre = gnext = NULL;
			}
		};

		typedef Node<K, V> CNode;
	public:
		ORCache(size_t capacity = 10):_capacity(capacity)
		{
            if(_capacity < 10) _capacity = 1000;//限制最小容量

			_map = new std::map<K, CNode*>();	
			_ghead = new CNode();
			_gtail = new CNode();

			_ghead->gnext = _gtail;
			_gtail->gpre = _ghead;
		}

		inline size_t size()
		{
			return _map->size();
		}

		inline bool empty()
		{
			return _map->empty();
		}

		ORCacheRetCode set(const K& key, const V& value)
		{
			ORCacheRetCode ret = ORCacheRetCode_Succ;
			typename std::map<K, CNode*>::iterator it = _map->find(key);			
			if(it != _map->end()) return ORCacheRetCode_Exit; 
			else
			{
			    if(_map->size() == _capacity)
                { 
                    CNode *tnode = _gtail->gpre;
                    remove_glist(tnode);
                    _map->erase(tnode->key);
                    delete(tnode);
                    ret = ORCacheRetCode_Full;
                }
				CNode* tnode = new CNode(key, value);
				add_ghead(tnode);
				_map->insert(std::pair<K, CNode*>(key, tnode));
			}
            return ret;
		}

		ORCacheRetCode get(const K& key, V& value)
		{
			typename std::map<K, CNode*>::iterator it = _map->find(key);
			if(it == _map->end()) return ORCacheRetCode_Non_Exit;
			value = it->second->value;
            move_to_ghead(it->second);
			return ORCacheRetCode_Succ;
		}

		void erase(const K& key)
		{
			typename std::map<K, CNode*>::iterator it = _map->find(key);
			if(it != _map->end())
			{
				CNode *tnode = it->second;
				remove_glist(tnode);
				_map->erase(it);
				delete(tnode);
			}
		}

        size_t clear(size_t cnt = 0)
        {
            if(cnt == 0) cnt = _capacity / 10; //默认清除总容量的10%;
            CNode *tnode = _gtail->gpre;
            size_t num = 0;
            while(cnt > 0 && tnode != _ghead)
            {
                _map->erase(tnode->key);
                tnode = tnode->gpre;
                delete(tnode->gnext);
                ++num;
            }
            tnode->gnext = _gtail;
            _gtail->gpre = tnode;
            return num;
        }

		void clearall()
		{
			CNode *tnode = _ghead->gnext;
			CNode *tnnode;
			_map->clear();
			while(tnode != _gtail)
			{
				tnnode = tnode;
				tnode = tnnode->gnext;
				delete(tnnode);
			}
			_ghead->gnext = _gtail;
			_gtail->gpre = _ghead;

		}
		void printall()
		{
			CNode *tnode = _ghead->gnext;
			CNode *tnnode;
			while(tnode != _gtail)
			{
				tnnode = tnode;
				tnode = tnode->gnext;
				std::cout << tnnode->key << ":" << tnnode->value << "  ";
			}
			std::cout << std::endl;
		}

		~ORCache()
		{
			CNode *tnode = _ghead->gnext;
			CNode *tnnode;
			while(tnode != _gtail)
			{
				tnnode = tnode;
				tnode = tnode->gnext;
				delete(tnnode);
			}
			delete(_map);
			delete(_ghead);
			delete(_gtail);
		}

	private:
		inline typename std::map<K, CNode*>::iterator find(const K& key)
		{
			return _map->find(key);
		}
		inline void move_to_ghead(CNode* cur)
		{
			if(cur == NULL || cur ==_ghead->gnext) return;
			cur->gpre->gnext = cur->gnext;
			cur->gnext->gpre = cur->gpre;
			cur->gnext =_ghead->gnext;
			_ghead->gnext->gpre = cur;
			_ghead->gnext = cur;
			cur->gpre = _ghead;
		}
		inline void add_ghead(CNode* cur)
		{
			if(cur == NULL) return;
			cur->gnext =_ghead->gnext;
			_ghead->gnext->gpre = cur;
			_ghead->gnext = cur;
			cur->gpre = _ghead;
		}
		inline void remove_glist(CNode* cur)
		{
			if(cur == NULL) return;
			cur->gpre->gnext = cur->gnext;
			cur->gnext->gpre = cur->gpre;	
		}

		std::map<K, CNode*> *_map;
		CNode *_ghead, *_gtail;//get list
		size_t _capacity;
	};

}
#endif
