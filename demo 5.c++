#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QDateTime>
#include <QRegExp>

class AdminLogin : public QWidget {
    Q_OBJECT

public:
    AdminLogin(QWidget *parent = nullptr);
    ~AdminLogin();

private slots:
    void loginAdmin();

private:
    void initDB();

    QSqlDatabase db;
    QLineEdit *usernameLineEdit;
    QLineEdit *passwordLineEdit;
    QPushButton *loginButton;
};

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void manageFields();
    void showReservations();
    void addNewField();
    void manageCustomers();
    void addNewCustomer();
    void cancelReservation();
    void viewCustomerReservations();
    void updateFieldPrice();
    void backupDatabase();
    void restoreDatabase();
    void generateIncomeReport();
    void viewPopularFields();
    void addNewAdmin();
    void updateAdminPassword();
    void makeReservation();
    void checkReservationConflict();
    void updateFieldStatus(int fieldId, const QString &status);
    void viewPopularReservationTimes();

private:
    void loadReservations();
    void loadFields();
    void loadCustomers();
    bool isReservationConflict(int fieldId, const QString &reservationTime, int duration);
    bool isEmailValid(const QString &email);

    QTableWidget *reservationTable;
    QTableWidget *fieldTable;
    QTableWidget *customerTable;
};

AdminLogin::AdminLogin(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    usernameLineEdit = new QLineEdit(this);
    usernameLineEdit->setPlaceholderText("Username");

    passwordLineEdit = new QLineEdit(this);
    passwordLineEdit->setPlaceholderText("Password");
    passwordLineEdit->setEchoMode(QLineEdit::Password);

    loginButton = new QPushButton("Login", this);
    connect(loginButton, &QPushButton::clicked, this, &AdminLogin::loginAdmin);

    layout->addWidget(usernameLineEdit);
    layout->addWidget(passwordLineEdit);
    layout->addWidget(loginButton);

    initDB();
}

AdminLogin::~AdminLogin() {
    db.close();
}

void AdminLogin::initDB() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("halisaha.db");

    if (!db.open()) {
        QMessageBox::critical(this, "Database Error", "Failed to open the database!");
        return;
    }

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS admin_users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT NOT NULL, password TEXT NOT NULL, role_id INTEGER NOT NULL)");
    query.exec("CREATE TABLE IF NOT EXISTS user_roles (id INTEGER PRIMARY KEY AUTOINCREMENT, role_name TEXT NOT NULL)");
    query.exec("CREATE TABLE IF NOT EXISTS fields (id INTEGER PRIMARY KEY AUTOINCREMENT, field_name TEXT NOT NULL, price_per_minute DOUBLE NOT NULL, status TEXT NOT NULL)");
    query.exec("CREATE TABLE IF NOT EXISTS customers (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, email TEXT NOT NULL, phone TEXT NOT NULL)");
    query.exec("CREATE TABLE IF NOT EXISTS reservations (id INTEGER PRIMARY KEY AUTOINCREMENT, customer_id INTEGER NOT NULL, field_id INTEGER NOT NULL, reservation_time TEXT NOT NULL, duration INTEGER NOT NULL, FOREIGN KEY (customer_id) REFERENCES customers(id), FOREIGN KEY (field_id) REFERENCES fields(id))");

    query.exec("INSERT INTO user_roles (role_name) VALUES ('Superadmin')");
    query.exec("INSERT INTO user_roles (role_name) VALUES ('Field Manager')");
    
    query.exec("INSERT INTO admin_users (username, password, role_id) VALUES ('admin', 'admin123', 1)");
}

