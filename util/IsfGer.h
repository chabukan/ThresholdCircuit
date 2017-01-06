//specify = 1 と なる部分を lfでおきかえる。（＊を狭める）
inline void ISF::specify(Bdd* lf, Bdd* specify)
{
    low =low | ( (*specify) & (*lf) ) ;
    up = up & ~( (*specify) & ~(*lf) ) ;
}



//bdd のところを*にする
// low ---> f=1 のところはすべて０に
// up  ---> f=1 のところはすべて1に
inline void ISF::maskISF(Bdd* f)
{
    low = low & ~(*f);
    up =  up | (*f);
}


inline int ISF::topVarId() const{
    int rankl = low.top();   
    int ranku = up.top();    
    if(rankl > ranku){
      return low.top();   // low の最上位の方がグラフの上位なのでその
                         //番号を返す。
      }
    else{
      return up.top();
    }
}


/* レベル lev(varid) の変数に０（１）を入れたISFオブジェクトをつくり返す*/
// 単にlowとupのcofactorをとっているだけ
inline ISF ISF::rstr0(const int lev) const{
    ISF result(low.rstr0(lev),up.rstr0(lev));
    return result;
}

inline ISF ISF::rstr1(const int lev) const{
    ISF result(low.rstr1(lev),up.rstr1(lev));
    return result;
}

// ISF = 1 となる関数を返す。
inline Bdd ISF::pf1() const{
    return low;
}

// ISF = 0 となる関数を返す。
inline Bdd ISF::pf0() const{
//    return ( (~low) & (~up) );
    return (~up) ;
} 

// ISF = 0 または １となる関数を返す。
inline Bdd ISF::pfSpecified(){
    return ( (~up) | low );
}


// ISF = * となる関数を返す。
inline Bdd ISF::pfStar() const{
    return ( (~low) & (up) );
  }

inline int ISF:: IsSatisfy(Bdd lf){
  if( (low <= lf) && (lf<=up)) return 1;
  else return 0;
}


inline int ISF::AllTrue(){
   return( (up == Bdd::one) );
}

inline int ISF::AllFalse(){
   return( (low == Bdd::zero) );
}





inline ISF::ISF():low(Bdd::zero),up(Bdd::zero){;}

inline ISF::ISF(const ISF& oprand) : low(oprand.low),up(oprand.up){;}
inline ISF::ISF(Bdd f0, Bdd f1):low(f0),up(f1){;}    
inline ISF::ISF(Bdd lf):low(lf),up(lf){;}    

//inline ISF::ISF():low(Bdd()),up(Bdd()){}
//inline ISF::ISF(const ISF& oprand) : low(Bdd(oprand.low)),up(Bdd(oprand.up)){}
//inline ISF::ISF(Bdd f0, Bdd f1):low(Bdd(f0)),up(Bdd(f1)){}    
                     // Bddのコピーコンストラクタ呼ばれてるかな？
    
inline ISF ISF::operator = (const ISF oprand) {
    if ( *this != oprand ) {
        low = oprand.low;       
        up  = oprand.up;
    }
    return *this;
}


//inline ISF ISF::operator ~ () const {
//    ISF  result(up,low);                   // low <-> up
//    return result;
//}

inline ISF ISF::operator ~ () const {
  Bdd newlow = pf0();        //さっき0だったところ
  Bdd newup = newlow | pfStar();
    ISF  result(newlow, newup); 
    return result;
}



// low <= up となってなかったら0, なってたら1 
inline int ISF::check(){
    return ((low <= up));
}


// どちらにも 含まれる 許容関数集合を求める。
// そのような集合がなければ、low=Bdd::one ,up=Bdd::zeroとする。
//　そうすればcheckでひっかかるので

inline ISF operator & (const ISF lhs, const ISF rhs) {
    ISF result((lhs.low | rhs.low),(lhs.up & rhs.up));
    if( !(result.check())){
	result.low = Bdd::one;
	result.up = Bdd::zero;
    }
    return result;
}




inline int operator == (const ISF lhs, const ISF rhs) {
    return ((lhs.low == rhs.low) && (lhs.up == rhs.up));
}

inline int operator != (const ISF lhs, const ISF rhs) {
    return (! ((lhs.low == rhs.low) && (lhs.up == rhs.up)) );
}

inline int operator <= (const ISF lhs, const ISF rhs) {
    return ( (rhs.low <= lhs.low) && (lhs.up <= rhs.up) );
}
