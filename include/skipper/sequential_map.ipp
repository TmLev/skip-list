#ifndef SKIPPER_SEQUENTIAL_MAP_IPP
#define SKIPPER_SEQUENTIAL_MAP_IPP

#include <algorithm>
#include <iostream>
#include <memory>
#include <utility>

#include "skipper//sequential_map.hpp"

namespace skipper {

////////////////////////////////////////////////////////////////////////////////
////
//// SequentialSkipListMap::Node
////

template <typename Key, typename Value>
struct SequentialSkipListMap<Key, Value>::Node {
 public:
  Node(Key key, Value value, Level level);

  auto Next() const -> Node*;

 public:
  Key key;
  Value value;
  ForwardNodePtrs forward;
};

template <typename Key, typename Value>
SequentialSkipListMap<Key, Value>::Node::Node(Key k, Value v, Level level)
    : key(std::move(k)),
      value(std::move(v)),
      forward(static_cast<std::size_t>(level) + 1) {
}

template <typename Key, typename Value>
auto SequentialSkipListMap<Key, Value>::Node::Next() const
    -> SequentialSkipListMap<Key, Value>::Node* {
  return forward[0].get();
}

////////////////////////////////////////////////////////////////////////////////
////
//// SequentialSkipListMap::Iterator
////

template <typename Key, typename Value>
SequentialSkipListMap<Key, Value>::Iterator::Iterator(
    SequentialSkipListMap::Node* ptr)
    : ptr_(ptr) {
}

template <typename Key, typename Value>
auto SequentialSkipListMap<Key, Value>::Iterator::operator*() const
    -> const std::pair<Key, Value>& {
  return std::pair<Key, Value>(ptr_->key, ptr_->value);
}

template <typename Key, typename Value>
auto SequentialSkipListMap<Key, Value>::Iterator::operator->() const
    -> const std::pair<Key, Value>* {
  return &std::pair<Key, Value>(ptr_->key, ptr_->value);
}

template <typename Key, typename Value>
auto SequentialSkipListMap<Key, Value>::Iterator::operator++(/* prefix */)
    -> SequentialSkipListMap::Iterator& {
  ptr_ = ptr_->Next();
  return *this;
}

template <typename Key, typename Value>
auto SequentialSkipListMap<Key, Value>::Iterator::operator++(int /* postfix */)
    -> SequentialSkipListMap::Iterator {
  auto copy = *this;
  ++(*this);
  return copy;
}

template <typename Key, typename Value>
auto SequentialSkipListMap<Key, Value>::Iterator::operator==(
    const SequentialSkipListMap::Iterator& other) const -> bool {
  return ptr_ == other.ptr_;
}

template <typename Key, typename Value>
auto SequentialSkipListMap<Key, Value>::Iterator::operator!=(
    const SequentialSkipListMap::Iterator& other) const -> bool {
  return !(*this == other);  // NOLINT (simplification will lead to recursion)
}

////////////////////////////////////////////////////////////////////////////////
////
//// SequentialSkipListMap: public interface
////

template <typename Key, typename Value>
auto SequentialSkipListMap<Key, Value>::Find(const Key& key) const
    -> SequentialSkipListMap::Iterator {
  if (auto node = Traverse(key); node && node->key == key) {
    return Iterator{node.get()};
  }
  return End();
}

template <typename Key, typename Value>
auto SequentialSkipListMap<Key, Value>::Insert(const Key& key,
                                               const Value& value)
    -> std::pair<Iterator, bool> {
  auto update = ForwardNodePtrs{kMaxLevel + 1};
  auto node = Traverse(key, &update);

  if (node && !(value < node->value)) {
    return {Iterator{node.get()}, false};
  }

  auto node_level = GenerateRandomLevel();
  if (node_level > level_) {
    std::fill(std::begin(update) + level_ + 1,
              std::begin(update) + node_level + 1, head_);
    level_ = node_level;
  }

  auto new_node = std::make_shared<Node>(key, value, node_level);
  for (auto level = Level{0}; level <= node_level; ++level) {
    auto i = static_cast<std::size_t>(level);
    new_node->forward[i] = std::exchange(update[i]->forward[i], new_node);
  }

  return {Iterator{new_node.get()}, true};
}

template <typename Key, typename Value>
auto SequentialSkipListMap<Key, Value>::Begin() const
    -> SequentialSkipListMap::Iterator {
  return Iterator{head_->Next()};
}

template <typename Key, typename Value>
auto SequentialSkipListMap<Key, Value>::End() const
    -> SequentialSkipListMap::Iterator {
  return Iterator{nullptr};
}

////////////////////////////////////////////////////////////////////////////////
////
//// SequentialSkipListMap: private interface
////

template <typename Key, typename Value>
auto SequentialSkipListMap<Key, Value>::Traverse(
    const Key& key, SequentialSkipListMap::ForwardNodePtrs* update) const
    -> SequentialSkipListMap::NodePtr {
  auto node = head_;

  for (auto level = level_; level >= 0; --level) {
    auto i = static_cast<std::size_t>(level);
    while (node->forward[i] && node->forward[i]->key < key) {
      node = node->forward[i];
    }
    if (update) {
      (*update)[i] = node;
    }
  }

  return node->forward[0];
}

template <typename Key, typename Value>
auto SequentialSkipListMap<Key, Value>::GenerateRandomLevel() const
    -> SequentialSkipListMap::Level {
  auto level = Level{0};
  // TODO add some randomness
  return level;
}

}  // namespace skipper

#endif  // SKIPPER_SEQUENTIAL_MAP_IPP