void AdminLogin::loginAdmin() {
    QString username = usernameLineEdit->text();
    QString password = passwordLineEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter both username and password.");
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT u.username, r.role_name FROM admin_users u JOIN user_roles r ON u.role_id = r.id WHERE u.username = ? AND u.password = ?");
    query.addBindValue(username);
    query.addBindValue(password);

    if (query.exec() && query.next()) {
        QString role = query.value(1).toString();
        QMessageBox::information(this, "Login Successful", "Admin logged in successfully as " + role);
        this->close();
        MainWindow *mainWindow = new MainWindow();
        mainWindow->show();
    } else {
        QMessageBox::warning(this, "Login Failed", "Invalid username or password.");
    }
}

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QPushButton *manageFieldsButton = new QPushButton("Manage Fields", this);
    connect(manageFieldsButton, &QPushButton::clicked, this, &MainWindow::manageFields);

    QPushButton *showReservationsButton = new QPushButton("Show Reservations", this);
    connect(showReservationsButton, &QPushButton::clicked, this, &MainWindow::showReservations);

    QPushButton *manageCustomersButton = new QPushButton("Manage Customers", this);
    connect(manageCustomersButton, &QPushButton::clicked, this, &MainWindow::manageCustomers);

    QPushButton *cancelReservationButton = new QPushButton("Cancel Reservation", this);
    connect(cancelReservationButton, &QPushButton::clicked, this, &MainWindow::cancelReservation);

    QPushButton *viewCustomerReservationsButton = new QPushButton("View Customer Reservations", this);
    connect(viewCustomerReservationsButton, &QPushButton::clicked, this, &MainWindow::viewCustomerReservations);

    QPushButton *updateFieldPriceButton = new QPushButton("Update Field Price", this);
    connect(updateFieldPriceButton, &QPushButton::clicked, this, &MainWindow::updateFieldPrice);

    QPushButton *generateIncomeReportButton = new QPushButton("Generate Income Report", this);
    connect(generateIncomeReportButton, &QPushButton::clicked, this, &MainWindow::generateIncomeReport);

    QPushButton *viewPopularFieldsButton = new QPushButton("View Popular Fields", this);
    connect(viewPopularFieldsButton, &QPushButton::clicked, this, &MainWindow::viewPopularFields);

    QPushButton *backupButton = new QPushButton("Backup Database", this);
    connect(backupButton, &QPushButton::clicked, this, &MainWindow::backupDatabase);

    QPushButton *restoreButton = new QPushButton("Restore Database", this);
    connect(restoreButton, &QPushButton::clicked, this, &MainWindow::restoreDatabase);

    QPushButton *addAdminButton = new QPushButton("Add New Admin", this);
    connect(addAdminButton, &QPushButton::clicked, this, &MainWindow::addNewAdmin);

    QPushButton *updateAdminPasswordButton = new QPushButton("Update Admin Password", this);
    connect(updateAdminPasswordButton, &QPushButton::clicked, this, &MainWindow::updateAdminPassword);

    QPushButton *makeReservationButton = new QPushButton("Make Reservation", this);
    connect(makeReservationButton, &QPushButton::clicked, this, &MainWindow::makeReservation);

    QPushButton *checkReservationConflictButton = new QPushButton("Check Reservation Conflict", this);
    connect(checkReservationConflictButton, &QPushButton::clicked, this, &MainWindow::checkReservationConflict);

    mainLayout->addWidget(manageFieldsButton);
    mainLayout->addWidget(showReservationsButton);
    mainLayout->addWidget(manageCustomersButton);
    mainLayout->addWidget(cancelReservationButton);
    mainLayout->addWidget(viewCustomerReservationsButton);
    mainLayout->addWidget(updateFieldPriceButton);
    mainLayout->addWidget(generateIncomeReportButton);
    mainLayout->addWidget(viewPopularFieldsButton);
    mainLayout->addWidget(backupButton);
    mainLayout->addWidget(restoreButton);
    mainLayout->addWidget(addAdminButton);
    mainLayout->addWidget(updateAdminPasswordButton);
    mainLayout->addWidget(makeReservationButton);
    mainLayout->addWidget(checkReservationConflictButton);

    reservationTable = new QTableWidget(this);
    fieldTable = new QTableWidget(this);
    customerTable = new QTableWidget(this);
    mainLayout->addWidget(reservationTable);
    mainLayout->addWidget(fieldTable);
    mainLayout->addWidget(customerTable);

    setLayout(mainLayout);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("halisaha.db");
    if (!db.open()) {
        QMessageBox::critical(this, "Database Error", "Failed to open the database!");
        return;
    }
}

