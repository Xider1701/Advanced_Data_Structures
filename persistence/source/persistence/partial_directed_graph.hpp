// Copyright 2021 Roger Peralta Aranibar
#ifndef SOURCE_PERSISTENCE_PARTIAL_DIRECTED_GRAPH_HPP_
#define SOURCE_PERSISTENCE_PARTIAL_DIRECTED_GRAPH_HPP_

#include <utility>

namespace ADE {
namespace Persistence {

template <typename Type>
class PartialNode;

template <typename Type>
struct Mod { //clase para cada modificacion
    Mod() : version(0), value(nullptr) {}

    Mod(std::size_t version_, Type const& value_) {
        version = version_;
        value = new Type(value_);
    }

    Mod(std::size_t version_, std::size_t const& i, PartialNode<Type>* ptr_) {
        version = version_;
        value = NULL;
        ptr = std::pair<std::size_t, PartialNode<Type>*>(i, ptr_);
    }

    ~Mod() {if (value != NULL) {delete value;}}

    std::size_t version;
    Type* value;
    std::pair<std::size_t,PartialNode<Type>*> ptr;
};

template <typename Type>
class PartialNode {
 public:
  typedef Type data_type;

  PartialNode() : data_(nullptr), forward_(nullptr), backward_(nullptr), 
                out_ptrs_size_(0), in_ptrs_size_(0), my_version_(0), max_version_(0),
                current_modifications_size_(0), current_back_pointer_size_(0),
                mods_size(0), mods(nullptr) {}

  PartialNode(data_type const& data, std::size_t const& out_ptrs_size,std::size_t const& in_ptrs_size, std::size_t const& my_version){
      data_ = new data_type(data);
      out_ptrs_size_ = out_ptrs_size;
      in_ptrs_size_ = in_ptrs_size;
      my_version_ = my_version;
      max_version_ = my_version;
      backward_ = new PartialNode<Type>*[in_ptrs_size]();
      forward_ = new PartialNode<Type>*[out_ptrs_size]();
      current_modifications_size_ = 0;
      current_back_pointer_size_ = 0;
      mods_size = 2 * in_ptrs_size;
      mods = new Mod<Type>[2 * in_ptrs_size];
  }

  //~PartialNode() { delete data_; delete[] backward_; delete[] forward_; delete[] mods;}

  data_type get_data(unsigned int version_) { 
      if (version_ < my_version_) {}// si la version requerida es mas antigua a las almacenadas en el nodo

      else if (version_ == my_version_) {return *data_;}

      else {
          data_type aux = *data_;
          for (int i = 0; i < mods_size; ++i) {
              if (mods[i].value && version_ == mods[i].version) {// si es una mod. de valor && si es la version requerida
                aux = *(mods[i].value); break;
              }   
          }

          return aux;
      }
  }

  bool set_data(data_type const& data){ 
      if (current_modifications_size_ < mods_size) {// tabla de modificaciones no llena
          max_version_++; current_modifications_size_++;
          mods[current_modifications_size_ - 1] = Mod<Type>(max_version_, data);
          return 1;
      }
      else {//tabla llena
          int i = 0;
          PartialNode* aux_node = new PartialNode(data, out_ptrs_size_, in_ptrs_size_, max_version_ + 1);
          aux_node->max_version_ = max_version_ + 1;
          for (i = 0; i < in_ptrs_size_; ++i) { aux_node->backward_[i] = backward_[i]; }

          for (i = 0; i < in_ptrs_size_; ++i) {
              if (backward_[i]) { backward_[i]->update_edge(i, aux_node); }
          }

          for (i = 0; i < out_ptrs_size_; ++i) { aux_node->forward_[i] = forward_[i]; }

          for (i = 0; i < out_ptrs_size_; ++i) {
              if (forward_[i]) { forward_[i]->backward_[i] = aux_node; }
          }

          return 1;
      }
  }
  
  PartialNode* insert_vertex(std::size_t const& position,data_type const& data){
      PartialNode* new_node = new PartialNode(data, out_ptrs_size_, in_ptrs_size_, 0);
      update_edge(position, new_node);
      return new_node;      
  }

