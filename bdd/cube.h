#ifndef _cube_h
#define _cube_h

#include <stdio.h>
#include <string.h>
#include "IsvList.H"
#include "arrayint.h"
#include "bddop.h"
class Sop;

#define isPosiLit(lit) ((lit) >= 0)
#define isNegaLit(lit) ((lit) < 0)
#define rmNegaLit(lit) (((lit) < 0) ? ~(lit) : (lit))

class Cube : public ILNode {    // 1出力を仮定，添字は0からでNode.inputと同じ．
public:
	Cube(int n_in) : size(n_in), rep( new char[n_in+1] ) {
	    for ( int i = n_in; --i>=0; ) rep[i] = 'x';
	    rep[n_in] = '\0';
	}
	Cube(int n_in, const char* val)
    	: size(n_in), rep( new char[n_in+1] ) { strcpy(rep, val); }
    	Cube(const Cube& orig) 
    	: size(orig.size), rep( new char[orig.size +1] ) {
	    strcpy(rep, orig.rep);
	}
    	virtual ~Cube() { delete [] rep; }

    int operator == (const Cube& obj);

    void	setValue(const char* val) { strcpy(rep, val); }
    char	value(int idx) { return rep[idx]; }
    void	setValue(int idx, char value) { rep[idx] = value; }
    void	rstr(int varIdx);	// varIdx: 1そのまま 0否定~
    void	jumpup(int i, int j); // インデックスiをjにまでjumpup
    int		empty() { return size <= 0 ; }
    void	print(FILE* sout=stdout, char* postString=(char*)"",
		      int* permutation=0);
	// rep[i] が permutation[i]の場所に表示される．0の場合は i の場所に．
    Bdd		getBdd(Arrayint& lev2idx);

private:
    int		size;	// 配列の長さ(依存する変数の数ではない)
    char*	rep;	// in[0]からはじまる．
			// 最後に'0'を入れるので，size+1の長さ．
};

class CubeSet {
public:
    	CubeSet(int nIn) : numIn(nIn) {}
    	CubeSet(const CubeSet& orig) : numIn(orig.numIn), f1(orig.f1) {};
    	CubeSet(int nIn, const Sop& origSop);
    	~CubeSet() {}

    void	rstr(int varIdx);	// varIdx: 1そのまま 0否定~
    // scope内で最も多く出現する変数を，freqLitに入れる．
    // リテラルの正/負は，正:freqLit，負:~freqLit
    // freqLit変数の出現回数を返す．
    void	compress();	// 関係のない変数を消す
    void	jumpup(int i, int j); // インデックスiをjにまでjumpup
    int		nIn() { return numIn; }
    int		nCube() { return f1.size(); }
    void	print(FILE* sout=stdout, char* postString=(char*)"",
		      int* permutation=0);
    Bdd		getBdd(Arrayint& lev2idx);

    static void	printSop(Sop f, FILE* sout=stderr,
                             char* postString = (char*)"", int* var2idx=0);
    // var2idx: Sopのvar(1から始まる)から，表示時のidx(0から始まる)への写像
    static void printSop(const Bdd& f, FILE* sout=stderr, int* var2idx=0);


  // -------- adding by ger 1998 06 29  ------------
  // for ( Cube* ptr = a.first(); ptr; a.next(ptr) ) { use ptr; }
  Cube*          first() const { return (Cube*)f1.first(); }
  void        next(Cube*& ptr) const {
    f1.next(ptr);
  }
  // -------- adding by ger 1998 06 29  ------------



private:
    void	makeCubeSop(int nin, const Sop& f, Cube* nowCube, int nowno);

    int			numIn;
    IsvList<Cube>	f1;
};

#endif
