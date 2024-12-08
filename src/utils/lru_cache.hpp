#pragma once

#include <list>
#include <unordered_map>
#include <stdexcept>

namespace cache {

template <typename Key, typename Value>
class LRUCache {
 public:
  explicit LRUCache(size_t capacity) : capacity_(capacity) {
    if (capacity == 0) {
      throw std::invalid_argument("Cache capacity must be greater than 0");
    }
  }

  void Put(const Key& key, const Value& value) {
    auto it = cache_map_.find(key);
    if (it != cache_map_.end()) {
      // Если ключ уже существует, удаляем старую запись
      cache_list_.erase(it->second);
      cache_map_.erase(it);
    }

    // Добавляем новую пару ключ-значение в начало списка
    cache_list_.emplace_front(key, value);
    cache_map_[key] = cache_list_.begin();

    // Если размер превышает емкость, удаляем наименее используемый элемент
    if (cache_map_.size() > capacity_) {
      auto last = cache_list_.end();
      --last;
      cache_map_.erase(last->first);
      cache_list_.pop_back();
    }
  }

  Value Get(const Key& key) {
    auto it = cache_map_.find(key);
    if (it == cache_map_.end()) {
      throw std::runtime_error("Key not found");
    }

    // Перемещаем используемый элемент в начало списка
    auto list_it = it->second;
    cache_list_.splice(cache_list_.begin(), cache_list_, list_it);

    return list_it->second;
  }

  bool Contains(const Key& key) const {
    return cache_map_.find(key) != cache_map_.end();
  }

 private:
  size_t capacity_;
  std::list<std::pair<Key, Value>> cache_list_;
  std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> cache_map_;
};

}  // namespace cache
