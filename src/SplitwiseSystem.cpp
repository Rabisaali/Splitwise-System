#include "../include/SplitwiseSystem.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

using namespace std;

namespace {
constexpr double EPS = 0.01;

string toLower(string text) {
    transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
        return static_cast<char>(tolower(c));
    });
    return text;
}

int splitTypeToInt(SplitType type) {
    return static_cast<int>(type);
}

SplitType intToSplitType(int value) {
    if (value == 1) return SplitType::EQUAL;
    if (value == 2) return SplitType::EXACT;
    if (value == 3) return SplitType::PERCENTAGE;
    throw runtime_error("Invalid split type in file");
}

void validatePositiveAmount(double amount) {
    if (amount <= 0.0) {
        throw runtime_error("Amount must be positive");
    }
}
}

Split::Split(const string& userId, double amount)
    : userId(userId), amount(amount) {}

vector<Split> EqualSplit::calculateSplit(
    double totalAmount,
    const vector<string>& userIds,
    const vector<double>&
) {
    if (userIds.empty()) {
        throw runtime_error("At least one involved user is required");
    }

    vector<Split> splits;
    const double amountPerUser = totalAmount / static_cast<double>(userIds.size());

    for (const string& userId : userIds) {
        splits.emplace_back(userId, amountPerUser);
    }

    return splits;
}

vector<Split> ExactSplit::calculateSplit(
    double totalAmount,
    const vector<string>& userIds,
    const vector<double>& values
) {
    if (userIds.size() != values.size()) {
        throw runtime_error("Exact split values count must match involved users count");
    }

    vector<Split> splits;
    for (size_t i = 0; i < userIds.size(); ++i) {
        if (values[i] < 0.0) {
            throw runtime_error("Exact split value cannot be negative");
        }
        splits.emplace_back(userIds[i], values[i]);
    }

    const double sum = accumulate(values.begin(), values.end(), 0.0);
    if (abs(sum - totalAmount) > EPS) {
        throw runtime_error("Exact split values must sum to total amount");
    }

    return splits;
}

vector<Split> PercentageSplit::calculateSplit(
    double totalAmount,
    const vector<string>& userIds,
    const vector<double>& values
) {
    if (userIds.size() != values.size()) {
        throw runtime_error("Percentage values count must match involved users count");
    }

    vector<Split> splits;

    for (size_t i = 0; i < userIds.size(); ++i) {
        if (values[i] < 0.0) {
            throw runtime_error("Percentage cannot be negative");
        }
        const double amount = totalAmount * values[i] / 100.0;
        splits.emplace_back(userIds[i], amount);
    }

    const double sumPercentage = accumulate(values.begin(), values.end(), 0.0);
    if (abs(sumPercentage - 100.0) > EPS) {
        throw runtime_error("Percentage values must sum to 100");
    }

    return splits;
}

SplitStrategy* SplitFactory::createStrategy(SplitType type) {
    if (type == SplitType::EQUAL) return new EqualSplit();
    if (type == SplitType::EXACT) return new ExactSplit();
    if (type == SplitType::PERCENTAGE) return new PercentageSplit();
    throw runtime_error("Unsupported split type");
}

User::User(const string& userId, const string& name, const string& email)
    : userID(userId), name(name), email(email) {}

const string& User::getId() const { return userID; }
const string& User::getName() const { return name; }
const string& User::getEmail() const { return email; }

void User::setName(const string& newName) { name = newName; }
void User::setEmail(const string& newEmail) { email = newEmail; }

void User::update(const string& message) {
    cout << "[Notification for " << name << "] " << message << "\n";
}

void User::updateBalance(const string& otherUserID, double amount) {
    balances[otherUserID] += amount;
    if (abs(balances[otherUserID]) < EPS) {
        balances.erase(otherUserID);
    }
}

void User::clearBalances() {
    balances.clear();
}

const map<string, double>& User::getBalances() const {
    return balances;
}

