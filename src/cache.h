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

#ifndef _CACHE_H_
#define _CACHE_H_
#include <map>
#include <cstddef>
namespace cache
{
	template<class K, class V> class Cache
	{
        enum CacheRetCode
        {
            CacheRetCode_Succ 0,//操作成功
            CacheRetCode_Full -1,//提前告知Cache已满
        };

		template<class NK, class NV> struct Node
		{
			typedef Node* value_point;
			NK key;
			NV value;
			value_point spre, snext;
			value_point gpre, gnext;
            bool dirty;
			Node()
			{
				spre = snext = gpre = gnext = NULL;
			}
			Node(NK key, NV value, bool dirty):key(key), value(value), dirty(dirty)
			{
				spre = snext = gpre = gnext = NULL;
			}
		};

		typedef Node<K, V> CNode;
	public:
		Cache(size_t capacity = 100000):_capacity(capacity)
		{
			_map = new std::map<K, CNode*>();	
			_shead = new CNode();
			_stail = new CNode();
			_ghead = new CNode();
			_gtail = new CNode();

			_shead->snext = _stail;
			_stail->spre = _shead;

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

		CacheRetCode set(const K& key, const V& value, bool dirty)
		{
			typename std::map<K, CNode*>::iterator it = _map.find(key);			
			if(it != _map.end())
			{
				CNode* tnode = it->second;
				tnode->value = value;
                tnode->dirty = dirty;
				move_to_shead(tnode);
				move_to_ghead(tnode);
			}
			else
			{
			    if(_map.size() >= _capacity - 1) return CacheRetCode_Full;
				//如果数量超过最大容量
				CNode* tnode = new CNode(key, value, dirty);
				add_shead(tnode);
				add_ghead(tnode);
                _map.insert(std::map<K, CNode*>(key, tnode);
			}
            return CacheRetCode_Succ;
		}

		bool get(const K& key, V& value)
		{
			typename std::map<K, CNode*>::iterator it = _map.find(key);
			if(it == _map.end()) return false;
			value = it->second->value;
            move_to_ghead(it->second);
			return true;
		}

		void erase(const K& key)
		{
			typename std::map<K, CNode*>::iterator it = _map.find(key);
			if(it != _map.end())
			{
				remove_slist(it->second);
				remove_glist(it->second);
				delete(it->second);
				_map.erase(it);

			}
		}

        size_t clear(size_t cnt = 0)
        {
            if(cnt == 0) cnt = _capacity / 10; //默认清除总容量的10%;
            CNode *tnode = _gtail->gpre;
            size_t num = 0;
            while(cnt > 0 && tnode != _ghead)
            {
                _map.erase(tnode->key);
                remove_slist(tnode);
                tnode = tonde->gpre;
                if(tnode->gnext->dirty)
                {
                    //脏数据回写到数据库中
                }
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
			while(tnode != _gtail)
			{
				tnnode = tnode;
				tnode = tnode->gnext;
				delete(tnnode);
			}
			_map.clear();
			_shead->snext = _stail;
			_stail->spre = _shead;

			_ghead->gnext = _gtail;
			_gtail->gpre = _ghead;

		}

		~Cache()
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
			delete(_shead);
			delete(_stail);
		}

	private:
		inline typename std::map<K, CNode*>::iterator find(const K& key)
		{
			return _map->find(key);
		}
		inline void move_to_shead(CNode* cur)
		{
			if(cur == NULL || cur == _shead->snext) return;
			cur->spre->snext = cur->snext;
			cur->snext->spre = cur->spre;
			cur->snext = _shead->snext;
			_shead->snext->spre = cur;
			_shead->snext = cur;
			cur->spre = _shead;
		}
		inline void add_shead(CNode* cur)
		{
			if(cur == NULL) return;
			cur->snext = _shead->snext;
			_shead->snext->spre = cur;
			_shead->snext = cur;
			cur->spre = _shead;	
		}
		inline void remove_slist(CNode* cur)
		{
			if(cur == NULL) return;
			cur->spre->snext = cur->snext;
			cur->snext->spre = cur->spre;	
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
		CNode *_ghead, *_gtail;//set list
		CNode *_shead, *_stail;//get list
		size_t _capacity;
	};

}
#endif
