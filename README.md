# Banking Management System (Pure C)

A complete, persistent, menu-driven **Banking Management System** developed entirely in standard C (C11). This project demonstrates modular architecture, custom data structures, binary file persistence, safe input validation, and defensive programming tailored for technical interviews at companies like **Sasken Technology**.

---

## 1. Project Overview & Resume Alignment

### Resume Bullet Points
> - *Developed a Banking Management System in C to manage customer accounts, deposits, withdrawals, balance inquiries, and transaction records.*
> - *Implemented custom structures, binary file handling, and robust input validation for efficient, persistent data management.*

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
├── accounts.dat     # Binary file storing Account records (auto-generated)
├── transactions.dat # Binary file storing Transaction records (auto-generated)
├── banking.exe      # Compiled binary executable
└── README.md        # Comprehensive documentation and interview guide
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

## 4. Compilation & Execution Guide

### Standard Compilation (GCC / Linux / macOS)
```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -o banking main.c banking.c file_handler.c
./banking
```

### Windows Compilation (MinGW GCC)
1. Ensure MinGW is installed (e.g., `C:\MinGW\bin`).
2. Add MinGW to your session PATH or environment:
   ```powershell
   $env:PATH = "C:\MinGW\bin;" + $env:PATH
   ```
3. Compile with strict flags:
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

## 8. Technical Interview Preparation (Sasken Technology)

### 60-90 Second Project Pitch (Elevator Speech)
> *"For my project, I built a modular Banking Management System in standard C designed around reliability and data integrity. It handles the core lifecycle of customer accounts, deposits, withdrawals, and an immutable transaction audit log.
>
> On the technical side, I placed special emphasis on memory safety and financial correctness. I avoided floating-point inaccuracies by storing all currency values as integer paise using 64-bit integers (`long long`). For user input, I strictly avoided unsafe functions like `gets()` or unbounded `scanf()`, implementing a robust line-reading routine with `fgets()` and `strtol()` to prevent buffer overflows and stream hangs.
>
> For persistence, I separated account records and transaction history into binary files using `fread()`, `fwrite()`, and `fflush()`, incorporating in-memory rollbacks if disk operations fail. This gave me hands-on practice with modular C architecture, pointers, structs, file handling, and defensive programming."*

---

### 25 Technical Interview Questions & Answers

#### Category 1: C Language Core, Structs & Memory

**Q1: Why did you use `typedef struct` instead of just `struct`?**
> **Answer**: In C, defining `struct Account { ... };` requires writing `struct Account acc;` every time a variable is declared. Using `typedef struct { ... } Account;` creates an alias, allowing us to declare variables directly as `Account acc;`, improving code readability and modularity.

**Q2: What is the size of your `Account` struct and how does struct padding/alignment affect it?**
> **Answer**: `Account` contains an `int` (4 bytes), `char name[100]` (100 bytes), `char account_type[20]` (20 bytes), and `long long balance_paise` (8 bytes). 
> Sum of members = 4 + 100 + 20 + 8 = 132 bytes. Depending on compiler alignment rules (typically 4-byte or 8-byte alignment for 64-bit integers), compilers may insert padding bytes before `balance_paise` or at the end so the struct size is a multiple of the largest member's alignment.

**Q3: Why pass `const Account *acc` to `display_account_details()` instead of `Account acc`?**
> **Answer**: Two reasons:
> 1. **Efficiency**: Passing by value copies the entire 132+ byte struct onto the call stack. Passing a pointer copies only 4 or 8 bytes (the address).
> 2. **Safety**: The `const` qualifier guarantees the function cannot accidentally modify the caller's struct data.

**Q4: What is the difference between `.` (dot) and `->` (arrow) operators?**
> **Answer**: The dot operator (`.`) accesses members of a struct variable directly (e.g., `new_acc.account_number`). The arrow operator (`->`) dereferences a pointer to a struct and accesses the member in one step (e.g., `acc->account_number` is equivalent to `(*acc).account_number`).

**Q5: What is the scope and lifetime of your global array `accounts[]`?**
> **Answer**: The array `accounts[]` is defined in `banking.c` at file scope and declared as `extern` in `banking.h`. Its lifetime is the entire execution duration of the program (static storage duration), residing in the BSS/Data segment rather than on the stack.

