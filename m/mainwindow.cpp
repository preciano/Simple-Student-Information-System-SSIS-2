#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileInfo>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->saveButton->hide();
    ui->saveButton2->hide();
    ui->cancelButton1->hide();
    ui->cancelButton2->hide();

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("C://Users//User//OneDrive//Documents//GitHub//Simple-Student-Information-System-SSIS-2//m//mainwindow.db");

    if (!db.open()) {
        qDebug() << "Error opening database:" << db.lastError().text();
        return;
    }

    QSqlQuery query("SELECT * FROM Students1");

    if (!query.exec()) {
        qDebug() << "Error executing query:" << query.lastError().text();
        db.close();
        return;
    }

    ui->studentT->setRowCount(0);

    ui->studentT->setColumnCount(7);
    ui->studentT->setHorizontalHeaderLabels({"Last Name", "First Name", "Middle Name", "ID Number", "Gender", "Year Level", "Course"});

    int row = 0;
    while (query.next()) {
        QString LastName = query.value("Surname").toString();
        QString FirstName = query.value("FirstName").toString();
        QString MiddleName = query.value("MiddleName").toString();
        QString IDNumber = query.value("IDNumber").toString();
        QString Gender = query.value("Gender").toString();
        QString YearLevel = query.value("YearLevel").toString();
        QString CourseCode = query.value("CourseCode").toString();

        ui->studentT->insertRow(row);

        ui->studentT->setItem(row, 0, new QTableWidgetItem(LastName));
        ui->studentT->setItem(row, 1, new QTableWidgetItem(FirstName));
        ui->studentT->setItem(row, 2, new QTableWidgetItem(MiddleName));
        ui->studentT->setItem(row, 3, new QTableWidgetItem(IDNumber));
        ui->studentT->setItem(row, 4, new QTableWidgetItem(Gender));
        ui->studentT->setItem(row, 5, new QTableWidgetItem(YearLevel));
        ui->studentT->setItem(row, 6, new QTableWidgetItem(CourseCode));

        row++;
    }


    //ui->tableWidget->resizeColumnsToContents();

    query.finish();

    QSqlQuery queryCourses(db);
    queryCourses.prepare("SELECT * FROM Course2");

    if (!queryCourses.exec()) {
        qDebug() << "Error executing courses query:" << queryCourses.lastError().text();
        db.close();
        return;
    }

    ui->courseT->setRowCount(0);
    ui->courseT->setColumnCount(2);
    ui->courseT->setHorizontalHeaderLabels({"Course Code", "Course Name"});

    row = 0;
    while (queryCourses.next()) {
        QString courseCode = queryCourses.value("CourseCode").toString();
        QString courseName = queryCourses.value("CourseName").toString();

        ui->courseT->insertRow(row);

        ui->courseT->setItem(row, 0, new QTableWidgetItem(courseCode));
        ui->courseT->setItem(row, 1, new QTableWidgetItem(courseName));

        row++;
    }
    ui->courseT->resizeColumnsToContents();

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_addButton1_clicked()
{
    // Retrieve values from input fields or widgets
    QString lastName = ui->lastName->text();
    QString firstName = ui->firstName->text();
    QString middleName = ui->middleName->text();
    QString idNumber = ui->ID->text();
    QString gender = ui->gender->currentText();
    QString yearLevel = ui->yrLvl->currentText();
    QString courseCode = ui->course->text();

    // Open the database connection
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "Database not open!";
        return;
    }

    // Check if the ID number is in the correct format (xxxx-xxxx)
    static QRegularExpression idRegex("\\d{4}-\\d{4}"); // Regular expression to match the pattern xxxx-xxxx
    if (!idRegex.match(idNumber).hasMatch()) {
        // ID number is not in the correct format, display a message to the user
        QMessageBox::warning(this, "Invalid ID Number", "Please enter a valid ID number in the format xxxx-xxxx (e.g., 2022-0001).");
        return;
    }

    // Check if the course code is in the correct format (XXXX)
    static QRegularExpression courseRegex("[A-Za-z]{4}"); // Regular expression to match the pattern XXXX
    if (!courseRegex.match(courseCode).hasMatch()) {
        // Course code is not in the correct format, display a message to the user
        QMessageBox::warning(this, "Invalid Course Code", "Please enter a valid course code in the format XXXX (e.g., BSCS).");
        return;
    }

    // Check if the ID already exists in the database
    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT IDNumber FROM Students1 WHERE IDNumber = :idNumber");
    checkQuery.bindValue(":idNumber", idNumber);
    if (!checkQuery.exec()) {
        qDebug() << "Error executing query to check ID existence:" << checkQuery.lastError().text();
        return;
    }

    if (checkQuery.next()) {
        // ID already exists, display a message to the user
        QMessageBox::warning(this, "Duplicate ID", "The entered ID already exists in the database.");
        return;
    }

    QSqlQuery courseQuery(db);
    courseQuery.prepare("SELECT CourseCode FROM Course2 WHERE CourseCode = :courseCode");
    courseQuery.bindValue(":courseCode", courseCode);
    if (!courseQuery.exec()) {
        qDebug() << "Error executing query to check course existence:" << courseQuery.lastError().text();
        return;
    }

    if (!courseQuery.next()) {
        // Course code does not exist, display a message to the user
        QMessageBox::warning(this, "Invalid Course Code", "The entered course code does not exist.");
        return;
    }

    // Start a transaction to ensure data consistency
    db.transaction();

    // Prepare the INSERT query
    QSqlQuery query(db);
    query.prepare("INSERT INTO Students1 (Surname, FirstName, MiddleName, IDNumber, Gender, YearLevel, CourseCode) "
                  "VALUES (:surname, :firstName, :middleName, :idNumber, :gender, :yearLevel, :courseCode)");
    query.bindValue(":surname", lastName);
    query.bindValue(":firstName", firstName);
    query.bindValue(":middleName", middleName);
    query.bindValue(":idNumber", idNumber);
    query.bindValue(":gender", gender);
    query.bindValue(":yearLevel", yearLevel);
    query.bindValue(":courseCode", courseCode);

    // Execute the query
    if (!query.exec()) {
        qDebug() << "Error executing INSERT query:" << query.lastError().text();
        // Rollback the transaction if an error occurs
        db.rollback();
        return;
    }

    // Commit the transaction
    db.commit();

    // Clear input fields after successful insertion
    ui->lastName->clear();
    ui->firstName->clear();
    ui->middleName->clear();
    ui->ID->clear();
    ui->course->clear();

    // Refresh the table view to display the newly added data
    // Clear existing items from the table
    ui->studentT->clearContents();
    // Set the number of rows to 0
    ui->studentT->setRowCount(0);

    // Refresh the table content
    QSqlQueryModel *model = new QSqlQueryModel;
    model->setQuery("SELECT * FROM Students1");
    if (model->lastError().isValid()) {
        qDebug() << "Error: Failed to execute query:" << model->lastError().text();
        delete model;
        return;
    }
    int row = 0;
    while (model->canFetchMore()) {
        model->fetchMore();
    }
    for (int i = 0; i < model->rowCount(); ++i) {
        ui->studentT->insertRow(row);
        for (int j = 0; j < model->columnCount(); ++j) {
            QString value = model->data(model->index(i, j)).toString();
            QTableWidgetItem *item = new QTableWidgetItem(value);
            ui->studentT->setItem(row, j, item);
        }
        ++row;
    }
    db.close();
}


