/*
 * banking.h
 * Banking Management System - Main Header File
 *
 * Contains structure definitions, constants, and function declarations
 * for account management and banking operations.
 *
 * Key Design Decision:
 *   All monetary values are stored as 'long long' in paise
 *   (1 Rupee = 100 Paise) to avoid floating-point rounding errors.
 *   For example, Rs. 1234.56 is stored as 123456 paise.
 */

#ifndef BANKING_H
#define BANKING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>

/* ==================== Constants ==================== */

#define MAX_NAME_LEN        100     /* Maximum length of customer name       */
#define MAX_ACCOUNTS        1000    /* Maximum number of accounts supported  */
#define MAX_ACCT_TYPE_LEN   20      /* Length of account type string          */
#define MAX_TXN_TYPE_LEN    20      /* Length of transaction type string      */
#define MAX_DATETIME_LEN    30      /* Length of date-time string             */
#define STARTING_ACCT_NUM   1001    /* First auto-generated account number   */

#define ACCOUNTS_FILE       "accounts.dat"      /* Binary file for accounts     */
#define TRANSACTIONS_FILE   "transactions.dat"  /* Binary file for transactions */

/* ==================== Structures ==================== */

/*
 * Account - Stores customer account information.
 *
 * Members:
 *   account_number  - Unique identifier (auto-generated, starts at 1001)
 *   name            - Customer's full name
 *   account_type    - "Savings" or "Current"
 *   balance_paise   - Account balance in paise (avoids floating-point errors)
 */
typedef struct {
    int account_number;
    char name[MAX_NAME_LEN];
    char account_type[MAX_ACCT_TYPE_LEN];
    long long balance_paise;
} Account;

/*
 * Transaction - Stores a record of a single deposit or withdrawal.
 *
 * Members:
 *   transaction_id          - Unique ID for this transaction
 *   account_number          - Which account this belongs to
 *   type                    - "Deposit" or "Withdrawal"
 *   amount_paise            - Transaction amount in paise
 *   datetime                - When the transaction occurred ("YYYY-MM-DD HH:MM:SS")
 *   resulting_balance_paise - Account balance after this transaction
 */
typedef struct {
    int transaction_id;
    int account_number;
    char type[MAX_TXN_TYPE_LEN];
    long long amount_paise;
    char datetime[MAX_DATETIME_LEN];
    long long resulting_balance_paise;
} Transaction;

/* ==================== Global Data ==================== */

/*
 * All accounts are stored in this global array during runtime.
 * account_count tracks how many accounts are currently loaded.
 * These are defined in banking.c and accessed via 'extern' here.
 */
extern Account accounts[MAX_ACCOUNTS];
extern int account_count;

/* ==================== Banking Operations ==================== */

/* Account management */
void create_account(void);
void search_account(void);
void display_all_accounts(void);

/* Banking transactions */
void deposit_money(void);
void withdraw_money(void);
void check_balance(void);
void view_transaction_history(void);

/* ==================== Utility Functions ==================== */

int  find_account_index(int account_number);
int  read_line(char *buf, int size);
int  read_int(const char *prompt, int *value);
int  read_amount(const char *prompt, long long *paise);
int  parse_amount_to_paise(const char *str, long long *paise);
void format_amount(long long paise, char *buf, int size);
void get_current_datetime(char *buf, int size);
void display_account_details(const Account *acc);
void display_menu(void);

#endif /* BANKING_H */
