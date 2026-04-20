#ifndef EXPENSE_H
#define EXPENSE_H

#include "Split.h"
#include "SplitType.h"

#include <string>
#include <vector>

using namespace std;

class Expense {
public:
    // Static counter to auto-generate expense IDs.
    static int nextExpenseID;
    string expenseID;
    string description;
    double totalAmount;
    string paidByUserID;
    // Final computed per-user amounts for this expense.
    vector<Split> splits;
    // Empty for individual expense, non-empty for group expense.
    string groupID;
    // Users who are part of this expense.
    vector<string> involvedUsers;
    SplitType splitType;
    // Input values used by exact/percentage split types.
    vector<double> splitValues;

    Expense(string description, double amount, string paidByUserID, vector<Split> splits, string groupID = "", vector<string> involvedUsers = {}, SplitType splitType = SplitType::EQUAL, vector<double> splitValues = {});
};

#endif
