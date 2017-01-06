#ifndef __NODE_H__
#define __NODE_H__

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iterator>
#include <unordered_set>

#include "factor.h" // bddディレクトリにあり

#include "utility.h"
#include "network.h"
#include "general.h"

#include <cudd.h>
#include <cuddObj.hh>

using namespace std;

class Network;

class Node{
public:
    /*------ name，id，type, parent ------*/
    // todo: c_str()の方は将来消していいかも
    inline const char* name() { return nameRep.c_str(); } // 名前
    inline std::string getName() { return nameRep; }
    inline void        clearName() { nameRep.resize(0); }
    inline int         getId() { return id; }             // Networkクラスによってつけられる
    inline int         getUniqueId() { return uniqueId; } // 一意性を持つ
    inline node_types  getType() { return typeRep; }
    inline void        setType(node_types tpe) { typeRep = tpe; }
    inline Network*    parentNet() { return parent; } // 所属する Network

    /*------ input, output の情報 ------*/
    inline int nIn() { return input.size(); }  // 入力数
    inline int nOut() { return output.size(); }; // 出力数

    inline vector<Node*>& getInput() { return input; }     // 入力の配列を返す
    inline vector<Node*>& getOutput() { return output; }     // 入力の配列を返す
    inline vector<int>&      getLev2idx() { return lev2idx; } // lev2idxを返す

    /*------ 内部論理関数 ------*/
    inline Bdd  getFunc() const { return func; }    // 内部論理関数を見る
    // todo: what is cset
    inline Sop  getCset() const { return cset; }    // cube setを見る
    inline int  getPolarity() const { return polarity; } // polarityを見る

    /*------ データの print ------*/
    // todo: for debug
    void dprint() { print(1); }
    void print(int rec_flag);
    void printBlif(FILE *fp);
    void printName(FILE *fp);

    // nodeの内部論理をblif 形式で表示する。
    // lev2index を 見て 変数番号と fan-in との関係をしらべ、
    // fanin の1番目がblif出力の1番に対応するようにしてる。
    // ★func は ちゃんとcompressされてること
    // ★また、level2index もちゃんと設定されていること
    void printIntFunc(FILE * sout, Node * node);

    //出力ノードの1つ前にたいしてだけこれが呼ばれる
    void printBlif_before_output(FILE *fp);

    //★  ファイルに出力しても 何も変更しないバージョン
    void printBlif_before_output_no_change(FILE *fp);
    void printBlif_no_change(FILE *fp);

    // しきい値の情報
    inline vector<pair<Node*, int> >& getWeight() { return weight; } //重みを返す
    inline int getTh() { return T; }
    //inline vector<Node*>& getPiNet() { return pi_net; } 
    inline std::unordered_set<Node*>& getPiNet() { return pi_net; } 

    int             start_var;
    BDD             func_cudd;
    friend class Network;

private:
    enum node_types typeRep;  // type of Node
    int             id;       // id number
    int             uniqueId; // guarantees uniqueness (from 1)
    Network*        parent;   // 所属する Network
    std::string     nameRep;  // name

    vector<Node*>   input;    // inputs  connect to
    vector<Node*>   output;   // outputs connect to

    Bdd             func;     // 内部論理関数
    Sop             cset;     // cube set
    Factor          fac;      // factored form
    int             polarity; // 出力の極性[0反転,1そのまま(default)]
    // csetとfacのみに影響，funcには既に反映
    // これらの変数順はすべて同じで，lev2idxで表現
    vector<int>        lev2idx;

    //しきい値用
    vector<pair<Node*, int> > weight;
    int T;
    std::vector<string> th_func;
    //vector<Node*>   pi_net; //そのノードにつながりのあるPI
    std::unordered_set<Node*>   pi_net;
    bool one_dontc;  //[**11]のノードであることをチェック
    bool zero_dontc; //[**00]のノードであることをチェック
    std::vector<Node*> plus_spare; //１本目の代替候補
    std::vector<Node*> minus_spare; //2本目の代替候補
    bool trans_checked;
    bool candi_checked;


    /*------ 生成，削除 ------*/
    Node(node_types tpe); // 生成
    Node(const Node& op); // コピー
    virtual ~Node();      // 多Nodeとの接続，Networkへの登録を気にせず消滅
    static int nObj;      // 存在するオブジェクトの数
    static int nBorn;     // 生成されたオブジェクトの数 -> uniqueIdに反映

