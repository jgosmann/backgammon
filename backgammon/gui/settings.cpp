#include <QSettings>

#include "settings.h"

QColor load_color( const QString &name, const QColor &fallback )
{
    QSettings settings;

    return QColor(
        settings.value( name + "/r", fallback.red() ).toInt(),
        settings.value( name + "/g", fallback.green() ).toInt(),
        settings.value( name + "/b", fallback.blue() ).toInt() );
}

void save_color( const QString &name, const QColor &color )
{
    QSettings settings;

    settings.setValue( name + "/r", color.red() );
    settings.setValue( name + "/g", color.green() );
    settings.setValue( name + "/b", color.blue() );
}
