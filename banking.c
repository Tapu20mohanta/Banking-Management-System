/*
 * banking.c
 * Banking Management System - Core Implementation
 *
 * Contains all account management operations, banking transactions,
 * input validation, and utility functions.
 *
 * Key C concepts demonstrated:
 *   - struct for Account and Transaction data
 *   - Pointers (const pointers, pointer to struct members)
 *   - Arrays (global accounts array, character arrays)
 *   - String functions (strcmp, strncpy, strlen, strtol)
 *   - Functions with meaningful names and single responsibilities
 *   - Input validation without gets() or unbounded scanf()
 *   - Error handling with rollback on file save failure
 */

#include "banking.h"
#include "file_handler.h"

/* ==================== Global Data Definition ==================== */

/*
 * accounts[] is the in-memory store for all customer accounts.
 * Data is loaded from accounts.dat at startup and saved back
 * after every modification (create, deposit, withdraw).
 */
Account accounts[MAX_ACCOUNTS];
int account_count = 0;

/* ==================== Static (Internal) Helper Functions ==================== */

/*
 * generate_account_number (static = visible only in this file)
 *
 * Scans the accounts[] array to find the highest existing account number,
 * then returns (highest + 1). If no accounts exist, starts at STARTING_ACCT_NUM.
 *
 * This guarantees unique, sequential account numbers even after restart,
 * because accounts are loaded from file before this function is called.
 *
 * Time Complexity: O(n) where n = account_count
 */
static int generate_account_number(void) {
    int max_num = STARTING_ACCT_NUM - 1;
    int i;

    for (i = 0; i < account_count; i++) {
        if (accounts[i].account_number > max_num) {
            max_num = accounts[i].account_number;
        }
    }

    return max_num + 1;
}

/*
 * is_valid_name (static = visible only in this file)
 *
 * Checks that a name string contains at least one non-whitespace character.
 * Iterates through each character using pointer arithmetic.
 *
 * Returns: 1 if valid, 0 if empty or whitespace-only.
 */
static int is_valid_name(const char *name) {
    while (*name != '\0') {
        if (!isspace((unsigned char)*name)) {
            return 1;   /* Found a non-space character */
        }
        name++;  /* Pointer arithmetic: move to next character */
    }
    return 0;   /* All characters were whitespace */
}

/* ==================== Utility Functions ==================== */

/*
 * read_line
 * Safely reads one line from stdin into buf using fgets().
 *
 * Why fgets() and not gets():
 *   gets() does not limit the number of characters read, so it can
 *   overflow the buffer. fgets() takes a size parameter and reads
 *   at most (size - 1) characters, preventing buffer overflow.
 *
 * After reading:
 *   - If a newline '\n' is found, it is removed.
 *   - If no newline is found, the input was longer than the buffer,
 *     so remaining characters are flushed from stdin to prevent them
 *     from being read by the next input call (avoiding infinite loops).
 *
 * Returns: 1 on success, 0 on failure (EOF or error).
 */
int read_line(char *buf, int size) {
    if (fgets(buf, size, stdin) == NULL) {
        buf[0] = '\0';
        return 0;
    }

    size_t len = strlen(buf);

    if (len > 0 && buf[len - 1] == '\n') {
        /* Normal case: newline found, remove it */
        buf[len - 1] = '\0';
    } else {
        /* Input exceeded buffer size - flush remaining characters */
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;  /* Discard extra characters */
    }

    return 1;
}

/*
 * read_int
 * Displays a prompt and reads an integer from the user.
 *
 * Uses read_line() + strtol() instead of scanf("%d") because:
 *   - scanf leaves unread characters in stdin on bad input
 *   - This can cause infinite loops if not handled carefully
 *   - strtol() gives us full control over validation
 *
 * strtol() sets endptr to point to the first character it could NOT parse.
 * If endptr == line, no digits were parsed (e.g., user typed "abc").
 * If *endptr != '\0', there was trailing garbage (e.g., "123abc").
 *
 * Returns: 1 if a valid integer was read, 0 otherwise.
 */
