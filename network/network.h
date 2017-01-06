#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <stdio.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <iterator>
#include <list>
#include <time.h>

#include "utility.h"
#include "general.h"
#include "node.h"
#include "ger-base-old.h"    //Please add this

// for cudd
#include <util.h>
#include <cudd.h>
#include <cuddObj.hh>

using namespace std;

class Node;

class Network
{
    public:
    enum if_format_t { BLIF, PLA };
    if_format_t         if_format;

    /*------ 生成，削除 ------*/
    Network();
    virtual    ~Network();

    /*------ PLA: 読み込み, BLIF: 読み込み・書き出し ------*/
    void readFilePla(FILE *fp, const char* fn);
    void readFileBlif(FILE* filep, const char* module);
    void outBlif(FILE *fp);
    
    //しきい値回路のファイルの読み込み
    void readFileTh(FILE *fp, const char* fn);

    /*------ primary output の関数計算 ------*/
    void    calAllSpecPla(char swBddOrder='1');
    void    calAllSpecBlif(char swBddOrder='1');  
    //しきい値回路のoutputの関数計算
    void    calAllSpecTh(char swBddOrder='1');

    // outfuncsとspecPOが作られる．
    // 内部で (pla,blif)PIorderが呼ばれるため，lev2piidxとvariablesも作られる
    // todo: for debug
    // todo: 消す
    int    verify(); // POのoutfがspecPOを満たすかを確かめる．

    /*------ Network全体の管理 ------*/
    // marimo: cleanは使用タイミングに注意
    // todo: cleanのみ使用されてる
    void    clean();    // nOut() == 0 を削除
    void    sweep();    // 定数Node，1入力Nodeなどを削除
    void    sweepClean() { sweep(); clean(); }
    void    delAllNode();    // 内部ノードをすべて消す
    void    elimDupNode();   // 同じ入力，同じ内部関数のNodeを削除
    // todo: 例：2入力ノードが２つあるとき，3入力ノードにしたほうが良い場合に合体
    void    collapseSmallValue(); // valueの小さいNodeをcollapse
    void    collapseUsedUnate();  // unateでしか使われていないNodeをcollapse
    void    forceSameNameBeforePO(); // 外部出力OUTPUTに直結しているノードの名前をOUTPUTと同じにする

    /*------ 各種データを見る ------*/
    inline int nPI() const { return primaryI.size()-1; } // 回路の入力数
    inline int nPO() const { return primaryO.size()-1; } // 回路の出力数
    inline int nNode() const { return num_node; }         // 回路の内部ノード数
    inline int nConnect() const { return num_connect; }   // 回路の結線数
    inline int nLevel() const { return num_level; }       // 回路のレベル
    int     literalCount();    // それぞれのノードのsopに使われているリテラルの数の総和
    int     literalCountFac();

    inline const char* name() { return nameRep.c_str(); };
    // PI, POを見る
    inline Node* getPI(int idx) const { return primaryI[idx]; };
    inline Node* getPO(int idx) const { return primaryO[idx]; };

    // 定数Node
    void        makeConstantNodes(); // 定数Nodeを作る
    inline Node*       getConstant0() { return constant0; } // 作られていなかったら
    inline Node*       getConstant1() { return constant1; } // 0を返す．

    inline vector<int>    getLev2piidx() { return lev2piidx; } // indexは1から始まる
    vector<int>    getLev2IdxFrom0(); // indexは0から始まる

    // todo: for debug
    void        printAllNode();    // デバック用
    void        printCost(char *msg);

    /*------ outfunc, idepth, odepth ------*/
    void    calcOutfuncAll();
    void    calcOutfuncAll_Cudd(); // cudd
    void    calcIdepthAll();
    void    calcOdepthAll();
    void    clearOutfuncAll();
    void    clearIdepthAll();
    void    clearOdepthAll();

    void    trimNodeName();
    void    copyPOname();


    //しきい値回路のやつ
    void    setCSPF_AllTh();
    void    transcircTh(); 

    inline unordered_map<void*,int>& getIdepths(){ return idepths;}
    inline unordered_map<void*,int>& getOdepths(){ return odepths;}

