#include "../include/SplitwiseSystem.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <stdexcept>

using namespace std;
//keeps the implementation private to this file to avoid name clashes and all
namespace {
// Small tolerance used for floating-point comparisons.
// Example: 99.999 and 100.000 are treated as equal within this margin.
const double EPS = 0.01;

// function to ensure amounts are always positive.
void validatePositiveAmount(double amount) {
    if (amount <= 0.0) {
        throw runtime_error("Amount must be positive");
    }
}

// Converts number from file to enum.
// a data type consisting of named values like elements, members, etc., that represent integral constants
SplitType intToSplitType(int value) {
    if (value == 1) return SplitType::EQUAL;
    if (value == 2) return SplitType::EXACT;
    if (value == 3) return SplitType::PERCENTAGE;
    throw runtime_error("Invalid split type");
}

// Converts enum to number for file storage.
int splitTypeToInt(SplitType type) {
    return static_cast<int>(type);
}
}

// Static counters for auto-generated IDs.
int User::nextUserID = 0;
int Expense::nextExpenseID = 0;
int Group::nextGroupID = 0;
Splitwise* Splitwise::instance = nullptr;

// One split entry means: this user should pay this amount in an expense.
Split::Split(string userID, double amount) {
    this->userID = userID;
    this->amount = amount;
}

vector<Split> EqualSplit::calculateSplit(double totalAmount, vector<string> userIDs, vector<double>) {
    if (userIDs.empty()) {
        throw runtime_error("At least one user is required");
    }

    vector<Split> splits;
    // Equal split: everyone gets same share.
    const double amountPerUser = totalAmount / static_cast<double>(userIDs.size());
    for (string userID : userIDs) {
        splits.push_back(Split(userID, amountPerUser));
    }
    return splits;
}

vector<Split> ExactSplit::calculateSplit(double totalAmount, vector<string> userIDs, vector<double> values) {
    if (userIDs.size() != values.size()) {
        throw runtime_error("Exact split values do not match user count");
    }

    const double sum = accumulate(values.begin(), values.end(), 0.0);
    // Exact split must fully match total amount.
    if (abs(sum - totalAmount) > EPS) {
        throw runtime_error("Exact split values must add up to total amount");
    }

    vector<Split> splits;
    for (int i = 0; i < static_cast<int>(userIDs.size()); ++i) {
        splits.push_back(Split(userIDs[i], values[i]));
    }
    return splits;
}

vector<Split> PercentageSplit::calculateSplit(double totalAmount, vector<string> userIDs, vector<double> values) {
    if (userIDs.size() != values.size()) {
        throw runtime_error("Percentage split values do not match user count");
    }

    const double sum = accumulate(values.begin(), values.end(), 0.0);
    // Percentage split values must total 100.
    if (abs(sum - 100.0) > EPS) {
        throw runtime_error("Percentage split values must add up to 100");
    }

    vector<Split> splits;
    for (int i = 0; i < static_cast<int>(userIDs.size()); ++i) {
        double amount = totalAmount * values[i] / 100.0;
        splits.push_back(Split(userIDs[i], amount));
    }
    return splits;
}

SplitStrategy* SplitFactory::getSplitStrategy(SplitType type) {
    // Factory chooses strategy object based on split type.
    if (type == SplitType::EQUAL) return new EqualSplit();
    if (type == SplitType::EXACT) return new ExactSplit();
    if (type == SplitType::PERCENTAGE) return new PercentageSplit();
    throw runtime_error("Unsupported split type");
}

User::User(string name, string email) {
    // IDs are generated as user1, user2, ...
    userID = "user" + to_string(++nextUserID);
    this->name = name;
    this->email = email;
}

void User::update(string message) {
    cout << "[NOTIFICATION to " << name << "]: " << message << endl;
}

void User::updateBalance(string otherUserID, double amount) {
    // balances is a map:
    // key   -> other user ID
    // value -> money relation with that user
    // positive: other user owes this user
    // negative: this user owes other user
    balances[otherUserID] += amount;
    // Remove near-zero balance to keep map clean.
    if (abs(balances[otherUserID]) < EPS) {
        balances.erase(otherUserID);
    }
}

double User::getTotalOwed() {
    double total = 0;
    for (auto& balance : balances) {
        if (balance.second < 0) {
            total += abs(balance.second);
        }
    }
    return total;
}