int read_int(const char *prompt, int *value) {
    char line[32];
    char *endptr;
    long val;

    printf("%s", prompt);

    if (!read_line(line, sizeof(line))) {
        return 0;
    }

    val = strtol(line, &endptr, 10);

    /* Skip any trailing whitespace */
    while (isspace((unsigned char)*endptr)) {
        endptr++;
    }

    /* Reject if no digits were parsed or trailing non-space characters */
    if (endptr == line || *endptr != '\0') {
        return 0;
    }

    /* Reject values outside int range */
    if (val < INT_MIN || val > INT_MAX) {
        return 0;
    }

    *value = (int)val;
    return 1;
}

/*
 * parse_amount_to_paise
 *
 * Converts a monetary string like "1234.56" to 123456 paise (long long).
 * This is the core of our floating-point avoidance strategy.
 *
 * Why avoid float/double for money?
 *   float and double use binary fractions and cannot represent many
 *   decimal values exactly. For example, 0.1 in binary is a repeating
 *   fraction, leading to tiny errors that accumulate over many operations.
 *   By using integer paise, calculations are always exact.
 *
 * Parsing rules:
 *   - Accepts: "100", "100.5", "100.50", "100.05", ".50"
 *   - Rejects: negative numbers, zero, more than 2 decimal places,
 *              non-numeric characters, empty input
 *
 * Uses character-by-character parsing with isdigit() to validate each char.
 * Cast to (unsigned char) before passing to isdigit() to avoid undefined
 * behavior with negative char values on some platforms.
 *
 * Returns: 1 on success (paise > 0), 0 on invalid input.
 */
int parse_amount_to_paise(const char *str, long long *paise) {
    const char *p = str;
    long long rupees = 0;
    long long paisa_part = 0;
    int decimal_digits = 0;

    /* Skip leading whitespace */
    while (isspace((unsigned char)*p)) {
        p++;
    }

    /* Reject empty string, negative sign, or non-numeric start */
    if (*p == '\0' || *p == '-') {
        return 0;
    }
    if (!isdigit((unsigned char)*p) && *p != '.') {
        return 0;
    }

    /* Parse the integer part (rupees) */
    while (isdigit((unsigned char)*p)) {
        rupees = rupees * 10 + (*p - '0');
        if (rupees > 999999999999LL) {
            return 0;   /* Overflow protection */
        }
        p++;
    }

    /* Parse the decimal part (paise) if present */
    if (*p == '.') {
        p++;    /* Skip the decimal point */
        while (isdigit((unsigned char)*p) && decimal_digits < 2) {
            paisa_part = paisa_part * 10 + (*p - '0');
            decimal_digits++;
            p++;
        }
        /* Reject if more than 2 decimal digits */
        if (isdigit((unsigned char)*p)) {
            return 0;
        }
    }

    /* If only 1 decimal digit was given, multiply by 10 */
    /* Example: "1.5" means 1 rupee 50 paise, not 5 paise */
    if (decimal_digits == 1) {
        paisa_part *= 10;
    }

    /* Skip trailing whitespace */
    while (isspace((unsigned char)*p)) {
        p++;
    }

    /* Must be at end of string (no trailing garbage) */
    if (*p != '\0') {
        return 0;
    }

    *paise = rupees * 100 + paisa_part;

    /* Reject zero amounts */
    return (*paise > 0) ? 1 : 0;
}

/*
 * read_amount
 * Prompts the user and reads a monetary amount, converted to paise.
 * Combines read_line() and parse_amount_to_paise() for convenience.
 *
 * Returns: 1 on success, 0 on invalid input.
 */
int read_amount(const char *prompt, long long *paise) {
    char line[64];

    printf("%s", prompt);
    if (!read_line(line, sizeof(line))) {
        return 0;
    }

    return parse_amount_to_paise(line, paise);
}

/*
 * format_amount
 * Converts paise back to a display string.
 * Example: 123456 paise -> "1234.56"
 *
 * Uses snprintf() for safe, bounded string formatting.
 * The format "%lld.%02lld" ensures exactly 2 decimal places.
 */
