#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <utility>
#include "sat.h"

using namespace std;

class gate {
    public:
        gate() {}
        ~gate() {}
        Var _var;
};

class square {
    public:
        square() {  // default constructor
            x = 0;
            y = 0;
            state = -2;
            gate_var.resize(10);
        }
        ~square() {;}
        void setPos(int x_i, int y_i) {
            x = x_i;
            y = y_i;
        }
        void setState(int state_i) {
            state = state_i;
        }
        int getX() { return x; }
        int getY() { return y; }
        int getState() { return state; }
        vector<gate> gate_var;

    private:
        int x;
        int y;
        int state;
};

// basic function of SAT solver
Var addANDCNF(SatSolver& s, Var a, Var b, bool a_bool, bool b_bool);
Var addXORCNF(SatSolver& s, Var a, Var b, bool a_bool, bool b_bool);
Var addORCNF(SatSolver& s, Var a, Var b, bool a_bool, bool b_bool);

Var genModel(SatSolver& s, vector<vector<square> > &M); // gen SAT model for MineSweeper 
Var genSubModel_1(SatSolver& s, square& m); // gen (am'+a0')(am'+a1')...(a7'+a8') = 1
Var genSubModel_2(SatSolver& s, square& m); // gen (am'+a0')(am'+a1')...(a7'+a8') = 1
Var genSubModel_3(SatSolver& s, vector<vector<square> >& m, int x, int y);  // gen Mine number implication CNF

// check boundary square
bool checkX(int x, int row_num);
bool checkY(int y, int col_num);

Var gen_Enum(SatSolver &s, vector<Var> &neighbor_var, Var a, int r);    // sub function of genSubModel_3
// enumerate set of combinations
void Combination(vector<int> &source, vector<vector<int> > &record, int r);
void Combination_Util(vector<int> &source, vector<vector<int> > &record, vector<int> &data, int s, int e, int index, int r);

void initialSate(SatSolver &s, vector<vector<square> >& m); // initialize model state

void reportResult(SatSolver &s, bool result, vector<vector<square> > &m, vector<vector<int> > &record); // print result and record
int find_state(SatSolver &s, square &m);    // find which state is true for each square

int main(int argc, char* argv[]) {
    fstream fin;
    fstream fout;
    fin.open(argv[1], ios::in);
    if(!fin) {
        cout << endl;
        cout << argv[1] << " can not be opened! " << endl;
    }
    fout.open(argv[2], ios::out);

    int row_num, col_num;   // row/column number of MineSweeper game plane
    fin >> row_num >> col_num;

    // square
    vector< vector<square> > MineSweeper(row_num, vector<square> (col_num, square()));

    for(int i=0; i<row_num; i++) {
        for(int j=0; j<col_num; j++) {
            int state;
            fin >> state;
            MineSweeper[i][j].setPos(i+1, j+1);
            MineSweeper[i][j].setState(state);
        }
    }

    /*
    for(int i=0; i<row_num; i++) {
        for(int j=0; j<col_num; j++) {
            cout << MineSweeper[i][j].getState() << " ";
        }
        cout << endl;
    }
    */

    SatSolver s;
    s.initialize();
    Var target;
    target = genModel(s, MineSweeper);
    s.assumeProperty(target, 1);
    // cout << "done" << endl;
    initialSate(s, MineSweeper);
    bool result;
    result = s.assumpSolve();
    vector<vector<int> > record(row_num, vector<int>(col_num, 0));
    reportResult(s, result, MineSweeper, record);

    /*
    if(result) {
        for(int i=0; i<row_num; i++) {
            for(int j=0; j<col_num; j++) {
                for(int k=0; k<MineSweeper[i][j].gate_var.size(); k++) {
                    Var v = MineSweeper[i][j].gate_var[k]._var;
                    cout << s.getValue(v) << " ";
                }
                cout << endl;
            }
        }
        cout << endl;
    }*/

    // write output file
    fout << row_num << " " << col_num << endl;
    if(result) {
        fout << "SAT" << endl;
        for(int i=0; i<row_num; i++) {
            for(int j=0; j<col_num; j++) {
                if(record[i][j] == -1) {
                    fout << "X" << " ";
                }
                else {
                    fout << record[i][j] << " ";
                }
            }
            fout << endl;
        }
    }
    else {
        fout << "UNSAT" << endl;
    }

    return 0;
}

Var addANDCNF(SatSolver& s, Var a, Var b, bool a_bool, bool b_bool) {
    Var out;
    out = s.newVar();
    s.addAigCNF(out, a, a_bool, b, b_bool);
    return out;
}

Var addXORCNF(SatSolver& s, Var a, Var b, bool a_bool, bool b_bool) {
    Var out;
    out = s.newVar();
    s.addXorCNF(out, a, a_bool, b, b_bool);
    return out;
}