void MainWindow::on_addButton2_clicked()
{
    QString courseC = ui->cC->text();
    QString courseN = ui->cN->text();

    // Open the database connection
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "Database not open!";
        return;
    }

    // Check if the ID already exists in the database
    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT CourseCode FROM Course2 WHERE CourseCode = :courseCode");
    checkQuery.bindValue(":courseCode", courseC);
    if (!checkQuery.exec()) {
        qDebug() << "Error executing query to check Course Code existence:" << checkQuery.lastError().text();
        return;
    }

    if (checkQuery.next()) {
        // Course Code already exists, display a message to the user
        QMessageBox::warning(this, "Duplicate Course", "The entered Course already exists in the database.");
        return;
    }

    db.transaction();

    QSqlQuery query(db);
    query.prepare("INSERT INTO Course2(CourseCode, CourseName) Values(:courseCode, :courseName)");
    query.bindValue(":courseCode", courseC);
    query.bindValue(":courseName", courseN);

    // Execute the query
    if (!query.exec()) {
        qDebug() << "Error executing INSERT query:" << query.lastError().text();
        // Rollback the transaction if an error occurs
        db.rollback();
        return;
    }

    // Commit the transaction
    db.commit();

    ui->cC->clear();
    ui->cN->clear();

    // Refresh the table view to display the newly added data
    // Clear existing items from the table
    ui->courseT->clearContents();
    // Set the number of rows to 0
    ui->courseT->setRowCount(0);

    // Refresh the table content
    QSqlQueryModel *model = new QSqlQueryModel;
    model->setQuery("SELECT * FROM Course2");
    if (model->lastError().isValid()) {
        qDebug() << "Error: Failed to execute query:" << model->lastError().text();
        delete model;
        return;
    }
    int row = 0;
    while (model->canFetchMore()) {
        model->fetchMore();
    }
    for (int i = 0; i < model->rowCount(); ++i) {
        ui->courseT->insertRow(row);
        for (int j = 0; j < model->columnCount(); ++j) {
            QString value = model->data(model->index(i, j)).toString();
            QTableWidgetItem *item = new QTableWidgetItem(value);
            ui->courseT->setItem(row, j, item);
        }
        ++row;
    }
    db.close();
}


