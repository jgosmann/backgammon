/*!
 * \file main.cpp
 * \brief Contains the main() function wihich Starts the bgsrvstart user
 *        interface.
 * \date Sa May 23 2009
 * \author Jan Gosmann (jan@hyper-world.de)
 *
 * \b Copyright: Copyright (C) 2009 Jan Gosmann
 */

/***************************************************************************
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   *
 *                                                                         *
 *  See the GNU General Public License for more details.                   *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program; if not, write to the Free Software            *
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              *
 ***************************************************************************/

#include "../backgammon/config.h"

#include <cstdlib>
#include <stdexcept>

#include <QApplication>
#include <QBuffer>
#include <QByteArray>
#include <QDialog>
#include <QHostInfo>
#include <QMainWindow>
#include <QSettings>
#include <QString>
#include <QTextCodec>
#include <QTranslator>

#include "bgserver.h"
#include "serveroptions.h"

#include "ui_srvrunwin.h"

////////////////////////////////////////////////////////////////////////////////
/// main()-Function
////////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    QTextCodec::setCodecForLocale( QTextCodec::codecForName( "UTF-8" ) );

    QCoreApplication::setOrganizationName( ORGANIZATION );
    QCoreApplication::setOrganizationDomain( HOMEPAGE );
    QCoreApplication::setApplicationName( PACKAGE );
    QSettings settings;

    QTranslator translator;
    if( settings.value( "language", "en" ).toString() != "de" )
    {
        if( translator.load( "bgsrvstart_"
                + settings.value( "language", "en" ).toString() ) )
        {
        }
        else if( translator.load( "bgsrvstart_"
                + settings.value( "language", "en" ).toString(),
                                  QCoreApplication::applicationDirPath() ) )
        {
        }
        else
        {
#ifdef Q_WS_MAC
            QString tr_dir( QCoreApplication::applicationDirPath()
                    + "/../Resources" );
#else
#ifdef Q_WS_X11
            QString tr_dir( "/usr/local/share/backgammon" );
#else
            QString tr_dir( "" );
#endif
#endif
            if( !translator.load( "bgsrvstart_"
                    + settings.value( "language", "en" ).toString(), tr_dir ) )
                QMessageBox::warning( NULL, "Language",
                                      "Language file could not be loaded." );

        }
    }
    app.installTranslator( &translator );

    ServerOptions srvOptDlg;
    QByteArray xml = settings.value( "lastconf", "" ).toByteArray();
    QBuffer buf( &xml );
    buf.open( QIODevice::ReadOnly );
    try
    {
        srvOptDlg.get_settings().readFrom( &buf );
    }
    catch( std::runtime_error e )
    {
    } // If there is no configuration saved (like on the first start) the
    // readFrom function will fail. But we want to ignore that error.
    srvOptDlg.updateGui();
    if( srvOptDlg.exec() == QDialog::Rejected )
        return EXIT_SUCCESS;

    buf.close();
    buf.open( QIODevice::WriteOnly );
    srvOptDlg.get_settings().writeTo( &buf );
    settings.setValue( "lastconf", xml );

    BGServer server( &srvOptDlg.get_settings() );
    server.start_listen();

    QMainWindow mw;
    Ui::srvRunWin srvRunWin;
    srvRunWin.setupUi( &mw );

    srvRunWin.srvAddr->setText( QHostInfo::localHostName() );
    srvRunWin.srvPort->setText( QString::number( server.serverPort() ) );

    mw.show();
    return app.exec();
}
