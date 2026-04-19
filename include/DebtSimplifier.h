#ifndef DEBT_SIMPLIFIER_H
#define DEBT_SIMPLIFIER_H

#include <map>
#include <string>

using namespace std;

class DebtSimplifier {
public:
    static map<string, map<string, double>> simplifyDebt(map<string, map<string, double>> groupBalances);
};

#endif