    /*------ input, output の変更 (ダブルリンクsafeなもの) ------*/
    void  collapse(Node* fanin);     // faninを使わずに表現
    int   collapseTest(Node* fanin); // Factorのリテラルの減少数を返す
    void  delDupInput();             // 同じ入力があれば削除
    void  changeInput(Node* before, Node* after);  //入力を置き換える
    void  replaceNode(Node* replaced, int alsoName=1); // replacedをthisで置き換える．alsoNameでnameのコピー
    void  connect(Node* fanin);     // OUTPUT, XOR, ANDのみ
    void  disconnect(Node* fanin);  // OUTPUTのみ．将来的にXOR, ANDも．

    void  recLetNouse(); // 出力数が0であるNodeを削除し，再帰的に不必要なNodeを削除する
    int   del_1input();  // 入力数が1以下であるものに対し，その出力側を変更する．

    Node* mergeSameType();     // 入力に同じType(XOR,AND)のNodeがあれば一つにする
    int   mergedIfUsedUnate(); // fanoutが1つで，fanout先での自分に対するリテラルがunateであればmerge
    int   eliminate();
    // 自分のfanoutが1つで，fanout先での自分に対するリテラルが，
    // factored formで1つであればmerge

    /*------ 内部論理関数 ------*/
    Sop  makeCsetIfNot(); // csetを返す．なければ作る．
    Bdd  makeFuncIfNot(); // funcを返す．なければ作る．

    int  trimLev2idx();   // 必要なだけの長さにして，最大のlevelを返す
    void compressFunc();  // 内部関数を compress して lev2idx を変更

    vector<int> reorderLev2idx(); // cubeOrder()を使って変数順を変える．変数順の変更を返す．

    // todo: 使える
    void invFunc();             // 内部関数を反転する
    void invInput(Node* fanin); // fanin入力の極性を反転

    // node2levelを変数として
    Bdd  calcFunc(const unordered_map<void*,int>& node2level);
    Sop  calcSop(const unordered_map<void*,int>& node2level);
    Sop  calcSop_first(const unordered_map<void*,int>& node2level);

    static int isSameFunc(Node* ni, Node* nj);

    int  nLitFac(); // factored formのリテラル数を返す
    int  nLitSop(); // sum-of-productsのリテラル数を返す

    Bdd  calcFunc_sub(unordered_map<void*,Bdd>& fanin2func);
    // fanin2funcにfaninの関数が入っていること

    /*------ 内部関数と入力を同時に変更 ------*/
    void makeConstant(const Bdd& f); // CONSTANTにする

    // level2node と f or cs に従って，入力をつなぎ，内部関数を更新
    // todo: marimo: makeMAJを作る
    void makeLUT(const Bdd& f, const vector<Node*>& level2node);
    void makeSOP(const Sop& cs, int pola, const vector<Node*>& level2node);

    // base_n の入力リストと f or cs に従って，入力をつなぎ，内部関数を更新
    void makeLUT(const Bdd& f, Node* base_n);
    void makeSOP(const Sop& cs, int pola, Node* base_n);

    // todo: node-ger.h
    //引数の仕様が少し違う
    // level2index は 0 ではだめで，必ず与えるべし
    void makeLUT(const Bdd& f, const vector<Node*> varlist, int* level2index);
    // todo: ----------node-ger.h

    // f or cs に内部関数を更新．
    // new_nは今までにない入力，new_n_levはそのlevel．
    // todo: いる？
    void updateLUT(const Bdd& f, Node* new_n, int new_n_lev);
    void updateSOP(const Sop& cs, int pola, Node* new_n, int new_n_lev);

    void getFaninCorn(vector<Node*>& result); // 入力側のNodeのリスト

    // todo: ----------node-ger.h

    /*------------------------------------*/
    /*------ 以下はobsoleteにしたい ------*/
    /*------------------------------------*/

    //    void    connectInput(Bdd f, Arrayw<Node*>& varlist, int* lev2idx=0);
    void connectInput(Bdd f, vector<Node*>& varlist, int* lev2idx);
    // Nodeの内部論理をfとし，fにおける各変数のレベル lev に対して，
    // 入力に varlist[ lev2idx[ lev ] ] をつなぐ
    //   varlist は index から fanin を返す
    //   lev2idx は varlist における indexを返す (lev2idx==0の時はlevelがindex)

