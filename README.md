# Banking Management System

A modular, persistent, menu-driven **Banking Management System** developed entirely in standard C (C11). This project demonstrates clean modular architecture, custom data structures, binary file persistence, safe input validation, exact financial arithmetic, and defensive systems programming.

---

## 1. Project Overview

### Core Capabilities
- **Account Management**: Create accounts with auto-generated sequential IDs (starting at 1001), account type categorization (Savings/Current), and full details search.
- **Financial Transactions**: Deposit and withdraw money with zero/negative amount rejection and strict overdraft prevention.
- **Transaction History**: Audit trail recording transaction ID, account number, operation type, timestamp (`YYYY-MM-DD HH:MM:SS`), and resulting balance.
- **Binary Persistence**: Fast binary storage using `accounts.dat` and `transactions.dat`. Accounts are loaded on startup and updated persistently.
- **Monetary Precision**: Money stored as integer **paise** (`long long`), completely avoiding floating-point rounding errors.
- **Defensive Input Handling**: Safe buffered input using `fgets()` and custom parsing; immune to `scanf()` infinite loops and buffer overflows.

---

## 2. Directory Structure

```text
banking management system/
├── main.c           # Application entry point and interactive menu loop
├── banking.h        # Struct definitions, constants, and function prototypes
├── banking.c        # Account management, transactions, and input validation
├── file_handler.h   # File persistence declarations
├── file_handler.c   # Binary file I/O operations (accounts.dat, transactions.dat)
├── Makefile         # Build automation for Linux, macOS, and Windows
├── build.bat        # Automated one-click build script for Windows
├── build.sh         # Automated build script for Linux/macOS
├── .gitignore       # Git ignore rules for binaries, object files, and data
└── README.md        # Comprehensive project documentation
```

---

## 3. Architecture & Data Structures

### High-Level Architecture

```
                    +-----------------------------+
                    |     User Terminal / UI      |
                    +--------------+--------------+
                                   |
                                   v
                    +-----------------------------+
                    |           main.c            |
                    |   (Menu Loop & Dispatch)    |
                    +--------------+--------------+
                                   |
                +------------------+------------------+
                |                                     |
                v                                     v
+-------------------------------+   +---------------------------------+
|           banking.c           |   |         file_handler.c          |
|  - create_account()           |   |  - load_accounts()              |
|  - search_account()           |   |  - save_all_accounts()          |
|  - deposit_money()            |   |  - append_transaction()         |
|  - withdraw_money()           |   |  - get_next_transaction_id()    |
|  - view_transaction_history() |   |  - display_transactions()       |
|  - Validation & formatting    |   +----------------+----------------+
+---------------+---------------+                    |
                |                                    |
                v                                    v
     In-Memory State: `accounts[]`       Binary Files on Disk:
     `account_count`                     `accounts.dat` & `transactions.dat`
```

---

### Core Data Structures (`banking.h`)

#### 1. `Account` Structure
```c
typedef struct {
    int account_number;                  /* Unique account ID (e.g., 1001, 1002) */
    char name[MAX_NAME_LEN];             /* Customer full name (up to 99 chars + \0) */
    char account_type[MAX_ACCT_TYPE_LEN];/* "Savings" or "Current" */
    long long balance_paise;             /* Stored in paise: Rs. 100.50 -> 10050 */
} Account;
```
- **Why `balance_paise` as `long long`?** IEEE-754 floating point numbers (`float`/`double`) cannot accurately represent decimal fractions like `0.1` or `0.01`. In financial computing, using integer units (cents/paise) prevents cumulative rounding errors.
- **Fixed-size character arrays**: Eliminates heap fragmentation and allows clean `fread`/`fwrite` directly to binary files.

#### 2. `Transaction` Structure
```c
typedef struct {
    int transaction_id;                  /* Monotonically increasing ID (1, 2, ...) */
    int account_number;                  /* Foreign key linking to Account */
    char type[MAX_TXN_TYPE_LEN];         /* "Deposit" or "Withdrawal" */
    long long amount_paise;              /* Amount of transaction in paise */
    char datetime[MAX_DATETIME_LEN];     /* ISO-like timestamp: YYYY-MM-DD HH:MM:SS */
    long long resulting_balance_paise;   /* Balance immediately after transaction */
} Transaction;
```

---

## 4. Build & Execution Guide

### Option 1: Using Makefile (Cross-Platform)

```bash
# Build the project
make

# Run the application
make run

# Clean build artifacts and executables
make clean
```

### Option 2: Automated Scripts

**Windows (Command Prompt / PowerShell):**
```cmd
build.bat
```

**Linux / macOS:**
```bash
chmod +x build.sh
./build.sh
```

### Option 3: Manual GCC Compilation

**Linux / macOS:**
```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -o banking main.c banking.c file_handler.c
./banking
```