void MainWindow::on_deleteButton1_clicked()
{
    // Get the selected row index
    int selectedRow = ui->studentT->currentRow();
    if (selectedRow < 0) {
        // No row selected, display a message to the user
        QMessageBox::warning(this, "No Row Selected", "Please select a row to delete.");
        return;
    }

    // Retrieve the IDNumber of the selected row
    QString idNumber = ui->studentT->item(selectedRow, 3)->text();

    // Open the database connection
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "Database not open!";
        return;
    }

    // Start a transaction to ensure data consistency
    db.transaction();

    // Prepare the DELETE query
    QSqlQuery query(db);
    query.prepare("DELETE FROM Students1 WHERE IDNumber = :idNumber");
    query.bindValue(":idNumber", idNumber);

    // Execute the query
    if (!query.exec()) {
        qDebug() << "Error executing DELETE query:" << query.lastError().text();
        // Rollback the transaction if an error occurs
        db.rollback();
        return;
    }

    // Commit the transaction
    db.commit();

    // Remove the selected row from the table widget
    ui->studentT->removeRow(selectedRow);

    db.close();
}


void MainWindow::on_editButton1_clicked()
{
    // Get the selected row index
    int selectedRow = ui->studentT->currentRow();
    if (selectedRow < 0) {
        // No row selected, display a message to the user
        QMessageBox::warning(this, "No Row Selected", "Please select a row to edit.");
        return;
    }

    // Populate the input fields with the data of the selected row
    ui->lastName->setText(ui->studentT->item(selectedRow, 0)->text());
    ui->firstName->setText(ui->studentT->item(selectedRow, 1)->text());
    ui->middleName->setText(ui->studentT->item(selectedRow, 2)->text());
    ui->ID->setText(ui->studentT->item(selectedRow, 3)->text());
    ui->gender->setCurrentText(ui->studentT->item(selectedRow, 4)->text());
    ui->yrLvl->setCurrentText(ui->studentT->item(selectedRow, 5)->text());
    ui->course->setText(ui->studentT->item(selectedRow, 6)->text());

    // Disable the ID field as it should not be editable
    ui->ID->setEnabled(false);

    // Show the save button
    ui->saveButton->show();
    ui->cancelButton1->show();
}