MainWindow::~MainWindow() {}

void MainWindow::manageFields() {
    addNewField();
}

void MainWindow::showReservations() {
    loadReservations();
    loadFields();
    loadCustomers();
}

void MainWindow::loadReservations() {
    QSqlQuery query("SELECT r.id, c.name, f.field_name, r.reservation_time, r.duration FROM reservations r JOIN customers c ON r.customer_id = c.id JOIN fields f ON r.field_id = f.id");

    reservationTable->setRowCount(0);
    int row = 0;
    while (query.next()) {
        reservationTable->insertRow(row);
        reservationTable->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        reservationTable->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        reservationTable->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        reservationTable->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));
        reservationTable->setItem(row, 4, new QTableWidgetItem(query.value(4).toString()));
        row++;
    }
}

void MainWindow::loadFields() {
    QSqlQuery query("SELECT id, field_name, price_per_minute, status FROM fields");

    fieldTable->setRowCount(0);
    int row = 0;
    while (query.next()) {
        fieldTable->insertRow(row);
        fieldTable->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        fieldTable->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        fieldTable->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        fieldTable->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));
        row++;
    }
}

void MainWindow::loadCustomers() {
    QSqlQuery query("SELECT id, name, email, phone FROM customers");

    customerTable->setRowCount(0);
    int row = 0;
    while (query.next()) {
        customerTable->insertRow(row);
        customerTable->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        customerTable->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        customerTable->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        customerTable->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));
        row++;
    }
}

// Yeni Rezervasyon Yapma
void MainWindow::makeReservation() {
    bool ok;
    int customerId = QInputDialog::getInt(this, "Customer ID", "Enter Customer ID:", 0, 0, 10000, 1, &ok);
    
    if (ok) {
        int fieldId = QInputDialog::getInt(this, "Field ID", "Enter Field ID for reservation:", 0, 0, 10000, 1, &ok);
        
        if (ok) {
            QString reservationTime = QInputDialog::getText(this, "Reservation Time", "Enter Reservation Time (yyyy-MM-dd hh:mm:ss):", QLineEdit::Normal, "", &ok);
            
            if (ok && !reservationTime.isEmpty()) {
                int duration = QInputDialog::getInt(this, "Duration", "Enter duration (minutes):", 0, 1, 1440, 1, &ok);
                
                if (ok && duration > 0) {
                    // Çakışma kontrolü
                    if (isReservationConflict(fieldId, reservationTime, duration)) {
                        QMessageBox::warning(this, "Conflict Error", "Reservation time conflicts with an existing reservation.");
                        return;
                    }
                    
                    // Rezervasyon ekleme
                    QSqlQuery query;
                    query.prepare("INSERT INTO reservations (customer_id, field_id, reservation_time, duration) VALUES (?, ?, ?, ?)");
                    query.addBindValue(customerId);
                    query.addBindValue(fieldId);
                    query.addBindValue(reservationTime);
                    query.addBindValue(duration);
                    
                    if (query.exec()) {
                        QMessageBox::information(this, "Success", "Reservation made successfully!");
                        loadReservations();
                    } else {
                        QMessageBox::warning(this, "Error", "Failed to make reservation.");
                    }
                }
            }
        }
    }
}

bool MainWindow::isReservationConflict(int fieldId, const QString &reservationTime, int duration) {
    QSqlQuery query;
    query.prepare("SELECT * FROM reservations WHERE field_id = ? AND reservation_time = ? AND duration = ?");
    query.addBindValue(fieldId);
    query.addBindValue(reservationTime);
    query.addBindValue(duration);
    query.exec();
    
    return query.next();  // Eğer bir kayıt dönerse çakışma var demektir.
}