double User::getTotalOwing() {
    double total = 0;
    for (auto& balance : balances) {
        if (balance.second > 0) {
            total += balance.second;
        }
    }
    return total;
}

Expense::Expense(string description, double amount, string paidByUserID, vector<Split> splits, string groupID, vector<string> involvedUsers, SplitType splitType, vector<double> splitValues) {
    // IDs are generated as expense1, expense2, ...
    expenseID = "expense" + to_string(++nextExpenseID);
    this->description = description;
    this->totalAmount = amount;
    this->paidByUserID = paidByUserID;
    this->splits = splits;
    this->groupID = groupID;
    this->involvedUsers = involvedUsers;
    this->splitType = splitType;
    this->splitValues = splitValues;
}

map<string, map<string, double>> DebtSimplifier::simplifyDebt(map<string, map<string, double>> groupBalances) {
    // groupBalances is a nested map:
    // outer key   -> a user
    // inner key   -> another user
    // inner value -> balance relation between the two users
    //
    // Purpose: store pairwise debts for all members of a group.
    // We convert this to net amounts and then rebuild a smaller set of debts.
    map<string, double> netAmounts;
    for (auto& userBalance : groupBalances) {
        netAmounts[userBalance.first] = 0;
    }

    for (auto& userBalance : groupBalances) {
        const string creditorID = userBalance.first;
        for (auto& balance : userBalance.second) {
            const string debtorID = balance.first;
            const double amount = balance.second;
            if (amount > 0) {
                netAmounts[creditorID] += amount;
                netAmounts[debtorID] -= amount;
            }
        }
    }

    vector<pair<string, double>> creditors;
    vector<pair<string, double>> debtors;
    // Users with positive net amount are creditors (they should receive money).
    // Users with negative net amount are debtors (they should pay money).
    for (auto& net : netAmounts) {
        if (net.second > EPS) creditors.push_back({net.first, net.second});
        else if (net.second < -EPS) debtors.push_back({net.first, -net.second});
    }

    sort(creditors.begin(), creditors.end(), [](const pair<string, double>& a, const pair<string, double>& b) {
        return a.second > b.second;
    });
    sort(debtors.begin(), debtors.end(), [](const pair<string, double>& a, const pair<string, double>& b) {
        return a.second > b.second;
    });

    map<string, map<string, double>> simplifiedBalances;
    // Initialize empty structure with all users.
    for (auto& userBalance : groupBalances) {
        simplifiedBalances[userBalance.first] = map<string, double>();
    }

    size_t i = 0, j = 0;
    // Greedy matching: largest creditor with largest debtor.
    // This reduces number of transactions while preserving final result.
    while (i < creditors.size() && j < debtors.size()) {
        string creditorID = creditors[i].first;
        string debtorID = debtors[j].first;
        double settleAmount = min(creditors[i].second, debtors[j].second);

        simplifiedBalances[creditorID][debtorID] = settleAmount;
        simplifiedBalances[debtorID][creditorID] = -settleAmount;

        creditors[i].second -= settleAmount;
        debtors[j].second -= settleAmount;

        if (creditors[i].second < EPS) ++i;
        if (debtors[j].second < EPS) ++j;
    }

    return simplifiedBalances;
}

User* Group::getUserByUserID(string userID) {
    // Linear search in group member list.
    for (User* member : members) {
        if (member->userID == userID) {
            return member;
        }
    }
    return nullptr;
}

Group::Group(string name) {
    // IDs are generated as group1, group2, ...
    groupID = "group" + to_string(++nextGroupID);
    this->name = name;
}

Group::~Group() {
    for (auto& pair : groupExpenses) {
        delete pair.second;
    }
}

void Group::addMember(User* user) {
    members.push_back(user);
    // groupBalances map tracks debts inside this group only.
    // Create empty inner map for new member.
    groupBalances[user->userID] = map<string, double>();
    cout << user->name << " added to group " << name << endl;
}

bool Group::removeMember(string userID) {
    if (!canUserLeaveGroup(userID)) {
        cout << "\nUser is not allowed to leave group without clearing expenses" << endl;
        return false;
    }

    auto it = remove_if(members.begin(), members.end(), [&](User* user) {
        return user->userID == userID;
    });
    members.erase(it, members.end());

    groupBalances.erase(userID);
    for (auto& memberBalance : groupBalances) {
        memberBalance.second.erase(userID);
    }
    return true;
}