void format_amount(long long paise, char *buf, int size) {
    if (buf == NULL || size <= 0) {
        return;
    }

    long long rupees = paise / 100;
    int paisa_part = (int)(paise % 100);
    if (paisa_part < 0) {
        paisa_part = -paisa_part;
    }

    /*
     * Extract rupees digits into a temporary buffer in reverse order.
     * This avoids compiler-specific format specifiers (%lld vs %I64d)
     * across different C runtimes (MSVCRT vs glibc).
     */
    char temp[64];
    int idx = 0;
    int is_neg = 0;

    if (rupees < 0) {
        is_neg = 1;
        rupees = -rupees;
    } else if (rupees == 0) {
        temp[idx++] = '0';
    }

    while (rupees > 0) {
        temp[idx++] = (char)('0' + (rupees % 10));
        rupees /= 10;
    }

    if (is_neg) {
        temp[idx++] = '-';
    }

    /* Reverse digits to form the correct rupee string */
    char rup_str[64];
    int r_idx = 0;
    while (idx > 0) {
        rup_str[r_idx++] = temp[--idx];
    }
    rup_str[r_idx] = '\0';

    /* Combine rupees and 2-digit paise using standard %s.%02d */
    snprintf(buf, (size_t)size, "%s.%02d", rup_str, paisa_part);
}

/*
 * get_current_datetime
 * Fills buf with the current date/time in "YYYY-MM-DD HH:MM:SS" format.
 *
 * Uses time() to get the current time as seconds since epoch,
 * localtime() to convert to a struct tm (local timezone),
 * and strftime() to format it as a readable string.
 */
void get_current_datetime(char *buf, int size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    if (t != NULL) {
        strftime(buf, (size_t)size, "%Y-%m-%d %H:%M:%S", t);
    } else {
        strncpy(buf, "Unknown", (size_t)size - 1);
        buf[size - 1] = '\0';
    }
}

/*
 * display_account_details
 * Prints the details of a single account in a formatted layout.
 *
 * Takes a const pointer to Account:
 *   - 'const' promises we will NOT modify the account through this pointer
 *   - Using a pointer avoids copying the entire struct (more efficient)
 *   - We access members using the arrow operator (->) since acc is a pointer
 */
void display_account_details(const Account *acc) {
    char bal_str[32];
    format_amount(acc->balance_paise, bal_str, sizeof(bal_str));

    printf("  Account Number : %d\n", acc->account_number);
    printf("  Customer Name  : %s\n", acc->name);
    printf("  Account Type   : %s\n", acc->account_type);
    printf("  Balance        : Rs. %s\n", bal_str);
}

/*
 * find_account_index
 * Searches the accounts[] array for an account with the given number.
 *
 * Uses linear search: checks each element one by one.
 * Returns the array index if found, or -1 if not found.
 *
 * Time Complexity: O(n) where n = account_count
 * Space Complexity: O(1) - no extra memory used
 *
 * For 1000 accounts, this is fast enough. For millions of accounts,
 * we would use binary search (O(log n)) on a sorted array, or a
 * hash table (O(1) average). This is a common interview discussion point.
 */
int find_account_index(int account_number) {
    int i;
    for (i = 0; i < account_count; i++) {
        if (accounts[i].account_number == account_number) {
            return i;   /* Found at index i */
        }
    }
    return -1;  /* Not found */
}

/*
 * display_menu
 * Prints the main menu options for the Banking Management System.
 */
void display_menu(void) {
    printf("\n");
    printf("================================================\n");
    printf("       BANKING MANAGEMENT SYSTEM\n");
    printf("================================================\n");
    printf("  1. Create Account\n");
    printf("  2. Search Account\n");
    printf("  3. Display All Accounts\n");
    printf("  4. Deposit Money\n");
    printf("  5. Withdraw Money\n");
    printf("  6. Check Balance\n");
    printf("  7. View Transaction History\n");
    printf("  8. Exit\n");
    printf("================================================\n");
}

/* ==================== Banking Operations ==================== */

