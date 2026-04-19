#ifndef SPLITWISE_SYSTEM_H
#define SPLITWISE_SYSTEM_H

#include "DebtSimplifier.h"
#include "Expense.h"
#include "Group.h"
#include "SplitFactory.h"
#include "SplitType.h"
#include "User.h"

#include <map>
#include <string>
#include <vector>

using namespace std;

class Splitwise {
private:
    map<string, User*> users;
    map<string, Group*> groups;
    map<string, Expense*> expenses;
    static Splitwise* instance;

    Splitwise();

public:
    static Splitwise* getInstance();

    User* createUser(string name, string email);
    User* getUser(string userID);
    Group* createGroup(string name);
    Group* getGroup(string groupID);
    void addUserToGroup(string userID, string groupID);
    bool removeUserFromGroup(string userID, string groupID);
    void addExpenseToGroup(string groupID, string description, double amount, string paidByUserID, vector<string> involvedUsers, SplitType splitType, vector<double> splitValues = {});
    void settlePaymentInGroup(string groupID, string fromUserID, string toUserID, double amount);
    void settleIndividualPayment(string fromUserID, string toUserID, double amount);
    void addIndividualExpense(string description, double amount, string paidByUserID, string toUserID, SplitType splitType, vector<double> splitValues = {});
    void showUserBalance(string userID);
    void showGroupBalances(string groupID);
    void simplifyGroupDebts(string groupID);

    void saveToFile(string fileName);
    void loadFromFile(string fileName);
};

#endif