  bool update_edge(std::size_t const& position, PartialNode* v) { 
      if (out_ptrs_size_ < position && in_ptrs_size_ < position) {
          throw std::out_of_range("Insert position is out of edges range.");
      }

      if (forward_[position] == NULL) {
          forward_[position] = v;
          v->backward_[position] = this;
          return 1;
      }
      else {
          if (current_modifications_size_ < mods_size) {// tabla de modificaciones no llena
              max_version_++; current_modifications_size_++;
              mods[current_modifications_size_ - 1] = Mod<Type>(max_version_, position, v);
              v->backward_[position] = this;
              return 1;
          }
          else {//tabla llena
              int i = 0;
              PartialNode* aux_node = new PartialNode(*data_, out_ptrs_size_, in_ptrs_size_, max_version_+1);
              aux_node->max_version_ = max_version_ + 1;
              for (i = 0; i < in_ptrs_size_; ++i) {aux_node->backward_[i] = backward_[i];}

              for (i = 0; i < in_ptrs_size_; ++i) { 
                  if (backward_[i]) {backward_[i]->update_edge(i, aux_node);}
              }

              for (i = 0; i < out_ptrs_size_; ++i) { aux_node->forward_[i] = forward_[i]; }

              for (i = 0; i < out_ptrs_size_; ++i) {
                  if (forward_[i]) { forward_[i]->backward_[i]=aux_node; }
              }

              aux_node->forward_[position] = v;
              v->backward_[position] = aux_node;
              return 1;
          }
      }
  }

  PartialNode& operator[](std::pair<std::size_t, unsigned int> const& position_version) const {
      if (out_ptrs_size_ < position_version.first) {
          throw std::out_of_range("Index out of node edges range.");
      }
      if (!forward_[position_version.first]) {
          throw std::logic_error("Access to null reference.");
      }

      if (position_version.second < my_version_) {}// si la version requerida es mas antigua a las almacenadas en el nodo

      else if (position_version.second == my_version_) { return *forward_[position_version.first]; }

      else {
          PartialNode* aux = forward_[position_version.first];
          for (int i = 0; i < mods_size; ++i) {
              if (!(mods[i].value) && position_version.second == mods[i].version) {// si es una mod. de ptr && si es la version requerida
                  aux = forward_[position_version.first]; break;
              }
          }

          return *aux;
      }

  }

  data_type* data_;
  std::size_t out_ptrs_size_;//# de salidas
  std::size_t in_ptrs_size_;//# de entradas
  std::size_t mods_size;
  std::size_t my_version_;//version en que fue creado el nodo
  std::size_t max_version_;//version maxima que tiene el nodo

  PartialNode** forward_;//almacenar los punteros salientes
  PartialNode** backward_;//almacenar los punteros entrantes

  std::size_t current_modifications_size_;
  std::size_t current_back_pointer_size_;
  Mod<Type>* mods;
  // TODO: Table mods, 2 * in_ptrs_size_
};

template <typename Type, typename Node>
class PartialDirectedGraph {
 public:
  typedef Type data_type;

  PartialDirectedGraph(data_type const data, std::size_t const& out_ptrs_size, std::size_t const& in_ptrs_size) {
      root_ptr_ = new Node(data, out_ptrs_size, in_ptrs_size, 0);
      out_ptrs_size_ = out_ptrs_size;
      in_ptrs_size_ = in_ptrs_size;
  }

  ~PartialDirectedGraph() { delete root_ptr_; }

  Node* get_root_ptr(unsigned int const& version) { return root_ptr_; }

  Node get_root(unsigned int const& version) { return *root_ptr_; }

  bool add_edge(Node* u, Node* v, std::size_t position) {
      if (u->out_ptrs_size_ < position) {
          throw std::out_of_range("Position out of first argument node.");
      }

      u->update_edge(position, v);
      return true;
  }

 //protected:
  Node* root_ptr_;

  std::size_t in_ptrs_size_;
  std::size_t out_ptrs_size_;
};

} // namespace Persistence
} // namespace ADE

#endif  // SOURCE_PERSISTENCE_PARTIAL_DIRECTED_GRAPH_HPP_
