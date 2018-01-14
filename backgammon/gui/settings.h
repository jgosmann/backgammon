#include <QColor>
#include <QString>

QColor load_color(const QString &name, const QColor &fallback);
void save_color(const QString &name, const QColor &color);
