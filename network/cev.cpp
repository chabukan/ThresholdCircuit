#include "cev.h"

using namespace std;



/*新しいデータに１をセットする関数。アルゴリズム参照*/
void CEV::weight_table::set_veriable_on(weight_table &w_on,weight_table &wn_table,int index){
  unsigned int j;
  for(j=0;j<wn_table.flag.size();j++){
    w_on.flag.push_back(wn_table.flag[j]);
    w_on.w_table.push_back(wn_table.w_table[j]);
  }
  w_on.th_value=wn_table.th_value;
  w_on.flag[index]=1;
  w_on.th_value-=w_on.w_table[index];
   
}

/*新しいデータに０をセットする関数。アルゴリズム参照*/
void CEV::weight_table::set_veriable_off(weight_table &w_off,weight_table &wn_table,int index){
  unsigned int j;
  for(j=0;j<wn_table.flag.size();j++){
    w_off.flag.push_back(wn_table.flag[j]);
    w_off.w_table.push_back(wn_table.w_table[j]);
  }
  w_off.th_value=wn_table.th_value;
  w_off.flag[index]=0;
}

/*CEVから論理式式を求める
引数：CEV,CEVのサイズ 
*/
void CEV::res0(std::vector<bool> a,int s){

  int w;/**/
  int z=0;/*どの文字を使うかのためのカウンタ*/
  string st;

  
  if(r_flag==1){
    result.push_back("+");
  }
  else if(r_flag==0){
    r_flag=1;
  }

  for(w=0;w<s;w++){
    if(a[w]==1){
      //st = '0' + z;
      st = std::to_string(z);
      //st = z;
      result.push_back(st);
    }
    z++;
  }

}



/*ネームスペースを分けもわからず使っている。
  だが、消すとエラーが...*/
/*namespace ThGate_t{
  void find_CEVs();
  void recur_CEV(weight_table &w_table,int index);
  };

  using namespace ThGate_t;
*/

/*init_weight_table()
  初期化を行う。
  入力１：入力数（重みの数）
  入力２：CEVを求めたいしきい値論理
  例：
  ２
  １　１　２
  １１　
*/
void CEV::init_weight_table(weight_table &weight_array){
  //std::cout<<"CEV Search"<<std::endl;
  int n,i;

  //std::cout<<"number of input:";
  //std::cin >> n;
  
  n = weight.size();

  for(i=0;i<n;i++){
    weight_array.flag.push_back(0);
  }

  for(auto input = weight.begin(); input != weight.end(); ++input) {
    weight_array.w_table.push_back(abs(input->second));
    if(input->second < 0)
      T = T + abs(input->second);
  }
  weight_array.th_value=abs(T);
  weight.clear();
  
}

/*recur_cev()
  CEVを見つけた際に_CEVSにそのまま連続で入れていっている
*/

void CEV::recur_CEV(weight_table &w_table, unsigned int index){

  if(index>w_table.flag.size()){
    return;
  }
  unsigned int i;
  vector<bool> temp_CEV;
  if(w_table.th_value<=0){
    for( i=0;i<w_table.flag.size();i++){
      temp_CEV.push_back(w_table.flag[i]);
    }
    int s=temp_CEV.size();
    res0(temp_CEV,s);
			 
    for(i=0;i<w_table.flag.size();i++){
      _CEVs.push_back(temp_CEV[i]);
    }
    v_length=w_table.flag.size();
    v_num++;
    return;
  }
  int w_sum=0;
  for(i=0;i<w_table.w_table.size();i++){
    w_sum+=w_table.w_table[i];
  }

  if(w_sum<w_table.th_value){
    std::cout<<"Weights are not enough."<<std::endl;
    return;
  }

  weight_table w_on;
  weight_table w_off;
  w_on.set_veriable_on(w_on,w_table,index);
  w_off.set_veriable_off(w_off,w_table,index);
  recur_CEV(w_on,index+1);
  recur_CEV(w_off,index+1);
}


/*CEV探す関数*/
std::vector<string> CEV::find_CEVs(){
  const int INDEX=0; 
  weight_table weight_array;  

  //初期化
  v_num=0;
  v_length=0;
  r_flag=0;

  //重みとしきい値の情報を取り出す
  init_weight_table(weight_array);

  //CEVの表作成→論理式作成
  _CEVs.clear();
  recur_CEV(weight_array,INDEX);
  


  /*CEVの表示*/
  /*
  for(i=0;i<(v_num)*(v_length);i++){
    std::cout<<_CEVs[i];
    if((i+1)%v_length==0){
      std::cout<<endl;
    }
  }
  */

  /*式の表示*/

  /*
  unsigned int i;
  for(i = 0; i < result.size(); i++){
    std::cout<<result[i];
  }
  std::cout<<endl;
  */

  return result;

}

//重みをもらう関数
void CEV::get_weight(vector<pair<Node*, int> >& get_w){
  weight = get_w;
}

//しきい値をもらう関数
void CEV::get_th(int get_t){
  T = get_t;
}
