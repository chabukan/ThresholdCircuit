////////////////////////////////////////////////////////////////////////////
//Support変数集合を扱うためのクラス

#include <stdio.h>
#include <stdlib.h>
#include <string>

class SupVars{
private:
  Bdd       zbdd;    //zbdd 表現であること     cube より変更
public:
    inline int SupNum() const { return zbdd.lit();};  
//cardでも行けるはず
//  inline int SupNum() const { return zbdd.card();};  
  //  inline Bdd CubeBdd() const { return cube;}; 
  inline Bdd GetZBdd() const { return zbdd;}; 

  // ★★ ZBDD を 渡さなければいけない． lf.varSet()
  inline SupVars(Bdd zbdd);  

  inline SupVars();   //空集合 = Bdd::empty
  inline friend int operator == (const SupVars lhs, const SupVars rhs);  

  inline SupVars delete_sup_vars(SupVars * DeleteSup);
//SupVarsで、 DeleteSupに含まれる変数を削除したSupVarsをつくってかえす
//★自分自身, DeleteSupは変更されない

//merge_Sup を自分とmergeしたsupを新たにつくり返す。
// x1x2x3 + x3x4x5 -> x1x2x3x4x5 を返す。
  inline SupVars merge_sup_vars(SupVars * merge_Sup) const;

  SupVars(const SupVars& orig){
    this->zbdd = orig.zbdd;
  };

  inline int delete_top(); //top の変数の番号を返し、またそれを集合から抜く
         //空集合のときはなにもせずに、０を返す(これをループの判定につかえばいい）
   //★★　要注意　これは this自身を変更するので注意しなバグの原因となるよ。

////////// 
/*    SupVars this の 各変数に対してなにかする コード例

      for(SupVars temp = this; temp.SupNum() > 0 ; ){
         Xi = temp.delete_top();
	
         do something to Xi; 
  
      }
 */
////////// 


//For debug 含まれてる変数番号を全て表示
  void print_sup();

  inline int includeKa(int var_id) const ; 
     //var_id の変数を含んでいたら1を返す。 そうでないなら0


  inline  SupVars     operator = (const SupVars oprand);

//  int SupVars::return_top() const;
  int return_top() ;
       //top の変数の番号を返す。空集合のときは０を返す
       // supvars自体はなにも変更しない。

  inline void add_supVars(int varid); 
     //varid番の変数を追加
     //（すでにあるかのチェックもしてあれば何もせず、warning のみだす)
};