void Group::notifyMembers(string message) {
    for (User* member : members) {
        member->update(message);
    }
}

bool Group::isMember(string userID) {
    // Checking membership by key existence in groupBalances map.
    return groupBalances.find(userID) != groupBalances.end();
}

void Group::updateGroupBalance(string fromUserID, string toUserID, double amount) {
    // Nested map purpose:
    // groupBalances[A][B] means how much B owes A.
    // store mirrored entries so both directions stay consistent.
    groupBalances[fromUserID][toUserID] += amount;
    groupBalances[toUserID][fromUserID] -= amount;

    // Remove almost-zero entries to avoid clutter.
    if (abs(groupBalances[fromUserID][toUserID]) < EPS) groupBalances[fromUserID].erase(toUserID);
    if (abs(groupBalances[toUserID][fromUserID]) < EPS) groupBalances[toUserID].erase(fromUserID);
}

bool Group::canUserLeaveGroup(string userID) {
    if (!isMember(userID)) {
        throw runtime_error("User is not a part of this group");
    }

    map<string, double> userBalanceSheet = groupBalances[userID];
    for (auto& balance : userBalanceSheet) {
        if (abs(balance.second) > EPS) return false;
    }
    return true;
}

map<string, double> Group::getUserGroupBalances(string userID) {
    if (!isMember(userID)) {
        throw runtime_error("User is not a part of this group");
    }
    return groupBalances[userID];
}

bool Group::addExpense(string description, double amount, string paidByUserID, vector<string> involvedUsers, SplitType splitType, vector<double> splitValues) {
    if (!isMember(paidByUserID)) {
        throw runtime_error("Paid by user is not a part of this group");
    }

    for (string userID : involvedUsers) {
        if (!isMember(userID)) {
            throw runtime_error("Involved users are not a part of this group");
        }
    }

    // Choose strategy at runtime (equal/exact/percentage).
    SplitStrategy* strategy = SplitFactory::getSplitStrategy(splitType);
    vector<Split> splits = strategy->calculateSplit(amount, involvedUsers, splitValues);
    delete strategy;

    // Save expense in group map by expense ID.
    Expense* expense = new Expense(description, amount, paidByUserID, splits, groupID, involvedUsers, splitType, splitValues);
    groupExpenses[expense->expenseID] = expense;

    // For each participant (except payer), update group debt towards payer.
    for (Split& split : splits) {
        if (split.userID != paidByUserID) {
            updateGroupBalance(paidByUserID, split.userID, split.amount);
        }
    }

    cout << endl << "----------- Sending Notifications ----------" << endl;
    notifyMembers("New expense added: " + description + " (Rs " + to_string(amount) + ")");
    return true;
}

bool Group::settlePayment(string fromUserID, string toUserID, double amount) {
    if (!isMember(fromUserID) || !isMember(toUserID)) {
        cout << "User is not a part of this group" << endl;
        return false;
    }

    if (amount <= 0.0) {
        cout << "Settlement amount must be positive" << endl;
        return false;
    }

    // groupBalances[creditor][debtor] means debtor owes creditor.
    // If fromUser pays toUser, then fromUser must owe toUser first.
    const double currentDebt = groupBalances[toUserID][fromUserID];
    if (currentDebt <= EPS) {
        cout << fromUserID << " does not owe " << toUserID << " in this group" << endl;
        return false;
    }

    if (amount - currentDebt > EPS) {
        cout << "Settlement amount exceeds outstanding debt (Rs " << fixed << setprecision(2) << currentDebt << ")" << endl;
        return false;
    }

    // Reduce debt: fromUser pays toUser.
    updateGroupBalance(toUserID, fromUserID, -amount);
    notifyMembers("Settlement made: " + fromUserID + " paid " + toUserID + " Rs " + to_string(amount));
    return true;
}

void Group::showGroupBalances() {
    cout << "\n--- Group Balances for " << name << " ---" << endl;
    bool hasOutstanding = false;
    for (auto& pair : groupBalances) {
        string memberID = pair.first;
        User* member = getUserByUserID(memberID);
        string memberName = member ? member->name : memberID;

        for (auto& userBalance : pair.second) {
            double balance = userBalance.second;
            if (balance <= EPS) {
                continue;
            }

            hasOutstanding = true;
            string otherMemberUserID = userBalance.first;
            User* otherMember = getUserByUserID(otherMemberUserID);
            string otherName = otherMember ? otherMember->name : otherMemberUserID;
            cout << " " << otherName << " owes " << memberName << ": Rs " << fixed << setprecision(2) << balance << endl;
        }
    }

    if (!hasOutstanding) {
        cout << " No outstanding balances" << endl;
    }
}

