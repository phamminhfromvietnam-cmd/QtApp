#include <QApplication>
#include <QDebug>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QTableView>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QVariantList>
#include <QStringList>
static constexpr int kMaxSearchLen = 100;
static constexpr int kMaxRows = 500;
const QStringList VietnameseTerm={};
const QStringList EnglishTerm={};
static bool initDb()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("app.db");
    if (!db.open())
        return false;

    QSqlQuery q(db);
    if (!q.exec("CREATE TABLE IF NOT EXISTS translations ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "vietnamese TEXT NOT NULL, "
                "english TEXT NOT NULL)"))
        return false;

    q.exec("SELECT COUNT(*) FROM translations");
    if (q.next() && q.value(0).toInt() == 0) {
        db.transaction();
        q.prepare("INSERT INTO translations (vietnamese, english) VALUES (?,?)");
        QVariantList original;
        QVariantList translated;
        for(int i=0; i<VietnameseTerm.size(); i++){
            translated<<EnglishTerm[i];
            original<<VietnameseTerm[i];
        }
        q.addBindValue(original);
        q.addBindValue(translated);
        if(!q.execBatch()){
            qDebug()<<"Batch failed"<<q.lastError().text();
            return false;
        }
        db.commit();
    }
    return true;
}

// Neutralize LIKE wildcards so "%" and "_" are matched literally.
static QString escapeLike(const QString &s)
{
    QString out;
    out.reserve(s.size() * 2);
    for (QChar c : s) {
        if (c == u'\\' || c == u'%' || c == u'_')
            out += u'\\';
        out += c;
    }
    return out;
}

static void runSearch(QSqlQueryModel &model, const QString &raw)
{
    // Sanitize: trim, cap length, strip control characters
    QString text = raw.trimmed().left(kMaxSearchLen);
    text.removeIf([](QChar c) { return c.isNonCharacter() || c.category() == QChar::Other_Control; });

    QSqlQuery q(QSqlDatabase::database());
    q.prepare("SELECT id AS \"ID\", vietnamese AS \"Vietnamese\", english AS \"English\" FROM translations "
              "WHERE vietnamese LIKE :pat ESCAPE '\\' "
              "ORDER BY id LIMIT :lim");
    q.bindValue(":pat", "%" + escapeLike(text) + "%"); // bound, never concatenated into SQL
    q.bindValue(":lim", kMaxRows);

    if (!q.exec()) {
        qWarning() << "Search failed:" << q.lastError().text();
        return;
    }
    model.setQuery(std::move(q));
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (!initDb()) {
        QMessageBox::critical(nullptr, "Database error",
                              QSqlDatabase::database().lastError().text());
        return 1;
    }

    QSqlQueryModel model;

    // Proxy is used only so clicking column headers sorts the results
    QSortFilterProxyModel proxy;
    proxy.setSourceModel(&model);

    QWidget window;
    window.setWindowTitle("Translation for Mathematical Terms");
    window.resize(420, 360);

    auto *search = new QLineEdit;
    search->setPlaceholderText("Search for translation...");
    search->setClearButtonEnabled(true);
    search->setMaxLength(kMaxSearchLen);

    auto *view = new QTableView;
    view->setModel(&proxy);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSortingEnabled(true);
    view->horizontalHeader()->setStretchLastSection(true);

    auto *layout = new QVBoxLayout(&window);
    layout->addWidget(search);
    layout->addWidget(view);

    // Debounce: run the query 200 ms after the user stops typing
    QTimer debounce;
    debounce.setSingleShot(true);
    debounce.setInterval(200);
    QObject::connect(&debounce, &QTimer::timeout,
                     [&] { runSearch(model, search->text()); });
    QObject::connect(search, &QLineEdit::textChanged,
                     &debounce, qOverload<>(&QTimer::start));

    runSearch(model, QString()); // initial load: shows all rows

    window.show();
    return app.exec();
}