void MainWindow::updateRecord(int row)
{
    // Retrieve values from input fields or widgets
    QString lastName = ui->lastName->text();
    QString firstName = ui->firstName->text();
    QString middleName = ui->middleName->text();
    QString idNumber = ui->ID->text();
    QString gender = ui->gender->currentText();
    QString yearLevel = ui->yrLvl->currentText();
    QString courseCode = ui->course->text();

    // Open the database connection
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "Database not open!";
        return;
    }

    // Start a transaction to ensure data consistency
    db.transaction();

    // Prepare the UPDATE query
    QSqlQuery query(db);
    query.prepare("UPDATE Students1 SET Surname = :surname, FirstName = :firstName, MiddleName = :middleName, "
                  "Gender = :gender, YearLevel = :yearLevel, CourseCode = :courseCode WHERE IDNumber = :idNumber");
    query.bindValue(":surname", lastName);
    query.bindValue(":firstName", firstName);
    query.bindValue(":middleName", middleName);
    query.bindValue(":gender", gender);
    query.bindValue(":yearLevel", yearLevel);
    query.bindValue(":courseCode", courseCode);
    query.bindValue(":idNumber", idNumber);

    // Execute the query
    if (!query.exec()) {
        qDebug() << "Error executing UPDATE query:" << query.lastError().text();
        // Rollback the transaction if an error occurs
        db.rollback();
        return;
    }

    // Commit the transaction
    db.commit();

    // Update the table widget with the edited data
    ui->studentT->item(row, 0)->setText(lastName);
    ui->studentT->item(row, 1)->setText(firstName);
    ui->studentT->item(row, 2)->setText(middleName);
    ui->studentT->item(row, 4)->setText(gender);
    ui->studentT->item(row, 5)->setText(yearLevel);
    ui->studentT->item(row, 6)->setText(courseCode);

    // Clear input fields after successful update
    clearInputFields();
}

void MainWindow::clearInputFields()
{
    // Clear input fields after editing or cancelling
    ui->lastName->clear();
    ui->firstName->clear();
    ui->middleName->clear();
    ui->ID->clear();
    ui->course->clear();

    // Enable the ID field for further editing
    ui->ID->setEnabled(true);
}

void MainWindow::on_saveButton_clicked()
{
    QString courseCode = ui->course->text();

    // Get the selected row index
    int selectedRow = ui->studentT->currentRow();
    if (selectedRow < 0) {
        // No row selected, display a message to the user
        QMessageBox::warning(this, "No Row Selected", "Please select a row to save changes.");
        return;
    }

    // Open the database connection
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "Database not open!";
        return;
    }

    QSqlQuery courseQuery(db);
    courseQuery.prepare("SELECT CourseCode FROM Course2 WHERE CourseCode = :courseCode");
    courseQuery.bindValue(":courseCode", courseCode);
    if (!courseQuery.exec()) {
        qDebug() << "Error executing query to check course existence:" << courseQuery.lastError().text();
        return;
    }

    if (!courseQuery.next()) {
        // Course code does not exist, display a message to the user
        QMessageBox::warning(this, "Invalid Course Code", "The entered course code does not exist.");
        return;
    }

    // Trigger the update operation
    updateRecord(selectedRow);

    // Hide the save button after saving changes
    ui->saveButton->hide();
    ui->cancelButton1->hide();
    db.close();
}

