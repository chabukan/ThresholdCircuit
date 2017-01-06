#include "ger-base.h"
#include "ger-base-old.h"
#include "isf.h"
#include "Cmap.H"
using namespace std;


//template class Array<Bdd>;
//template class PList<Node>;

int word_len(const char* c) {
    assert( *c != ' ' );        // 1文字目は空白でないこと
    int length=0;
    while ( *c != ' ' && *c != '\0' ) {
        c++;
        length++;
    }
    return length;
}





void printArrayInt(Arrayint Array)
{
    cerr << " ** printqing Arrayint ** \n";
    int size = Array.size();
    cerr << "    Size =  " << size << "  \n";
    for(int i =0 ; i<size; i++){
        cerr << " index  " << i  << " -> " << Array[i] << "  \n";
    }
    cerr << " ** Finished printing Arrayint ** \n";
}




// var2idx: Bddのvar(1から始まる)から，表示時のidx(0から始まる)への写像
//void BddPrintPLA(FILE * sout, const Bdd& lf, int* var2idx=0)
void BddPrintPLA(FILE * sout, const Bdd& lf, int* var2idx)
{
    if(lf == Bdd::null){
        cerr << "print PLA Warning!! Bdd::null cannot display \n";
        return;
    }
    if(lf == Bdd::zero){ //何も表示しない
        return;
    }
    else if(lf == Bdd::one){ //1 だけ表示
        fprintf(stderr, "1\n");
        return;
    }
    //  ISF isf(lf, lf);
    //  Sop isop =  isf.makeISOPnew();
    // ★ 12/25  hashtable ややこしいので，IsfLU->makeISOP に統一
    IsfLU isf = IsfLU(lf, lf);
    Sop isop =  isf.makeISOP();

    //  ISF NOTisf(~lf, ~lf);
    //  Sop NOTisop = NOTisf.makeISOPnew();
    IsfLU notisf = IsfLU(~lf, ~lf);
    Sop NOTisop =  notisf.makeISOP();


    if( NOTisop.nCube() < isop.nCube()){
        CubeSet::printSop(NOTisop, sout, (char*)" 0", var2idx);
    }
    else{
        CubeSet::printSop(isop, sout, (char*)" 1", var2idx);
    }
}






//x1, x2 と表示するために、level = 1の変数から展開していく。
//現在までにlevel番まで展開済み
static void bddprint_table_normal(FILE *sout, int n_in, const Bdd f, 
                                  unsigned int assigned, int level) 
{
    if ( level == n_in ) {         //展開終了（終端ノードを指してるはず）
        for ( int i=n_in-1; i>= 0; i--)
            fprintf(sout, " %d ", (assigned>>i) & 1);

        if ( f == Bdd::zero ) fprintf(sout, "| 0\n");
        else if ( f == Bdd::one ) fprintf(sout, "| 1\n");
        else  cerr << "Bdd print_table error -- something wrong \n";
        return;
    }
    else {
        Bdd f0 = f.rstr0(level+1);   
        Bdd f1 = f.rstr1(level+1);   
        bddprint_table_normal(sout, n_in, f0, assigned, level+1);
        bddprint_table_normal(sout, n_in, f1, assigned | ( 1<<(n_in - level -1)), level+1);
    }
}


//x1, x2 と表示するために、level = 1の変数から展開していく。
// assigned の一番下のビットが、Xi (i=n_in)に相当
// Gray-Code で 表現(flag = 1 でその次を反転させる）
static void bddprint_table_gray(FILE *sout, int n_in, const Bdd f, 
                                unsigned int assigned, int level, int flag) 
{
    if ( level == n_in ) {         //展開終了（終端ノードを指してるはず）
        for ( int i=n_in-1; i>= 0; i--)
            fprintf(sout, " %d ", (assigned>>i) & 1);

        if ( f == Bdd::zero ) fprintf(sout, "| 0\n");
        else if ( f == Bdd::one ) fprintf(sout, (char*)"| 1\n");
        else  cerr << "Bdd print_table error -- something wrong \n";
        return;
    }
    else {
        Bdd f0 = f.rstr0(level+1);   
        Bdd f1 = f.rstr1(level+1);   
        if(flag==0){
            bddprint_table_gray(sout, n_in, f0, assigned, level+1, 0);
            bddprint_table_gray(sout, n_in, f1, assigned|(1<<(n_in-level-1)), level+1, 1);
        }
        else{
            bddprint_table_gray(sout, n_in, f1, assigned|(1<<(n_in-level-1)), level+1, 0);
            bddprint_table_gray(sout, n_in, f0, assigned, level+1, 1);
        }

    }
}

