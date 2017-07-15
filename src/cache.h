/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Seantian
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
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
		template<class NK, class NV> struct Node
		{
			NK key;
			NV value;
			Node *spre, *snext;
			Node *gpre, *gnext;list
			Node(NK key, NV value):key(key), value(value)
			{
				spre = snext = gpre = gnext = NULL;
			}
		};

	public:
		Cache(size_t capacity = 100000):_capacity(capacity)
		{
			_map = new std::map<K, Node<K, V>* >();	
			_shead = new Node<K, V>();
			_stail = new Node<K, V>();
			_ghead = new Node<K, V>();
			_gtail = new Node<K, V>();

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

		void set(const K& key, const V& value)
		{
			std::map<K, Node<K, V>*>::iterator it = _map.find(key);			
			if(it != _map.end())
			{
				Node<K, V>* tnode = it->second;
				tnode->value = value;
				move_to_shead(tnode);
				move_to_ghead(tnode);
			}
			else
			{
				//如果数量超过最大容量
				Node<K, V>* tnode = new Node(key, value);
				add_shead(tnode);
				add_ghead(tnode);
			}
		}

		bool get(const K& key, V& value)
		{
			std::map<K, Node<K, V>*>::iterator it = _map.find(key);
			if(it == _map.end()) return false;
			value = it->second->value;
			return true;
		}

		void erase(const K& key)
		{
			std::map<K, Node<K, V>*>::iterator it = _map.find(key);
			if(it != _map.end())
			{
				remove_slist(it->second);
				remove_glist(it->second);
				free(it->second);
				_map.erase(it);

			}
		}

		void clear()
		{
			Node<K, V> *tnode = _ghead->gnext;
			Node<K, V> *tnnode;
			while(tnode != _gtail)
			{
				tnnode = tnode;
				tnode = tnode->gnext;
				free(tnnode);
			}
			_map.clear();
			_shead->snext = _stail;
			_stail->spre = _shead;

			_ghead->gnext = _gtail;
			_gtail->gpre = _ghead;

		}

		~Cache()
		{
			Node<K, V> *tnode = _ghead->gnext;
			Node<K, V> *tnnode;
			while(tnode != _gtail)
			{
				tnnode = tnode;
				tnode = tnode->gnext;
				free(tnnode);
			}
			free(_map);
			free(_ghead);
			free(_gtail);
			free(_shead);
			free(_stail);
		}

	private:
		inline typename std::map<K, Node<K, V>*>::iterator find(const K& key)
		{
			return _map->find(key);
		}
		inline void move_to_shead(Node<K, V>* cur)
		{
			if(cur == NULL || cur == shead->snext) return;
			cur->spre->snext = cur->snext;
			cur->snext->spre = cur->spre;
			cur->snext = shead->snext;
			shead->snext->spre = cur;
			shead->snext = cur;
			cur->spre = shead;
		}
		inline void add_shead(Node<K, V>* cur)
		{
			if(cur == NULL) return;
			cur->snext = shead->snext;
			shead->snext->spre = cur;
			shead->snext = cur;
			cur->spre = head;	
		}
		inline void remove_slist(Node<K, V>* cur)
		{
			if(cur == NULL) return;
			cur->spre->snext = cur->snext;
			cur->snext->spre = cur->spre;	
		}
		inline void move_to_ghead(Node<K, V>* cur)
		{
			if(cur == NULL || cur == ghead->gnext) return;
			cur->gpre->gnext = cur->gnext;
			cur->gnext->gpre = cur->gpre;
			cur->gnext = ghead->gnext;
			ghead->gnext->gpre = cur;
			ghead->gnext = cur;
			cur->gpre = ghead;
		}
		inline void add_ghead(Node<K, V>* cur)
		{
			if(cur == NULL) return;
			cur->gnext = ghead->gnext;
			ghead->gnext->gpre = cur;
			ghead->gnext = cur;
			cur->gpre = ghead;
		}
		inline void remove_glist(Node<K, V>* cur)
		{
			if(cur == NULL) return;
			cur->gpre->gnext = cur->gnext;
			cur->gnext->gpre = cur->gpre;	
		}

		std::map<K, Node<K, V>* > *_map;
		Node<K, V> *_ghead, *_gtail;//set list
		Node<K, V> *_shead, *_stail;//get list
		size_t _capacity;
	};

}
#endif
