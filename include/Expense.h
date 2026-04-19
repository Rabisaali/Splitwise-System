#ifndef EXPENSE_H
#define EXPENSE_H

#include "Split.h"
#include "SplitType.h"

#include <string>
#include <vector>

using namespace std;

class Expense {
public:
    static int nextExpenseID;
    string expenseID;
    string description;
    double totalAmount;
    string paidByUserID;
    vector<Split> splits;
    string groupID;
    vector<string> involvedUsers;
    SplitType splitType;
    vector<double> splitValues;

    Expense(string description, double amount, string paidByUserID, vector<Split> splits, string groupID = "", vector<string> involvedUsers = {}, SplitType splitType = SplitType::EQUAL, vector<double> splitValues = {});
};

#endif