bool MainWindow::isEmailValid(const QString &email) {
    QRegExp regex("^[A-Za-z0-9]+([._%+-]*[A-Za-z0-9])*@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    return regex.exactMatch(email);
}

// Gelir Raporu
void MainWindow::generateIncomeReport() {
    QSqlQuery query("SELECT f.field_name, SUM(r.duration * f.price_per_minute) AS total_income FROM reservations r JOIN fields f ON r.field_id = f.id GROUP BY f.field_name");

    QString report = "Income Report:\n";
    while (query.next()) {
        QString fieldName = query.value(0).toString();
        double totalIncome = query.value(1).toDouble();
        report += QString("%1: %2 TL\n").arg(fieldName).arg(totalIncome);
    }
    
    QMessageBox::information(this, "Income Report", report);
}

// Popüler Sahaları Görüntüleme
void MainWindow::viewPopularFields() {
    QSqlQuery query("SELECT f.field_name, COUNT(r.id) AS reservation_count FROM reservations r JOIN fields f ON r.field_id = f.id GROUP BY f.field_name ORDER BY reservation_count DESC LIMIT 5");

    QString report = "Top 5 Popular Fields:\n";
    while (query.next()) {
        QString fieldName = query.value(0).toString();
        int reservationCount = query.value(1).toInt();
        report += QString("%1: %2 reservations\n").arg(fieldName).arg(reservationCount);
    }
    
    QMessageBox::information(this, "Popular Fields", report);
}

// Veritabanı Yedekleme
void MainWindow::backupDatabase() {
    QString fileName = QFileDialog::getSaveFileName(this, "Backup Database", "", "SQLite Database Files (*.db)");
    if (fileName.isEmpty()) return;

    QFile::copy("halisaha.db", fileName);
    QMessageBox::information(this, "Backup Success", "Database backup successful!");
}

// Veritabanı Geri Yükleme
void MainWindow::restoreDatabase() {
    QString fileName = QFileDialog::getOpenFileName(this, "Restore Database", "", "SQLite Database Files (*.db)");
    if (fileName.isEmpty()) return;

    QFile::copy(fileName, "halisaha.db");
    QMessageBox::information(this, "Restore Success", "Database restored successfully!");
}

// Yeni Admin Ekleme
void MainWindow::addNewAdmin() {
    bool ok;
    QString username = QInputDialog::getText(this, "Username", "Enter new admin username:", QLineEdit::Normal, "", &ok);
    if (!ok || username.isEmpty()) return;

    QString password = QInputDialog::getText(this, "Password", "Enter new admin password:", QLineEdit::Normal, "", &ok);
    if (!ok || password.isEmpty()) return;

    QSqlQuery query;
    query.prepare("INSERT INTO admin_users (username, password) VALUES (?, ?)");
    query.addBindValue(username);
    query.addBindValue(password);

    if (query.exec()) {
        QMessageBox::information(this, "Success", "New admin added successfully!");
    } else {
        QMessageBox::warning(this, "Error", "Failed to add new admin.");
    }
}

// Admin Şifre Güncelleme
void MainWindow::updateAdminPassword() {
    bool ok;
    QString currentPassword = QInputDialog::getText(this, "Current Password", "Enter current admin password:", QLineEdit::Normal, "", &ok);
    if (!ok || currentPassword.isEmpty()) return;

    QString newPassword = QInputDialog::getText(this, "New Password", "Enter new admin password:", QLineEdit::Normal, "", &ok);
    if (!ok || newPassword.isEmpty()) return;

    QSqlQuery query;
    query.prepare("UPDATE admin_users SET password = ? WHERE password = ?");
    query.addBindValue(newPassword);
    query.addBindValue(currentPassword);

    if (query.exec()) {
        QMessageBox::information(this, "Success", "Password updated successfully!");
    } else {
        QMessageBox::warning(this, "Error", "Failed to update password.");
    }
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    
    AdminLogin loginWindow;
    loginWindow.setWindowTitle("Admin Login");
    loginWindow.show();
    
    return a.exec();
}
