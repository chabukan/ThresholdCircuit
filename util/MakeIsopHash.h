/////////  //以下はmakeISOPのみが使う (hashのため)

const int ISOPhashSize = 1 << 20;


#define DEBUG_ISOP 1

//For Debug
#ifdef DEBUG_ISOP 
extern int ISOPhashHit; 
extern int makeISOPcount;    //for debug  (called の回数)
extern int ISOPhashMissHit;  //Cmapには同じキー値でエントリあったけど、違う場合
#endif DEBUG_ISOP



class ResultPair{
public:
  ResultPair();   //Cmapのデフォルト　として使うやつ
  ResultPair(ISF isf, Sop isop);
  ResultPair(const ResultPair& oprand);
//  ~ResultPair() {}
  ISF isf;
  Sop isop;
  ResultPair   operator = (const ResultPair& oprand);
  inline int Equal(ISF& isf1){ return (isf == isf1);};
 inline friend int   operator != (const ResultPair& lhs, const ResultPair& rhs);
 inline friend int   operator == (const ResultPair& lhs, const ResultPair& rhs);
};

inline int operator == (const ResultPair& lhs, const ResultPair& rhs)
{
  return (lhs.isf == rhs.isf) && (lhs.isop == rhs.isop);
}

inline int operator != (const ResultPair& lhs, const ResultPair& rhs)
{
  return (lhs.isf != rhs.isf) || (lhs.isop != rhs.isop);
}

inline ResultPair::ResultPair(const ResultPair& oprand)
: isf(oprand.isf), isop(oprand.isop) {
 }
  

inline ResultPair::ResultPair(ISF isf, Sop isop) 
: isf(isf), isop(isop) { }

inline ResultPair::ResultPair() 
{ }



inline ResultPair ResultPair::operator = (const ResultPair& oprand) {
  if ( *this != oprand ) {
    isf = oprand.isf;
    isop = oprand.isop;
  }
  return *this;
}




/* 以下は，昔ger-baseにあったやつ．


////////////新しい　ISOP は　Sopクラスで実現しよう　(06/30)
// Bdd -> 非冗長な sop を返すルーチン、これができたら、
// ISOP* ISF::makeISOP() は　obsolete になるし、
//  printIntFunc(FILE * sout, Node * node); なども書き換える必要有り、

// 08/06 hash をつかったバージョン
// ★　ぶつかったときは、あきらめてもいいんやったら、わざわざCmapつかわんでも
//   ResultPair の　配列だけでもよかったな。でも、どうせ早さはそんなにかわらんやろから
//  とりあえず、以下の実現でいいことにしよかな

//  makeISOP 用 hash

inline int  hashfunc(ISF& lf){
  int f = lf.Low().value();
  int g = lf.Up().value();
  if(f==g) return f;
  else return  ( (f)^((f)<<4)^(g)) ;  //hashSize で　Cmapの中でどうせ割られる。
}


inline int  hashfunc2(int& a){
  return a;
}

#ifdef DEBUG_ISOP 
int ISOPhashHit = 0;
int ISOPhashMissHit = 0;  //Cmapには同じキー値でエントリがあるが、違う場合
int makeISOPcount = 0;
#endif DEBUG_ISOP

//  ISF -> int (hashfuncで)　この段階で、違うISFから、同じintできることあり。
//  次に int key -> Cmap -> ResultPair (これは、違うkey から必ず違うResultPair引ける）
//   (でも、違うISFから、おなじkeyできることあるので（あまりないようだが）
//    ResultPair->isf が 引いたISFと同じだったかどうかをチェックする。


ResultPair dft;
//Cmap<int, ResultPair> ISOPhash = Cmap<int, ResultPair>(ResultPair(), hashfunc2, ISOPhashSize); 
Cmap<int, ResultPair> ISOPhash = Cmap<int, ResultPair>(dft, hashfunc2, ISOPhashSize); 
//Cmap 内の ISF, Sop（つまるところBdd)は実際あるものをコピーしている。）
// そのため、ISF SOPなどが例え消されたとしても、hashを引ける。だだし明示的に全ての
//Bddをけすには、以下の ClearMakeISOPHash を呼ばないとだめ。

void ClearMakeIsopHash()
{
//  Bdd::gc();
//  printf("Bdd::objnum = %d\n", Bdd::objnum);
  ISOPhash.clear();
//  Bdd::gc();
//  printf("After Bdd::objnum = %d\n", Bdd::objnum);
}

// ★ Hash ありのバージョン
Sop ISF::makeISOPnew(int* lev2idx)
{
#ifdef DEBUG_ISOP 
  makeISOPcount++;
#endif DEBUG_ISOP

  if(AllFalse()){       // 恒偽関数
    return Sop::zero;
  }
  else if(AllTrue()){
    return Sop::one;  // 恒真関数   
  }   

//Read the Result Table
  int key = hashfunc(*this);
  ResultPair Oldresult = ISOPhash[key];
  if(Oldresult != ResultPair()){       //Cmap のdflt
           //dflt でなければとりあえず、このキーのitemがtableにある。
    if(Oldresult.Equal(*this) == 1){
#ifdef DEBUG_ISOP 
      ISOPhashHit++;
//      cerr << " Hash Hit \n";
#endif DEBUG_ISOP
      return (Oldresult.isop);     //これでコピーしたことになるはず
    }
#ifdef DEBUG_ISOP 
    else{
      ISOPhashMissHit++;
    }
#endif DEBUG_ISOP
  }


  int level=this.topVarId(); //展開すべき変数の番号（rankではない）
//    cerr << "\n*** expandinv id = " << level << "*****\n"
  ISF f0(rstr0(level));
  ISF f1(rstr1(level));
  ISF f00((f0.low & ~(f1.up)),f0.up);
  ISF f11((f1.low & ~(f0.up)),f1.up);
  Sop result_f0  = f00.makeISOPnew();           // isop0に相当
  Sop result_f1  = f11.makeISOPnew();
  Bdd g0 = (result_f0).getFunc(0);
  Bdd g1 = (result_f1).getFunc(0);
  ISF f000((f0.low & ~(g0)),f0.up);
  ISF f111((f1.low & ~(g1)),f1.up);
    ISF isopAlpha( ( (f000.low & f111.up) | (f000.up & f111.low) )
		   , (f000.up & f111.up)); 

  Sop result_fr = isopAlpha.makeISOPnew(); 

  int index;
  if(lev2idx == 0){
    index = level;
  }
  else{
    index = lev2idx[level];
  }
  Sop result = result_f0.and0(index) + result_f1.and1(index) + result_fr;

//write to the result table

  ResultPair NewResult = ResultPair(*this, result);  
  ISOPhash[key] = NewResult;   //代入時　左辺free するはず

  return result;
}
*/