Expense::Expense(
    int expenseId,
    const string& description,
    double totalAmount,
    const string& paidByUserId,
    const vector<string>& involvedUsers,
    SplitType splitType,
    const vector<double>& splitValues,
    const vector<Split>& splits
)
    : expenseID(expenseId),
      description(description),
      totalAmount(totalAmount),
      paidByUserID(paidByUserId),
      involvedUsers(involvedUsers),
      splitType(splitType),
      splitValues(splitValues),
      splits(splits) {}

int Expense::getId() const { return expenseID; }
const string& Expense::getDescription() const { return description; }
double Expense::getTotalAmount() const { return totalAmount; }
const string& Expense::getPaidByUserId() const { return paidByUserID; }
const vector<string>& Expense::getInvolvedUsers() const { return involvedUsers; }
SplitType Expense::getSplitType() const { return splitType; }
const vector<double>& Expense::getSplitValues() const { return splitValues; }
const vector<Split>& Expense::getSplits() const { return splits; }

void Expense::setDescription(const string& newDescription) { description = newDescription; }
void Expense::setTotalAmount(double amount) { totalAmount = amount; }
void Expense::setSplitValues(const vector<double>& newSplitValues) { splitValues = newSplitValues; }
void Expense::setSplits(const vector<Split>& newSplits) { splits = newSplits; }

Splitwise::Splitwise()
    : nextUserID(1), nextExpenseID(1) {}

Splitwise::~Splitwise() {
    clearDynamicMemory();
}

void Splitwise::clearDynamicMemory() {
    for (auto& entry : users) {
        delete entry.second;
    }
    users.clear();

    for (auto& entry : expenses) {
        delete entry.second;
    }
    expenses.clear();
}

User* Splitwise::requireUser(const string& userID) const {
    auto it = users.find(userID);
    if (it == users.end()) {
        throw runtime_error("User not found: " + userID);
    }
    return it->second;
}

Expense* Splitwise::requireExpense(int expenseID) const {
    auto it = expenses.find(expenseID);
    if (it == expenses.end()) {
        throw runtime_error("Expense not found: " + to_string(expenseID));
    }
    return it->second;
}

void Splitwise::recalculateBalances() {
    for (auto& userEntry : users) {
        userEntry.second->clearBalances();
    }

    for (const auto& expenseEntry : expenses) {
        const Expense* expense = expenseEntry.second;
        const string& payerId = expense->getPaidByUserId();

        for (const Split& split : expense->getSplits()) {
            if (split.userId == payerId) {
                continue;
            }
            users[payerId]->updateBalance(split.userId, split.amount);
            users[split.userId]->updateBalance(payerId, -split.amount);
        }
    }
}

string Splitwise::addUser(const string& name, const string& email) {
    if (name.empty() || email.empty()) {
        throw runtime_error("Name and email cannot be empty");
    }

    const string userId = "user" + to_string(nextUserID++);
    users[userId] = new User(userId, name, email);
    return userId;
}

bool Splitwise::updateUser(const string& userID, const string& newName, const string& newEmail) {
    User* user = requireUser(userID);
    if (newName.empty() || newEmail.empty()) {
        throw runtime_error("Updated name and email cannot be empty");
    }

    user->setName(newName);
    user->setEmail(newEmail);
    return true;
}

bool Splitwise::deleteUser(const string& userID) {
    requireUser(userID);

    for (const auto& expenseEntry : expenses) {
        const Expense* expense = expenseEntry.second;
        if (expense->getPaidByUserId() == userID) {
            throw runtime_error("Cannot delete user who paid an expense");
        }

        const auto& involved = expense->getInvolvedUsers();
        if (find(involved.begin(), involved.end(), userID) != involved.end()) {
            throw runtime_error("Cannot delete user who is involved in an expense");
        }
    }

    delete users[userID];
    users.erase(userID);
    return true;
}