**Windows (MinGW):**
```powershell
gcc -std=c11 -Wall -Wextra -Wpedantic -o banking.exe main.c banking.c file_handler.c
.\banking.exe
```

*Note: The project compiles cleanly with **zero warnings** under `-std=c11 -Wall -Wextra -Wpedantic`.*

---

## 5. Operation Walkthroughs

### 1. Account Creation (`create_account`)
1. Checks if `account_count < MAX_ACCOUNTS` (boundary defense).
2. Reads name safely using `read_line()` (wraps `fgets()`).
3. Validates non-empty name with `is_valid_name()`.
4. Reads account type selection (`1` for Savings, `2` for Current).
5. Scans existing accounts via `generate_account_number()` to compute `max(account_number) + 1` (starts at 1001).
6. Copies string safely with `strncpy()` and ensures explicit null-termination (`\0`).
7. Initializes balance to `0` paise.
8. Writes to `accounts.dat` using `save_all_accounts()`.
9. **Rollback guarantee**: If the file save fails, the in-memory counter is decremented and the account is revoked.

### 2. Deposit Flow (`deposit_money`)
1. Reads and validates the target account number.
2. Locates account index using linear search (`find_account_index()`).
3. Prompts for amount; parses string via `parse_amount_to_paise()`.
4. Rejects non-numeric input, negative values, zero, or amounts with more than two decimal places.
5. Saves the previous balance (`old_balance`).
6. Updates in-memory balance: `balance_paise += amount_paise`.
7. Calls `save_all_accounts()`. If write fails, reverts `balance_paise = old_balance`.
8. Creates a `Transaction` struct, stamps timestamp using `time()` and `strftime()`.
9. Appends record to `transactions.dat` via `append_transaction()`.

### 3. Withdrawal Flow (`withdraw_money`)
1. Locates account by account number.
2. Validates amount input to positive paise.
3. **Guard condition**: Checks `if (amount_paise > accounts[index].balance_paise)`. If true, displays descriptive error with available balance and aborts.
4. Performs in-memory deduction, saves to `accounts.dat` (with rollback on failure), and appends to `transactions.dat`.

---

## 6. Defensive Programming & Data Integrity

| Risk | Naive C Approach | Our Implementation |
|---|---|---|
| **Buffer Overflow** | `gets(buf)` or `scanf("%s", buf)` | `fgets()` with exact buffer size limits; trailing newline stripped; excess characters flushed. |
| **Input Stream Lockup** | `scanf("%d", &val)` on invalid string leaves tokens in `stdin`, causing infinite loops. | Reads whole line as string using `fgets()`, parses with `strtol()` and pointer validation (`endptr`). |
| **Float Precision Loss** | `float balance = 100.10f;` causes precision drift (e.g. `100.099998`). | Stored as integer paise (`long long`). `100.50` -> `10050LL`. |
| **Silent Write Failure** | Calling `fwrite()` without checking return values. | Return value of `fwrite()` and `fflush()` verified against expected element counts. |
| **Incomplete Persistence** | Program crash during write damages file. | In-memory balance rolls back if disk write fails; rollback prevents dirty memory state. |

---

## 7. Sample Interactive Session

### A. Successful Deposit and Withdrawal
```text
================================================
       BANKING MANAGEMENT SYSTEM
================================================
  1. Create Account
  2. Search Account
  3. Display All Accounts
  4. Deposit Money
  5. Withdraw Money
  6. Check Balance
  7. View Transaction History
  8. Exit
================================================
  Enter your choice: 1

--- Create New Account ---

Enter customer name: Rahul Sharma
Enter account type (1 = Savings, 2 = Current): 1

=== Account Created Successfully ===
  Account Number : 1001
  Customer Name  : Rahul Sharma
  Account Type   : Savings
  Balance        : Rs. 0.00
====================================

  Enter your choice: 4

--- Deposit Money ---

Enter account number: 1001
Account: Rahul Sharma | Current Balance: Rs. 0.00

Enter deposit amount (e.g., 1000.50): 5000.50

=== Deposit Successful ===
  Amount Deposited : Rs. 5000.50
  New Balance      : Rs. 5000.50
  Transaction ID   : 1
==========================
```

### B. Rejected Withdrawal (Insufficient Funds)
```text
  Enter your choice: 5

--- Withdraw Money ---

Enter account number: 1001
Account: Rahul Sharma | Current Balance: Rs. 5000.50

Enter withdrawal amount (e.g., 500.00): 6000.00
Error: Insufficient balance.
  Requested : Rs. 6000.00
  Available : Rs. 5000.50
```

---

## 8. Technical Design & Implementation Highlights