    /*
     ++------ cvarset (回路的に依存している変数集合) circuit var set ------++
     cvarset: 未定義はBdd::null，変数を持つNodeはその変数，その他は再帰的計算

     [使用例]
     Cmap<void*,Bdd> cvarset(Bdd::null, ptrHash);
     set_cvar(variables, cvarset);
     for ( Node* no = firstPO(); no; no = nextPO() )
     cal_cvarset( no, cvarset );
     */
    Bdd  calcCvarset(Node* no, unordered_map<void*,Bdd>& cvarset);
    // cvarset[no]に結果が入り，同じものが返る．
    void clearCvarset(Node* no, unordered_map<void*,Bdd>& cvarset);

    void setCvar(unordered_map<void*,Bdd>& cvarset);
    // Bdd::var( variables[node] ) を cvarset[node] にセットする

    //以下のは，セットされているかに関わらず★自分につい再計算する
    //★もし自分が変化した場合のみ，自分の後段についても同じことをする．
    // 再計算を高速に行いたい時などに利用する．
    int recalcIdepthRecursive(Node* no);
    int recalcOdepthRecursive(Node* no);
    Bdd recalcOutfuncRecursive(Node* no);

    // todo: ilev -> idepthに置換 (同名関数があって死ぬので保留)
    // todo: nodeとnetworkでidepthやodepthをそれぞれ管理しようとしているように見える問題
    void    calcIlevAll();
    // network内の全てのNode のidepthをセット
    // idepth = -1であるのは無効であると思って再計算する
    //一番最初はclearIlevAll()を読んでから、これを呼ぶ
    //計算したついでに、num_levelをセットしてまたその値を返す(不要??)

    void    clearIlevAll();
    //全てのnodeのidepthを無効化する (=-1)

    int    calcOlevAll();
    // network内の全てのNode のodepthをセット
    // odepth = -1であるのは無効であると思って再計算する
    //計算したついでに、num_levelをセットしてまたその値を返す(不要??)

    void    clearOlevAll();
    //全てのnodeのodepthを無効化する (=-1)

    //Inputs, Outputs は Arrayの0番目からNode ＊をいれて おくこと
    //対応するノードの名前には、"Sub-"が付加される
    Network*    createSubNet( vector<Node*> Inputs, vector<Node*> Outputs);
    Network*    createSubNet( Node* node );

    void   insertSubNet(vector<Node*> Inputs, vector<Node*> Outputs,
                         Network* SubNet);
    void   insertSubNet(Node* node, Network* SubNet);
    vector<Node*>  insertSubNetWithNode( Node* node,  Network* SubNet,
                                         vector<Node*> Correspond);
    vector<Node*> insertSubNetWithNode(vector<Node*> Inputs, vector<Node*> Outputs,
                                       Network* SubNet, vector<Node*> Correspond);
    int    calcNumConnection();
    void   compressIntNode();

    // ファイルname (openする)を渡して、そのファイルから読み込んで
    // network をつくる
    // 前のを破壊しないので，new してすぐにこれを呼ぶべき
    // file が openできなければ exit(1)
    // Bdd が初期化されてないといけない。
    void readFileName(const char * filename);

    // ファイルname (openする)を渡して、そのファイルをopenして、
    // Blif形式で書き込む file が openできなければ retun 1
    // 書き込めればreturn 0
    int printFileNameBlif(const char * filename);

    //node compressfunc しないバージョン
    void printBlifNoChange(FILE *fp);

    //i番目の SpecPO を返す。
    inline Bdd  getSpecPO(int i) {return specPO[i];}

    // todo: for debug
    // Debug 用 stderr に出力する．DD’ does
    void printInfoForDebug();

    // todo: for debug
    //一応計算しなおす。(sweepCleanをする。)
    void printInfo(FILE *fp)
    {
        sweepClean();
        clearIlevAll();
        //fprintf(fp, " & %d & %d & %d \n", nNode(), calcNumConnection() - nPO(),  calcIlevAll()-1);
    }

    //protected -> public に変更
    inline vector<Node*>&    getPIlist() { return primaryI; }
    inline vector<Node*>&    getPOlist() { return primaryO; }


    /*------ Node の生成，削除 ------*/
    Node*   createNode(enum node_types tpe, const std::string& name="");
    Node*   createPI(const std::string& name="");
    Node*   createPO(const std::string& name="");
    void    delNode(Node* node);    // 削除されるNodeは，出力数が0
    void    delPI(Node* node);      // 削除されるNodeは，出力数が0
    void    delPO(Node* node);