void Bdd_printTable_normal(FILE *sout, int n_in, const Bdd F)
{
    //  fprintf(sout,"******************************************* \n");
    //  fprintf(sout,"***** BDD print_table (Normal Coding) \n");
    //  fprintf(sout,"*****    -- xi corresponds to Bdd::var(i) \n");
    //  fprintf(sout,"******************************************* \n");
    //  fprintf(sout," \n");
    if(F == Bdd::null){ 
        fprintf(stderr,"BDD NULL \n");
        return;
    }
    if ( n_in > 9 ){              //最大でも32=intのビット幅まで
        cerr << "********Sorry: too many input to display.******* \n";
        cerr << "******** BDD_PRINT can treat within 8 inputs ******* \n";
        return;
    }
    int i;
    for( i=1; i <= n_in;i++){
        fprintf(sout, "x%d ",i);
    }               
    fprintf(sout, "| F\n");
    for(i=n_in; i > 0;i--){
        fprintf(sout, "---");
    }               
    fprintf(sout, "+---\n");
    bddprint_table_normal(sout, n_in, F, 0, 0);
}



void Bdd_printTable_gray(FILE *sout, int n_in, const Bdd F)
{
    //  fprintf(sout,"******************************************* \n");
    //  fprintf(sout,"***** BDD print_table (Gray Coding) \n");
    //  fprintf(sout,"*****    -- xi corresponds to Bdd::var(i) \n");
    //  fprintf(sout,"******************************************* \n");
    //  fprintf(sout," \n");
    if(F == Bdd::null){ 
        fprintf(stderr,"BDD NULL \n");
        return;
    }
    if ( n_in > 9 ){              //最大でも32=intのビット幅まで
        cerr << "********Sorry: too many input to display.******* \n";
        cerr << "******** BDD_PRINT can treat within 8 inputs ******* \n";
        return;
    }
    int i;
    for( i=1; i <= n_in;i++){
        fprintf(sout, "x%d ",i);
    }               
    fprintf(sout, "| F\n");
    for(i=n_in; i > 0;i--){
        fprintf(sout, "---");
    }               
    fprintf(sout, "+---\n");
    bddprint_table_gray(sout, n_in, F, 0, 0, 0);
}



void bddprintR(FILE *sout, Bdd f, int flag) {  // flag =0 で （）=非表示
    if(f == Bdd::null){ 
        fprintf(stderr,"BDD NULL \n");
        return;
    }
    Bdd f_, f0, f1;                             // flag =1 de その前に
    if (f == Bdd::zero) {                           // 項がある。
        fprintf(sout, "BDDF");
        return;
    }
    f_ = ~f;
    if (f_ == Bdd::zero) {
        fprintf(sout, "BDDT");
        return;
    }

    //    f0 = f.rstrtop(0);
    //    f1 = f.rstrtop(1);

    f0 = f.rstrtop0();
    f1 = f.rstrtop1();

    int varid = f.top();
    
    if(! bddInter(f1, ~(f0))){                // f1 <= f0
        if(flag==1 && (f1 != Bdd::zero)){
            fprintf(sout, "(");
        }
        if(f1 != Bdd::zero){
            bddprintR(sout,f1,0);
            fprintf(sout, "+");
        }
        fprintf(sout, "~x%d", varid); 
        if(f0 != Bdd::one){
            bddprintR(sout,f0,1);
        }

        if(flag==1 && (f1 != Bdd::zero)){
            fprintf(sout, ")");
        }

    }
    else if(! bddInter(f0, ~(f1))){       // f0 <= f1
        if(flag==1 && (f0 != Bdd::zero)){
            fprintf(sout, "(");
        }
        fprintf(sout, "x%d", varid );
        if(f1 != Bdd::one){
            bddprintR(sout,f1,1);
        }
        if(f0 != Bdd::zero){
            fprintf(sout, "+");
            bddprintR(sout,f0,0);
        }
        if(flag==1 && (f0 != Bdd::zero)){
            fprintf(sout, ")");
        }

    }
    else{   // f1 f0 = Bdd::zero Bdd::one ではないはず
        if(flag == 1){
            fprintf(sout, "(");
        }
        fprintf(sout, "x%d", varid);
        bddprintR(sout,f1,1);
        fprintf(sout, "+~x%d", varid);
        bddprintR(sout,f0,1);
        if(flag == 1){
            fprintf(sout, ")");
        }
    }
}



void Bdd_print_SOP(FILE *sout, Bdd F) {
    //  fprintf(sout,"         ******************************************* \n");
    //  fprintf(sout,"         ***** BDD print_SOP  \n");
    //  fprintf(sout,"         *****    -- xi corresponds to Bdd::var(i) \n");
    //  fprintf(sout,"         ******************************************* \n");
    //  fprintf(sout, " **-- The logic function = ");
    bddprintR(sout, F, 0);
    fprintf(sout, "\n");
}







