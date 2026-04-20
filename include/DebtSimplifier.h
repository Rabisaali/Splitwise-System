#ifndef DEBT_SIMPLIFIER_H
#define DEBT_SIMPLIFIER_H

#include <map>
#include <string>

using namespace std;

class DebtSimplifier {
public:
    // Accepts nested map of group balances and returns a simplified version.
    // Outer key: user, inner key: other user, value: amount between them.
    static map<string, map<string, double>> simplifyDebt(map<string, map<string, double>> groupBalances);
};

#endif