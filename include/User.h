#ifndef USER_H
#define USER_H

#include "Observer.h"

#include <map>
#include <string>

using namespace std;

class User : public Observer {
public:
    // Static counter to auto-generate user IDs like user1, user2, ...
    static int nextUserID;
    string userID;
    string name;
    string email;
    // balances map purpose:
    // key   -> other user's ID
    // value -> signed amount between current user and that user
    // positive means other user owes this user
    // negative means this user owes other user
    map<string, double> balances;

    User(string name, string email);

    void update(string message) override;

    void updateBalance(string otherUserID, double amount);
    double getTotalOwed();
    double getTotalOwing();
};

#endif
