#include <stdlib.h>
#include <stdio.h>
#include "ValList.H"
#include "util.h"

template <class T>
ValList<T>::ValList(const ValList<T>& orig) : head(0), tail(0), n_elem(0)
{
    VLNode<T>* ptr = orig.head;
    while ( ptr ) {
	append( ptr->obj );
	ptr = ptr->fw;
    }
}    

template <class T>
void ValList<T>::clear()
{
    VLNode<T>* head_b;
    while ( head ) {
	head_b = head->fw;
	delete head;
	head = head_b;
    }
    n_elem = 0;
    tail = 0;
}

//// 最後尾につく
template <class T>
Pix ValList<T>::append(const T& item)
{
    VLNode<T>* item_node = new VLNode<T>(item, 0, 0);

    if ( head == 0 ) {
	head = item_node;
	tail = item_node;
    }
    else {
	item_node->bw = tail;
        tail->fw = item_node;
	tail = item_node;
    }
    n_elem++;
    return item_node;
}

//// headにつく
template <class T>
Pix ValList<T>::prepend(const T& item)
{
    VLNode<T>* item_node = new VLNode<T>(item, 0, 0);

    if ( head == 0 ) {
	head = item_node;
	tail = item_node;
    }
    else {
	item_node->fw = head;
        head->bw = item_node;
	head = item_node;
    }
    n_elem++;
    return item_node;
}

// beforeの後にitemを挿入
template <class T>
Pix ValList<T>::insertAfter(const T& item, const T& before)
{
    VLNode<T>* ptr = search_bw(before);
    if ( ptr == 0 ) {
	error("ValList<T>::insertAfter(item, before), cannot find before");
    }

    VLNode<T>* item_node = new VLNode<T>(item, ptr->fw, ptr);

    if ( ptr == tail ) tail = item_node;
    else ptr->fw->bw = item_node;
    ptr->fw = item_node;

    n_elem++;
    return item_node;
}

// afterの前にitemを挿入
template <class T>
Pix ValList<T>::insertBefore(const T& item, const T& after)
{
    VLNode<T>* ptr = search_fw(after);
    if ( ptr == 0 ) {
	error("ValList<T>::insertBefore(item, after), cannot find after");
    }

    VLNode<T>* item_node = new VLNode<T>(item, ptr, ptr->bw);

    if ( ptr == head ) head = item_node;
    else ptr->bw->fw = item_node;
    ptr->bw = item_node;

    n_elem++;
    return item_node;
}

template <class T>
void ValList<T>::remove(const T& item)
{
    Pix ptr = search_fw(item);
    if ( ptr == 0 ) {
	error("ValList<T>::remove(item), cannot find item");
    }

    remove(ptr);
}

template <class T>
void ValList<T>::remove(Pix& p)
{
    n_elem--;

    VLNode<T>* ptr = (VLNode<T>*)p;

    // pは後のobjectを指す．
    if ( ptr != tail ) p = ptr->fw;
    else p = 0;

    // ptrを取り除いて，deleteする．
    if ( ptr != head ) ptr->bw->fw = ptr->fw;
    else head = ptr->fw;

    if ( ptr != tail ) ptr->fw->bw = ptr->bw;
    else tail = ptr->bw;

    delete ptr;
}

template <class T>
VLNode<T>* ValList<T>::search_fw(const T& item)
{
    if ( head == 0 ) return 0;
    if ( head->obj == item ) return head;

    VLNode<T>* ptr = head;
    while ( ptr ) {
	if ( ptr->obj == item ) {
	    return ptr;
	}
	ptr = ptr->fw;
    }
    return 0;
}

template <class T>
VLNode<T>* ValList<T>::search_bw(const T& item)
{
    if ( tail == 0 ) return 0;
    if ( tail->obj == item ) return tail;

    VLNode<T>* ptr = tail;
    while ( ptr ) {
	if ( ptr->obj == item ) {
	    return ptr;
	}
	ptr = ptr->bw;
    }
    return 0;
}