static void bddprint_table_gray_array(FILE *sout, int n_in, Array<Bdd> array, 
                                      unsigned int assigned, int level, int flag) 
{
    int array_num = array.size();
    Bdd f;  //temporaly
    int j;
    if ( level == n_in ) {         //展開終了（終端ノードを指してるはず）
        for ( int i=n_in-1; i>= 0; i--)
            fprintf(sout, " %d ", (assigned>>i) & 1);
        for(j = 0; j < array_num ; j++){
            f = array[j];
            if ( f == Bdd::zero ) fprintf(sout, "| 0  ");
            else if ( f == Bdd::one ) fprintf(sout, "| 1  ");
            else  cerr << "Bdd print_table error -- something wrong \n";
        }
        fprintf(sout, "|\n");
        return;
    }
    else {
        Bdd f1, f0; //temporaly
        Array<Bdd> array_0, array_1;
        for(j = 0; j < array_num ; j++){
            f = array[j];
            f0 = f.rstr0(level+1);   
            f1 = f.rstr1(level+1);   
            array_0.add(f0);
            array_1.add(f1);
        }
    
        if(flag==0){
            bddprint_table_gray_array(sout, n_in, array_0, assigned, level+1, 0);
            bddprint_table_gray_array(sout, n_in, array_1, assigned|(1<<(n_in-level-1)), level+1, 1);
        }
        else{
            bddprint_table_gray_array(sout, n_in, array_1, assigned|(1<<(n_in-level-1)), level+1, 0);
            bddprint_table_gray_array(sout, n_in, array_0, assigned, level+1, 1);
        }

    }
}



// 複数のBdd （Array<Bdd)を横に並べて表示 --> 比較しやすい
// 10こ以上の表示はずれるので、やめるべき
void Array_Bdd_print(FILE *sout, Array<Bdd> Bdd_array, int n_in)
{
    //  fprintf(sout,"******************************************* \n");
    //  fprintf(sout,"***** Array<Bdd> print_table (Gray Coding) \n");
    //  fprintf(sout,"*****    -- xi corresponds to Bdd::var(i) \n");
    //  fprintf(sout,"******************************************* \n");
    //  fprintf(sout," \n");
    if ( n_in > 9 ){              //最大でも32=intのビット幅まで
        cerr << "********Sorry: too many input to display.******* \n";
        return;
    }
    int array_num = Bdd_array.size();
    int i;
    for(i=n_in*3 + array_num *5; i > 0;i--){
        fprintf(sout, "-");
    }            
    fprintf(sout, "+\n");

    for(i=1; i <= n_in;i++){
        fprintf(sout, "x%d ",i);
    }
    for(i = 1; i<= array_num ;i++){
        fprintf(sout, "| F%i ", i);
    }
    fprintf(sout, "|\n");

    for(i=n_in; i > 0;i--){
        fprintf(sout, "---");
    }            
    for(i=array_num; i > 1;i--){
        fprintf(sout, "+----");
    }            
    fprintf(sout, "+----|\n");
    bddprint_table_gray_array(sout, n_in, Bdd_array, 0, 0, 0);

    for(i=n_in; i > 0;i--){
        fprintf(sout, "---");
    }            
    for(i=array_num; i > 1;i--){
        fprintf(sout, "+----");
    }            
    fprintf(sout, "+----|\n");
}


// 下位bit分に対して、binaryを２進数とおもって、それをgray codeにしたものを返す。
// binary の上位ビットの１つ左のビット（bit+1ビット目)は必ず０でないとだめ。
static int binary2gray(int binary, int bit)
{
    //与えられたもとのbinaryの１つ左のビットが1であるとき、そこを反転すれば、
    // grayコードになる。
    int gray = binary;
    int checkbit; //現在注目してるビットの１つ前のビット
    int mask;
    for(int i = bit; i >= 1 ; i--){ //現在iビット目を注目
        checkbit = (binary>>i) & 1;  //一つ上のビット
        if(checkbit == 1){   //gray の iビット目を反転
            mask = 1<<(i-1); //iビットめだけが1
            gray = gray ^ mask; //gray の iビット目だけ反転
        }
    }
    return gray;
}


//効率わるいけど、８変数でもいけてるよう
//gray-codeの作りがあってるかちょっと不安やけど...とりあえずはいけてるよう。
void Bdd_Karnaugh_Map(FILE *sout, int n_in, const Bdd F)
{
    //  fprintf(sout,"******************************************* \n");
    //  fprintf(sout,"*****  Printing Karnaugh Map  \n");
    //  fprintf(sout,"*****    -- xi corresponds to Bdd::var(i) \n");
    //  fprintf(sout,"******************************************* \n");
    //  fprintf(sout," \n");
    if(F == Bdd::null){ 
        fprintf(stderr," cannot display BDD null\n");
        return;
    }
    if ( n_in > 9 ){              //最大でも32=intのビット幅まで
        cerr << "********Sorry: too many input to display.******* \n";
        return;
    }
    int upper = n_in /2;      //表の上にくる変数の数 (半分よりは少ない)
    int lower = n_in - upper;
    int array_num =  1 << upper; //横にならぶ変数のかず
    int i;

    //上部に線

    int lowerHaba = 3;   //左側の１変数に割り当てる範囲
    int upperHaba = 5; //上側の１つの部分関数に割り当てる範囲

    for(i=lower * lowerHaba + 1 + array_num * upperHaba; i > 0;i--){
        fprintf(sout, "-");
    }
    fprintf(sout, "\n");
    //2行目
    for(i=1; i <= (lower*lowerHaba);i++){  
        fprintf(sout, " ");
    }
    fprintf(sout, "|");
    for(i=lower+1; i <= n_in;i++){
        fprintf(sout, "  x%d ",i);
    }            
    fprintf(sout, "\n");
    //3行目
    for(i=1; i <= lower;i++){
        fprintf(sout, "x%d ",i);
    }
    fprintf(sout, "|");

    //ここから横方向への表示
    Array<Bdd> Bdd_array;
    Bdd tempBdd;
    int k, j; //for loop only
    int varid;
    int checkbit;
    int assigned;  
    for(i=1; i<= array_num; i++){
        //assignedの計算 
        assigned = binary2gray(i-1, upper);  //assigned = gray codeに
        fprintf(sout, " ");
        for(k=upper-1; k>=0; k--){ //assignedを upper bitだけ表示
            fprintf(sout, "%d", ((assigned>>k) &1) );
        }
        for(j=1+upper;j<= (upperHaba-1); j++){
            fprintf(sout, " ");
        }
        //assignedから、Bddの計算
        tempBdd = F;
        for(k=1; k<=upper; k++){ //upperのうちのk番目の処理
            varid = lower + k;
            checkbit = (assigned >>(upper-k)) & 1;
            if(checkbit == 1) tempBdd = tempBdd.rstr1(varid);
            else tempBdd = tempBdd.rstr0(varid);
        }
        //assignedから、Bddの計算終わり
        Bdd_array.add(tempBdd);
    }
    fprintf(sout, "\n");

    //境界線
    for(i=lower; i>0 ; i--){
        fprintf(sout, "---");
    }    
    fprintf(sout, "+");
    for(i=array_num; i >= 1;i--){
        fprintf(sout, "-----");
    }            
    fprintf(sout, "\n");

    bddprint_table_gray_array(sout, lower, Bdd_array, 0, 0, 0);

    for(i=(lower*3 + 1 + array_num * 5); i > 0;i--){
        fprintf(sout, "-");
    }
    fprintf(sout, "\n");  
}

