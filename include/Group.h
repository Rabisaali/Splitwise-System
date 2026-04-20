#ifndef GROUP_H
#define GROUP_H

#include "Expense.h"
#include "SplitStrategy.h"
#include "SplitFactory.h"
#include "User.h"

#include <map>
#include <string>
#include <vector>

using namespace std;

class Group {
private:
    User* getUserByUserID(string userID);

public:
    static int nextGroupID;
    string groupID;
    string name;
    vector<User*> members;
    // Stores all expenses that belong to this group.
    // key = expense ID, value = expense object pointer.
    map<string, Expense*> groupExpenses;
    // Nested map that stores pairwise balances inside the group.
    // groupBalances[A][B] represents money relation between A and B.
    // This structure helps quickly update or read debts per member pair.
    map<string, map<string, double>> groupBalances;

    Group(string name);
    ~Group();

    void addMember(User* user);
    bool removeMember(string userID);
    void notifyMembers(string message);
    bool isMember(string userID);
    void updateGroupBalance(string fromUserID, string toUserID, double amount);
    bool canUserLeaveGroup(string userID);
    map<string, double> getUserGroupBalances(string userID);
    bool addExpense(string description, double amount, string paidByUserID, vector<string> involvedUsers, SplitType splitType, vector<double> splitValues = {});
    bool settlePayment(string fromUserID, string toUserID, double amount);
    void showGroupBalances();
    void simplifyGroupDebts();
};

#endif