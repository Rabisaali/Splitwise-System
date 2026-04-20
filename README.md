# Splitwise System (C++ OOP Project)

## Project Title and Description
**Project Title:** Splitwise System

**Description:**
This project is a console-based Splitwise-style expense management system built in C++ using Object-Oriented Programming principles. It allows users to create accounts, form groups, add shared or individual expenses, settle payments, simplify debts, and persist data using file save/load functionality.

The system supports multiple split methods:
- Equal split
- Exact split
- Percentage split

---

## Made by:

Rabisa Ali (25K-0876)


---

## Use Cases (System Scenarios)
1. **Create a new user**
   - Actor creates a user profile with name and email.

2. **Create a group and add members**
   - Actor creates a group and adds existing users to that group.

3. **Add a group expense**
   - Actor records a group expense with description, amount, payer, involved users, and split type (equal/exact/percentage).

4. **Add an individual expense**
   - Actor records an expense between two users outside any group.

5. **Settle payments**
   - Actor settles debt either within a group or directly between two users.

6. **View and simplify balances**
   - Actor views user/group balances and simplifies group debts to reduce the number of transactions.

---

## Clone and Setup
Use the following steps to clone and open the project:

```bash
git clone https://github.com/Rabisaali/OOP-Project.git
cd OOP-Project
```

If you are keeping this project inside a parent folder (like `C++_programmes`), clone it there and then open the folder in VS Code.

---

## How to Compile the Project
### Option 1: Using g++ (MSYS2/MinGW)
From the `OOP-Project` folder, run:

```bash
g++ src/main.cpp src/Splitwise.cpp -o Splitwise_app
```

### Option 2: Using GCC task in VS Code
- Open `src/main.cpp`
- Run the task: **C/C++: gcc.exe build active file**

Note: Because the project has multiple source files, Option 1 is recommended to ensure all implementation files are linked.

---

## How to Run the Project
After successful compilation:

```bash
./Splitwise_app
```

On Windows Command Prompt/PowerShell, you can also run:

```powershell
.\Splitwise_app.exe
```

---

## How to Use the System
When the program starts, a menu is displayed. Choose the corresponding number to perform an action:

- `1` Add user
- `2` Add group
- `3` Add user to group
- `4` Add expense to group
- `5` Add individual expense
- `6` Settle payment in group
- `7` Settle individual payment
- `8` Show user balance
- `9` Show group balances
- `10` Simplify group debts
- `11` Remove user from group
- `12` Save data to file
- `13` Load data from file
- `0` Exit

### Suggested flow for first-time use
1. Add users (`1`)
2. Add a group (`2`)
3. Add users to the group (`3`)
4. Add group expenses (`4`)
5. Check balances (`8`, `9`)
6. Simplify debts (`10`)
7. Settle payments (`6` or `7`)
8. Save data (`12`)

---

## Assumptions and Limitations
- The application is **console-based** (no GUI).
- User IDs, group IDs, and expense IDs are auto-generated at runtime.
- Input is interactive and depends on valid user entries.
- Data persistence is file-based and uses a custom plain-text format.
- Concurrency and multi-user real-time access are not supported.
- Currency is shown as `Rs` in output messages.
- No authentication/authorization is implemented.

---

## Project Structure
- `include/` - Header files for classes and interfaces
- `src/main.cpp` - Menu-driven user interaction
- `src/Splitwise.cpp` - Core business logic implementation