// stderr に 表示 ２項以上あれば()をつける。
// 恒真、恒偽関数では何もしない。
// flag = 0 の時は 括弧をつけない。

void  sop_print(FILE * sout, Sop  sop, int flag)
{
    if(sop == Sop::one){fprintf(sout,"BDDT"); return;}
    else if(sop == Sop::zero){fprintf(sout,"BDDF"); return;}
    int varid = sop.topvar();
    int termNum = 0;
    Sop f0 = sop.factor0(varid);
    Sop f1 = sop.factor1(varid);
    Sop fr  = sop.factorR(varid);

    if( (f0 != Sop::zero) ) termNum++;
    if( (f1 != Sop::zero) ) termNum++;
    if( (fr != Sop::zero) ) termNum++;

    if((termNum > 1) &&(flag) ){
        fprintf(sout,"(");
    }
    int plusNeed = 0 ; //1なら　＋　を かかなだめ
    if( f0 != Sop::zero ) {
        fprintf(sout,"~x%d", varid);   
        if( f0 != Sop::one){ 
            sop_print(sout, f0, 1);
        }
        plusNeed = 1;
    }

    if( f1 != Sop::zero ) {
        if(plusNeed == 1) fprintf(sout,"+");
        fprintf(sout,"x%d", varid);   
        if( f1 != Sop::one){ 
            sop_print(sout, f1, 1);
        }
        plusNeed = 1;
    }

    if( fr != Sop::zero ) {
        if(plusNeed == 1){ //前にf0かf1がある。
            fprintf(sout,"+");	
            sop_print(sout, fr, 0);
        }
        else{ 
            sop_print(sout, fr, flag);
        }
    }
    
    if((termNum > 1) &&(flag) ){
        fprintf(sout,")");
    }
}



// Bdd のstderrへの非冗長積和形の表示
void printISOP(Bdd lf)
{
    if(lf == Bdd::null){ 
        fprintf(stderr,"BDD NULL \n");
    }
    else if(lf == Bdd::one){
        fprintf(stderr,"BDDT \n");
    }
    else if(lf == Bdd::zero){
        fprintf(stderr,"BDDF \n");
    }
    else{

        //    ISF isf(lf, lf);
        //    Sop isop =  isf.makeISOPnew();
        IsfLU isf = IsfLU(lf, lf);
        Sop isop =  isf.makeISOP();
        sop_print(stderr, isop, 0 );
	fprintf(stderr,"\n");
    }
}




//cubeBdd に含まれるリテラルすべてについてsmoothしたのを返す。
// cubeBdd は 正リテラルの積でないとだめ
Bdd  smooth_cube_all(Bdd F, Bdd cubeBdd)
{
    Bdd Result = F;
    Bdd temp = cubeBdd;
    int lev;
    while( temp != Bdd::one){
        lev = temp.top();
        Result = Result.smooth(lev);
        //    temp = temp.rstrtop(1);
        temp = temp.rstrtop1();
    }
    return Result;
}



//SupVars に含まれるリテラルすべてについてsmoothしたのを返す。
Bdd  smooth_all(Bdd& F,  SupVars& sup_vars)
{
    Bdd Result = F;
    int Xi;
    for(SupVars temp = sup_vars; temp.SupNum() > 0 ; ){
        Xi = temp.delete_top();
        Result = Result.smooth(Xi);
    }
    return Result;
}




// 論理関数の一般的な操作


