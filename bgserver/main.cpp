/*!
 * \file bgserver/main.cpp
 * \brief Contains the main() function which starts the server.
 * \date So Mai 18 2008
 * \author Jan Gosmann (jan@hyper-world.de)
 *
 * \b Copyright: Copyright (C) 2006 - 2008 Jan Gosmann
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

#include <iostream>

#include <QCoreApplication>
#include <QString>
#include <QStringList>

#include "bgserver.h"
#include "serversettings.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
/// main() function
/////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] )
{
  QCoreApplication app( argc, argv );
  QStringList arguments = app.arguments();

  QString conffile = "bgserver.conf";
  if( arguments.contains( "--conffile" ) )
    {
      if( arguments.size() <= arguments.indexOf( "--conffile" ) + 1 )
        {
          cerr << "bgserver: Argument missing!" << endl;
          return EXIT_FAILURE;
        }
      conffile = arguments[ arguments.indexOf( "--conffile" ) + 1 ];
    }

  ServerSettings settings( conffile );

  BGServer server( &settings );
  server.start_listen();

  return app.exec();
}
