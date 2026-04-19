#ifndef USER_H
#define USER_H

#include "Observer.h"

#include <map>
#include <string>

using namespace std;

class User : public Observer {
public:
    static int nextUserID;
    string userID;
    string name;
    string email;
    map<string, double> balances;

    User(string name, string email);

    void update(string message) override;

    void updateBalance(string otherUserID, double amount);
    double getTotalOwed();
    double getTotalOwing();
};

#endif