/////////////////////////////////////////////////////////////////////
// 不完全指定論理関数 の クラス ISF
/////////////////////////////////////////////////////////////////////


ISOP* ISF::makeISOP(){

    if(AllFalse()){
        ISOP* isopfalse = new ISOP(-2);       // 恒偽関数
        return isopfalse;
    }
    else if(AllTrue()){
        ISOP* isoptrue = new  ISOP(-1);    // 恒真関数	
        return isoptrue;
    }   

    int level=this->topVarId(); //展開すべき変数の番号（rankではない）
    //    cerr << "\n*** expandinv id = " << level << "*****\n"
    ISOP* result = new ISOP(level - 1);    //これでいいかな？
    ISF f0(rstr0(level));
    ISF f1(rstr1(level));
    ISF f00((f0.low & ~(f1.up)),f0.up);
    ISF f11((f1.low & ~(f0.up)),f1.up);
    result->f0  = f00.makeISOP();           // isop0に相当
    result->f1  = f11.makeISOP();
    Bdd g0 = (result->f0)->makeLF();
    Bdd g1 = (result->f1)->makeLF();
    ISF f000((f0.low & ~(g0)),f0.up);
    ISF f111((f1.low & ~(g1)),f1.up);
    ISF isopAlpha( ( (f000.low & f111.up) | (f000.up & f111.low) )
                   , (f000.up & f111.up)); 

    // 実は、topVar の番号の変数が low up は 依存していても、 ISOP として、f0, f1も
    // これに関係せず、f0=f1=BDDF(つまりno =-2) で、 fr が あるISOPとなる。
    // これは、少し気持ち悪いので、 このような場合は、frを resultとして返す
    // adding by ger 20/50
    // この 省く部分はちゃんとチェックした 21/05
    if( (result->f0->TrueOrFalse()==0) &&  (result->f1->TrueOrFalse()==0) ){
        //      cerr << " DEBUG COND. happen - skipped id = " << level -1 << "\n";
        delete result;
        return isopAlpha.makeISOP(); 
    }
    // adding by ger 20/50
    result->fr = isopAlpha.makeISOP(); 
    return result;
}





// Bdd のstderrへの非冗長積和形の表示
/*   06/31以降 ISOP は　obsolete
 void printISOP(Bdd lf)
 {
 if(lf == Bdd::null){ 
 fprintf(stderr,"BDD NULL \n");
 }
 else if(lf == Bdd::one){
 fprintf(stderr,"BDDT \n");
 }
 else if(lf == Bdd::zero){
 fprintf(stderr,"BDDF \n");
 }
 else{

 ISF isf(lf,lf);
 ISOP* isoptemp = isf.makeISOP();
 isoptemp->print(0);
 fprintf(stderr,"\n");
 delete isoptemp;
 }
 }
 */






// 不完全指定関数において、その neck を表す不完全指定関数をつくり、
// それを返す。 i は注目している変数番号（１から）
// ここでは、neckの全ての条件は表せず、xiの一部となっている。
// 計算結果はxiの存在い義空間みたいなものかな？

ISF ISF::makeNECK(int i){
    Bdd xi = Bdd::var(i);
    // neck は xi に ついて、ISF を 折り返した時、必ず食い違うところが
    // １になる関数
    Bdd neck = (low.rstr1(i) & ~(up.rstr0(i))) | 
        (low.rstr0(i) & ~(up.rstr1(i))); 
    ISF result = ISF((xi & neck), (xi| (~neck)) );
    // result の １ ---  xi のうちneck=1の部分、
    // result の ０ ---- ~xi のうちneck=1の部分
    return result;
}

// Gray-Code で 表現(flag = 1 でその次を反転させる）
// 非公開関数にしようかな
void ISFprint_table(FILE *sout, int n_in, Bdd lowf, Bdd upf, char assigned, int level,int flag) 
{
    if ( level == 0 ) {
        for ( int i=n_in-1; i>= 0; i--)
            fprintf(sout, " %d ", (assigned>>i) & 1);

        if (lowf == Bdd::zero){
            if(upf == Bdd::zero) fprintf(sout, "| 0\n");
            else fprintf(sout, "| *\n");
        }
        else{
            fprintf(sout, "| 1\n");
        }
        return;
    }
    else {
        /*	int lowf0 = bddrstr(lowf, level, 0);
         int lowf1 = bddrstr(lowf, level, 1);
         int upf0 = bddrstr(upf, level, 0);
         int upf1 = bddrstr(upf, level, 1);
         */

        Bdd lowf0 = lowf.rstr0(level);
        Bdd lowf1 = lowf.rstr1(level);
        Bdd upf0 = upf.rstr0(level);
        Bdd upf1 = upf.rstr1(level);

        if(flag==0){
            ISFprint_table(sout, n_in, lowf0, upf0, assigned, level-1,0);
            ISFprint_table(sout, n_in, lowf1, upf1, assigned | 
                           (1<<(level-1)),level-1,1);
        }
        else{
            ISFprint_table(sout, n_in, lowf1, upf1, assigned | 
                           (1<<(level-1)),level-1,0);
            ISFprint_table(sout, n_in, lowf0, upf0, assigned, level-1,1);
        }
    }

}