void Group::simplifyGroupDebts() {
    // Replaces current debt graph with simplified equivalent graph.
    map<string, map<string, double>> simplifiedBalances = DebtSimplifier::simplifyDebt(groupBalances);
    groupBalances = simplifiedBalances;
    cout << "\nDebts have been simplified for group: " << name << endl;
}

Splitwise* Splitwise::getInstance() {
    // Lazy singleton creation -> instance is only created when requested.
    if (instance == nullptr) {
        instance = new Splitwise();
    }
    return instance;
}

Splitwise::Splitwise() {}

User* Splitwise::createUser(string name, string email) {
    User* user = new User(name, email);
    // users map: key=userID, value=User object pointer.
    users[user->userID] = user;
    cout << "User created: " << name << " (ID: " << user->userID << ")" << endl;
    return user;
}

User* Splitwise::getUser(string userID) {
    auto it = users.find(userID);
    if (it != users.end()) {
        return it->second;
    }
    return nullptr;
}

Group* Splitwise::createGroup(string name) {
    Group* group = new Group(name);
    // groups map: key=groupID, value=Group object pointer.
    groups[group->groupID] = group;
    cout << "Group created: " << name << " (ID: " << group->groupID << ")" << endl;
    return group;
}

Group* Splitwise::getGroup(string groupID) {
    auto it = groups.find(groupID);
    if (it != groups.end()) {
        return it->second;
    }
    return nullptr;
}

void Splitwise::addUserToGroup(string userID, string groupID) {
    User* user = getUser(userID);
    Group* group = getGroup(groupID);
    if (user && group) {
        group->addMember(user);
    }
}

bool Splitwise::removeUserFromGroup(string userID, string groupID) {
    Group* group = getGroup(groupID);
    if (!group) {
        cout << "Group not found\n";
        return false;
    }

    User* user = getUser(userID);
    if (!user) {
        cout << "User not found!" << endl;
        return false;
    }

    bool removed = group->removeMember(userID);
    if (removed) {
        cout << user->name << " successfully left " << group->name << endl;
    }
    return removed;
}

void Splitwise::addExpenseToGroup(string groupID, string description, double amount, string paidByUserID, vector<string> involvedUsers, SplitType splitType, vector<double> splitValues) {
    Group* group = getGroup(groupID);
    if (!group) {
        cout << "Group not found" << endl;
        return;
    }
    group->addExpense(description, amount, paidByUserID, involvedUsers, splitType, splitValues);
}

void Splitwise::settlePaymentInGroup(string groupID, string fromUserID, string toUserID, double amount) {
    Group* group = getGroup(groupID);
    if (!group) {
        cout << "Group not found" << endl;
        return;
    }
    group->settlePayment(fromUserID, toUserID, amount);
}

void Splitwise::settleIndividualPayment(string fromUserID, string toUserID, double amount) {
    User* fromUser = getUser(fromUserID);
    User* toUser = getUser(toUserID);
    if (fromUser && toUser) {
        // User-level map balances are updated in opposite directions.
        fromUser->updateBalance(toUserID, amount);
        toUser->updateBalance(fromUserID, -amount);
        cout << fromUser->name << " settled Rs " << amount << " with " << toUser->name << endl;
    }
}

void Splitwise::addIndividualExpense(string description, double amount, string paidByUserID, string toUserID, SplitType splitType, vector<double> splitValues) {
    // Reuse split strategy logic even for 2 users.
    SplitStrategy* strategy = SplitFactory::getSplitStrategy(splitType);
    vector<Split> splits = strategy->calculateSplit(amount, {paidByUserID, toUserID}, splitValues);
    delete strategy;

    Expense* expense = new Expense(description, amount, paidByUserID, splits, "", {paidByUserID, toUserID}, splitType, splitValues);
    // expenses map stores non-group (individual) expenses.
    expenses[expense->expenseID] = expense;

    User* paidByUser = getUser(paidByUserID);
    User* toUser = getUser(toUserID);
    if (!paidByUser || !toUser) {
        throw runtime_error("User not found for individual expense");
    }

    paidByUser->updateBalance(toUserID, amount);
    toUser->updateBalance(paidByUserID, -amount);
    cout << "Individual expense added " << description << " (Rs " << amount << ") paid by " << paidByUser->name << " for " << toUser->name << endl;
}