/*
 * create_account
 *
 * Steps:
 *   1. Check if the maximum account limit has been reached.
 *   2. Read and validate the customer name.
 *   3. Read and validate the account type (Savings or Current).
 *   4. Generate a unique account number.
 *   5. Initialize the account with zero balance.
 *   6. Add the account to the in-memory array.
 *   7. Save all accounts to file.
 *   8. If save fails, rollback by removing the account from memory.
 *
 * Demonstrates: struct initialization, strncpy (safe copy), array usage,
 *               error handling with rollback.
 */
void create_account(void) {
    char name[MAX_NAME_LEN];
    char type_input[16];
    Account new_acc;

    printf("\n--- Create New Account ---\n\n");

    /* Step 1: Check account limit */
    if (account_count >= MAX_ACCOUNTS) {
        printf("Error: Maximum account limit (%d) reached.\n", MAX_ACCOUNTS);
        return;
    }

    /* Step 2: Read and validate customer name */
    printf("Enter customer name: ");
    if (!read_line(name, sizeof(name))) {
        printf("Error: Failed to read name.\n");
        return;
    }
    if (!is_valid_name(name)) {
        printf("Error: Name cannot be empty or whitespace only.\n");
        return;
    }

    /* Step 3: Read and validate account type */
    printf("Enter account type (1 = Savings, 2 = Current): ");
    if (!read_line(type_input, sizeof(type_input))) {
        printf("Error: Failed to read account type.\n");
        return;
    }

    /* Use strcmp() to compare strings (cannot use == for strings in C) */
    if (strcmp(type_input, "1") == 0) {
        strcpy(new_acc.account_type, "Savings");
    } else if (strcmp(type_input, "2") == 0) {
        strcpy(new_acc.account_type, "Current");
    } else {
        printf("Error: Invalid account type. Enter 1 for Savings or 2 for Current.\n");
        return;
    }

    /* Step 4: Generate unique account number */
    new_acc.account_number = generate_account_number();

    /* Step 5: Copy name safely using strncpy and ensure null-termination */
    strncpy(new_acc.name, name, MAX_NAME_LEN - 1);
    new_acc.name[MAX_NAME_LEN - 1] = '\0';  /* Always null-terminate */

    /* Step 6: Initialize balance to zero (0 paise) */
    new_acc.balance_paise = 0;

    /* Step 7: Add to in-memory array */
    accounts[account_count] = new_acc;  /* Struct copy (all members) */
    account_count++;

    /* Step 8: Save to file; rollback on failure */
    if (!save_all_accounts()) {
        account_count--;    /* Rollback: remove from array */
        printf("Error: Failed to save account to file. Account not created.\n");
        return;
    }

    /* Success */
    printf("\n=== Account Created Successfully ===\n");
    display_account_details(&accounts[account_count - 1]);
    printf("====================================\n");
}

/*
 * search_account
 * Prompts for an account number and displays the account details if found.
 * Demonstrates find_account_index() for linear search.
 */
void search_account(void) {
    int acc_num;
    int index;

    printf("\n--- Search Account ---\n\n");

    if (!read_int("Enter account number: ", &acc_num)) {
        printf("Error: Invalid account number format.\n");
        return;
    }

    index = find_account_index(acc_num);
    if (index == -1) {
        printf("Error: Account %d not found.\n", acc_num);
        return;
    }

    printf("\n=== Account Found ===\n");
    display_account_details(&accounts[index]);
    printf("=====================\n");
}

/*
 * display_all_accounts
 * Shows all accounts in a formatted table.
 * Demonstrates array traversal with a for loop.
 */
void display_all_accounts(void) {
    int i;
    char bal_str[32];

    printf("\n--- All Customer Accounts ---\n\n");

    if (account_count == 0) {
        printf("No accounts found. Create an account first.\n");
        return;
    }

    /* Print table header */
    printf("%-10s  %-30s  %-10s  %15s\n",
           "Acc. No.", "Customer Name", "Type", "Balance (Rs.)");
    printf("------------------------------------------------------------------------\n");

    /* Print each account as a table row */
    for (i = 0; i < account_count; i++) {
        format_amount(accounts[i].balance_paise, bal_str, sizeof(bal_str));
        printf("%-10d  %-30s  %-10s  %15s\n",
               accounts[i].account_number,
               accounts[i].name,
               accounts[i].account_type,
               bal_str);
    }

    printf("------------------------------------------------------------------------\n");
    printf("Total accounts: %d\n", account_count);
}

