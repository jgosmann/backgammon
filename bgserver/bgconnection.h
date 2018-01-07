/*!
 * \file bgconnection.h
 * \brief Contains the declaration of the \ref BGConnection class.
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

#ifndef BGCONNECTION_H
#define BGCONNECTION_H 1

#include <QObject>
#include <QString>
#include <QTcpSocket>

class BGConnection;

#include "bggame.h"

/////////////////////////////////////////////////////////////////////////////
/// This class manages the connection to one client.
/////////////////////////////////////////////////////////////////////////////
class BGConnection : public QObject
{
  Q_OBJECT

  public:
    BGConnection( QTcpSocket *socket, QObject *parent = NULL );
    // Destructor resultet in SegVault and there seems to be no need to
    // delete my_socket. It shouldn't even be the task of this class.
    // ~BGConnection( void ) { delete my_socket; };

    inline void error( QString cmd, QString msg )
      { write( "ERROR:" + cmd + ";" + msg + "\r\n", false ); };

    inline void write( QString msg, bool only_logged_in = true )
      {
        if( !only_logged_in || i_logged_in )
          my_socket->write( msg.toAscii() );
      };

    inline QTcpSocket * get_socket( void ) { return my_socket; };
    inline bool am_i_ready( void ) const { return i_am_ready; };
    inline bool confirmed( void ) const { return i_confirmed; };
    inline bool logged_in( void ) const { return i_logged_in; };
    inline bool joined_game( void ) const { return i_joined_game; };
    inline QString get_username( void ) const { return my_username; };
    inline BGGame * get_game( void ) { return my_game; };

    inline void set_am_i_ready( bool value ) { i_am_ready = value; };
    inline void set_logged_in( bool value ) { i_logged_in = value; };
    inline void set_confirmed( bool value ) { i_confirmed = value; };
    inline void set_joined_game( bool value ) { i_joined_game = value; };
    inline void set_username( QString value ) { my_username = value; };
    inline void set_game( BGGame *game ) { my_game = game; };

  signals:
    void chat( BGConnection *sender, QString receiver, QString msg );
    void disconnected( BGConnection *sender );
    void endgame( int game_number, int rounds_left, int win_height );
    void leave_game_request( BGConnection *player, bool confirm );
    void list_request( BGConnection *sender );
    void login_request( BGConnection *sender, QString user, QString pass );
    void join_request( BGConnection *sender, int game_number,
                       QString password, unsigned int n_rounds );
    void userlist_request( BGConnection *sender );

  private slots:
    void on_readyRead( void );
    inline void on_disconnected( void ) { emit disconnected( this ); };

  private:
    QTcpSocket *my_socket;
    bool i_logged_in;
    bool i_joined_game;
    bool i_am_ready;
    bool i_confirmed;
    QString my_username;
    BGGame *my_game;
};

#endif // BGCONNECTION_H
