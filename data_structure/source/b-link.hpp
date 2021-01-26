// Copyright
#ifndef SOURCE_B_LINK_HPP_
#define SOURCE_B_LINK_HPP_

#include <utility>
#include <thread>
#include <mutex>
#include <vector>

namespace EDA {
namespace Concurrent {

template <std::size_t B, typename Type>
class BLinkTree {
 public:
  typedef Type data_type;

  struct BNode {
	  BNode(bool leaf_) {
		  leaf = leaf_;
		  size = 0;
		  lateral = NULL;
		  values = new data_type[B - 1];
		  childs = new BNode*[B];
		  for (int i = 0; i < B; ++i) {childs[i] = NULL;}
	  }

	  bool leaf;
	  std::size_t size;

	  data_type* values;
	  BNode** childs;
	  BNode* lateral;

	  std::mutex write;
	  std::mutex read;

	  friend class BLinkTree;
  };

  BLinkTree() {
	  root = new BNode(true);
  }

  ~BLinkTree() {}

  std::size_t size() const {}

  void BSort(data_type*& x, int n) {
	  int i, j, aux;
	  for (i = 0; i < n; i++) {
		  for (j = 0; j < n - i; j++) {
			  if (x[j] > x[j + 1]) {aux = x[j]; x[j] = x[j + 1]; x[j + 1] = aux;}
		  }
	  }
  }

  bool empty() const {
	if (root->values[0] == NULL) {
		return true;
	}
	return false;
  }

  bool search(const data_type& value){
	  BNode** ptr = &root; BNode** ptr_dad = NULL;

	  while (!((*ptr)->leaf)) {// recorre hasta encontrar la hoja donde deberia estar
		  if ((*ptr)->values[0] > value) {
			  ptr_dad = ptr;
			  ptr = &((*ptr)->childs[0]);
		  }
		  else if (value >= (*ptr)->values[(*ptr)->size - 1]) {
			  ptr_dad = ptr;
			  ptr = &((*ptr)->childs[(*ptr)->size]);
		  }
		  else {
			  for (int i = 1; i < (*ptr)->size; ++i) {
				  if ((*ptr)->values[i] > value) {
					  ptr_dad = ptr;
					  ptr = &((*ptr)->childs[i]);
					  break;
				  }
			  }
		  }
	  }

	  for (int i = 0; i < (*ptr)->size; ++i) {//recorre la hoja
		  if ((*ptr)->values[i] == value) {
			  return true;
		  }
	  }
	  return false;
  }

  bool search(const data_type& value, BNode**& ptr, BNode**& ptr_dad){
	  ptr = &root;

	  while (!((*ptr)->leaf)){// recorre hasta encontrar la hoja donde deberia estar
		  if ((*ptr)->values[0] > value) {
			  ptr_dad = ptr;
			  ptr = &((*ptr)->childs[0]);
		  }
		  else if (value >= (*ptr)->values[(*ptr)->size - 1]) {
			  ptr_dad = ptr;
			  ptr = &((*ptr)->childs[(*ptr)->size]);
		  }
		  else {
			  for (int i = 1; i < (*ptr)->size; ++i) {
				  if ((*ptr)->values[i] > value) {
					  ptr_dad = ptr;
					  ptr = &((*ptr)->childs[i]);
					  break;
				  }
			  }
		  }
	  }

	  for (int i = 0; i < (*ptr)->size; ++i) {//recorre la hoja
		  if ((*ptr)->values[i] == value) {
			  return true;
		  }
	  }
	  return false;
  }

