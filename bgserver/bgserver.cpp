/*!
 * \file bgserver.cpp
 * \brief Implementation of the \ref BGServer class.
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

#include "backgammon.h"
#include "bgconnection.h"
#include "bgserver.h"

/////////////////////////////////////////////////////////////////////////////
/// Constructor to which the server settings have to passed with \a settings.
/////////////////////////////////////////////////////////////////////////////
BGServer::BGServer( const ServerSettings *settings )
  : my_settings( settings )
{
  connect( this, SIGNAL( newConnection( void ) ),
           this, SLOT( on_newConnection( void ) ) );
}

/////////////////////////////////////////////////////////////////////////////
/// Player \a player leaves his game. There will be sent no confirmation
/// message to this player, if \a confirm is false.
/////////////////////////////////////////////////////////////////////////////
void BGServer::leave_game( BGConnection *player, bool confirm )
{
  if( !player->get_game() )
    {
      player->error( "JOIN", "Not in a game." );
      return;
    }

  BGConnection *other = NULL;
  if( player->get_game()->get_player( BG::WHITE ) == player )
    {
      player->get_game()->set_player( BG::WHITE, NULL );
      other = player->get_game()->get_player( BG::BLACK );
    }
  else
    {
      player->get_game()->set_player( BG::BLACK, NULL );
      other = player->get_game()->get_player( BG::WHITE );
    }

  unsigned int game_number = player->get_game()->get_game_number();
  QString username;
  if( other )
    {
      username = other->get_username();
      if( player->get_game()->started() )
        {
          other->write( "ENDGAME:WIN;0;0 0\r\n" );
          other->write( "INFO:JOIN;-1 " + username + "\r\n" );
          other->set_game( NULL );
          other->set_joined_game( false );
          other->set_am_i_ready( false );
          other->set_confirmed( false );
        }
    }
  else
    username = player->get_username();


  /* Delete the game? */
  if( (   player->get_game()->get_player( BG::WHITE ) == NULL
       && player->get_game()->get_player( BG::BLACK ) == NULL)
      || player->get_game()->started() )
    {
      for( int i = 0; i < my_games.size(); i++ )
        {
          if( my_games.at( i ) == player->get_game() )
            {
              for( int j = 0; j < my_connections.size(); j++ )
                {
                  my_connections[ j ]->write(
                    "INFO:ENDGAME;" + QString::number( game_number ) + " "
                    + username + " 0 0 0\r\n" );
                }
              delete my_games[ i ];
              my_games.removeAt( i );
              break;
            }
        }
    }

  player->set_game( NULL );
  player->set_joined_game( false );
  player->set_am_i_ready( false );
  player->set_confirmed( false );

  if( confirm )
    player->write( "CONFIRM:JOIN;-1\r\n" );
  for( int i = 0; i < my_connections.size(); i++ )
    my_connections[ i ]->write( "INFO:JOIN;-1 " + player->get_username()
                                + "\r\n" );
}

/////////////////////////////////////////////////////////////////////////////
/// Forward a chat message.
/////////////////////////////////////////////////////////////////////////////
void BGServer::on_chat( BGConnection *sender, QString receiver, QString msg )
{
  for( int i = 0; i < my_connections.size(); i++ )
    {
      if( my_connections.at( i )->get_username() == receiver )
        {
          my_connections[ i ]->write( "CHAT:" + sender->get_username() + ";"
                                      + msg + "\r\n" );
          sender->write( "CONFIRM\r\n" );
          return;
        }
    }
  sender->error( "CHAT", "User not found." );
}

/////////////////////////////////////////////////////////////////////////////
/// Closes a connection.
/////////////////////////////////////////////////////////////////////////////
void BGServer::on_disconnected( BGConnection *sender )
{
  sender->disconnect();
  my_connections.removeAt( my_connections.indexOf( sender ) );
  QString user = sender->get_username();

  BGConnection *other = NULL;
  if( sender->get_game() )
    {
      if( sender->get_game()->get_player( BG::WHITE ) != sender )
        other = sender->get_game()->get_player( BG::WHITE );
      else
        other = sender->get_game()->get_player( BG::BLACK );

      other->write( "ENDGAME:WIN;0;0 0\r\n" );
      leave_game( other, false );
      leave_game( sender, false );
    }

  sender->set_logged_in( false );
  sender->set_joined_game( false );
  sender->set_am_i_ready( false );
  sender->set_confirmed( false );
  sender->set_game( NULL );

  sender->deleteLater();
}

/////////////////////////////////////////////////////////////////////////////
/// Sends an endgame info to all clients.
/////////////////////////////////////////////////////////////////////////////
void BGServer::on_endgame( int game_number, int rounds_left, int win_height )
{
  QString buffer = "INFO:ENDGAME;" + QString::number( game_number ) + " "
                   + QString::number( rounds_left ) + " "
                   + QString::number( win_height ) + " 0\r\n";
  for( int i = 0; i < my_connections.size(); i++ )
    my_connections[ i ]->write( buffer );
}

/////////////////////////////////////////////////////////////////////////////
/// Sends the list of active games to the client.
/////////////////////////////////////////////////////////////////////////////
void BGServer::on_list_request( BGConnection *sender )
{
  QString buffer = "LIST:";
  for( int i = 0; i < my_games.size(); i++ )
    {
      if( i > 0 )
        buffer += ";";

      buffer += QString::number( my_games[ i ]->get_game_number() ) + " ";

      if( my_games[ i ]->get_player( BG::WHITE ) != NULL )
        buffer += my_games[ i ]->get_player( BG::WHITE )->get_username();
      else
        buffer += "NOONE";
      buffer += " ";
      if( my_games[ i ]->get_player( BG::BLACK ) != NULL )
        buffer += my_games[ i ]->get_player( BG::BLACK )->get_username();
      else
        buffer += "NOONE";
      buffer += " ";

      if( my_games[ i ]->get_password() == "" )
        buffer += "f";
      else
        buffer += "p";
      buffer += " ";

      buffer += "0 ";

      buffer += QString::number( my_games[ i ]->get_n_rounds() );
    }
  buffer += "\r\n";
  sender->write( buffer );
}

