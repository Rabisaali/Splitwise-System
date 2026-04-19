#include "../include/SplitwiseSystem.h"

#include <iostream>
#include <limits>
#include <stdexcept>

using namespace std;

namespace {
int readInt(string prompt) {
    int value = 0;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cout << "Invalid integer input. Try again.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

double readPositiveDouble(string prompt) {
    double value = 0.0;
    while (true) {
        cout << prompt;
        if (cin >> value && value > 0.0) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cout << "Invalid amount. Enter a positive number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

string readLine(string prompt) {
    string value;
    while (true) {
        cout << prompt;
        getline(cin, value);
        if (!value.empty()) {
            return value;
        }
        cout << "Input cannot be empty. Try again.\n";
    }
}

SplitType readSplitType() {
    while (true) {
        cout << "Select split type: 1) Equal 2) Exact 3) Percentage: ";
        int choice = 0;
        if (cin >> choice) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (choice == 1) return SplitType::EQUAL;
            if (choice == 2) return SplitType::EXACT;
            if (choice == 3) return SplitType::PERCENTAGE;
        } else {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        cout << "Invalid split type selection.\n";
    }
}

void printMenu() {
    cout << "\n====== Splitwise Menu ======\n";
    cout << "1. Add user\n";
    cout << "2. Add group\n";
    cout << "3. Add user to group\n";
    cout << "4. Add expense to group\n";
    cout << "5. Add individual expense\n";
    cout << "6. Settle payment in group\n";
    cout << "7. Settle individual payment\n";
    cout << "8. Show user balance\n";
    cout << "9. Show group balances\n";
    cout << "10. Simplify group debts\n";
    cout << "11. Remove user from group\n";
    cout << "12. Save data to file\n";
    cout << "13. Load data from file\n";
    cout << "0. Exit\n";
}
}

int main() {
    Splitwise* manager = Splitwise::getInstance();

    while (true) {
        printMenu();
        int choice = readInt("Enter your choice: ");

        if (choice == 0) {
            cout << "Exiting application.\n";
            break;
        }

        try {
            if (choice == 1) {
                string name = readLine("Enter user name: ");
                string email = readLine("Enter user email: ");
                User* user = manager->createUser(name, email);
                cout << "User created with ID: " << user->userID << "\n";
            } else if (choice == 2) {
                string groupName = readLine("Enter group name: ");
                Group* group = manager->createGroup(groupName);
                cout << "Group created with ID: " << group->groupID << "\n";
            } else if (choice == 3) {
                string userID = readLine("Enter user ID: ");
                string groupID = readLine("Enter group ID: ");
                manager->addUserToGroup(userID, groupID);
            } else if (choice == 4) {
                string groupID = readLine("Enter group ID: ");
                string description = readLine("Enter expense description: ");
                double amount = readPositiveDouble("Enter total amount: ");
                string paidByUserID = readLine("Enter payer user ID: ");

                int count = readInt("Enter number of involved users: ");
                if (count <= 0) {
                    throw runtime_error("Number of involved users must be positive");
                }

                vector<string> involvedUsers;
                for (int i = 0; i < count; ++i) {
                    involvedUsers.push_back(readLine("Enter involved user ID: "));
                }

                SplitType splitType = readSplitType();
                vector<double> splitValues;
                if (splitType == SplitType::EXACT || splitType == SplitType::PERCENTAGE) {
                    for (int i = 0; i < count; ++i) {
                        splitValues.push_back(readPositiveDouble("Enter value for user " + involvedUsers[i] + ": "));
                    }
                }

                manager->addExpenseToGroup(groupID, description, amount, paidByUserID, involvedUsers, splitType, splitValues);
            } else if (choice == 5) {
                string description = readLine("Enter expense description: ");
                double amount = readPositiveDouble("Enter total amount: ");
                string paidByUserID = readLine("Enter payer user ID: ");
                string toUserID = readLine("Enter other user ID: ");
                manager->addIndividualExpense(description, amount, paidByUserID, toUserID, SplitType::EQUAL);
            } else if (choice == 6) {
                string groupID = readLine("Enter group ID: ");
                string fromUserID = readLine("Enter from user ID: ");
                string toUserID = readLine("Enter to user ID: ");
                double amount = readPositiveDouble("Enter amount: ");
                manager->settlePaymentInGroup(groupID, fromUserID, toUserID, amount);
            } else if (choice == 7) {
                string fromUserID = readLine("Enter from user ID: ");
                string toUserID = readLine("Enter to user ID: ");
                double amount = readPositiveDouble("Enter amount: ");
                manager->settleIndividualPayment(fromUserID, toUserID, amount);
            } else if (choice == 8) {
                string userID = readLine("Enter user ID: ");
                manager->showUserBalance(userID);
            } else if (choice == 9) {
                string groupID = readLine("Enter group ID: ");
                manager->showGroupBalances(groupID);
            } else if (choice == 10) {
                string groupID = readLine("Enter group ID: ");
                manager->simplifyGroupDebts(groupID);
            } else if (choice == 11) {
                string userID = readLine("Enter user ID: ");
                string groupID = readLine("Enter group ID: ");
                manager->removeUserFromGroup(userID, groupID);
            } else if (choice == 12) {
                string fileName = readLine("Enter file path to save data: ");
                manager->saveToFile(fileName);
                cout << "Data saved successfully.\n";
            } else if (choice == 13) {
                string fileName = readLine("Enter file path to load data: ");
                manager->loadFromFile(fileName);
                cout << "Data loaded successfully.\n";
            } else {
                cout << "Invalid menu choice.\n";
            }
        } catch (exception& ex) {
            cout << "Error: " << ex.what() << "\n";
        }
    }

    return 0;
}
