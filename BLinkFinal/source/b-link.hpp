// Copyright
#ifndef SOURCE_B_LINK_HPP_
#define SOURCE_B_LINK_HPP_

#include <utility>
#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <stack>

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
		  range[0] = 1; range[1] = -1;
		  lateral = NULL;
		  values = new data_type[B];
		  childs = new BNode*[B];
		  for (int i = 0; i < B; ++i) {childs[i] = NULL;}
		  for (int i = 0; i < B; ++i) { values[i] = 0; }
	  }

	  ~BNode(){
		  delete[] values;
		  delete[] childs;
	  }

	  bool leaf;
	  std::size_t size;
	  data_type range[2];

	  data_type* values;
	  BNode** childs;
	  BNode* lateral;

	  std::mutex mtx;

	  friend class BLinkTree;
  };

  BLinkTree() {
	  root = new BNode(true);
  }

  ~BLinkTree() {
	  delete[] root;
  }

  //std::size_t size() const {}

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

  void reversing_queue(std::queue<BNode*>& locks){
	  std::stack<BNode*> Stack;
	  while (!locks.empty()) {
		  Stack.push(locks.front());
		  locks.pop();
	  }
	  while (!Stack.empty()) {
		  locks.push(Stack.top());
		  Stack.pop();
	  }
  }

  bool search(const data_type& value){
	  BNode** ptr = &root; BNode** ptr_dad = NULL;

	  while (!((*ptr)->leaf)) {// recorre hasta encontrar la hoja donde deberia estar
		  /*if (value >= (*ptr)->range[1] && (*ptr)->range[1] != -1) {//fuera de rango
				ptr_dad = &((*ptr_dad)->lateral);
				ptr = &((*ptr)->lateral);
		  }*/
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

  bool search(const data_type& value, BNode**& ptr, BNode**& ptr_dad, std::queue<BNode*>& locks){
	  root->mtx.lock();
	  ptr = &root;
	  locks.push(root);

	  while (!((*ptr)->leaf)){// recorre hasta encontrar la hoja donde deberia estar
		  /*if (value >= (*ptr)->range[1] && (*ptr)->range[1] != -1) {//fuera de rango
		        ((*ptr_dad)->lateral)->mtx.lock();
				ptr_dad = &((*ptr_dad)->lateral);
				((*ptr)->lateral)->mtx.lock();
				ptr = &((*ptr)->lateral);
		  }*/
		  if ((*ptr)->values[0] > value) {
			  ((*ptr)->childs[0])->mtx.lock();
			  ptr_dad = ptr;
			  ptr = &((*ptr)->childs[0]);
			  if ((*ptr)->size == B-1) {locks.push(*ptr); }
			  else {
				  while (!locks.empty()) {
					  (locks.front())->mtx.unlock();
					  locks.pop();
				  }
				  locks.push(*ptr);
			  }
		  }
		  else if (value >= (*ptr)->values[(*ptr)->size - 1]) {
			  ((*ptr)->childs[(*ptr)->size])->mtx.lock();
			  ptr_dad = ptr;
			  ptr = &((*ptr)->childs[(*ptr)->size]);
			  if ((*ptr)->size == B - 1) { locks.push(*ptr); }
			  else {
				  while (!locks.empty()) {
					  (locks.front())->mtx.unlock();
					  locks.pop();
				  }
				  locks.push(*ptr);
			  }
		  }
		  else {
			  for (int i = 1; i < (*ptr)->size; ++i) {
				  if ((*ptr)->values[i] > value) {
					  ((*ptr)->childs[i])->mtx.lock();
					  ptr_dad = ptr;
					  ptr = &((*ptr)->childs[i]);
					  if ((*ptr)->size == B - 1) { locks.push(*ptr); }
					  else {
						  while (!locks.empty()) {
							  (locks.front())->mtx.unlock();
							  locks.pop();
						  }
						  locks.push(*ptr);
					  }
					  break;
				  }
			  }
		  }
	  }

	  for (int i = 0; i < (*ptr)->size; ++i) {//recorre la hoja
		  if ((*ptr)->values[i] == value) {return true; }
	  }
	  return false;
  }

  void search_internal(const data_type& value, BNode**& ptr, BNode**& ptr_dad) {
	  BNode** ptr_guide = &root;
	  ptr_dad = NULL;

	  while ((*ptr_guide)!=(*ptr)) {// recorre hasta encontrar el padre del nodo recibido
		  if ((*ptr_guide)->values[0] > value) {
			  ptr_dad = ptr_guide;
			  ptr_guide = &((*ptr_guide)->childs[0]);
		  }
		  else if (value >= (*ptr_guide)->values[(*ptr_guide)->size - 1]) {
			  ptr_dad = ptr_guide;
			  ptr_guide = &((*ptr_guide)->childs[(*ptr_guide)->size]);
		  }
		  else {
			  for (int i = 1; i < (*ptr_guide)->size; ++i) {
				  if ((*ptr_guide)->values[i] > value) {
					  ptr_dad = ptr_guide;
					  ptr_guide = &((*ptr_guide)->childs[i]);
					  break;
				  }
			  }
		  }
	  }
  }

  void split_internal(const data_type& value, BNode**& ptr, std::queue<BNode*>& locks) {
	  BNode** ptr_dad = NULL;
	  BNode* node_1 = new BNode(false); BNode* node_2 = new BNode(false);
	  int index = 0;
	  //std::cout << "MULTICAST x" << fun << std::endl;
	  //search_internal(value, ptr, ptr_dad);//busca su padre(secuencial)

	  if(locks.size() > 1){//actualiza el padre
		  locks.pop();
		  ptr_dad = &(locks.front()); 
	  }

	  if (!ptr_dad) {
		  BNode* new_root = new BNode(false);// nuevo root
		  new_root->values[0] = (*ptr)->values[particion];
		  new_root->size++;

		  new_root->childs[1] = node_2;//actualiza el puntero siguiente al index al nuevo nodo 2
		  node_1->lateral = node_2;//actualiza los laterales

		  node_1->range[1] = new_root->values[0]; node_2->range[0] = new_root->values[0];//actualiza los rangos

		  new_root->childs[0] = (*ptr);
		  ptr = &(new_root->childs[0]);
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
		  if (index - 1 >= 0) { ((*ptr_dad)->childs[index-1])->lateral = node_1; }//actualiza los laterales
		  node_1->lateral = node_2;
		  node_2->lateral = (*ptr)->lateral;

		  if (index-1 >= 0) {node_1->range[0] = (*ptr_dad)->values[index-1]; }//actualiza los rangos(hijo izquierdo)
		  else {node_1->range[0] = (*ptr_dad)->range[0]; }
		  node_1->range[1] = (*ptr_dad)->values[index];

		  if (index+1 == (*ptr_dad)->size){ node_2->range[1] = (*ptr_dad)->range[1]; }//actualiza los rangos(hijo derecho)
		  else {node_2->range[1] = (*ptr_dad)->values[index+1]; }
		  node_2->range[0] = (*ptr_dad)->values[index];
	  }

	  for (int i = 0; i < particion; ++i) {//copia la primera parte del nodo(datos e hijos)
		  node_1->values[i] = (*ptr)->values[i];
		  node_1->size++;

		  node_1->childs[i] = (*ptr)->childs[i];
	  }
	  node_1->childs[particion] = (*ptr)->childs[particion];


	  for (int j = 0, i = particion+1; i < (*ptr)->size; ++i, ++j) {//copia la segunda parte del nodo(datos e hijos)
		  node_2->values[j] = (*ptr)->values[i];
		  node_2->size++;

		  node_2->childs[j] = (*ptr)->childs[i];
	  }
	  node_2->childs[particion] = (*ptr)->childs[(*ptr)->size];

	  (*ptr)->mtx.unlock();
	  delete (*ptr);//actualiza el puntero ptr
	  *ptr = node_1;

	  if ((*ptr_dad)->size == B) {split_internal(value, ptr_dad, locks); }
  }

  bool split(BNode**& ptr, BNode**& ptr_dad, const data_type& value){
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

	  (*ptr)->mtx.unlock();
	  delete (*ptr);//actualiza el puntero index
	  *ptr = node_1;

	  if (value >= (*ptr_dad)->values[index]) {//coloca ptr en el nuevo nodo donde se debe hacer la insercion
		  ptr = &((*ptr_dad)->childs[index + 1]);
	  }

	  if ((*ptr_dad)->size == B) { return true;}//si es necesario hacer split internal

	  return false;
  }

  void insert(const data_type value) {
	  BNode** ptr = NULL; BNode** ptr_dad = NULL;
	  bool internal = false;
	  std::queue<BNode*> locks;

	  if (search(value,ptr,ptr_dad,locks)) {
		  return;
	  }
	  reversing_queue(locks);

	  if ((*ptr)->size == B - 1) {// si es necesario hacer split
		  //std::cout << "Time to split" << std::endl;
		  internal = split(ptr, ptr_dad, value);
		  locks.pop();
		  (*ptr)->values[(*ptr)->size] = value;
		  (*ptr)->size++;
		  BSort((*ptr)->values, ((*ptr)->size) - 1);

		  if (internal) { // si es necesario hacer split internal
			  split_internal(value,ptr_dad,locks);
		  }
	  }

	  else {// si no es necesario hacer split
		  (*ptr)->values[(*ptr)->size] = value;
		  (*ptr)->size++;
		  BSort((*ptr)->values, ((*ptr)->size) - 1);
		  
		  while (!locks.empty()) {
			  (locks.front())->mtx.unlock();
			  locks.pop();
		  }
	  }
  }

  void remove(const data_type& value) {
	  BNode** ptr = NULL; BNode** ptr_dad = NULL;
	  bool internal = false;
	  std::queue<BNode*> locks;

	  if (!(search(value, ptr, ptr_dad, locks))) {
		  return;
	  }

	  if ((*ptr)->size == B/2 ) {// si es necesario hacer split
		  //std::cout << "Time to unsplit" << std::endl;
	  }

	  else {// si no es necesario hacer unsplit
		  for (int i = 0; i < (*ptr)->size; ++i) {//recorre la hoja
			  if ((*ptr)->values[i] == value) {
				  (*ptr)->values[i] = 0;
			  }
		  }
		  (*ptr)->size--;
		  BSort((*ptr)->values, ((*ptr)->size) - 1);
	  }
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
  const std::size_t particion = B / 2;
};

}  // namespace Concurrent
}  // namespace EDA

#endif  // SOURCE_B_LINK_HPP_
