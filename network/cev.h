#ifndef __CEV_H__
#define __CEV_H__

#include <vector>
#include <string>
#include <map>
#include <cstdlib>

#include "node.h"
#include "network.h"

using namespace std;

class CEV {

private:
  vector<pair<Node*, int> > weight; //重みを格納
  int T; //しきい値を格納

  std::vector<bool> _CEVs;/*CEVを格納*/
  std::vector<string> result;/*論理式が格納される*/
  
  int v_num; /*CEV表示のために使っている変数*/
  int v_length; /*同上*/
  int r_flag; /*１になるとresultに"+"が入るようになる*/

  /*データ構造
    重み、しきい値、０，１入力
    ０をセットする関数と１をセットする関数(なんでここにはいってんねん）←修正予定*/
  struct weight_table{
    std::vector<int> w_table;
    int th_value;
    std::vector<bool> flag;  
    void set_veriable_on(weight_table &w_on,weight_table &wn_table,int index);
    void set_veriable_off(weight_table &w_off,weight_table &wn_table,int index);

  };

  void set_veriable_on(weight_table &w_on, weight_table &wn_table, int index);
  void set_veriable_off(weight_table &w_off,weight_table &wn_table,int index);
  void res0(std::vector<bool> a,int s);
  void init_weight_table(weight_table &weight_array);
  void recur_CEV(weight_table &w_table,unsigned int index);

public:
  std::vector<string> find_CEVs();
  void get_weight(vector<pair<Node*, int> >& get_w); //ノードから重みをもらう
  void get_th(int get_t);//ノードからしきい値をもらう

};

#endif