/////////////////////////////////////////////////////////////////////////////
/// Verifies the login data.
/////////////////////////////////////////////////////////////////////////////
void BGServer::on_login_request( BGConnection *sender, QString user,
                                QString pass )
{
  if( sender->logged_in() )
    {
      sender->error( "LOGIN", "Already logged in." );
      return;
    }
  if(    (!my_settings->user_exists( user )
          && !my_settings->unregistered_allowed())
      || ( my_settings->user_exists( user )
          && my_settings->get_password( user ) != pass ) )
    {
      sender->error( "LOGIN", "Invalid username or password." );
      return;
    }
  for( int i = 0; i < my_connections.size(); i++ )
    {
      if( my_connections.at( i )->get_username() == user )
        {
          sender->error( "LOGIN", "Username already in use." );
          return;
        }
    }

  sender->set_username( user );
  sender->set_logged_in( true );
  sender->write( "CONFIRM:LOGIN;Logged in as " + user + ".\r\n" );
  for( int i = 0; i < my_connections.size(); i++ )
    my_connections[ i ]->write( "INFO:LOGIN;" + user + "\r\n" );
}

/////////////////////////////////////////////////////////////////////////////
/// Lets a client join a game or creates one.
/////////////////////////////////////////////////////////////////////////////
void BGServer::on_join_request(
  BGConnection *sender, int game_number, QString password,
  unsigned int n_rounds )
{
  int game_idx; // Index of the game in the my_games list.

  /* Leave game? */
  if( game_number == -1 )
    {
      leave_game( sender );
      return;
    }
  /* Join game */
  else
    {
      for( game_idx = 0; game_idx < my_games.size(); game_idx++ )
        {
          if( my_games.at( game_idx )->get_game_number() == game_number )
            {
              if(    my_games[ game_idx ]->get_player( BG::WHITE ) != NULL
                  && my_games[ game_idx ]->get_player( BG::BLACK ) != NULL )
                {
                  sender->error( "JOIN", "Game is full." );
                  return;
                }
              if( my_games[ game_idx ]->get_password() != password )
                {
                  sender->error( "JOIN", "Password is incorrect." );
                  return;
                }
              break;
            }
        }
    }

  /* Has a new game to be created? */
  if( game_idx == my_games.size() )
    my_games.push_back( new BGGame( game_number, password, n_rounds ) );

  if( my_games[ game_idx ]->get_player( BG::WHITE ) == NULL )
    my_games[ game_idx ]->set_player( BG::WHITE, sender );
  else
    my_games[ game_idx ]->set_player( BG::BLACK, sender );
  sender->set_joined_game( true );
  sender->set_game( my_games[ game_idx ] );

  sender->write( "CONFIRM:JOIN;" + QString::number( game_number ) + "\r\n" );
  for( int i = 0; i < my_connections.size(); i++ )
    my_connections[ i ]->write( "INFO:JOIN;" + QString::number( game_number )
                                + " " + sender->get_username() + "\r\n" );
}

/////////////////////////////////////////////////////////////////////////////
/// Establishes a connection with a client.
/////////////////////////////////////////////////////////////////////////////
void BGServer::on_newConnection( void )
{
  while( hasPendingConnections() )
    {
      my_connections.push_back( new BGConnection( nextPendingConnection(),
                                                  this ) );
      BGConnection *connection = my_connections.back();
      connection->write( "HELLO:" + my_settings->get_motd() + "\r\n",
                         false );
      connect( connection,
               SIGNAL( login_request( BGConnection *, QString, QString ) ),
               this,
               SLOT( on_login_request( BGConnection *, QString,
                                       QString ) ) );
      connect( connection, SIGNAL( disconnected( BGConnection * ) ),
               this, SLOT( on_disconnected( BGConnection * ) ) );

      connect( connection,
               SIGNAL( chat( BGConnection *, QString, QString ) ),
               this, SLOT( on_chat( BGConnection *, QString, QString ) ) );
      connect( connection, SIGNAL( endgame( int, int, int ) ),
               this, SLOT( on_endgame( int, int, int ) ) );
      connect( connection, SIGNAL( list_request( BGConnection * ) ),
               this, SLOT( on_list_request( BGConnection * ) ) );
      connect( connection, SIGNAL( leave_game_request( BGConnection *,
                                    bool ) ),
               this, SLOT( leave_game( BGConnection *, bool ) ) );
      connect( connection,
               SIGNAL( join_request( BGConnection *, int, QString,
                                     unsigned int ) ),
               this, SLOT( on_join_request( BGConnection *, int, QString,
                                            unsigned int ) ) );
      connect( connection, SIGNAL( userlist_request( BGConnection * ) ),
               this, SLOT( on_userlist_request( BGConnection * ) ) );
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Sends the userlist to a client.
/////////////////////////////////////////////////////////////////////////////
void BGServer::on_userlist_request( BGConnection *sender )
{
  QString buffer = "USERS:";
  for( int i = 0; i < my_connections.size(); i++ )
    {
      if( i > 0 )
        buffer += ";";
      buffer += my_connections.at( i )->get_username();
    }
  buffer += "\r\n";
  sender->write( buffer );
}
