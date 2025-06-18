#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

sqlite3 *db_rooms;
sqlite3 *db_guests;
sqlite3 *db_income;
sqlite3 *db_users;
int room_count;

void showError(const char *message) {
    fprintf(stderr, "Error: %s\n", message);
    exit(EXIT_FAILURE);
}

void createDatabases() {
    int rc = sqlite3_open("rooms.db", &db_rooms);
    if (rc != SQLITE_OK) showError("Failed to open rooms database");

    rc = sqlite3_open("guests.db", &db_guests);
    if (rc != SQLITE_OK) showError("Failed to open guests database");

    rc = sqlite3_open("income.db", &db_income);
    if (rc != SQLITE_OK) showError("Failed to open income database");

    rc = sqlite3_open("users.db", &db_users);
    if (rc != SQLITE_OK) showError("Failed to open users database");
}

void checkRoomCount() {
    FILE *file = fopen("rooms.txt", "r");

    if (file == NULL) {
        file = fopen("rooms.txt", "w");
        printf("Enter the number of rooms: ");
        scanf("%d", &room_count);
        fprintf(file, "%d", room_count);
        fclose(file);
    } else {
        fscanf(file, "%d", &room_count);
        fclose(file);
    }

    printf("Room count: %d\n", room_count);
}

void updateRoomCount() {
    FILE *file = fopen("rooms.txt", "w");
    fprintf(file, "%d", room_count);
    fclose(file);
}

void registerUser() {
    char username[50];
    char password[50];
    char role[50];

    printf("Username: ");
    scanf("%s", username);

    printf("Password: ");
    scanf("%s", password);

    printf("Role: ");
    scanf("%s", role);

    sqlite3_exec(db_users, "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT, password TEXT, role TEXT);", 0, 0, 0);

    char query[200];
    sprintf(query, "INSERT INTO users (username, password, role) VALUES ('%s', '%s', '%s');", username, password, role);

    if (sqlite3_exec(db_users, query, 0, 0, 0) != SQLITE_OK) {
        showError("Failed to insert user record");
    }

    printf("User registration completed successfully.\n");
}

void registerGuest() {
    char name[50], surname[50];
    int room_number;
    char checkin_date[20], checkout_date[20];
    double daily_rate;

    printf("First name: ");
    scanf("%s", name);

    printf("Last name: ");
    scanf("%s", surname);

    printf("Room number: ");
    scanf("%d", &room_number);

    if (room_number > room_count) {
        room_count = room_number;
        updateRoomCount();
    }

    printf("Check-in date (DD.MM.YYYY): ");
    scanf("%s", checkin_date);

    printf("Check-out date (DD.MM.YYYY): ");
    scanf("%s", checkout_date);

    printf("Daily rate: ");
    scanf("%lf", &daily_rate);

    sqlite3_exec(db_guests, "CREATE TABLE IF NOT EXISTS guests (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, surname TEXT, room_number INT, checkin_date TEXT, checkout_date TEXT);", 0, 0, 0);

    char query[300];
    sprintf(query, "INSERT INTO guests (name, surname, room_number, checkin_date, checkout_date) VALUES ('%s', '%s', %d, '%s', '%s');", name, surname, room_number, checkin_date, checkout_date);

    if (sqlite3_exec(db_guests, query, 0, 0, 0) != SQLITE_OK) {
        showError("Failed to insert guest record");
    }

    FILE *feeFile = fopen("fees.txt", "a");
    fprintf(feeFile, "%.2f\n", daily_rate);
    fclose(feeFile);

    FILE *readFees = fopen("fees.txt", "r");
    double total_income = 0;

    while (fscanf(readFees, "%lf", &daily_rate) != EOF) {
        total_income += daily_rate;
    }

    fclose(readFees);

    sqlite3_exec(db_income, "CREATE TABLE IF NOT EXISTS income (id INTEGER PRIMARY KEY AUTOINCREMENT, total REAL);", 0, 0, 0);

    sprintf(query, "INSERT INTO income (total) VALUES (%.2f);", total_income);

    if (sqlite3_exec(db_income, query, 0, 0, 0) != SQLITE_OK) {
        showError("Failed to insert income record");
    }

    printf("Guest registration completed successfully.\n");
}

void viewIncome() {
    FILE *file = fopen("fees.txt", "r");

    if (file == NULL) {
        showError("Failed to read fee data");
    }

    double rate;
    double total = 0;

    while (fscanf(file, "%lf", &rate) != EOF) {
        total += rate;
    }

    fclose(file);

    printf("Total Income: %.2f\n", total);
}

int adminLogin() {
    char username[50];
    char password[50];

    printf("Admin Username: ");
    scanf("%s", username);

    printf("Admin Password: ");
    scanf("%s", password);


    return 1;
}

int main() {
    createDatabases();
    checkRoomCount();

    int choice;
    int isAdmin = 0;

    do {
        printf("1. Register User\n2. Register Guest\n3. Check Room Count\n4. View Income\n");

        if (isAdmin) {
            printf("6. Exit\n");
        } else {
            printf("5. Admin Login\n6. Exit\n");
        }

        printf("Your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                if (isAdmin) {
                    registerUser();
                } else {
                    printf("User registration requires admin login.\n");
                }
                break;
            case 2:
                registerGuest();
                break;
            case 3:
                printf("Room count: %d\n", room_count);
                break;
            case 4:
                viewIncome();
                break;
            case 5:
                isAdmin = adminLogin();
                break;
            case 6:
                printf("Exiting program.\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 6);

    sqlite3_close(db_rooms);
    sqlite3_close(db_guests);
    sqlite3_close(db_income);
    sqlite3_close(db_users);

    return 0;
}