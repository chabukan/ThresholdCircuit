//#define DEBUG_ISOP 1

#ifdef DEBUG_ISOP 
  static int makeISOPcount;
  static int  ISOPhashHit;
#endif 

//★ IsfFD の　support_min は，IsfLU にしてから求める．

//(0, 1, *)を用いて 真理値表（グレイコード)に表示 
//8入力以下の関数しか表示しない．明示的に，n_inの指定をしないとだめ．
// (X)と表示されるのは，low = 0, up=1　のようなあり得ないビットを意味する．

//  void Karnaugh_Map(FILE *sout, int n_in){
//      Print_Karnaugh_Map(this, sout, n_in, " 0 * X 1");
//   }

void Print_Karnaugh_Map(FILE *sout, int n_in) const;

//stderr に 論理式を表示
void PrintStderr() const;


//int IsfLU::topVarId() const{
//int IsfLU::topVarId() {
int topVarId() {
  int rankl = getL().top();
  int ranku = getU().top();
  if(rankl > ranku){
    return rankl;   // low の最上位の方がグラフの上位なのでその番号を返す。
  }
  else{
    return ranku;
  }
}




// bit1  <= bit2 となってなかったら0, なってたら1 
// IsfLU の論理的な整合性をチェックする．
 int check(){
    return ((Bit1() <= Bit2()));
}


// X_lev = 0 とした　IsfLU を　新しく作って返す
IsfLU rstr0(const int lev) const{
    IsfLU result(Bit1().rstr0(lev), Bit2().rstr0(lev));
    return result;
}

// X_lev = 1 とした　IsfLU を　新しく作って返す
IsfLU rstr1(const int lev) const{
    IsfLU result(Bit1().rstr1(lev), Bit2().rstr1(lev));
    return result;
}


// IsfLU = 1 となる関数(Bdd)を返す。
Bdd OnSet() const{
    return Bit1();
}

// IsfLU = 0 となる関数を返す。
Bdd OffSet() const{
    return (~Bit2()) ;
} 

// IsfLU = 0 または １となる関数を返す。
Bdd CareSet(){
    return ( (~Bit2()) | Bit1() );
}

// IsfLU = * となる関数を返す。
Bdd DontCareSet() const{
    return ( (~Bit1()) & (Bit2()) );
}

//lf ば IsfLU に含まれる関数であれば trueを返す．
bool  includeFunc (const Bdd& lf) const{
  if( (Bit1() <= lf) && (lf<=Bit2())) return true;
  else return false;
}

//IsfLU が 完全なドントケアを表現するなら，１
int AllDC() const{
   return( (Bit1() == Bdd::zero) && (Bit2() == Bdd::one) );
}


//IsfLU が 恒真関数を含んでいたら，１
int AllTrue(){
   return( (Bit2() == Bdd::one) );
}

//IsfLU が 恒偽関数を含んでいたら，１
int AllFalse(){
   return( (Bit1() == Bdd::zero) );
}


//IsfLU の　否定を表すIsfLU を　新しく作って返す
// 0 -> 1, 1-> 0 *->* となる．
IsfLU operator ~ () const {
  Bdd newlow = OffSet();        //さっき0だったところ
  Bdd newup = newlow | DontCareSet();
  IsfLU  result(newlow, newup); 
  return result;
}


Sop makeISOP(){return this->makeISOP(0);} 
//●IsfLU -> ISOP を作って返す．

Sop makeISOP(Arrayint lev2idx){return this->makeISOP(lev2idx.getPtr());} 
//●IsfLU -> ISOP を作って返す．この際に，IsfLUの中でのBddの変数番号を lev2idxで
// 与えられる規則で読み替えて，ISOPをつくる．
//  (lev2idx[2]=3 なら，IsfLU でx2 が Sopでは，x3と扱われる．

Sop makeISOP(int* lev2idx);  //上の2つの実体はこれ


// どちらにも 含まれる 許容関数集合を求める。
// そのような集合がなければ、low=Bdd::one ,up=Bdd::zeroとする。
//　そうすればcheckでひっかかるので


friend IsfLU operator & (const IsfLU& lhs, const IsfLU& rhs)
{
IsfLU result((lhs.Bit1() | rhs.Bit1()),(lhs.Bit2() & rhs.Bit2()));
if( !(result.check())){
result.change_bit1(Bdd::one);
	result.change_bit2(Bdd::zero);
    }
    return result;
}


friend int operator <= (const IsfLU& lhs, const IsfLU& rhs) {
    return ( (rhs.Bit1() <= lhs.Bit1()) && (lhs.Bit2() <= rhs.Bit2()) );
}



/*

friend IsfLU support_min(IsfLU* f,int sup,int* varTable,int* nsup, int* varResult);
// 不完全指定論理関数 f の support数最小の不完全指定論理関数を返す。
// sup   --- 最低でも関係している変数の数
// varTable -- 最低でも関係している変数番号（BDDの番号なので、１から）
// varTable[0]-varTable[sup-1] の配列
// nsup -- 最小のsupport数
// varResult[i] = 0 なら varTable[i] の変数は使用される。
// varResult[i] = 1 なら varTable[i] の変数は使用されない。
// この、０、１も設定する。
// varTable[sup-1] から 除けるかチェックしていくので、BDDの変数が昇順にこの
// 配列に入っている方が処理時間が早くなることが期待される

 */