Var addORCNF(SatSolver& s, Var a, Var b, bool a_bool, bool b_bool) {
    Var out1, out2;
    out1 = s.newVar();
    s.addAigCNF(out1, a, !a_bool, b, !b_bool);
    out2 = s.newVar();
    s.addAigCNF(out2, out1, true, out1, true);
    return out2;
}

Var genModel(SatSolver& s, vector<vector<square> > &M) {
    int row_num = M.size();
    int col_num = M[0].size();
    Var out = s.newVar();
    s.assumeProperty(out, 1);

    for(int i=0; i<row_num; i++) {  // variable assignment
        for(int j=0; j<col_num; j++) {
            for(int k=0; k<M[i][j].gate_var.size();  k++) {
                M[i][j].gate_var[k]._var = s.newVar();
            }
        }
    }
    

    for(int i=0; i<row_num; i++) {
        for(int j=0; j<col_num; j++) {
            Var m1 = genSubModel_1(s, M[i][j]);
            Var m2 = genSubModel_2(s, M[i][j]);
            Var m3 = genSubModel_3(s, M, i, j);
            out = addANDCNF(s, out, m1, 0, 0);
            out = addANDCNF(s, out, m2, 0, 0);
            out = addANDCNF(s, out, m3, 0, 0);
        }
    }
    return out;
}

Var genSubModel_1(SatSolver& s, square& m) {    // gen (am+a0+a1+a2+...+a8) = 1
    Var v;
    v  = addORCNF(s, m.gate_var[0]._var, m.gate_var[1]._var, 0, 0);
    for(int i=2; i<m.gate_var.size(); i++) {
        v = addORCNF(s, v, m.gate_var[i]._var, 0, 0);
    }
    return v;
}

Var genSubModel_2(SatSolver& s, square& m) {    // gen (am'+a0')(am'+a1')...(a7'+a8') = 1
    Var v1, v2;
    v1 = addORCNF(s, m.gate_var[0]._var, m.gate_var[1]._var, 1, 1);
    v2 = v1;
    for(int i=0; i<9; i++) {
        for(int j=i+1; j<10; j++) {
            v1 = addORCNF(s, m.gate_var[i]._var, m.gate_var[j]._var, 1, 1);
            v2 = addANDCNF(s, v1, v2, 0, 0); 
        }
    }
    return v2;
}

Var genSubModel_3(SatSolver& s, vector<vector<square> >& m, int x, int y) {    // gen Mine number implication CNF

    int row_num = m.size();
    int col_num = m[0].size();
    bool b_x = checkX(x, row_num);
    bool b_y = checkY(y, col_num);
    Var v0, v1, v2, v3, v4, v5, v6, v7, v8;

    vector<int> adding = {-1, 0, 1};
    vector<pair<int, int>> neighbor;
    vector<Var> neighbor_var;
    for(int i=0; i<adding.size(); i++) {
        for(int j=0; j<adding.size(); j++) {
            if(!(i == 1 && j == 1)) {
                neighbor.push_back(make_pair(x+adding[i], y+adding[j]));
            }
        }
    }

    for(int i=0; i<neighbor.size(); i++) {
        int x = neighbor[i].first;
        int y = neighbor[i].second;
        if(x >= 0 && x < row_num && y >=0 && y < col_num) {
            neighbor_var.push_back(m[x][y].gate_var[9]._var);
        }
    }

    // a0 implication
    Var a0 = m[x][y].gate_var[0]._var;
    v0 = s.newVar();
    s.assumeProperty(v0, 1);
    for(int i=0; i<neighbor_var.size(); i++) {
        v0 = addANDCNF(s, v0, neighbor_var[i], 0, 1);
    }
    v0 = addORCNF(s, a0, v0, 1, 0);

    // a1 implication
    Var a1 = m[x][y].gate_var[1]._var;
    v1 = gen_Enum(s, neighbor_var, a1, 1);

    // a2 implication
    Var a2 = m[x][y].gate_var[2]._var;
    v2 = gen_Enum(s, neighbor_var, a2, 2);

    // a3 implication
    Var a3 = m[x][y].gate_var[3]._var;
    v3 = gen_Enum(s, neighbor_var, a3, 3);

    // a4 implication
    Var a4 = m[x][y].gate_var[4]._var;
    if(b_x && b_y) {
        s.assumeProperty(a4, 0);
        v4 = s.newVar();
        s.assumeProperty(v4, 1);
    }
    else {
        v4 = gen_Enum(s, neighbor_var, a4, 4);
    }

    // a5 implication
    Var a5 = m[x][y].gate_var[5]._var;
    if(b_x && b_y) {
        s.assumeProperty(a5, 0);
        v5 = s.newVar();
        s.assumeProperty(v5, 1);
    }
    else {
        v5 = gen_Enum(s, neighbor_var, a5, 5);
    }

    //a6 implication
    Var a6 = m[x][y].gate_var[6]._var;
    if(b_x || b_y) {
        s.assumeProperty(a6, 0);
        v6 = s.newVar();
        s.assumeProperty(v6, 1);
    }
    else {
        v6 = gen_Enum(s, neighbor_var, a6, 6);
    }

    // a7 implication
    Var a7 = m[x][y].gate_var[7]._var;
    if(b_x || b_y) {
        s.assumeProperty(a7, 0);
        v7 = s.newVar();
        s.assumeProperty(v7, 1);
    }
    else {
        v7 = gen_Enum(s, neighbor_var, a7, 7);
    }

    // a8 implication
    Var a8 = m[x][y].gate_var[8]._var;
    if(b_x || b_y) {
        s.assumeProperty(a8, 0);
        v8 = s.newVar();
        s.assumeProperty(v8, 1);
    }
    else {
        v8 = gen_Enum(s, neighbor_var, a8, 8);
    }

    Var out;
    out = s.newVar();
    s.assumeProperty(out, 1);
    out = addANDCNF(s, out, v0, 0, 0);
    out = addANDCNF(s, out, v1, 0, 0);
    out = addANDCNF(s, out, v2, 0, 0);
    out = addANDCNF(s, out, v3, 0, 0);
    out = addANDCNF(s, out, v4, 0, 0);
    out = addANDCNF(s, out, v5, 0, 0);
    out = addANDCNF(s, out, v6, 0, 0);
    out = addANDCNF(s, out, v7, 0, 0);
    out = addANDCNF(s, out, v8, 0, 0);

    return out;
}