int Splitwise::addExpense(
    const string& description,
    double amount,
    const string& paidByUserID,
    const vector<string>& involvedUsers,
    SplitType splitType,
    const vector<double>& splitValues
) {
    if (description.empty()) {
        throw runtime_error("Description cannot be empty");
    }
    validatePositiveAmount(amount);
    requireUser(paidByUserID);

    if (involvedUsers.empty()) {
        throw runtime_error("At least one involved user is required");
    }

    for (const string& userId : involvedUsers) {
        requireUser(userId);
    }

    SplitStrategy* strategy = SplitFactory::createStrategy(splitType);
    vector<Split> splits;
    try {
        splits = strategy->calculateSplit(amount, involvedUsers, splitValues);
    } catch (...) {
        delete strategy;
        throw;
    }
    delete strategy;

    const int expenseId = nextExpenseID++;
    expenses[expenseId] = new Expense(
        expenseId,
        description,
        amount,
        paidByUserID,
        involvedUsers,
        splitType,
        splitValues,
        splits
    );

    recalculateBalances();

    for (const string& userId : involvedUsers) {
        users[userId]->update("Expense added: " + description + " (ID " + to_string(expenseId) + ")");
    }

    return expenseId;
}

bool Splitwise::updateExpense(int expenseID, const string& newDescription, double newAmount) {
    validatePositiveAmount(newAmount);

    Expense* expense = requireExpense(expenseID);
    if (newDescription.empty()) {
        throw runtime_error("Updated description cannot be empty");
    }

    vector<double> newSplitValues = expense->getSplitValues();

    if (expense->getSplitType() == SplitType::EXACT) {
        const double oldAmount = expense->getTotalAmount();
        if (oldAmount <= 0.0) {
            throw runtime_error("Invalid existing amount for exact split update");
        }
        const double factor = newAmount / oldAmount;
        for (double& value : newSplitValues) {
            value *= factor;
        }
    }

    SplitStrategy* strategy = SplitFactory::createStrategy(expense->getSplitType());
    vector<Split> newSplits;
    try {
        newSplits = strategy->calculateSplit(newAmount, expense->getInvolvedUsers(), newSplitValues);
    } catch (...) {
        delete strategy;
        throw;
    }
    delete strategy;

    expense->setDescription(newDescription);
    expense->setTotalAmount(newAmount);
    expense->setSplitValues(newSplitValues);
    expense->setSplits(newSplits);

    recalculateBalances();
    return true;
}

bool Splitwise::deleteExpense(int expenseID) {
    Expense* expense = requireExpense(expenseID);
    delete expense;
    expenses.erase(expenseID);

    recalculateBalances();
    return true;
}

vector<User*> Splitwise::searchUsersByName(const string& keyword) const {
    if (keyword.empty()) {
        throw runtime_error("Search keyword cannot be empty");
    }

    const string needle = toLower(keyword);
    vector<User*> results;

    for (const auto& userEntry : users) {
        const string hay = toLower(userEntry.second->getName());
        if (hay.find(needle) != string::npos) {
            results.push_back(userEntry.second);
        }
    }

    return results;
}

vector<Expense*> Splitwise::filterExpensesByPayer(const string& payerUserId) const {
    requireUser(payerUserId);

    vector<Expense*> results;
    for (const auto& expenseEntry : expenses) {
        if (expenseEntry.second->getPaidByUserId() == payerUserId) {
            results.push_back(expenseEntry.second);
        }
    }

    return results;
}

void Splitwise::showAllUsers() const {
    cout << "\nUsers:\n";
    if (users.empty()) {
        cout << "No users found\n";
        return;
    }

    for (const auto& userEntry : users) {
        const User* user = userEntry.second;
        cout << user->getId() << " | " << user->getName() << " | " << user->getEmail() << "\n";
    }
}

void Splitwise::showAllExpenses() const {
    cout << "\nExpenses:\n";
    if (expenses.empty()) {
        cout << "No expenses found\n";
        return;
    }

    for (const auto& expenseEntry : expenses) {
        const Expense* expense = expenseEntry.second;
        cout << "ID " << expense->getId() << " | "
                  << expense->getDescription() << " | Rs "
                  << fixed << setprecision(2) << expense->getTotalAmount()
                  << " | paid by " << expense->getPaidByUserId() << "\n";
    }
}