void Splitwise::showUserBalance(string userID) {
    User* user = getUser(userID);
    if (!user) return;

    map<string, double> combinedBalances = user->balances;
    for (auto& groupPair : groups) {
        Group* group = groupPair.second;
        if (!group->isMember(userID)) {
            continue;
        }

        map<string, double> groupBalances = group->getUserGroupBalances(userID);
        for (auto& balance : groupBalances) {
            combinedBalances[balance.first] += balance.second;
            if (abs(combinedBalances[balance.first]) < EPS) {
                combinedBalances.erase(balance.first);
            }
        }
    }

    double totalOwed = 0.0;
    double totalOwing = 0.0;
    for (const auto& balance : combinedBalances) {
        if (balance.second < 0) {
            totalOwed += abs(balance.second);
        } else if (balance.second > 0) {
            totalOwing += balance.second;
        }
    }

    cout << endl << "---------- Balance for " << user->name << " ----------" << endl;
    cout << "Total you owe: Rs " << fixed << setprecision(2) << totalOwed << endl;
    cout << "Total others owe you: Rs " << fixed << setprecision(2) << totalOwing << endl;

    cout << "Detailed balances: " << endl;
    if (combinedBalances.empty()) {
        cout << " No outstanding balances" << endl;
        return;
    }

    // combinedBalances includes both individual and group-level balances.
    for (auto& balance : combinedBalances) {
        User* otherUser = getUser(balance.first);
        if (otherUser) {
            if (balance.second > 0) {
                cout << " " << otherUser->name << " owes you: Rs " << fixed << setprecision(2) << balance.second << endl;
            } else {
                cout << " You owe " << otherUser->name << ": Rs " << fixed << setprecision(2) << abs(balance.second) << endl;
            }
        }
    }
}

void Splitwise::showGroupBalances(string groupID) {
    Group* group = getGroup(groupID);
    if (!group) return;
    group->showGroupBalances();
}

void Splitwise::simplifyGroupDebts(string groupID) {
    Group* group = getGroup(groupID);
    if (!group) return;
    group->simplifyGroupDebts();
}

void Splitwise::saveToFile(string fileName) {
    ofstream out(fileName);
    if (!out.is_open()) {
        throw runtime_error("Unable to open file for saving");
    }

    // File header/version marker.
    out << "SPLITWISE_V1\n";
    // Save static counters so ID continuity is preserved after loading.
    out << User::nextUserID << " " << Group::nextGroupID << " " << Expense::nextExpenseID << "\n";

    // Save all users from users map.
    out << users.size() << "\n";
    for (auto& pair : users) {
        User* user = pair.second;
        out << quoted(user->userID) << " " << quoted(user->name) << " " << quoted(user->email) << "\n";
    }

    // Save groups and member IDs (not full user objects).
    out << groups.size() << "\n";
    for (auto& pair : groups) {
        Group* group = pair.second;
        out << quoted(group->groupID) << " " << quoted(group->name) << " " << group->members.size();
        for (User* member : group->members) {
            out << " " << quoted(member->userID);
        }
        out << "\n";
    }

    vector<Expense*> allExpenses;
    // Combine group expenses and individual expenses into one list for storage.
    for (auto& pair : groups) {
        Group* group = pair.second;
        for (auto& expensePair : group->groupExpenses) {
            allExpenses.push_back(expensePair.second);
        }
    }
    for (auto& pair : expenses) {
        allExpenses.push_back(pair.second);
    }

    out << allExpenses.size() << "\n";
    for (Expense* expense : allExpenses) {
        out << quoted(expense->expenseID) << " " << quoted(expense->description) << " " << fixed << setprecision(2) << expense->totalAmount << " "
            << quoted(expense->paidByUserID) << " " << quoted(expense->groupID) << " " << splitTypeToInt(expense->splitType) << " "
            << expense->involvedUsers.size();
        for (string userID : expense->involvedUsers) {
            out << " " << quoted(userID);
        }
        out << " " << expense->splitValues.size();
        for (double value : expense->splitValues) {
            out << " " << fixed << setprecision(2) << value;
        }
        out << "\n";
    }
}