// function for check boundary square
bool checkX(int x, int row_num) {
    return (x == 0 || x == row_num-1) ? true : false;
}

bool checkY(int y, int col_num) {
    return (y == 0 || y == col_num-1) ? true : false;
}

// enumerate combination
Var gen_Enum(SatSolver &s, vector<Var> &neighbor_var, Var a, int r) {
    vector<int> source;
    int source_num = neighbor_var.size();
    for(int i=0; i<source_num; i++) {
        source.push_back(i);
    }
    vector<vector<int> > record;
    Combination(source, record, r);

    Var temp_v = s.newVar();
    s.assumeProperty(temp_v, 0);
    for(int i=0; i<record.size(); i++) {
        int p = 0;
        Var v = s.newVar();
        s.assumeProperty(v, 1);
        for(int j=0; j<neighbor_var.size(); j++) {
            if(p < record[i].size()) {
                if(j == record[i][p]) {
                    v = addANDCNF(s, v, neighbor_var[j], 0, 0);
                    p++;
                }
                else {
                    v = addANDCNF(s, v, neighbor_var[j], 0, 1);
                }
            }
            else {
                v = addANDCNF(s, v, neighbor_var[j], 0, 1);
            }
        }
        temp_v = addORCNF(s, temp_v, v, 0, 0);
    }
    temp_v = addORCNF(s, a, temp_v, 1, 0);
    return temp_v;
}

void Combination(vector<int> &source, vector<vector<int> > &record, int r) {
    vector<int> data(r);
    int n = source.size();
    Combination_Util(source, record, data, 0, n-1, 0, r);
}

void Combination_Util(vector<int> &source, vector<vector<int> > &record, vector<int> &data, int s, int e, int index, int r) {
    if(index == r) {
        record.push_back(data);
        return;
    }

    for(int i=s; i<=e && e-i+1>=r-index; i++) {
        data[index] = source[i];
        Combination_Util(source, record, data, i+1, e, index+1, r);
    }
}

void initialSate(SatSolver &s, vector<vector<square> >& m) {
    int row_num = m.size();
    int col_num = m[0].size();
    for(int i=0; i<row_num; i++) {
        for(int j=0; j<col_num; j++) {
            int state = m[i][j].getState();
            if(state != -2) {
                if(state == -1) {
                    Var v = m[i][j].gate_var[9]._var;
                    s.assumeProperty(v, 1);
                }else {
                    Var v = m[i][j].gate_var[state]._var;
                    s.assumeProperty(v, 1);
                }
            }
        }
    }
}

void reportResult(SatSolver &s, bool result, vector<vector<square> > &m, vector<vector<int> > &record) {
    int row_num = m.size();
    int col_num = m[0].size();
    s.printStats();
    cout << (result ? "SAT" : "UNSAT") << endl;
    if(result) {
        for(int i=0; i<row_num; i++) {
            for(int j=0; j<col_num; j++) {
                int t = find_state(s, m[i][j]);
                if(t == 9) {
                    cout << "X" << " ";
                    record[i][j] = -1;
                }
                else {
                    cout << t << " ";
                    record[i][j] = t;
                }
            }
            cout << endl;
        }
    }
}

int find_state(SatSolver &s, square &m) {
    int t;
    for(int i=0; i<m.gate_var.size(); i++) {
        bool value = s.getValue(m.gate_var[i]._var);
        if(value) t = i;
    }
    return t;
}