void MainWindow::on_searchButton1_clicked()
{
    // Retrieve the ID entered by the user
    QString idNumber = ui->lineSearch->text();

    // Open the database connection
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "Database not open!";
        return;
    }

    // Prepare the query to retrieve student details by ID
    QSqlQuery query(db);
    query.prepare("SELECT * FROM Students1 WHERE IDNumber = :idNumber");
    query.bindValue(":idNumber", idNumber);

    // Execute the query
    if (!query.exec()) {
        qDebug() << "Error executing search query:" << query.lastError().text();
        return;
    }

    bool found = query.next();
    if (found) {
        // Student found, get the row index
        int rowIndex = -1;
        QList<QTableWidgetItem *> items = ui->studentT->findItems(idNumber, Qt::MatchExactly);
        if (!items.isEmpty()) {
            QTableWidgetItem *item = items.first();
            if (item) {
                rowIndex = item->row();
            }
        }

        // Highlight the row
        if (rowIndex != -1) {
            ui->studentT->selectRow(rowIndex);
        }

        // Scroll to the highlighted row
        if (rowIndex != -1) {
            ui->studentT->scrollToItem(ui->studentT->item(rowIndex, 0));
        }
    } else {
        // Student not found, display a message
        QMessageBox::warning(this, "Student Not Found", "No student found with the provided ID.");
    }

    db.close();
}


void MainWindow::on_searchButton2_clicked()
{
    // Retrieve the Course Code entered by the user
    QString courseC = ui->lineSearch2->text();

    // Open the database connection
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "Database not open!";
        return;
    }

    // Prepare the query to retrieve course details by Course Code
    QSqlQuery query(db);
    query.prepare("SELECT * FROM Course2 WHERE CourseCode = :courseC");
    query.bindValue(":courseC", courseC);

    // Execute the query
    if (!query.exec()) {
        qDebug() << "Error executing search query:" << query.lastError().text();
        return;
    }

    bool found = query.next();
    if (found) {
        // Course found, get the row index
        int rowIndex = -1;
        QList<QTableWidgetItem *> items = ui->courseT->findItems(courseC, Qt::MatchExactly);
        if (!items.isEmpty()) {
            QTableWidgetItem *item = items.first();
            if (item) {
                rowIndex = item->row();
            }
        }

        // Highlight the row
        if (rowIndex != -1) {
            ui->courseT->selectRow(rowIndex);
        }

        // Scroll to the highlighted row
        if (rowIndex != -1) {
            ui->courseT->scrollToItem(ui->courseT->item(rowIndex, 0));
        }
    } else {
        // Course not found, display a message
        QMessageBox::warning(this, "Course Not Found", "No course found with the provided Course Code.");
    }

    db.close();
}


void MainWindow::on_editButton2_clicked()
{
    // Get the selected row index
    int selectedRow = ui->courseT->currentRow();
    if (selectedRow < 0) {
        // No row selected, display a message to the user
        QMessageBox::warning(this, "No Row Selected", "Please select a row to edit.");
        return;
    }


    // Populate the input fields with the data of the selected row
    ui->cC->setText(ui->courseT->item(selectedRow, 0)->text());
    ui->cN->setText(ui->courseT->item(selectedRow, 1)->text());

    // Show the save button
    ui->saveButton2->show();
    ui->cancelButton2->show();
}