void Splitwise::showBalances() const {
    cout << "\nBalances:\n";
    for (const auto& userEntry : users) {
        const User* user = userEntry.second;
        cout << user->getName() << " (" << user->getId() << ")\n";
        const auto& balances = user->getBalances();

        if (balances.empty()) {
            cout << "  No outstanding balances\n";
            continue;
        }

        for (const auto& balanceEntry : balances) {
            const string& otherId = balanceEntry.first;
            const double amount = balanceEntry.second;
            auto otherIt = users.find(otherId);
            const string otherName = otherIt != users.end() ? otherIt->second->getName() : otherId;

            if (amount > 0) {
                cout << "  " << otherName << " owes this user: Rs "
                     << fixed << setprecision(2) << amount << "\n";
            } else {
                cout << "  This user owes " << otherName << ": Rs "
                     << fixed << setprecision(2) << abs(amount) << "\n";
            }
        }
    }
}

void Splitwise::saveToFile(const string& filePath) const {
    ofstream out(filePath);
    if (!out.is_open()) {
        throw runtime_error("Failed to open file for saving");
    }

    out << nextUserID << " " << nextExpenseID << "\n";

    out << users.size() << "\n";
    for (const auto& userEntry : users) {
        const User* user = userEntry.second;
        out << quoted(user->getId()) << " "
            << quoted(user->getName()) << " "
            << quoted(user->getEmail()) << "\n";
    }

    out << expenses.size() << "\n";
    for (const auto& expenseEntry : expenses) {
        const Expense* expense = expenseEntry.second;
        out << expense->getId() << " "
            << quoted(expense->getDescription()) << " "
            << fixed << setprecision(2) << expense->getTotalAmount() << " "
            << quoted(expense->getPaidByUserId()) << " "
            << splitTypeToInt(expense->getSplitType()) << " ";

        const auto& involved = expense->getInvolvedUsers();
        out << involved.size() << " ";
        for (const string& userId : involved) {
            out << quoted(userId) << " ";
        }

        const auto& values = expense->getSplitValues();
        out << values.size() << " ";
        for (double value : values) {
            out << fixed << setprecision(2) << value << " ";
        }
        out << "\n";
    }
}

void Splitwise::loadFromFile(const string& filePath) {
    ifstream in(filePath);
    if (!in.is_open()) {
        throw runtime_error("Failed to open file for loading");
    }

    clearDynamicMemory();

    in >> nextUserID >> nextExpenseID;

    size_t userCount = 0;
    in >> userCount;
    for (size_t i = 0; i < userCount; ++i) {
        string userId;
        string name;
        string email;
        in >> quoted(userId) >> quoted(name) >> quoted(email);
        users[userId] = new User(userId, name, email);
    }

    size_t expenseCount = 0;
    in >> expenseCount;

    for (size_t i = 0; i < expenseCount; ++i) {
        int expenseId = 0;
        string description;
        double amount = 0.0;
        string paidByUserId;
        int splitTypeValue = 0;

        in >> expenseId >> quoted(description) >> amount >> quoted(paidByUserId) >> splitTypeValue;

        size_t involvedCount = 0;
        in >> involvedCount;
        vector<string> involvedUsers(involvedCount);
        for (size_t j = 0; j < involvedCount; ++j) {
            in >> quoted(involvedUsers[j]);
        }

        size_t valueCount = 0;
        in >> valueCount;
        vector<double> splitValues(valueCount);
        for (size_t j = 0; j < valueCount; ++j) {
            in >> splitValues[j];
        }

        SplitType splitType = intToSplitType(splitTypeValue);
        SplitStrategy* strategy = SplitFactory::createStrategy(splitType);
        vector<Split> splits;
        try {
            splits = strategy->calculateSplit(amount, involvedUsers, splitValues);
        } catch (...) {
            delete strategy;
            throw;
        }
        delete strategy;

        expenses[expenseId] = new Expense(
            expenseId,
            description,
            amount,
            paidByUserId,
            involvedUsers,
            splitType,
            splitValues,
            splits
        );
    }

    recalculateBalances();
}
