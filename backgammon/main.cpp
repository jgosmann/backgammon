/*!
 * \file backgammon/main.cpp
 * \brief Enthält die main-Funktion, welche das Programm und die grafische
 *        Benutzeroberfläche startet.
 * \date Mi Nov 15 2006
 * \author Jan Gosmann (jan@hyper-world.de)
 *
 * \b Copyright: Copyright (C) 2006 Jan Gosmann
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

/*!
 * \mainpage backgammon Dokumentation
 *
 * Bei der Welfenlab Competition 2006 ging es um die Entwicklung einer
 * künstlichen Intelligenz für das Brettspiel Backgammon.
 *
 * backgammon ist davon eine Weiterentwicklung (vor allem um ein eigenes
 * Serverprogramm ergänzt), um das Programm mehr Leuten zugänglich zu machen.
 *
 * Diese Dokumentation ist hauptsächlich als Referenz für die Programmierung
 * an diesem Programm gedacht. Informationen zur Bedienung und eine genauere
 * Erläuterung zu den Funktionsweisen der Algorithmen finden sich in der
 * eigentlichen Ausarbeitung zur Welfenlab Competition, die dem Archiv
 * beiliegen sollte.
 */

#include "config.h"

#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QTranslator>

#include "backgammon.h"

#include "gui/mainwindow.h"

/////////////////////////////////////////////////////////////////////////////
/// main()-Funktion
/////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] )
{
  QApplication app( argc, argv );

  QCoreApplication::setOrganizationName( ORGANIZATION );
  QCoreApplication::setOrganizationDomain( HOMEPAGE );
  QCoreApplication::setApplicationName( PACKAGE );
  QSettings settings;

  QTranslator translator;
  if( settings.value( "language", "en" ).toString() != "de" )
    {
      if( translator.load( "backgammon_"
                     + settings.value( "language", "en" ).toString() ) )
        { }
      else if( translator.load( "backgammon_"
                          + settings.value( "language", "en" ).toString(),
                          QCoreApplication::applicationDirPath() ) )
        { }
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
          if( !translator.load( "backgammon_"
                          + settings.value( "language", "en" ).toString(),
                          tr_dir ) )
            QMessageBox::warning( NULL, "Language",
                                  "Language file could not be loaded." );

        }
    }
  app.installTranslator( &translator );

  MainWindow main_window;
  main_window.show();
  return app.exec();
}
