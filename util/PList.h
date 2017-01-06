

template <class T>
class PList{
public:
//  static int objnum;     デバッグ用
    PList<T>*	next;
    T*          body;
//    		PList() : next(nil), body(nil){/*objnum++;*/};
    		PList(PList<T>* n, T* b) : next(n), body(b){/*objnum++;*/};
    		PList(T* b) : next(nil),body(b){/*objnum++;*/};
                ~PList(){clear();/*objnum--;*/}; 

    void	clear();
    PList<T>*    newPList(T* item){
	                   PList<T>* returnPList = new PList(this,item);
	                   return returnPList;
                 };
// 該当するitemを除去し、そのリストを返す。
// 該当するものがなければ、えらーを表示して、thisを返す。
    PList<T>*      PListRemove(T* item);
    bool       includeKa(T* item);   // 含まれてたらtrue
    PList<T>*       apendLastIfNot(T* item);   //含まれていなかったら、最後につける。
    PList<T>*       reverse();
    PList<T>*       apendPList(PList<T>*  apend); 
          //apend を最後にくっつける。これは、ポインタをくっつけてるだけ
};



/*
template <class T>
inline void PList<T>::print(FILE *fp)
{
    PList<T>*  temp;
    for(temp=this;temp != nil;temp=temp->next){
	temp->body->print(fp);
    }
}
*/


template <class T>
inline PList<T>* PList<T>::reverse()
{
    PList<T>* temp = this;
    PList<T>* temppp;
    PList<T>* newPList = nil;
    while (temp != nil  ) {
        temppp = newPList->newPList(temp->body);
	newPList = temppp;
	temp = temp->next;
    }
    delete this;    
  //thisの値はかわららない。また、deleteされててもアクセス可能な場合おおい。
    return newPList;
}


// body の 指すオブジェクト自体のdelete はしてない。
template <class T>
inline void PList<T>::clear()
{
    if(this == nil)return;
    if(next == nil) return;     
    else{
	delete next;
    }

}



// 該当するitemを除去し、そのリストを返す。
// 該当するものがなければ、えらーを表示して、thisを返す。
template <class T>
inline PList<T>*  PList<T>::PListRemove(T* item){
    PList<T>*  temp;
    PList<T>*  backuptemp;
/*    if(this == nil){         nilのエラー処理なし
//	cerr << "PList removing Error -- nil PList \n";
	return this;
    }
*/
    if(this->body == item){
temp = this->next;

       this->next = nil;
       delete this;  

	return temp;
    }
    for(temp=this;temp->next != nil;temp=temp->next){
	if(temp->next->body == item){
	    backuptemp = temp->next;
	    temp->next = temp->next->next;

            backuptemp->next = nil;
	    delete backuptemp;

	    return this;
	}
    } 
//    cerr << "PList removing Error -- Item is not found \n";
    return this;
}

// PList に item が含まれていたら trueを返す。
template <class T>
inline bool  PList<T>::includeKa(T* item){
    PList<T>*  temp;
    for(temp=this;temp != nil;temp=temp->next){
	if(temp->body == item){
	    return true;
	}
    }
    return false;
}


//含まれていなかったら、最後につける。
// そしてそのリストを返す。
template <class T>
PList<T>* PList<T>:: apendLastIfNot(T* item)
{
  if(this==nil){
    return new PList(item);    //うまくいくなか？ 下のテストでおっけい
  }
  PList<T>*  temp;
  for(temp=this;temp->next != nil;temp=temp->next){
    if(temp->body == item){
      return this;          //あるのでなにもしない。
    }
  }
  // この時のtemp は next=nil だが bodyは何か入っててそのチェックはしてない。
  if(temp->body == item){
    return this;          //あるのでなにもしない。
  }
  PList<T>*  newlplist = new PList(item);
  temp->next = newlplist;
  return this;
}

/* テストプログラム
#include "ger-util.h"
int main(int argc, char **argv)
{
  PList<int>*  NotDone = nil;
  int a1 =1;
  int a2 =2;
  int a3 =3;
  int a4 =1;
  NotDone = NotDone->newPList(&a1);
  NotDone = NotDone->newPList(&a2);
  NotDone = NotDone->apendLastIfNot(&a3);
  NotDone = NotDone->apendLastIfNot(&a3);
  NotDone = NotDone->apendLastIfNot(&a4);
  NotDone = NotDone->apendLastIfNot(&a3);
  NotDone = NotDone->apendLastIfNot(&a3);
  NotDone = NotDone->apendLastIfNot(&a1);
  NotDone = NotDone->apendLastIfNot(&a2);
  NotDone = NotDone->apendLastIfNot(&a4);
  PList<int>*  temp;
  for(temp=NotDone;temp!=nil;temp=temp->next){
//    cerr << *(temp->body) << "\n";
  }

}
 */





template <class T>
inline PList<T>* PList<T>::apendPList(PList<T>*  apend)
{
    register PList<T>*  temp;
    if(this==nil)return apend;
    for(temp=this;temp->next != nil;temp=temp->next){
      ;
    } 
    temp->next = apend;
    return this;

}

