/*
 * file_handler.h
 * Banking Management System - File Handler Header
 *
 * Declares functions for loading, saving, and managing
 * persistent account and transaction records stored in binary files.
 */

#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include "banking.h"

/*
 * load_accounts
 * Reads all accounts from ACCOUNTS_FILE into the global accounts[] array.
 * If the file does not exist, starts with zero accounts (not an error).
 * Returns: 1 on success, 0 on read error.
 */
int load_accounts(void);

/*
 * save_all_accounts
 * Writes the entire accounts[] array to ACCOUNTS_FILE.
 * Overwrites the file each time (simple but not atomic).
 * Returns: 1 on success, 0 on write error.
 */
int save_all_accounts(void);

/*
 * append_transaction
 * Appends a single Transaction record to TRANSACTIONS_FILE.
 * Creates the file if it does not exist.
 * Returns: 1 on success, 0 on write error.
 */
int append_transaction(const Transaction *txn);

/*
 * get_next_transaction_id
 * Scans existing transactions to find the highest ID.
 * Returns: (highest_id + 1), or 1 if no transactions exist.
 */
int get_next_transaction_id(void);

/*
 * display_transactions_for_account
 * Reads TRANSACTIONS_FILE and prints all transactions for the
 * given account number.
 * Returns: the number of transactions displayed.
 */
int display_transactions_for_account(int account_number);

#endif /* FILE_HANDLER_H */