void Splitwise::loadFromFile(string fileName) {
    ifstream in(fileName);
    if (!in.is_open()) {
        throw runtime_error("Unable to open file for loading");
    }

    // Validate header to ensure correct file format.
    string header;
    in >> header;
    if (header != "SPLITWISE_V1") {
        throw runtime_error("Invalid file format");
    }

    // Clear old in-memory data before loading new data.
    for (auto& pair : users) delete pair.second;
    for (auto& pair : groups) delete pair.second;
    for (auto& pair : expenses) delete pair.second;
    users.clear();
    groups.clear();
    expenses.clear();

    // Restore ID counters exactly as saved.
    int savedNextUserID = 0;
    int savedNextGroupID = 0;
    int savedNextExpenseID = 0;
    in >> savedNextUserID >> savedNextGroupID >> savedNextExpenseID;

    // Recreate users map.
    size_t userCount = 0;
    in >> userCount;
    for (int i = 0; i < static_cast<int>(userCount); ++i) {
        string userID, name, email;
        in >> quoted(userID) >> quoted(name) >> quoted(email);
        User* user = new User(name, email);
        user->userID = userID;
        users[userID] = user;
    }

    // Recreate groups and reconnect members by user IDs.
    size_t groupCount = 0;
    in >> groupCount;
    for (int i = 0; i < static_cast<int>(groupCount); ++i) {
        string groupID, name;
        size_t memberCount = 0;
        in >> quoted(groupID) >> quoted(name) >> memberCount;
        Group* group = new Group(name);
        group->groupID = groupID;
        group->members.clear();
        group->groupBalances.clear();
        groups[groupID] = group;

        for (int j = 0; j < static_cast<int>(memberCount); ++j) {
            string memberID;
            in >> quoted(memberID);
            if (users.find(memberID) != users.end()) {
                group->members.push_back(users[memberID]);
                group->groupBalances[memberID] = map<string, double>();
            }
        }
    }

    // Recreate each expense and rebuild balances by replaying split effects.
    size_t expenseCount = 0;
    in >> expenseCount;
    for (int i = 0; i < static_cast<int>(expenseCount); ++i) {
        string expenseID, description, paidByUserID, groupID;
        double amount = 0.0;
        int splitTypeInt = 0;
        size_t involvedCount = 0;
        size_t valueCount = 0;

        in >> quoted(expenseID) >> quoted(description) >> amount >> quoted(paidByUserID) >> quoted(groupID) >> splitTypeInt >> involvedCount;
        vector<string> involvedUsers;
        for (int j = 0; j < static_cast<int>(involvedCount); ++j) {
            string userID;
            in >> quoted(userID);
            involvedUsers.push_back(userID);
        }

        in >> valueCount;
        vector<double> splitValues;
        for (int j = 0; j < static_cast<int>(valueCount); ++j) {
            double value = 0.0;
            in >> value;
            splitValues.push_back(value);
        }

        SplitType splitType = intToSplitType(splitTypeInt);
        SplitStrategy* strategy = SplitFactory::getSplitStrategy(splitType);
        vector<Split> splits = strategy->calculateSplit(amount, involvedUsers, splitValues);
        delete strategy;

        Expense* expense = new Expense(description, amount, paidByUserID, splits, groupID, involvedUsers, splitType, splitValues);
        expense->expenseID = expenseID;

        if (groupID.empty()) {
            expenses[expenseID] = expense;
            User* paidByUser = getUser(paidByUserID);
            if (paidByUser) {
                for (Split& split : splits) {
                    if (split.userID != paidByUserID) {
                        paidByUser->updateBalance(split.userID, split.amount);
                        User* otherUser = getUser(split.userID);
                        if (otherUser) {
                            otherUser->updateBalance(paidByUserID, -split.amount);
                        }
                    }
                }
            }
        } else {
            Group* group = getGroup(groupID);
            if (group) {
                group->groupExpenses[expenseID] = expense;
                for (Split& split : splits) {
                    if (split.userID != paidByUserID) {
                        group->updateGroupBalance(paidByUserID, split.userID, split.amount);
                    }
                }
            } else {
                delete expense;
            }
        }
    }

    User::nextUserID = savedNextUserID;
    Group::nextGroupID = savedNextGroupID;
    Expense::nextExpenseID = savedNextExpenseID;
}
