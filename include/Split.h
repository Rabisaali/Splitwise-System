#ifndef SPLIT_H
#define SPLIT_H

#include <string>

using namespace std;

struct Split {
    // User involved in expense and their share amount.
    string userID;
    double amount;

    Split(string userID, double amount);
};

#endif