// 不完全指定論理関数のtable 上の表示、反射２進符合
void ISF::print(FILE *sout, int n_in)
{
    // ISP のerrorチェックなし
    //    int lowf = low.value();
    //    int upf  = up.value();

    fprintf(sout,"ISF print_table (gray-code) \n");

    if ( n_in > 9 ){              //最大でも32=intのビット幅まで
        cerr << "********Sorry: too many input to display.******* \n";
        return;
    }
    int i;
    for(i=n_in-1; i >= 0;i--){
        fprintf(sout, "x%d ",i);
    }
    fprintf(sout, "| F\n");
    for(i=n_in; i > 0;i--){
        fprintf(sout, "---");
    }
    fprintf(sout, "+---\n");

    //    ISFprint_table(sout, n_in, lowf, upf, 0, n_in,0);
    ISFprint_table(sout, n_in, low, up, 0, n_in,0);
}

//不完全指定論理関数の非冗長積和表示
void ISF::print()
{
    ISOP* isop = this->makeISOP();
    isop->print(0);
    delete isop;
}




// support_min のmain の 関数の定義
// やりなおし
void supportMain(int sup, ISF *f,int* varTable, int Nsup, int elim, 
                 ISF* minf,int* minNsup,int* varResult, int* Usedflag){
    //Nsup -- 現在依存してる変数の数(初期値sup)
    //elim -- あととることが可能な変数の数(初期値sup)
    int Xelim = varTable[elim-1]; //elim するBDDの変数番号
    Bdd newL = f->low.rstr1(Xelim) | f->low.rstr0(Xelim);
    Bdd newU = f->up.rstr1(Xelim) & f->up.rstr0(Xelim);
    ISF newf(newL,newU);

    if(newf.check()){  //Xeim除去可能

        Usedflag[elim-1] = 1;

        if( Nsup == (*minNsup)){    // Nsup -1 < *minNsup を意味するはず
            (*minNsup) = Nsup - 1;
            (*minf) = newf;        //これでいいかな？

            for(register int i=0;i<sup;i++){
                varResult[i] = Usedflag[i];
            }
        }

        //        cerr << Xelim << " removed \n";

        if(elim == 1)return;
        else if((Nsup-elim) >= (*minNsup) )return;
        supportMain(sup, &newf, varTable, Nsup-1, elim-1, minf, minNsup,
                    varResult, Usedflag);
    }
    //Xelim除去せず
    //    cerr << Xelim << "not removed \n";

    if(elim == 1) return;
    else if( (Nsup-elim+1) >= (*minNsup) )return;


    for(register int i=0;i<elim;i++){
        Usedflag[i] = 0;        //elim-1 より前はすべて初期状態にもどす
    }

    supportMain(sup, f, varTable, Nsup, elim-1, minf, minNsup, 
                varResult, Usedflag);
}


//多分、例えば、x1の依存度をとった後の解でも x1, ~x1対称の部分がともにドントケアな
//場合、そのままのはず。
ISF support_min(ISF* f,int sup,int* varTable,int* nsup, int* varResult)
{
    // 最良解を保持する変数
    int minNsup = sup;
    ISF minf = (*f);
    int* Usedflag = new int[sup];
    for(register int i=0;i<sup;i++){
        Usedflag[i] = 0;      //除いた場合だけ１にする。
        varResult[i] = 0;     //初期化（１つも除けなかった時のため）
    }
    supportMain(sup, f, varTable, sup, sup, &minf, &minNsup,
                varResult, Usedflag);
    (*nsup) = minNsup;
    delete [] Usedflag;
    return minf;
}





//  昔のhashなし 
Sop ISF::makeISOPnewNoHash(int* lev2idx){
    #ifdef DEBUG_ISOP 
    makeISOPcount++;
    #endif 
    if(AllFalse()){       // 恒偽関数
        return Sop::zero;
    }
    else if(AllTrue()){
        return Sop::one;  // 恒真関数   
    }   

    int level=this->topVarId(); //展開すべき変数の番号（rankではない）
    //    cerr << "\n*** expandinv id = " << level << "*****\n"
    ISF f0(rstr0(level));
    ISF f1(rstr1(level));
    ISF f00((f0.low & ~(f1.up)),f0.up);
    ISF f11((f1.low & ~(f0.up)),f1.up);
    Sop result_f0  = f00.makeISOPnewNoHash();           // isop0に相当
    Sop result_f1  = f11.makeISOPnewNoHash();
    Bdd g0 = (result_f0).getFunc(0);
    Bdd g1 = (result_f1).getFunc(0);
    ISF f000((f0.low & ~(g0)),f0.up);
    ISF f111((f1.low & ~(g1)),f1.up);
    ISF isopAlpha( ( (f000.low & f111.up) | (f000.up & f111.low) )
                   , (f000.up & f111.up)); 

    Sop result_fr = isopAlpha.makeISOPnewNoHash(); 

    int index;
    if(lev2idx == 0){
        index = level;
    }
    else{
        index = lev2idx[level];
    }
    Sop result = result_f0.and0(index) + result_f1.and1(index) + result_fr;
    return result;
}