  void split(BNode**& ptr, BNode**& ptr_dad, const data_type& value){
	  std::size_t particion = B / 2;
	  int index = 0;
	  BNode* node_1 = new BNode(true); BNode* node_2 = new BNode(true);

	  if (!ptr_dad) {
		  BNode* new_root = new BNode(false);// nuevo root
		  new_root->values[index] = (*ptr)->values[particion];
		  new_root->size++;

		  new_root->childs[index + 1] = node_2;//actualiza el puntero siguiente al index al nuevo nodo 2
		  node_1->lateral = node_2;//actualiza los laterales
		  
		  new_root->childs[index] = (*ptr);
		  ptr = &(new_root->childs[index]);
		  root = new_root;
		  ptr_dad = &root;
	  }

	  else {
		  for (index = 0; index <= (*ptr_dad)->size; ++index) {//recorre la hoja para hallar el indice correspondiente
			  if ((*ptr_dad)->childs[index] == (*ptr)) {
				  break;
			  }
		  }

		  for (int i = (*ptr_dad)->size; i > index; --i) {//corre los elementos en el padre una posicion
			  (*ptr_dad)->values[i] = (*ptr_dad)->values[i - 1];
		  }

		  (*ptr_dad)->values[index] = (*ptr)->values[particion];//inserta el elemento en el nodo padre
		  (*ptr_dad)->size++;

		  for (int i = (*ptr_dad)->size; i > index + 1; --i) {//corre los punteros en el padre una posicion
			  (*ptr_dad)->childs[i] = (*ptr_dad)->childs[i - 1];
		  }

		  (*ptr_dad)->childs[index + 1] = node_2;//actualiza el puntero siguiente al index al nuevo nodo 2
		  if(index-1>=0){((*ptr_dad)->childs[index - 1])->lateral = node_1;}//actualiza los laterales
		  node_1->lateral = node_2;
		  node_2->lateral = (*ptr)->lateral;
	  }

	  for (int i = 0; i < particion; ++i) {//copia la primera parte del nodo
		  node_1->values[i] = (*ptr)->values[i];
		  node_1->size++;
	  }
	  
	  for (int j = 0, i = particion; i < (*ptr)->size; ++i, ++j) {//copia la segunda parte del nodo
		  node_2->values[j] = (*ptr)->values[i];
		  node_2->size++;
	  }

	  delete (*ptr);//actualiza el puntero index
	  *ptr = node_1;

	  if (value >= (*ptr_dad)->values[index]) {//coloca ptr en el nuevo nodo donde se debe hacer la insercion
		  ptr = &((*ptr_dad)->childs[index + 1]);
	  }
  }

  void insert(const data_type value) {
	  BNode** ptr = NULL; BNode** ptr_dad = NULL;

	  if (search(value,ptr,ptr_dad)) {
		  return;
	  }

	  if ((*ptr)->size == B - 1) {// si es necesario hacer split
		  std::cout << "Time to split" << std::endl;
		  split(ptr,ptr_dad,value);
		  (*ptr)->values[(*ptr)->size] = value;
		  (*ptr)->size++;
		  BSort((*ptr)->values, ((*ptr)->size) - 1);
	  }

	  else {// si no es necesario hacer split
		  (*ptr)->values[(*ptr)->size] = value;
		  (*ptr)->size++;
	  }
  }

  void remove(const data_type& value) {
  
  }

  void print_endl() {
	  int aux = 0;
	  std::vector <BNode*> c;
	  c.push_back(root);
	  int len = 0;
	  while (c.size() != 0) {
		  len = c.size();
		  std::cout << aux << ": "; aux++;
		  for (int i = 0; i < len; i++) {
			  for (int j = 0; j < c[i]->size; ++j) {
				  std::cout << c[i]->values[j] << " ";
				  if ((c[i]->childs[j]) != NULL) {
					  c.push_back(c[i]->childs[j]);
				  }
			  }
			  if ((c[i]->childs[c[i]->size]) != NULL) { c.push_back(c[i]->childs[c[i]->size]);}
			  std::cout << " --> ";
		  }
		  std::cout << std::endl;
		  c.erase(c.begin(), c.begin() + len);
	  }
	  std::cout << std::endl << std::endl;;
  }

 private:
  BNode* root;
};

}  // namespace Concurrent
}  // namespace EDA

#endif  // SOURCE_B_LINK_HPP_
