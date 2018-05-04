/*
* File:   lrucache.hpp
* Author: Alexander Ponomarev
*
* Created on June 20, 2013, 5:09 PM
*/

#ifndef _LRUCACHE_HPP_INCLUDED_
#define	_LRUCACHE_HPP_INCLUDED_

#include <unordered_map>
#include <list>
#include <cstddef>
#include <stdexcept>

namespace cache {

    template<typename key_t, typename value_t>
    class lru_cache {
    public:
        typedef typename std::pair<key_t, value_t> key_value_pair_t;
        typedef typename std::list<key_value_pair_t>::iterator list_iterator_t;

        lru_cache(size_t max_size) :
            _max_size(max_size) {
        }

        template <typename value_t_>
        void put(const key_t& key, value_t_&& value) {
            decltype(auto) it = _cache_items_map.find(key);
            _cache_items_list.push_front(key_value_pair_t(key, std::forward<value_t_>(value)));
            if(it != _cache_items_map.cend()) {
                _cache_items_list.erase(it->second);
                _cache_items_map.erase(it);
            }
            _cache_items_map[key] = _cache_items_list.begin();

            if(_cache_items_map.size() > _max_size) {
                decltype(auto) last = _cache_items_list.cend();
                last--;
                _cache_items_map.erase(last->first);
                _cache_items_list.pop_back();
            }
        }

        const value_t& get(const key_t& key) {
            decltype(auto) it = _cache_items_map.find(key);
            if(it == _cache_items_map.cend()) {
                throw std::range_error("There is no such key in cache");
            }
            else {
                _cache_items_list.splice(_cache_items_list.cbegin(), _cache_items_list, it->second);
                return it->second->second;
            }
        }

         value_t& modify(const key_t& key) {
             decltype(auto) it = _cache_items_map.find(key);
            if(it == _cache_items_map.cend()) {
                throw std::range_error("There is no such key in cache");
            }
            else {
                _cache_items_list.splice(_cache_items_list.cbegin(), _cache_items_list, it->second);
                return it->second->second;
            }
        }

         void erase(const key_t& key) {
             decltype(auto) it = _cache_items_map.find(key);
             if(it == _cache_items_map.cend()) {
                 throw std::range_error("There is no such key in cache");
             }
             else {
                 _cache_items_list.erase(it->second);
                 _cache_items_map.erase(key);
             }
         }

        bool exists(const key_t& key) const {
            return _cache_items_map.find(key) != _cache_items_map.cend();
        }

        size_t size() const {
            return _cache_items_map.size();
        }

    private:
        std::list<key_value_pair_t> _cache_items_list;
        std::unordered_map<key_t, list_iterator_t> _cache_items_map;
        size_t _max_size;
    };

} // namespace cache

#endif	/* _LRUCACHE_HPP_INCLUDED_ */
