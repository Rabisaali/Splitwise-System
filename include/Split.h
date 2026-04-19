#ifndef SPLIT_H
#define SPLIT_H

#include <string>

using namespace std;

struct Split {
    string userID;
    double amount;

    Split(string userID, double amount);
};

#endif