void MainWindow::on_saveButton2_clicked()
{
    QString courseCode = ui->cC->text();
    QString courseName = ui->cN->text();

    // Get the selected row index
    int selectedRow = ui->courseT->currentRow();
    // Store the old course code before editing
    QString oldCourseCode = ui->courseT->item(selectedRow, 0)->text();

    if (selectedRow < 0) {
        // No row selected, display a message to the user
        QMessageBox::warning(this, "No Row Selected", "Please select a row to save changes.");
        return;
    }

    // Open the database connection
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "Database not open!";
        return;
    }

    // Check if the ID already exists in the database
    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT CourseCode FROM Course2 WHERE CourseCode = :courseCode");
    checkQuery.bindValue(":courseCode", courseCode);
    if (!checkQuery.exec()) {
        qDebug() << "Error executing query to check Course Code existence:" << checkQuery.lastError().text();
        return;
    }

    if (checkQuery.next()) {
        // Course Code already exists, display a message to the user
        QMessageBox::warning(this, "Duplicate Course", "The entered Course already exists in the database.");
        return;
    }

    // Start a transaction to ensure data consistency
    db.transaction();

    // Prepare the UPDATE query for Course2 table
    QSqlQuery query(db);
    query.prepare("UPDATE Course2 SET CourseCode = :courseCode, CourseName = :courseName WHERE CourseCode = :oldCourseCode");
    query.bindValue(":courseCode", courseCode);
    query.bindValue(":courseName", courseName);
    query.bindValue(":oldCourseCode", oldCourseCode); // You need to define oldCourseCode somewhere

    // Execute the query for updating Course2 table
    if (!query.exec()) {
        qDebug() << "Error executing UPDATE query for Course2:" << query.lastError().text();
        // Rollback the transaction if an error occurs
        db.rollback();
        return;
    }

    // Prepare the UPDATE query for Students1 table
    QSqlQuery updateStudentsQuery(db);
    updateStudentsQuery.prepare("UPDATE Students1 SET CourseCode = :newCourseCode WHERE CourseCode = :oldCourseCode");
    updateStudentsQuery.bindValue(":newCourseCode", courseCode);
    updateStudentsQuery.bindValue(":oldCourseCode", oldCourseCode);

    // Execute the query for updating Students1 table
    if (!updateStudentsQuery.exec()) {
        qDebug() << "Error executing UPDATE query for Students1:" << updateStudentsQuery.lastError().text();
        // Rollback the transaction if an error occurs
        db.rollback();
        return;
    }

    // Commit the transaction
    db.commit();

    // Update the table widget with the edited data for Course2 table
    ui->courseT->item(selectedRow, 0)->setText(courseCode);
    ui->courseT->item(selectedRow, 1)->setText(courseName);

    // Clear input fields after successful update
    clearInputFields2();

    // Hide the save button after saving changes
    ui->saveButton2->hide();
    ui->cancelButton2->hide();

    db.close();
}

void MainWindow::updateRecord2(int row)
{
    QString courseCode = ui->cC->text();
    QString courseName = ui->cN->text();
    QString i;

    int selectedRow = ui->courseT->currentRow();
    if (selectedRow < 0) {
        // No row selected, return an empty string
        return;
    }

    QTableWidgetItem *item = ui->courseT->verticalHeaderItem(selectedRow);
    if (!item) {
        // No vertical header item found, return an empty string
        return;
    }

    // Open the database connection
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "Database not open!";
        return;
    }

    // Start a transaction to ensure data consistency
    db.transaction();

    // Prepare the UPDATE query
    QSqlQuery query(db);
    query.prepare("UPDATE Course2 SET CourseCode = :courseCode, CourseName = :courseName WHERE item = :i");
    query.bindValue(":courseCode", courseCode);
    query.bindValue(":courseName", courseName);
    query.bindValue(":i", i);

    // Execute the query
    if (!query.exec()) {
        qDebug() << "Error executing UPDATE query:" << query.lastError().text();
        // Rollback the transaction if an error occurs
        db.rollback();
        return;
    }

    // Commit the transaction
    db.commit();

    // Update the table widget with the edited data
    ui->courseT->item(row, 0)->setText(courseCode);
    ui->courseT->item(row, 1)->setText(courseName);

    // Clear input fields after successful update
    clearInputFields2();
}

void MainWindow::clearInputFields2()
{
    // Clear input fields after editing or cancelling
    ui->cC->clear();
    ui->cN->clear();

    // Enable the ID field for further editing
    ui->cC->setEnabled(true);
}