/*
 Sop ISF::makeISOPnew(){
 if(AllFalse()){       // 恒偽関数
 return Sop::zero;
 }
 else if(AllTrue()){
 return Sop::one;  // 恒真関数	
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
 Sop result = result_f0.and0(level) + result_f1.and1(level) + result_fr;
 return result;
 }

 */







///////////////////////////////////////////////////////////////////////
/////////////// 非冗長積和形表現 の ための Clss ISOP (06/31からObsolete) 

// ISOP の 表現する 論理関数を表すBddをつくって返す。
Bdd ISOP::makeLF(){
    if(i == -1) return Bdd::one;
    if(i == -2) return Bdd::zero;
    Bdd b0,b1,br;
    if(f0 == 0){
        b0 = Bdd::zero;
    }
    else{
        b0 = f0->makeLF();
    }
    if(f1 == 0){
        b1 = Bdd::zero;
    }
    else{
        b1 = f1->makeLF();
    }
    if(fr == 0){
        br = Bdd::zero;
    }
    else{
        br = fr->makeLF();
    }
    Bdd vi = Bdd::var(i+1);        //これでいいはず
    Bdd result = (vi & b1) | ((~vi) & b0) | br ;
    return result;
}

ISOP::ISOP(int j):i(j),f0(0),f1(0),fr(0){
    //    objnum++;
    //    fprintf(stderr,"Construct %d\n",objnum);
}

ISOP::~ISOP(){    // ちゃんと消してたよ
    //    objnum--;
    //    fprintf(stderr,"delete %d\n",objnum);
    delete f0;
    delete f1;
    delete fr;
}

// stderr に 表示 ２項以上あれば()をつける。
// 恒真、恒偽関数では何もしない。
// flag = 0 の時は 括弧をつけない。

void ISOP::print(int flag){
    int temp = TrueOrFalse();
    if(temp == 1){cerr << "BDDT \n"; return;}
    else if(temp == 0){cerr << "BDDF \n"; return;}
    int termNum = 0;
    if( (f0->TrueOrFalse() != 0) ) termNum++;
    if( (f1->TrueOrFalse() != 0) ) termNum++;
    if( (fr->TrueOrFalse() != 0) ) termNum++;
    if((termNum > 1) &&(flag) ){
        fprintf(stderr,"(");
    }
    if( (f0->TrueOrFalse() != 0) ){ //f0->i が -2 でないとき
        // f0->i == -2 はそれ以下がまったくないことを意味する。  
        fprintf(stderr,"~x%d",i+1);   //Bdd_varとあわせる
        if((f0->Number())!= -1){  //この時は、f0 はBDDTをさしてるので表示しない。
            f0->print(1);
        }
    }
    if( (f1->TrueOrFalse() != 0)){
        if((f0->TrueOrFalse() != 0))fprintf(stderr,"+");
        fprintf(stderr,"x%d",i+1);
        if((f1->Number())!= -1){ //
            f1->print(1);
        }
    }
    if((fr->TrueOrFalse() != 0)){
        if( ((f0->TrueOrFalse() != 0))||((f1->TrueOrFalse() != 0)))fprintf(stderr,"+");
        if((fr->Number())!= -1){
            //	    fr->print(0);  これだと（）つきで呼ばれた次の変数でfrしかかなければ
            //                  ()いるはずなのに、()つかなくなる。
            fr->print(flag);
        }
    }
    
    if((termNum > 1) &&(flag) ){
        fprintf(stderr,")");
    }
}





// Debug のため
void ISOP::print_for_debug()
{
    // ISOP自身のポインタ、 i,  f0 のポインタ、 f1のポインタ、ｆｒのポインタ
    cerr << "ISOP = " << (int64_t) this  ;
    cerr << " i = " <<  (int64_t) this->i  ;
    cerr << " f0 = " <<  (int64_t) this->f0  ;
    cerr << " f1 = " <<  (int64_t) this->f1  ;
    cerr << " fr = " <<  (int64_t) this->fr  ;
    cerr << "\n";
    if(this->f0 != nil) this->f0->print_for_debug();
    if(this->f1 != nil) this->f1->print_for_debug();
    if(this->fr != nil) this->fr->print_for_debug();

}

 
/// ZBDD 関係

