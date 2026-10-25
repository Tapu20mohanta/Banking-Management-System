/*
 * main.c
 * Banking Management System - Application Entry Point
 *
 * A menu-driven console application that manages customer bank accounts,
 * deposits, withdrawals, balance inquiries, and transaction records.
 *
 * Compilation (GCC on Linux/Mac):
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -o banking main.c banking.c file_handler.c
 *
 * Compilation (GCC/MinGW on Windows):
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -o banking.exe main.c banking.c file_handler.c
 *
 * Execution:
 *   ./banking        (Linux/Mac)
 *   banking.exe      (Windows)
 *
 * Author:  Tapu Mohanta
 * Purpose: Console-based Banking Management System demonstrating robust C programming
 *
 * DISCLAIMER: This is an educational project demonstrating C programming
 * concepts. It does not handle real banking data and must not be used
 * in production. Do not store real customer information.
 */

#include "banking.h"
#include "file_handler.h"

/*
 * main - Program entry point
 *
 * Flow:
 *   1. Display welcome message.
 *   2. Load existing accounts from accounts.dat.
 *   3. Enter the main loop:
 *      a. Display menu.
 *      b. Read user choice.
 *      c. Use switch statement to call the appropriate function.
 *      d. Repeat until user chooses Exit (option 8).
 *   4. Exit cleanly.
 *
 * The while(1) loop runs forever until the user selects Exit,
 * which calls return 0 to end the program normally.
 *
 * The switch statement handles each menu option. The 'default' case
 * catches any number outside 1-8. Invalid (non-numeric) input is
 * caught by read_int() before the switch is reached.
 */
int main(void) {
    int choice;

    printf("\n");
    printf("================================================\n");
    printf("   Welcome to the Banking Management System\n");
    printf("================================================\n");
    printf("\n");

    /* Load existing accounts from file at startup */
    if (!load_accounts()) {
        printf("Warning: Could not load existing accounts. Starting fresh.\n");
    }

    printf("\n");

    /* ===== Main Application Loop ===== */
    while (1) {
        /* Display the menu */
        display_menu();

        /* Read the user's choice safely */
        if (!read_int("  Enter your choice: ", &choice)) {
            printf("\n  Error: Invalid input. Please enter a number (1-8).\n");
            continue;   /* Skip to next iteration, show menu again */
        }

        /* Handle the choice using a switch statement */
        switch (choice) {
            case 1:
                create_account();
                break;

            case 2:
                search_account();
                break;

            case 3:
                display_all_accounts();
                break;

            case 4:
                deposit_money();
                break;

            case 5:
                withdraw_money();
                break;

            case 6:
                check_balance();
                break;

            case 7:
                view_transaction_history();
                break;

            case 8:
                printf("\n");
                printf("================================================\n");
                printf("  Thank you for using the Banking Management\n");
                printf("  System. Goodbye!\n");
                printf("================================================\n");
                printf("\n");
                return 0;   /* Exit the program successfully */

            default:
                printf("\n  Error: Invalid choice '%d'. Please select 1-8.\n",
                       choice);
                break;
        }
    }

    /* This line is never reached because of return 0 in case 8,
       but is included as good practice for main(). */
    return 0;
}
