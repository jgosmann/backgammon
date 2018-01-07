/*!
 * \file bgserver.h
 * \brief Contains the declaration of the \ref BGServer class.
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

#ifndef BGSERVER_H
#define BGSERVER_H 1

#include <QList>
#include <QTcpServer>

#include "bgconnection.h"
#include "bggame.h"
#include "serversettings.h"

/////////////////////////////////////////////////////////////////////////////
/// This class provides the functionality for a backgammon server.
/////////////////////////////////////////////////////////////////////////////
class BGServer : public QTcpServer
{
  Q_OBJECT

  public:
    BGServer( const ServerSettings *settings );

    inline void start_listen( void )
      { listen( QHostAddress::Any, my_settings->get_port() ); };

  protected slots:
    void leave_game( BGConnection *player, bool confirm = true );
    void on_chat( BGConnection *sender, QString receiver, QString msg );
    void on_disconnected( BGConnection *sender );
    void on_endgame( int game_number, int rounds_left, int win_height );
    void on_list_request( BGConnection *sender );
    void on_login_request( BGConnection *sender, QString user,
                           QString pass );
    void on_join_request( BGConnection *sender, int game_number,
                          QString password, unsigned int n_rounds );
    void on_newConnection( void );
    void on_userlist_request( BGConnection *sender );

  private:
    const ServerSettings * const my_settings;

    QList< BGConnection * > my_connections;
    QList< BGGame * > my_games;
};

#endif // BGSERVER_H