static void reverse(char *s)
{
    int     c,i,j;
    for(i=0,j=strlen(s)-1;i<j;i++,j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void itoa(int n, char *s)
{
    int     i,sign;
    i=0;
    if((sign = n) < 0)
        n= -n;
    do{
        s[i++] = n % 10 + '0';
    } while((n /= 10) >0 );
    if(sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}



//prestring は決定したときに表示する x1, x2 など（それまでのpath上の変数)
// first = 1 なら、集合の最初の要素を書こうとしている。
static void printZBDDrecusive(FILE * sout, char * prestring, Bdd& Zbdd)
{
    if(Zbdd == Bdd::base){
        if(strlen(prestring) == 0){ //Base の表示 0000...のpathに相当
            fprintf(sout, " (ε)");
        }
        else{
            fprintf(sout, " (%s)", prestring);
        }
        return;
    }
    else if(Zbdd == Bdd::empty){
        return;
    }
    int lev = Zbdd.top();
    char Now[Digit+2]; 
    strcpy(Now, "x\0");  //x000\n
    char NowLev[Digit+1];  //x000\n
    itoa(lev, NowLev);
    strcat(Now, NowLev);
    char * newString  = new char [Digit+2 + strlen(prestring)];  //x000\n
    newString = strcpy(newString, prestring);
    newString = strcat(newString, Now);
    //  Bdd f0= Zbdd.rstrtop0();   //これではだめでしょう。
    //  Bdd f1= Zbdd.rstrtop1();
    Bdd f0= Zbdd.subset0(lev);
    Bdd f1= Zbdd.subset1(lev);
    printZBDDrecusive(sout, prestring, f0);
    printZBDDrecusive(sout, newString, f1);
    delete newString;
}


// ZBDDの集合の要素を列挙して表示 
void printZBDD(FILE * sout, Bdd& Zbdd){
    if(Zbdd == Bdd::empty){
        fprintf(sout, "the Set of this ZBDD = Empty \n");
    }
    else{
        fprintf(sout, "the Set of this ZBDD = \{ ");
        //Base の表示   000000 のpathが1にいってたら、表示すればいいでしょう.
        //    if(Zbdd.value() < 0){
        //      fprintf(sout, "ε");
        //    }
        char prestring[1];
        prestring[0] = '\0';
        printZBDDrecusive(sout,  prestring, Zbdd);
        fprintf(sout, " } \n");
    }

}



// ZBDDの情報の表示 - 集合の要素など (for Debug)
void printZBDDinfo(Bdd& Zbdd)
{
    cerr << " *** Print Info. of a ZBDD \n";
    cerr << "    number of 1-paths (elements) = " << Zbdd.card()  << " \n";
    cerr << "    total number of literals (variables) in all elements  = " << Zbdd.lit()  << " \n";
    printZBDD(stderr, Zbdd);
}





// /////////////////////////////////



//var_id の変数を含んでいたら1を返す。 そうでないなら0
// もっと効率いいほうほうないかなーーーーーーーーーーーーーーーーーーーーーーーーー
inline int SupVars::includeKa (int var_id) const
{
    //  Bdd Xi = Bdd::var(var_id);
    Bdd Xi = Bdd::base.change(var_id);
    Bdd interSec = Bdd::intersec(Xi, this->zbdd);
    if(interSec == Bdd::empty) return 0;
    else return 1;
}






//SupVarsで、 DeleteSupに含まれる変数を削除したSupVarsをつくってかえす
//★自分自身, DeleteSupは変更されない
// - で　実装したため、 DeleteSupにしか含まれない変数が
//  あってもムシサレル．ようはintersec(this, DeleteSup)を差し引いた集合ということ
inline SupVars SupVars::delete_sup_vars(SupVars * DeleteSup)
{
    SupVars returnSup = SupVars(this->GetZBdd() - DeleteSup->GetZBdd());
    return returnSup;
}



inline SupVars SupVars::merge_sup_vars(SupVars * merge_Sup) const
{
    SupVars returnSup = SupVars(this->GetZBdd() + merge_Sup->GetZBdd());
    return returnSup;
}

//For debug 含まれてる変数番号を全て表示
inline void SupVars::print_sup()
{
    //cerr << "\n *** printting level in sup_vars \n ";
    for(SupVars temp = *this; temp.SupNum() > 0 ; ){
        //     cerr << " " << temp.delete_top();
    }
    //    cerr << "\n ";
}


inline void SupVars::add_supVars(int varid)
{
    //  Bdd Xi = Bdd::var(varid);
    Bdd Xi = Bdd::base.change(varid);

    Bdd newZbdd = this->zbdd + Xi;

    if(newZbdd == this->zbdd){
        //    cerr << "Warning: SupVars -- adding vars already exists - varid = " 
        //	  << varid << "\n";
        return;
    }
    else{
        this->zbdd = newZbdd;
        return;
    }
}

int SupVars::return_top() 
{
    if(SupNum() == 0) {
        return 0;
    }
    else{
        int toplev = zbdd.top();
        return toplev;
    }
}


inline int SupVars::delete_top()
{
    //  if( GetZBdd() == Bdd::base) {
    if( GetZBdd() == Bdd::empty) {
        return 0;
    }
    else{
        int toplev = zbdd.top();
        //    this->zbdd = this->zbdd.rstrtop1(); //★これだとうまくいかんよ 
        this->zbdd = this->zbdd.subset0(toplev);
        return toplev;
    }
}





inline SupVars::SupVars(Bdd Zbdd):
    zbdd(Zbdd)
{;}

inline SupVars::SupVars():
    //zbdd(Bdd::base){;}
    zbdd(Bdd::empty){;}


inline SupVars SupVars::operator = (const SupVars oprand) {
    if ( this != &oprand ) {
        zbdd = oprand.zbdd;       
    }
    return *this;
}


inline int operator == (const SupVars lhs, const SupVars rhs){
    return ( (lhs.zbdd == rhs.zbdd) );
}