**Q6: Why did you declare helper functions like `generate_account_number()` as `static`?**
> **Answer**: In C, declaring a function `static` gives it internal linkage, restricting its visibility to the translation unit (`banking.c`) where it is defined. This prevents symbol pollution and accidental name collisions across multiple source files.

---

#### Category 2: Strings & Input Validation

**Q7: Why is `gets()` considered dangerous, and what did you use instead?**
> **Answer**: `gets()` has no parameter to specify buffer capacity, making buffer overflow unavoidable if user input exceeds the allocated array. We used `fgets(buf, size, stdin)`, which guarantees reading at most `size - 1` characters and always leaves room for the null-terminator `\0`.

**Q8: What problem occurs with `scanf("%d", &choice)` in a menu loop, and how did you resolve it?**
> **Answer**: If the user inputs non-numeric characters (e.g., `"abc"`), `scanf("%d")` fails to match, leaving those characters in the `stdin` stream buffer. On the next loop iteration, `scanf()` reads the exact same invalid characters, causing an infinite loop. We resolved this by always reading the entire line into a string buffer via `fgets()`, then parsing it safely with `strtol()`.

**Q9: How does your `read_line()` function handle inputs longer than the buffer?**
> **Answer**: `read_line()` checks if the last character before `\0` is `\n`. If `\n` is absent, the input exceeded the buffer. The function enters a loop: `while ((ch = getchar()) != '\n' && ch != EOF);` to consume and discard remaining characters from `stdin` so subsequent prompts remain clean.

**Q10: Why use `strncpy()` instead of `strcpy()`, and what precaution is necessary with `strncpy()`?**
> **Answer**: `strcpy()` does not check destination buffer boundaries and can cause buffer overflows. `strncpy(dest, src, n)` copies at most `n` characters. However, if `src` is `n` characters or longer, `strncpy()` does not append a null terminator. We always append `dest[size - 1] = '\0';` immediately after.

**Q11: Why is `strcmp()` required to compare strings in C instead of `==`?**
> **Answer**: In C, string variables decay into pointers to their first characters. Using `str1 == str2` compares memory addresses (pointer equality), not content. `strcmp(str1, str2)` compares character by character until a mismatch or `\0` is found.

---

#### Category 3: Financial Precision & Integer Representation

**Q12: Why store monetary values in paise rather than `float` or `double`?**
> **Answer**: Floating-point numbers use base-2 binary representation (IEEE-754) and cannot accurately represent decimal fractions like 0.1 or 0.01. For example, repeated additions of 0.1 float introduce tiny rounding discrepancies (e.g., `0.1 + 0.2 = 0.30000000000000004`). In banking, every fractional unit must balance perfectly. Storing money as integer paise (`long long`) ensures exact arithmetic.

**Q13: What is the maximum balance your system can hold without integer overflow?**
> **Answer**: A signed 64-bit integer (`long long`) has a maximum value of $2^{63}-1 \approx 9.22 \times 10^{18}$ paise, which corresponds to over **Rs. 92,233,720,368,547,758.07** (92 quadrillion rupees). This is more than sufficient for any realistic banking application.

**Q14: How does your string-to-amount parser prevent overflow during input?**
> **Answer**: In `parse_amount_to_paise()`, during digit accumulation `rupees = rupees * 10 + digit`, we verify that `rupees <= 999999999999LL` before multiplying, actively preventing 64-bit arithmetic overflow on excessively large inputs.

---

#### Category 4: File Handling & Persistence

**Q15: What is the difference between binary mode (`"rb"`, `"wb"`, `"ab"`) and text mode (`"r"`, `"w"`, `"a"`)?**
> **Answer**: In text mode, the C runtime performs translation on newline characters (e.g., on Windows, `\n` is translated to `\r\n` on write and back on read). In binary mode, bytes are written and read directly without translation. For structured binary records using `fread()` and `fwrite()`, binary mode is mandatory to prevent byte distortion.

**Q16: How do `fread()` and `fwrite()` work in your application?**
> **Answer**: 
> - `fwrite(&accounts, sizeof(Account), account_count, fp)` writes `account_count` contiguous `Account` memory blocks directly to disk.
> - `fread(&accounts[i], sizeof(Account), 1, fp)` reads one struct block at a time from disk directly into our array element.
> - Both return the number of items successfully transferred, which we check for validation.

