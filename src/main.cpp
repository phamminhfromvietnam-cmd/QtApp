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

static constexpr int kMaxSearchLen = 200;   // was 100
static constexpr int kMaxRows = 2000;       // was 500
// Bump this whenever the schema or the term lists below change;
// the table is rebuilt automatically on the next start.
static constexpr int kSchemaVersion = 2;

const QStringList VietnameseTerm{"Số nguyên tố", "Số chính phương", "Đại số", "Nhân", "Chia", "Cộng", "Trừ",
    "Căn bậc hai", "Số mũ", "Giá trị tuyệt đối", "Phương trình", "Bất phương trình",
    "Hệ phương trình", "Giải bài toán bằng cách lập hệ phương trình", "Phương trình tích",
    "Phương trình chứa ẩn ở mẫu", "Phân tích đa thức thành nhân tử", "Hằng đẳng thức",
    "Đơn, đa thức", "Phân thức", "Căn thức", "Nghiệm", "Ẩn, biến", "Biện luận",
    "Giá trị nhỏ nhất", "Giá trị lớn nhất", "Điều kiện xác định", "Thỏa mãn",
    "Vì … nên", "Ta có", "Suy ra", "Vậy", "Thời gian", "Vận tốc", "Quãng đường",
    "Năng suất", "Bất đẳng thức", "Áp dụng bất đẳng thức", "Phương pháp dùng định nghĩa",
    "Chứng minh tương đương", "Phản chứng", "Đi từ cái bất đẳng thức đã biết",
    "Dấu \"=\" xảy ra khi", "Hàm số", "Hàm số bậc nhất", "Đồ thị hàm số",
    "Trục hoành", "Trục tung", "Hoành độ", "Tung độ", "Tọa độ", "Điểm cố định",
    "Hàm số đồng biến", "Hàm số nghịch biến", "Đường Parabol", "Ta lập bảng",
    "Đường thẳng", "Đoạn thẳng", "Hai đường thẳng vuông góc", "Tia", "Tia đối",
    "Giao điểm", "Đồng quy", "Điểm", "Góc", "Trung điểm", "Song song", "So le trong",
    "Đồng vị", "Trong cùng phía", "Đối đỉnh", "Kề bù", "Kề nhau", "Phụ nhau",
    "Bù nhau", "Góc vuông", "Góc nhọn", "Góc tù", "Cạnh", "Cạnh góc vuông",
    "Cạnh huyền", "Tam giác", "Tam giác nhọn", "Tam giác vuông", "Tam giác tù",
    "Tam giác cân", "Tam giác đều", "Nửa tam giác đều", "Tam giác bằng nhau",
    "Tam giác đồng dạng", "Đường trung tuyến", "Đường trung trực", "Đường phân giác",
    "Đường cao", "Xét tam giác", "Định lí Pythagore", "Định lí Thales", "Chứng minh",
    "Tứ giác", "Hình thang", "Hình thang cân", "Hình thang vuông", "Hình vuông",
    "Hình chữ nhật", "Hình thoi", "Hình bình hành", "Tính chất", "Dấu hiệu nhận biết",
    "Đường chéo", "Góc đối", "Góc kề", "Cạnh kề", "Cạnh đối", "Cạnh đáy",
    "Chiều cao", "Cạnh bên", "Tỉ số lượng giác",
    "Hệ thức lượng giữa các cạnh tam giác vuông",
    "Hệ thức lượng giữa các cạnh và góc tam giác vuông",
    "Đường tròn", "Bán kính", "Đường kính", "Dây", "Cung", "Tâm", "Đường tiếp tuyến",
    "Cắt nhau", "Cát tuyến", "Hai đường tròn cắt nhau", "Hai đường tròn ngoài nhau",
    "Hai đường tròn tiếp xúc", "Hai đường tròn tiếp xúc đôi một", "Hình khối",
    "Hình hộp chữ nhật", "Hình lập phương", "Hình nón", "Hình cầu", "Hình trụ",
    "Hình chóp tam giác đều", "Hình chóp tứ giác đều", "Thể tích",
    "Diện tích xung quanh", "Diện tích toàn phần", "Diện tích", "Chu vi", "Đáy",
    "Mặt chéo"
};
const QStringList EnglishTerm{"Prime number", "Square number / Perfect square", "Algebra",
    "Multiply / Multiplication", "Divide / Division", "Add / Addition",
    "Subtract / Subtraction", "Square root", "Exponent / Power", "Absolute value",
    "Equation", "Inequality", "System of equations",
    "Solving word problems by setting up a system of equations",
    "Factored equation / Product equation",
    "Rational equation / Equation with variables in the denominator",
    "Factoring polynomials / Factorization", "Algebraic identity",
    "Monomial, polynomial", "Algebraic fraction / Rational expression",
    "Radical expression", "Solution / Root", "Unknown, variable",
    "Discuss (the number of solutions based on parameters)", "Minimum value",
    "Maximum value", "Domain / Condition for existence", "Satisfy",
    "Since … therefore / Because … so", "We have / Given that",
    "Implies / It follows that / Therefore", "Thus / Hence / Therefore", "Time",
    "Speed / Velocity", "Distance", "Productivity / Rate of work", "Inequality",
    "Apply the inequality", "Proof by definition", "Proof by equivalence",
    "Proof by contradiction", "Deduce from a known inequality",
    "Equality holds when / Equality occurs if and only if", "Function",
    "Linear function", "Graph of a function", "Horizontal axis / x-axis",
    "Vertical axis / y-axis", "Abscissa / x-coordinate", "Ordinate / y-coordinate",
    "Coordinates", "Fixed point", "Increasing function", "Decreasing function",
    "Parabola", "We construct a table of values", "Line / Straight line",
    "Line segment", "Two perpendicular lines", "Ray", "Opposite ray",
    "Intersection point", "Concurrent", "Point", "Angle", "Midpoint", "Parallel",
    "Alternate interior (angles)", "Corresponding (angles)",
    "Consecutive interior / Co-interior (angles)", "Vertical (angles)",
    "Linear pair (of angles)", "Adjacent (angles)", "Complementary (angles)",
    "Supplementary (angles)", "Right angle", "Acute angle", "Obtuse angle",
    "Side / Edge", "Leg (of a right triangle)", "Hypotenuse", "Triangle",
    "Acute triangle", "Right triangle", "Obtuse triangle", "Isosceles triangle",
    "Equilateral triangle", "Half-equilateral triangle / 30-60-90 triangle",
    "Congruent triangles", "Similar triangles", "Median", "Perpendicular bisector",
    "Angle bisector", "Altitude / Height", "Consider triangle",
    "Pythagorean theorem", "Thales's theorem / Intercept theorem", "Prove / Proof",
    "Quadrilateral", "Trapezoid / Trapezium", "Isosceles trapezoid",
    "Right trapezoid", "Square", "Rectangle", "Rhombus", "Parallelogram",
    "Property / Properties", "Identification criteria / Conditions", "Diagonal",
    "Opposite angle", "Adjacent angle", "Adjacent side", "Opposite side", "Base",
    "Height / Altitude", "Lateral side / Leg", "Trigonometric ratio",
    "Metric relations between the sides of a right triangle",
    "Metric relations between the sides and angles of a right triangle", "Circle",
    "Radius", "Diameter", "Chord", "Arc", "Center", "Tangent line", "Intersect",
    "Secant line", "Two intersecting circles", "Two externally disjoint circles",
    "Two tangent circles", "Pairwise tangent circles", "Solid figures / 3D shapes",
    "Rectangular prism / Cuboid", "Cube", "Cone", "Sphere", "Cylinder",
    "Regular triangular pyramid / Regular tetrahedron", "Regular square pyramid",
    "Volume", "Lateral surface area", "Total surface area", "Area",
    "Perimeter / Circumference", "Base", "Diagonal plane / Diagonal face"
};