### 1. Financial Precision via Integer Representation
Standard floating-point types (`float`, `double`) adhere to IEEE-754 binary floating-point representation. They cannot represent decimal fractions like `0.1` or `0.01` without infinitesimal errors that accumulate over repetitive transactions.
- In this system, all balances and transaction amounts are stored in **paise** as signed 64-bit integers (`long long`).
- Rs. `1,250.75` is stored as `125075LL`.
- Maximum representable balance exceeds **Rs. 92 quadrillion** ($2^{63}-1$ paise), eliminating any possibility of overflow in real-world scenarios while preserving exact precision down to the single paisa.

### 2. Robust Input Sanitization & Buffer Security
- Vulnerable C input routines such as `gets()` and unbounded `scanf()` are completely avoided.
- All user inputs are read line-by-line via `fgets()` with fixed buffer bounds, followed by automatic newline stripping and stream purging if inputs exceed the buffer size.
- Numeric conversions are processed via `strtol()` with complete pointer validation (`endptr`), rejecting trailing garbage characters and preventing infinite loops caused by non-numeric inputs.

### 3. Binary File Persistence & Rollback Guarantees
- Account state is maintained in `accounts.dat` using binary records (`Account` struct), enabling rapid batch read/write operations without string parsing overhead.
- Transaction logs are stored as an immutable audit trail in `transactions.dat` using append-only mode (`"ab"`).
- **Two-phase rollback**: If a disk write fails during a deposit or withdrawal, the in-memory balance is immediately restored to its pre-transaction state, and the transaction is aborted to prevent divergence between memory and disk.

### 4. Modular Separation of Concerns
The codebase is decoupled into three distinct layers:
- **Presentation Layer (`main.c`)**: Interactive menu presentation, option dispatch, and user loop control.
- **Domain Logic Layer (`banking.c`, `banking.h`)**: Account management, transaction processing, input validation, and currency formatting.
- **Persistence Layer (`file_handler.c`, `file_handler.h`)**: Direct binary file operations, disk serialization, and ledger retrieval.

---

## 9. Verification & Automated Test Results

The system was verified against a comprehensive functional and edge-case test suite:

| # | Test Scenario | Input / Action | Expected Result | Actual Result | Status |
|---|---|---|---|---|---|
| 1 | Create Account | Name: "Rahul Sharma", Type: 1 | Account 1001 created, balance 0.00 | Account 1001 created | **PASSED** |
| 2 | Auto ID Increment | Name: "Priya Patel", Type: 2 | Account 1002 created (no ID collision) | Account 1002 created | **PASSED** |
| 3 | Empty / Space Name | Name: `""` or `" "` | Rejected: "Name cannot be empty" | Displayed error, re-prompted | **PASSED** |
| 4 | Invalid Account Type | Type: `9` | Rejected: "Invalid account type" | Displayed error, returned | **PASSED** |
| 5 | Deposit Valid Amount | Acc: 1001, Amount: `5000.50` | Balance becomes 5000.50, Txn ID 1 | Balance Rs. 5000.50, Txn 1 logged | **PASSED** |
| 6 | Deposit Negative / Zero | Acc: 1001, Amount: `-500`, `0` | Rejected: "Invalid amount" | Displayed error, no balance change | **PASSED** |
| 7 | Overdraft Withdrawal | Acc: 1001, Amount: `6000.00` | Rejected: "Insufficient balance" | Displayed error, balance unchanged | **PASSED** |
| 8 | Valid Withdrawal | Acc: 1001, Amount: `1500.25` | Balance becomes 3500.25, Txn ID 2 | Balance Rs. 3500.25, Txn 2 logged | **PASSED** |
| 9 | Balance Inquiry | Acc: 1001 | Shows Rs. 3500.25 | Correct balance displayed | **PASSED** |
| 10 | Transaction Audit Log | Acc: 1001 | Shows Deposit 5000.50 & Withdrawal 1500.25 | Complete audit trail displayed | **PASSED** |
| 11 | Non-Existent Account | Acc: 9999 | Rejected: "Account 9999 not found" | Handled gracefully | **PASSED** |
| 12 | Invalid Menu Input | Input: `"abc"`, `99` | Reject without hanging or crash | Clean error message, loop resumed | **PASSED** |
| 13 | App Restart & Persistence | Exit, relaunch, display accounts | Loaded 2 accounts from file with balances intact | Rs. 3500.25 & Rs. 0.00 verified | **PASSED** |
| 14 | Post-Restart ID Continuity | Create Account 3 after restart | Generates Account 1003 (no duplicate) | Account 1003 generated, Txn 3 logged | **PASSED** |
| 15 | Strict Compilation | `gcc -std=c11 -Wall -Wextra -Wpedantic` | Zero warnings, zero errors | Compiled cleanly | **PASSED** |

---

## 10. License & Disclaimer

This project is an open-source educational software system demonstrating robust C programming concepts, safe systems design, and data persistence techniques. It is licensed under the [MIT License](LICENSE).

*Disclaimer: This system is intended for educational and portfolio demonstration purposes. It does not implement real-world cryptographic authentication or encrypted network transmission and must not be used as production banking infrastructure.*