/*
 * deposit_money
 *
 * Steps:
 *   1. Find the account by account number.
 *   2. Read and validate the deposit amount.
 *   3. Save old balance (for rollback if file save fails).
 *   4. Update balance in memory: balance += amount.
 *   5. Save all accounts to file.
 *   6. If save fails, rollback the balance change.
 *   7. Record the transaction (append to transactions.dat).
 *   8. Display success message.
 *
 * Demonstrates: pointer-based struct modification, error recovery,
 *               two-phase commit (save balance, then log transaction).
 */
void deposit_money(void) {
    int acc_num, index;
    long long amount_paise;
    long long old_balance;
    Transaction txn;
    char amt_str[32], bal_str[32];

    printf("\n--- Deposit Money ---\n\n");

    /* Step 1: Find the account */
    if (!read_int("Enter account number: ", &acc_num)) {
        printf("Error: Invalid account number format.\n");
        return;
    }

    index = find_account_index(acc_num);
    if (index == -1) {
        printf("Error: Account %d not found.\n", acc_num);
        return;
    }

    /* Show current balance for reference */
    format_amount(accounts[index].balance_paise, bal_str, sizeof(bal_str));
    printf("Account: %s | Current Balance: Rs. %s\n\n",
           accounts[index].name, bal_str);

    /* Step 2: Read and validate amount */
    if (!read_amount("Enter deposit amount (e.g., 1000.50): ", &amount_paise)) {
        printf("Error: Invalid amount. Enter a positive number with up to 2 decimal places.\n");
        return;
    }

    /* Step 3: Save old balance for potential rollback */
    old_balance = accounts[index].balance_paise;

    /* Step 4: Update balance in memory */
    accounts[index].balance_paise += amount_paise;

    /* Step 5: Save to persistent file */
    if (!save_all_accounts()) {
        /* Step 6: Rollback on failure */
        accounts[index].balance_paise = old_balance;
        printf("Error: Failed to save. Deposit cancelled.\n");
        return;
    }

    /* Step 7: Record the transaction */
    txn.transaction_id = get_next_transaction_id();
    txn.account_number = acc_num;
    strcpy(txn.type, "Deposit");
    txn.amount_paise = amount_paise;
    get_current_datetime(txn.datetime, sizeof(txn.datetime));
    txn.resulting_balance_paise = accounts[index].balance_paise;

    if (!append_transaction(&txn)) {
        /*
         * The balance is already saved, but the transaction log failed.
         * We warn the user but do NOT reverse the deposit because:
         *   - The money IS in the account (balance is saved)
         *   - Reversing would create further inconsistency
         * This is a known limitation. A production system would use
         * database transactions for atomicity.
         */
        printf("Warning: Deposit successful but transaction record could not be saved.\n");
    }

    /* Step 8: Display success */
    format_amount(amount_paise, amt_str, sizeof(amt_str));
    format_amount(accounts[index].balance_paise, bal_str, sizeof(bal_str));
    printf("\n=== Deposit Successful ===\n");
    printf("  Amount Deposited : Rs. %s\n", amt_str);
    printf("  New Balance      : Rs. %s\n", bal_str);
    printf("  Transaction ID   : %d\n", txn.transaction_id);
    printf("==========================\n");
}

/*
 * withdraw_money
 *
 * Same as deposit but with an additional check:
 *   - The withdrawal amount must not exceed the available balance.
 *   - If it does, the operation is rejected and no changes are made.
 *
 * Demonstrates: comparison operators, guard clause pattern.
 */