**Q17: Why did you call `fflush()` before `fclose()`?**
> **Answer**: Standard C library file I/O is buffered in user space for performance. `fflush(fp)` forces the runtime to flush its internal buffer to the operating system immediately. Checking `fflush()` ensures write errors caused by disk space exhaustion or hardware disconnects are detected before closing.

**Q18: What is the persistence strategy for accounts vs. transactions?**
> **Answer**:
> - **Accounts**: Rewritten entirely (`"wb"`) whenever an account is added or a balance changes. This keeps the file compact and in sync with the in-memory array.
> - **Transactions**: Appended incrementally (`"ab"`). Completed transactions are immutable audit logs that are never modified or rewritten.

**Q19: What happens if `accounts.dat` does not exist when the application launches?**
> **Answer**: `load_accounts()` attempts `fopen(ACCOUNTS_FILE, "rb")`. If it returns `NULL`, the function detects that this is the application's first run, sets `account_count = 0`, and returns success without crashing. The file is created automatically when the first account is saved.

---

#### Category 5: Data Structures, Search & Algorithms

**Q20: What algorithm did you use to search for accounts, and what is its time complexity?**
> **Answer**: We used **Linear Search** (`find_account_index()`), iterating through the array from `0` to `account_count - 1`.
> - **Time Complexity**: $O(n)$ in the worst and average cases, $O(1)$ in the best case (element at index 0).
> - **Space Complexity**: $O(1)$ auxiliary space.

**Q21: How would you improve search time if the system scaled to 1,000,000 accounts?**
> **Answer**: 
> 1. **Binary Search**: Keep accounts sorted by `account_number`. Reduces search complexity to $O(\log n)$. Insertion becomes $O(n)$ to maintain order.
> 2. **Hash Table**: Use account numbers as keys in an open-addressing or chained hash table for $O(1)$ average search time.
> 3. **B-Tree / B+ Tree on Disk**: For millions of accounts that cannot fit in RAM, an on-disk B+ Tree index (similar to relational database engines) minimizes disk I/O operations.

**Q22: Why did you use an array instead of a dynamic linked list for `accounts[]`?**
> **Answer**: For a project with `MAX_ACCOUNTS = 1000`, a static array provides:
> 1. **Contiguous memory allocation**, yielding superior CPU cache locality and zero memory fragmentation.
> 2. **Direct indexing** ($O(1)$ random access).
> 3. **Single-call disk serialization**: We can write the entire array in one `fwrite()` call.
> 4. Avoidance of dynamic memory (`malloc`/`free`) leak risks.

---

#### Category 6: Error Handling & System Limitations

**Q23: How does your code maintain data consistency if a disk write fails during a deposit?**
> **Answer**: We implement a **two-phase update with rollback**:
> 1. We record `old_balance = accounts[index].balance_paise`.
> 2. We update the in-memory balance and attempt `save_all_accounts()`.
> 3. If `save_all_accounts()` fails, we immediately roll back: `accounts[index].balance_paise = old_balance`, alert the user, and do not log the transaction.

**Q24: What is the remaining limitation of this file persistence approach regarding ACID atomicity?**
> **Answer**: If power fails or the process is killed in the middle of `fwrite()` when saving `accounts.dat`, the file may be left in a partially written (corrupt) state. In a production database, this is solved via **Write-Ahead Logging (WAL)** or by writing to a temporary file (`accounts.dat.tmp`) and performing an atomic filesystem rename (`rename()`).

**Q25: What future improvements would you propose for this system?**
> **Answer**:
> 1. **Atomic File Swapping**: Write new account data to a temp file first, then use POSIX/Win32 atomic file rename to replace the live file.
> 2. **Dynamic Resizing**: Replace fixed-size array with a dynamically growing array (`realloc`) or on-disk B-Tree.
> 3. **Account Deactivation/Soft Deletes**: Add a `is_active` status flag rather than physically deleting records.
> 4. **Authentication & PIN**: Password/PIN hashing using SHA-256 for secure account access.
> 5. **Thread Safety**: Mutex locks (`pthread_mutex_t`) if multiple teller threads access the banking core concurrently.

---

## 9. Verification & Test Execution Results

The following test suite was executed against `banking.exe`:

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

## 10. Educational Disclaimer

*This project is an educational demonstration developed strictly for technical interview preparation. It is not intended for production banking environments and must not be used to store real financial or personally identifiable information (PII).*
