#include <stdlib.h>
#include <stdio.h>
#include "IsvList.H"

IsvListBase::IsvListBase(const IsvListBase& orig)
: head(0), tail(0), n_elem(0)
{
    ILNode* ptr = orig.head;
    while ( ptr ) {
	append( ptr );
	ptr = ptr->fw;
    }
}    

void IsvListBase::clear()
{
    ILNode* head_b;
    while ( head ) {
	head_b = head->fw;
	delete head;
	head = head_b;
    }
    n_elem = 0;
    tail = 0;
}

//// 最後尾につく
void IsvListBase::append(ILNode* item)
{
    if ( head == 0 ) {
	head = item;
	tail = item;
    }
    else {
	tail->fw = item;
	item->bw = tail;
	tail = item;
    }
    n_elem++;
}

//// headにつく
void IsvListBase::prepend(ILNode* item)
{
    if ( head == 0 ) {
	head = item;
	tail = item;
    }
    else {
	head->bw = item;
	item->fw = head;
	head = item;
    }
    n_elem++;
}

// beforeの後にitemを挿入
void IsvListBase::insertAfter(ILNode* item, ILNode* before)
{
    item->fw = before->fw;
    item->bw = before;

    if ( before == tail ) tail = item;
    else before->fw->bw = item;
    before->fw = item;

    n_elem++;
}

// afterの前にitemを挿入
void IsvListBase::insertBefore(ILNode* item, ILNode* after)
{
    item->fw = after;
    item->bw = after->bw;

    if ( after == head ) head = item;
    else after->bw->fw = item;
    after->bw = item;

    n_elem++;
}

ILNode* IsvListBase::remove(ILNode*& ptr)
{
    n_elem--;

    ILNode* lptr = ptr; // for local

    // ptrは後のobjectを指す．
    if ( ptr != tail ) ptr = ptr->fw;
    else ptr = 0;

    // lptrを取り除く．
    if ( lptr != head ) lptr->bw->fw = lptr->fw;
    else head = lptr->fw;

    if ( lptr != tail ) lptr->fw->bw = lptr->bw;
    else tail = lptr->bw;

    // lptrを返す．
    return lptr;
}