void MainWindow::on_cancelButton1_clicked()
{
    clearInputFields();

    ui->saveButton->hide();
    ui->cancelButton1->hide();
}

void MainWindow::on_cancelButton2_clicked()
{
    clearInputFields2();

    ui->saveButton2->hide();
    ui->cancelButton2->hide();
}

void MainWindow::onTabChanged(int index)
{
    if (index == ui->tabWidget->indexOf(ui->tab_2)) {
        // Refresh the studentT table
        QSqlDatabase db = QSqlDatabase::database();
        if (!db.isOpen()) {
            qDebug() << "Database not open!";
            return;
        }

        // Refresh the table content
        QSqlQueryModel *model = new QSqlQueryModel;
        model->setQuery("SELECT * FROM Students1");
        if (model->lastError().isValid()) {
            qDebug() << "Error: Failed to execute query:" << model->lastError().text();
            delete model;
            return;
        }

        // Clear existing items from the table
        ui->studentT->clearContents();
        // Set the number of rows to 0
        ui->studentT->setRowCount(0);

        // Populate the table with the fetched data
        int row = 0;
        while (model->canFetchMore()) {
            model->fetchMore();
        }
        for (int i = 0; i < model->rowCount(); ++i) {
            ui->studentT->insertRow(row);
            for (int j = 0; j < model->columnCount(); ++j) {
                QString value = model->data(model->index(i, j)).toString();
                QTableWidgetItem *item = new QTableWidgetItem(value);
                ui->studentT->setItem(row, j, item);
            }
            ++row;
        }

        db.close();
    }
}

void MainWindow::on_deleteButton2_clicked()
{
    // Get the selected row index
    int selectedRow = ui->courseT->currentRow();
    if (selectedRow < 0) {
        // No row selected, display a message to the user
        QMessageBox::warning(this, "No Row Selected", "Please select a row to delete.");
        return;
    }

    // Retrieve the CourseCode of the selected row
    QString courseCode = ui->courseT->item(selectedRow, 0)->text();

    // Confirmation message box
    QMessageBox::StandardButton confirmDelete;
    confirmDelete = QMessageBox::question(this, "Confirm Deletion. Are you sure?", "The students that have this course will also be deleted.",
                                          QMessageBox::Yes | QMessageBox::No);
    if (confirmDelete == QMessageBox::No) {
        // User clicked No, do nothing
        return;
    }

    // Open the database connection
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "Database not open!";
        return;
    }

    // Start a transaction to ensure data consistency
    db.transaction();

    // Prepare the DELETE query for Course2 table
    QSqlQuery courseQuery(db);
    courseQuery.prepare("DELETE FROM Course2 WHERE CourseCode = :courseCode");
    courseQuery.bindValue(":courseCode", courseCode);

    // Execute the DELETE query for Course2 table
    if (!courseQuery.exec()) {
        qDebug() << "Error executing DELETE query for Course2 table:" << courseQuery.lastError().text();
        // Rollback the transaction if an error occurs
        db.rollback();
        return;
    }

    // Prepare the DELETE query for Students1 table
    QSqlQuery deleteStudentsQuery(db);
    deleteStudentsQuery.prepare("DELETE FROM Students1 WHERE CourseCode = :courseCode");
    deleteStudentsQuery.bindValue(":courseCode", courseCode);

    // Execute the DELETE query for Students1 table
    if (!deleteStudentsQuery.exec()) {
        qDebug() << "Error executing DELETE query for Students1 table:" << deleteStudentsQuery.lastError().text();
        // Rollback the transaction if an error occurs
        db.rollback();
        return;
    }

    // Commit the transaction
    db.commit();

    // Remove the selected row from the course table widget
    ui->courseT->removeRow(selectedRow);

    // Clear input fields after successful deletion
    clearInputFields2();

    // Hide the save and cancel buttons
    ui->saveButton2->hide();
    ui->cancelButton2->hide();
    db.close();
}