    friend class Node;

protected:
    void    dfsOrder(list<Node*>& vlist); // todo: 謎

    /* (pla,blif) PIorder によって計算される */
    vector<int>        lev2piidx;  // BddのlevelからPIの番号
    unordered_map<void*,int>    variables;  // Node毎の変数のBddのlevel(設定されていれば)

    /* calAllSpec(Pla,Blif) によって計算される */
    unordered_map<void*, Bdd>    outfuncs;    // Node毎のoutfunc.
    vector<Bdd>         specPO;      // POが満たすべき関数．不完全指定．

    unordered_map<void*, int>    idepths;    // 外部入力からの段数の最大値
    unordered_map<void*, int>    odepths;    // 外部出力までの段数の最大値

    /* CUDD用のoutfuncs */
    unordered_map<void*, BDD*>    outfuncs_cudd;    // Node毎のoutfunc.
    Cudd mgr;
    int pi_count;

private:
    // CSPF ---
    typedef pair<Node*, Node*> NodePair; // mine
    typedef struct {
        Bdd f0; // 0にしなければならない場所が1
        Bdd f1;
    } CSPF;
    unordered_map<Node*, CSPF> node2cspf; // node -> cspf
    map<NodePair, CSPF> con2cspf; // 結線 -> cspf

    //Cudd用CSPF
    typedef struct {
        BDD* f0; // 0にしなければならない場所が1
        BDD* f1;
    } CSPF_Cudd;
    unordered_map<Node*, CSPF_Cudd> node2cspfcudd; // node -> cspf
    map<NodePair, CSPF_Cudd> con2cspfcudd; // 結線 -> cspf


    // --- CSPF

    std::string nameRep; // networkの名前

    vector<Node*> primaryI;
    vector<Node*> primaryO;  // primaryO[1] .. primaryO[numPO]
    vector<Node*> intNode;   // intNode[1] ...
    Node*         constant0; // marimo: ?
    Node*         constant1; // marimo: ?

    SopM        allcube;      // 変数順はファイルの通り
    vector<Sop> sopPO;        // POのSop，変数順はlev2piidxによる

    int         num_connect;   // 回路の結線数
    int         num_level;     // 回路の最大パス長
    int         num_node;      // ノードの数

    bool PIorig_name;
    bool POorig_name;

    unordered_map<std::string,Node*>*    name2node;   // 名前の一意性の保証のため
    enum              { DELETED_NODE = -1 }; // todo: 型
    // 一度登録されたが削除されたNodeを表す

    int new_node_nameID;

    // todo: 型変換（char* -> string?）
    char* aline;
    int   alineSize;
    int   row;

    void  createNameIfNot(Node* node);
    void  checkChangeName(Node* node);

    inline void setName(const char* nm) { // 名前の設定
        nameRep = std::string(nm);
        nameRep += '\0';
    };

    // CSPF ---
    // 設定用
    void setCSPF(Node* node);
    void setCSPF_PO();
    void setCSPF_All();
    void clearCSPF();
    void mergeCSPF(Node* node);
    void propagateCSPF(Node* node);

    // 判定用
    // nodeのCSPFがfを包含するか判定
    inline bool isIncludeFunc(Bdd f, Node* node) {
        return ((f | ~node2cspf[node].f1) == Bdd::one) &&
            ((f & node2cspf[node].f0) == Bdd::zero);
    }
    // --- CPSF



    // しきい値回路のCSPF
    void setCSPFTh(Node* node);
    void propagateCSPFTh(Node* node);
    Bdd  RevTargetInput(Node* node, Node* fin, vector<Bdd> upifunc, int count);

    //Cudd ver
    void setCSPFThCudd(Node* node);
    void mergeCSPFCudd(Node* node);
    void propagateCSPFThCudd(Node* node);
    BDD  RevTargetInputCudd(Node* node, Node* fin, const vector<BDD>& upifunc, int count);

    // ADD ver
    void propagateCSPFThCuddAdd(Node* node);

    // plaの回路を作るときに使用
    void createIlb(int n_pi, std::string *pi_names);
    void createOb(int n_po, std::string *po_names);
    SopM readCube(FILE *fp, int numPI, int numPO);