// Lowercase + strip diacritics so "Đường", "đường" and "duong" all match.
// (SQLite's LIKE only ignores case for ASCII, so we do the folding ourselves.)
static QString foldKey(const QString &s)
{
    QString d = s.normalized(QString::NormalizationForm_D).toLower();
    d.replace(u'đ', u'd');   // đ has no Unicode decomposition
    d.removeIf([](QChar c) { return c.category() == QChar::Mark_NonSpacing; });
    return d;
}

static bool initDb()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    if (!db.open())
        return false;

    QSqlQuery q(db);
    q.exec("PRAGMA user_version");
    const int version = q.next() ? q.value(0).toInt() : 0;
    q.finish();

    if (version == kSchemaVersion)
        return true;   // already up to date

    if (!db.transaction())
        return false;

    if (!q.exec("DROP TABLE IF EXISTS translations")
        || !q.exec("CREATE TABLE translations ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                   "vietnamese TEXT NOT NULL, "
                   "english TEXT NOT NULL, "
                   "search_key TEXT NOT NULL)")) {
        db.rollback();
        return false;
    }

    QVariantList vi, en, key;
    for (int i = 0; i < VietnameseTerm.size(); ++i) {
        vi << VietnameseTerm[i];
        en << EnglishTerm[i];
        key << foldKey(VietnameseTerm[i]);
    }

    q.prepare("INSERT INTO translations (vietnamese, english, search_key) VALUES (?,?,?)");
    q.addBindValue(vi);
    q.addBindValue(en);
    q.addBindValue(key);
    if (!q.execBatch()) {
        qDebug() << "Batch failed" << q.lastError().text();
        db.rollback();
        return false;
    }

    if (!db.commit())
        return false;

    q.exec(QString("PRAGMA user_version = %1").arg(kSchemaVersion));
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
              "WHERE search_key LIKE :pat ESCAPE '\\' "
              "ORDER BY id LIMIT :lim");
    q.bindValue(":pat", "%" + escapeLike(foldKey(text)) + "%"); // bound, never concatenated into SQL
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