void withdraw_money(void) {
    int acc_num, index;
    long long amount_paise;
    long long old_balance;
    Transaction txn;
    char amt_str[32], bal_str[32];

    printf("\n--- Withdraw Money ---\n\n");

    if (!read_int("Enter account number: ", &acc_num)) {
        printf("Error: Invalid account number format.\n");
        return;
    }

    index = find_account_index(acc_num);
    if (index == -1) {
        printf("Error: Account %d not found.\n", acc_num);
        return;
    }

    /* Show current balance */
    format_amount(accounts[index].balance_paise, bal_str, sizeof(bal_str));
    printf("Account: %s | Current Balance: Rs. %s\n\n",
           accounts[index].name, bal_str);

    if (!read_amount("Enter withdrawal amount (e.g., 500.00): ", &amount_paise)) {
        printf("Error: Invalid amount. Enter a positive number with up to 2 decimal places.\n");
        return;
    }

    /* *** Key Check: Prevent overdraft *** */
    if (amount_paise > accounts[index].balance_paise) {
        format_amount(amount_paise, amt_str, sizeof(amt_str));
        format_amount(accounts[index].balance_paise, bal_str, sizeof(bal_str));
        printf("Error: Insufficient balance.\n");
        printf("  Requested : Rs. %s\n", amt_str);
        printf("  Available : Rs. %s\n", bal_str);
        return;
    }

    /* Save old balance for rollback */
    old_balance = accounts[index].balance_paise;

    /* Update balance: subtract the withdrawal amount */
    accounts[index].balance_paise -= amount_paise;

    /* Save to file */
    if (!save_all_accounts()) {
        accounts[index].balance_paise = old_balance;    /* Rollback */
        printf("Error: Failed to save. Withdrawal cancelled.\n");
        return;
    }

    /* Record the transaction */
    txn.transaction_id = get_next_transaction_id();
    txn.account_number = acc_num;
    strcpy(txn.type, "Withdrawal");
    txn.amount_paise = amount_paise;
    get_current_datetime(txn.datetime, sizeof(txn.datetime));
    txn.resulting_balance_paise = accounts[index].balance_paise;

    if (!append_transaction(&txn)) {
        printf("Warning: Withdrawal successful but transaction record could not be saved.\n");
    }

    /* Display success */
    format_amount(amount_paise, amt_str, sizeof(amt_str));
    format_amount(accounts[index].balance_paise, bal_str, sizeof(bal_str));
    printf("\n=== Withdrawal Successful ===\n");
    printf("  Amount Withdrawn : Rs. %s\n", amt_str);
    printf("  New Balance      : Rs. %s\n", bal_str);
    printf("  Transaction ID   : %d\n", txn.transaction_id);
    printf("=============================\n");
}

/*
 * check_balance
 * Simple balance inquiry - finds an account and displays its balance.
 */
void check_balance(void) {
    int acc_num, index;
    char bal_str[32];

    printf("\n--- Check Balance ---\n\n");

    if (!read_int("Enter account number: ", &acc_num)) {
        printf("Error: Invalid account number format.\n");
        return;
    }

    index = find_account_index(acc_num);
    if (index == -1) {
        printf("Error: Account %d not found.\n", acc_num);
        return;
    }

    format_amount(accounts[index].balance_paise, bal_str, sizeof(bal_str));
    printf("\n=== Balance Inquiry ===\n");
    printf("  Account Number : %d\n", accounts[index].account_number);
    printf("  Customer Name  : %s\n", accounts[index].name);
    printf("  Account Type   : %s\n", accounts[index].account_type);
    printf("  Balance        : Rs. %s\n", bal_str);
    printf("=======================\n");
}

/*
 * view_transaction_history
 * Displays all transactions for a specific account.
 * Validates the account exists before querying the file.
 */
void view_transaction_history(void) {
    int acc_num, index, count;

    printf("\n--- Transaction History ---\n\n");

    if (!read_int("Enter account number: ", &acc_num)) {
        printf("Error: Invalid account number format.\n");
        return;
    }

    index = find_account_index(acc_num);
    if (index == -1) {
        printf("Error: Account %d not found.\n", acc_num);
        return;
    }

    printf("\n=== Transactions for Account %d (%s) ===\n\n",
           accounts[index].account_number, accounts[index].name);

    /* Read and display matching transactions from file */
    count = display_transactions_for_account(acc_num);

    if (count == 0) {
        printf("  No transactions found for this account.\n");
    } else {
        printf("\n  Total transactions: %d\n", count);
    }

    printf("==========================================\n");
}