    // blifの回路を作るときに使用
    void  createInputs();
    void  createOutputs();
    void  createNames(FILE* filep);
    Node* newGateName(char *c, enum node_types tpe);
    // Cube = 項, sop = 積和標準系
    Sop   readCubeSingle(FILE *fp, int numPI, int& invert); // 内部論理の計算

    /*------ PIに関するBddの変数順を決める ------*/
    void plaPIorder(char swBddOrder='1');
    void blifPIorder(char swBddOrder='1');
    // lev2piidxとvariablesが作られる

    /*------ Node の登録，登録解除 ------*/
    // todo: readblif, readplaとかの最後に呼ぶようにする
    // todo: foreach(PO){ 全てのpoに対してregnode(po)をする}
    void regNode(Node* node);
    void regPI(Node* node);
    void regPO(Node* node);
    void unregNode(Node* node); // 削除されるNodeは，出力数が0

    /*
     ++------ idepth, odepth (入力や出力からの段数) ------++
     idepth: 未定義は-1，入力を持たないNodeは0，その他は各入力の最大値+1
     odepth: 未定義は-1，出力を持たないNodeは0，その他は各出力の最大値+1

     [使用例]
     Cmap<void*,int> idepths(-1, ptrHash);
     for ( Node* no = firstPO(); no; no = nextPO() )
     cal_idepth(no, idepths);
     Cmap<void*,int> odepths(-1, ptrHash);
     for ( Node* no = firstPI(); no; no = nextPI() )
     cal_odepth(no, odepths);
     */
    int  calcIdepth(Node* no);
    // todo: 引数2つバージョンは，ほぼデバッグのためで最終的に消すかも
    int  calcIdepth(Node* no, unordered_map<void*, int>& idepths);
    void clearIdepth(Node* no);
    void clearIdepth(Node* no, unordered_map<void*, int>& idepths);
    int  calcOdepth(Node* no);
    int  calcOdepth(Node* no, unordered_map<void*, int>& odepths);
    void clearOdepth(Node* no);
    void clearOdepth(Node* no, unordered_map<void*, int>& odepths);

    /*
     ++------ outfunc (global論理関数) ------++
     outfunc: 未定義はBdd::null，変数を持つNodeはその変数，その他は再帰的計算

     [使用例]
     Cmap<void*,Bdd> outfuncs(Bdd::null, ptrHash);
     set_outfuncs(variables, outfuncs);
     for ( Node* no = firstPO(); no; no = nextPO() )
     cal_outfunc( no, outfuncs );
     */
    Bdd  calcOutfunc(Node* no);
    void calcOutfunc_Cudd(Node* no); // cudd
    Bdd  calcOutfunc(Node* no, unordered_map<void*, Bdd>& outfuncs);
    // outfuncs[no]に結果が入り，同じものが返る．
    void clearOutfunc(Node* no);
    void clearOutfunc(Node* no, unordered_map<void*, Bdd>& outfuncs);
    void setOutfuncs();
    void setOutfuncs_Cudd(); // cudd
    void setOutfuncs(const unordered_map<void*, int>& variables, unordered_map<void*, Bdd>& outfuncs);
    // Bdd::var( variables[node] ) を outfuncs[node] にセットする

    // しきい値論理でoutput計算
    void  calcOutfuncAllTh();
    void  calcOutfuncTh(Node* no, int *pi_count);
    void  calcOutfuncThCudd(Node* no);
    void  calcNodefuncCudd(Node* no);
    void  calcNodefuncCuddAdd(Node* no);

    // しきい値回路の回路変形
    void  transcircNodeTh(Node* no, std::unordered_set<Node*> all_fanouts = {});
    void  reductionTh(Node* no, std::unordered_set<Node*>& all_fanouts);
    void  check_reducenode(Node* no, Node* fin);
    void  cut_nodeTh(Node* no, Node* fin);
    void  delete_outputTh(Node* no, Node* fout);
    void  delete_nodeTh(Node* no);
    void  serch_fanout(Node* no, std::unordered_set<Node*>& all_fanouts);
    void  candi_clear();
    void  serch_spare(Node* no, const std::unordered_set<Node*>& all_fanouts);

};

#endif // #ifndef __NETWORK_H__
