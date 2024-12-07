#include <algorithm>
#include <optional>
#include <vector>

template <typename K, typename V>
class AList {
  public:
   // Constructor that takes a reference to a vector of key-value pairs
   explicit AList(const std::vector<std::pair<K, V>>& data)
       : data(data) {}

   // Find the value associated with a given key; returns NULL if key
   // is not found.
   const V* get(const K& key) const {
      auto iter = std::find_if(data.begin(), data.end(), [&key](const auto& pair) {
         return pair.first == key;
      });
      if (iter != data.end()) {
         return &iter->second;
      }
      return NULL;
   }

  private:
   const std::vector<std::pair<K, V>>& data;
};
