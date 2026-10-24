/*
 * file_handler.c
 * Banking Management System - File Handler Implementation
 *
 * Handles all persistent data storage using binary files.
 * - accounts.dat   : Stores Account structures (read/write all at once)
 * - transactions.dat: Stores Transaction structures (append-only)
 *
 * ======================== Data Persistence Strategy ========================
 *
 * ACCOUNTS (accounts.dat):
 *   After every account creation, deposit, or withdrawal, the ENTIRE
 *   accounts[] array is rewritten to accounts.dat using "wb" mode.
 *   This is simple but NOT atomic: if the write fails midway, the file
 *   may be corrupted. A production system would write to a temporary
 *   file first, then rename it (atomic replace). We keep the simple
 *   approach here for clarity and document this as a known limitation.
 *
 * TRANSACTIONS (transactions.dat):
 *   Each transaction is appended one at a time using "ab" mode.
 *   Since we never modify existing transactions, this is safe.
 *   If the append fails, the account balance is already saved but
 *   the transaction log will be incomplete (documented limitation).
 *
 * =========================================================================
 */

#include "file_handler.h"

/* ---------- load_accounts ---------- */
/*
 * Reads all Account records from accounts.dat into accounts[].
 * Sets account_count to the number of accounts loaded.
 *
 * If the file does not exist, we simply start with 0 accounts.
 * This is normal when running the program for the first time.
 *
 * Uses fread() to read one Account struct at a time.
 * Stops when fread returns 0 (end-of-file or error).
 * Uses ferror() to distinguish between EOF and a real error.
 */
int load_accounts(void) {
    account_count = 0;

    FILE *fp = fopen(ACCOUNTS_FILE, "rb");
    if (fp == NULL) {
        /* File doesn't exist yet - first run, start fresh */
        return 1;
    }

    /* Read accounts one at a time into the global array */
    while (account_count < MAX_ACCOUNTS) {
        size_t read_count = fread(&accounts[account_count], sizeof(Account), 1, fp);
        if (read_count == 0) {
            break;  /* End of file reached or read error */
        }
        account_count++;
    }

    /* Check if the loop ended due to an actual error */
    if (ferror(fp)) {
        fprintf(stderr, "Error: Failed to read from %s.\n", ACCOUNTS_FILE);
        fclose(fp);
        return 0;
    }

    fclose(fp);
    printf("Loaded %d account(s) from file.\n", account_count);
    return 1;
}

/* ---------- save_all_accounts ---------- */
/*
 * Writes the entire accounts[] array to accounts.dat.
 *
 * Opens the file in "wb" mode, which:
 *   - Creates the file if it doesn't exist
 *   - Truncates (empties) the file if it does exist
 *
 * After writing, calls fflush() to push data to the OS buffer.
 * Checks the return values of fwrite() and fflush() for errors.
 *
 * Limitation: If fwrite fails after partial data is written,
 * the file may contain incomplete data. See header comment.
 */
int save_all_accounts(void) {
    FILE *fp = fopen(ACCOUNTS_FILE, "wb");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open %s for writing.\n", ACCOUNTS_FILE);
        return 0;
    }

    if (account_count > 0) {
        size_t written = fwrite(accounts, sizeof(Account),
                                (size_t)account_count, fp);
        if (written != (size_t)account_count) {
            fprintf(stderr, "Error: Only wrote %lu of %d accounts to file.\n",
                    (unsigned long)written, account_count);
            fclose(fp);
            return 0;
        }
    }

    /* Flush the stream to ensure data reaches the OS */
    if (fflush(fp) != 0) {
        fprintf(stderr, "Error: Failed to flush data to %s.\n", ACCOUNTS_FILE);
        fclose(fp);
        return 0;
    }

    fclose(fp);
    return 1;
}

/* ---------- append_transaction ---------- */
/*
 * Appends a single Transaction record to transactions.dat.
 *
 * Opens in "ab" mode (append + binary):
 *   - Creates the file if it doesn't exist
 *   - Appends to the end without disturbing existing data
 *
 * This function is called AFTER the account balance has been
 * successfully saved. If this fails, the balance is updated
 * but the transaction won't appear in the history.
 */
int append_transaction(const Transaction *txn) {
    FILE *fp = fopen(TRANSACTIONS_FILE, "ab");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open %s for writing.\n",
                TRANSACTIONS_FILE);
        return 0;
    }

    size_t written = fwrite(txn, sizeof(Transaction), 1, fp);
    if (written != 1) {
        fprintf(stderr, "Error: Failed to write transaction to file.\n");
        fclose(fp);
        return 0;
    }

    /* Flush to ensure the transaction is persisted */
    if (fflush(fp) != 0) {
        fprintf(stderr, "Error: Failed to flush transaction data.\n");
        fclose(fp);
        return 0;
    }

    fclose(fp);
    return 1;
}

/* ---------- get_next_transaction_id ---------- */
/*
 * Scans all existing transactions to find the highest transaction_id.
 * Returns (highest_id + 1) so each new transaction gets a unique ID.
 *
 * If no transactions file exists, returns 1 (first transaction).
 *
 * This uses a simple linear scan. For a large number of transactions,
 * we could store the counter in a separate file, but this approach
 * is simpler and sufficient for our purposes.
 */
int get_next_transaction_id(void) {
    FILE *fp = fopen(TRANSACTIONS_FILE, "rb");
    if (fp == NULL) {
        return 1;   /* No transactions yet, start from 1 */
    }

    Transaction txn;
    int max_id = 0;

    while (fread(&txn, sizeof(Transaction), 1, fp) == 1) {
        if (txn.transaction_id > max_id) {
            max_id = txn.transaction_id;
        }
    }

    fclose(fp);
    return max_id + 1;
}

/* ---------- display_transactions_for_account ---------- */
/*
 * Reads transactions.dat and displays all Transaction records
 * that match the given account_number.
 *
 * Reads one transaction at a time using fread(), checks if the
 * account_number matches, and prints the details if it does.
 *
 * Returns the number of transactions displayed (0 if none found).
 */
int display_transactions_for_account(int account_number) {
    FILE *fp = fopen(TRANSACTIONS_FILE, "rb");
    if (fp == NULL) {
        return 0;   /* No transactions file yet */
    }

    Transaction txn;
    int count = 0;
    char amt_str[32];
    char bal_str[32];

    while (fread(&txn, sizeof(Transaction), 1, fp) == 1) {
        if (txn.account_number == account_number) {
            count++;

            /* Format amounts for display */
            format_amount(txn.amount_paise, amt_str, sizeof(amt_str));
            format_amount(txn.resulting_balance_paise, bal_str, sizeof(bal_str));

            printf("  Transaction ID : %d\n", txn.transaction_id);
            printf("  Type           : %s\n", txn.type);
            printf("  Amount         : Rs. %s\n", amt_str);
            printf("  Date & Time    : %s\n", txn.datetime);
            printf("  Balance After  : Rs. %s\n", bal_str);
            printf("  -----------------------------\n");
        }
    }

    fclose(fp);
    return count;
}
