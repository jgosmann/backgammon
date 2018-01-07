/*!
 * \file bgconnection.cpp
 * \brief Contains the implementation of the \ref BGConnection class.
 * \date So Mai 25 2008
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

#include "bgconnection.h"

#include <vector>

#include <QString>
#include <QStringList>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
/// Constructor, with \a socket has the socket to be passed which should be
/// used for communicating with the client. The connection should already
/// have been established with this socket.
/////////////////////////////////////////////////////////////////////////////
BGConnection::BGConnection( QTcpSocket *socket, QObject *parent )
 : QObject( parent ), my_socket( socket ), i_logged_in( false ),
   i_joined_game( false ), i_am_ready( false ), i_confirmed( false ),
   my_username( "" ), my_game( NULL )
{
  connect( my_socket, SIGNAL( readyRead( void ) ),
           this, SLOT( on_readyRead( void ) ) );
  connect( my_socket, SIGNAL( disconnected( void ) ),
           this, SLOT( on_disconnected( void ) ) );
};

/////////////////////////////////////////////////////////////////////////////
/// Processes newly arrived data from the client.
/////////////////////////////////////////////////////////////////////////////
void BGConnection::on_readyRead( void )
{
  while( my_socket->canReadLine() )
    {
      QString buffer = my_socket->readLine();
      buffer = buffer.remove( '\r' );
      buffer = buffer.remove( '\n' );
      const QStringList buffer_list = QString( buffer ).split( ':' );
      const QString command = buffer_list[ 0 ];
      const QStringList arguments = (buffer_list.size() > 1 )
                                    ? buffer_list[ 1 ].split( ';' )
                                    : QStringList();

      if( command == "LOGIN" )
        {
          if( arguments.size() >= 2 )
            emit login_request( this, arguments[ 0 ], arguments[ 1 ] );
          else if( arguments.size() == 1 )
            emit login_request( this, arguments[ 0 ], "" );
          else
            error( command, "Not enough arguments." );
        }
      else if( i_logged_in )
        {
          if( command == "LOGOUT" )
            my_socket->close();
          else if( command == "USERS" )
            emit userlist_request( this );
          else if( command == "CHAT" )
            {
              if( arguments.size() >= 2 )
                emit chat( this, arguments[ 0 ], arguments[ 1 ] );
              else
                error( command, "Not enough arguments." );
            }
          else if( command == "JOIN" )
            {
              if( arguments.size() >= 1 && arguments[ 0 ].toInt() != -1
                  && i_joined_game )
                {
                  error( command, "Please leave the joined game first." );
                  return;
                }

              if( arguments.size() >= 4 )
                emit join_request( this, arguments[ 0 ].toInt(),
                                   arguments[ 1 ], arguments[ 3 ].toInt() );
              else if( arguments.size() >= 2 )
                emit join_request( this, arguments[ 0 ].toInt(),
                                   arguments[ 1 ], 1 );
              else if( arguments.size() >= 1 )
                emit join_request( this, arguments[ 0 ].toInt(), "", 1 );
              else
                error( command, "Not enough arguments." );
            }
          else if( command == "LIST" )
            emit list_request( this );
          else if( command == "START" )
            {
              if( !my_game )
                {
                  error( command, "Unexpected command." );
                  return;
                }

              i_am_ready = true;

              if(    my_game->get_player( BG::WHITE )
                  && my_game->get_player( BG::BLACK )
                  && my_game->get_player( BG::WHITE )->am_i_ready()
                  && my_game->get_player( BG::BLACK )->am_i_ready() )
                {
                  my_game->set_started( true );
                  my_game->get_player( BG::WHITE )->write(
                    "CONFIRM:START\r\n" );
                  my_game->get_player( BG::BLACK )->write(
                    "CONFIRM:START\r\n" );
                  my_game->reset();
                  my_game->transmit_board();
                }
            }
          else if( command == "CONFIRM" )
            {
              if( !my_game )
                {
                  // No error message here, because the client will send
                  // a CONFIRM for the last transfered board. But at this
                  // time the server will have it already deleted.
                  return;
                }

              i_confirmed = true;
              if(    my_game->started()
                  && my_game->get_game_status() != BG::GAME_FINISHED
                  && my_game->get_player( BG::WHITE )->confirmed()
                  && my_game->get_player( BG::BLACK )->confirmed() )
                {
                  my_game->get_player( BG::WHITE )->set_confirmed( false );
                  my_game->get_player( BG::BLACK )->set_confirmed( false );
                  my_game->transmit_dice( );
                }
            }
          else if( command == "TURN" )
            {
              if( !my_game || !my_game->started() )
                {
                  error( command, "Unexpected command." );
                  return;
                }

              BG::Player act_player = my_game->get_act_player();

              if( arguments[ 0 ] == "NOTURN" )
                {
                  my_game->move();

                  if( act_player == BG::WHITE )
                    my_game->get_player( BG::BLACK )->write(
                                                        "INFO:TURN\r\n" );
                  else
                    my_game->get_player( BG::WHITE )->write(
                                                        "INFO:TURN\r\n" );
                  my_game->transmit_board();
                  return;
                }

              vector< BG::BackgammonMove > moves;
              for( int i = 0; i < arguments.size(); i++ )
                {
                  QStringList sub_args = arguments[ i ].split( ' ' );
                  if( sub_args.size() < 2 )
                    continue;

                  if( sub_args[ 1 ].toInt() == 25 )
                    moves.push_back( BG::BackgammonMove(
                                       BG::BAR, sub_args[ 0 ].toInt() ) );
                  else if( act_player == BG::WHITE )
                    moves.push_back( BG::BackgammonMove(
                                       24 - sub_args[ 1 ].toInt(),
                                       sub_args[ 0 ].toInt() ) );
                  else
                    moves.push_back( BG::BackgammonMove(
                                       sub_args[ 1 ].toInt() - 1,
                                       sub_args[ 0 ].toInt() ) );
                }

              BG::Backgammon copy;
              copy.copy_without_turn_list( *my_game );
              unsigned int prev_turn_number = copy.get_turn_number();
              if( !copy.move( moves )
                  || (   copy.get_turn_number() == prev_turn_number
                      && copy.get_game_status() != BG::GAME_FINISHED ) )
                {
                  error( command, "Invalid turn." );
                  return;
                }

              my_game->move( moves );

              QString turn = "INFO:TURN";
              for( unsigned short int i = 0; i < moves.size(); i++ )
                {
                  turn += ";" + QString::number( moves[ i ].get_distance() )
                          + " ";
                  if( moves[ i ].get_from() == BG::BAR )
                    turn += "27 " + QString::number(
                                      moves[ i ].get_distance() );
                  else if( act_player == BG::WHITE )
                    turn += QString::number( 1 + moves[ i ].get_from() )
                            + " " + QString::number(
                                      1 + moves[ i ].get_from()
                                      + moves[ i ].get_distance() );
                  else
                    turn += QString::number( 24 - moves[ i ].get_from() )
                            + " " + QString::number(
                                      24 - moves[ i ].get_from()
                                      + moves[ i ].get_distance() );
                }
              turn += "\r\n";
              if( act_player == BG::WHITE )
                my_game->get_player( BG::BLACK )->write( turn );
              else
                my_game->get_player( BG::WHITE )->write( turn );

              my_game->transmit_board();

              if( my_game->get_game_status() == BG::GAME_FINISHED )
                {
                  my_game->dec_rounds();
                  QString win_msg = "ENDGAME:WIN;"
                                    + QString::number(
                                        my_game->get_n_rounds() ) + ";"
                                    + QString::number(
                                        my_game->get_win_height() )
                                    + " 0\r\n";
                  QString lose_msg = "ENDGAME:LOSE;"
                                     + QString::number(
                                         my_game->get_n_rounds() ) + ";0 "
                                     + QString::number(
                                         my_game->get_win_height() )
                                     + "\r\n";

                  if( my_game->get_winner() == BG::WHITE )
                    {
                      my_game->get_player( BG::WHITE )->write( win_msg );
                      my_game->get_player( BG::BLACK )->write( lose_msg );
                    }
                  else
                    {
                      my_game->get_player( BG::BLACK )->write( win_msg );
                      my_game->get_player( BG::WHITE )->write( lose_msg );
                    }

                  emit endgame( my_game->get_game_number(),
                                my_game->get_n_rounds(),
                                my_game->get_win_height() );
                  my_game->get_player( BG::WHITE )->set_am_i_ready( false );
                  my_game->get_player( BG::BLACK )->set_am_i_ready( false );
                  my_game->get_player( BG::WHITE )->set_confirmed( false );
                  my_game->get_player( BG::BLACK )->set_confirmed( false );

                  if( my_game->get_n_rounds() <= 0 )
                    {
                      if( my_game )
                        emit leave_game_request(
                               my_game->get_player( BG::WHITE ), false );
                      if( my_game )
                        emit leave_game_request(
                               my_game->get_player( BG::BLACK ), false );
                    }
                }
            }
          else
            error( command, "Unknown command." );
        }
      else
        error( command, "Please login first." );
    }
}