    // -> makeLUT()やmakeSOP等で．


    // todo: node-ger.h

    // todo: what is IntDC
    inline Bdd getIntDC() { return IntDC; }
    inline void SetIntDC(Bdd set) { IntDC = set; }

    void compressFuncAndIntDC();

    /*------ successor, predecessor ------*/
    //  is Node が immediate successorなら true をかえす。
    // 出力を持ってたらIS
    inline bool isImmediateSuccessor(Node * isNode) {
        if (outContain(isNode) == true) return true;
        else return false;
    }
    //  is Node が successorなら true をかえす。
    bool isSuccessor(Node* isNode);
    //  is Node が immediate predecessorなら true をかえす。
    inline bool isImmediatePredecessor(Node * ipNode) {
        if(fanin2index(ipNode) == -1) return false;
        else return true;
    }
    //  is Node が predecessorなら true をかえす。
    bool isPredecessor(Node* ipNode);

    //    void	printInfo(FILE *sout, int mode=0);//nodeの情報を出力(For Debug)
    //    void	printInfo(FILE *sout, int mode=0, int mode2=0);//nodeの情報を出力(For Debug)
    void printInfo(FILE *sout, int mode, int mode2);//nodeの情報を出力(For Debug)

    // ----------node-ger.h

    void check(int numRecursive); // 入出力が正しいtypeかをチェック
    void delAllInput();           // 入力をすべて削除，双方向とも

    /*------ input, output の情報 ------*/
    //  level: funcにおけるBddのレベル int 1から
    //     index: 入力につけられた番号       int 0からnIn()-1
    //     fanin: 入力のNode          Node*
    //     lev2idx:     level -> index
    //     input:       index -> fanin
    // todo: レベルはBDDの変数順序?
    int   fanin2level(Node* fanin); // faninのlevel． 無いときは0
    int   fanin2index(Node* fanin); // faninのindex．無いときは-1
    Node* level2fanin(int level);   // レベルがlevelであるfanin，無いときは0
    int   level2index(int level);   // レベルがlevelであるfaninのindex
    int   index2level(int index);   // in(index)のlevel．無いときは0
    inline Node* index2fanin(int index) { return input[index]; }

    static int compareInputSet(Node* ni, Node* nj, unordered_map<void*,int>& node2level);
    // ni と nj の入力集合を比べる．
    // ni のが nj のを含むとき(同じときも)，1を返す．

    bool outContain(Node* fanout); // fanout が出力に含まれてるか

    // input, output の変更 (自分のリンクだけ変更)
    inline void addOutput(Node* fanout) { output.push_back(fanout); }
    inline void delOutput(Node* fanout) { output.erase(find(output.begin(), output.end(), fanout)); }
    inline void clearInput() { input.clear();input.shrink_to_fit(); }    // 入力を消去
    inline void clearOutput() { output.clear(); }  // 出力を消去

    // faninからthisへのリンク，登録されていたらあり，なければなし
    void addLink(Node* fanin); // リンク加える
    void delLink(Node* fanin); // リンクを削除する

    void connectInputSub(const Bdd& varset,
                          vector<Node*> varlist, int* level2index);
    void connectInputSub(const Bdd& varset, const vector<Node*>& level2node);
    void connectInputCore(vector<Node*> node_list, vector<int> level_list);

    Bdd  rec_calcFunc(unordered_map<void*,Bdd>& node2func);
    Sop  calcSop_sub(unordered_map<void*,Sop>& node2func);

    static void assignLevel(Node* fn, Node* gn, unordered_map<void*,int>& node2level);

    //     void    delOneInput(); // 1入力であるものを削除してその入出力をつなぐ

    //    friend class Network;
    // friend void Network::regNode(Node* node);
    // friend void Network::regPI(Node* node);
    // friend void Network::regPO(Node* node);

    // todo: node-ger.h

    //★このノードの出力のうち１つでも出力端子につながっていればtrue
    bool checkBeforeOutput();
    Bdd IntDC; //内部関数のdon't care  -> NetDecompSDD で使用(以下のpublicのみ)
    //★★ デフォルトは  Bdd::NULL なので 明示的に初期化必要

    // todo: ----------node-ger.h
};

#endif // #ifndef __NODE_H__

