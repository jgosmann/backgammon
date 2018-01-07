/*!
 * \file bggame.cpp
 * \brief Contains the implementation of the \ref BGGame class.
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

#include "bggame.h"

#include "backgammon.h"

/////////////////////////////////////////////////////////////////////////////
/// Constructor, \a game_number is the game_number for referencing the game,
/// \a password is an optional password for the game, and \a n_rounds is
/// the number of rounds to play.
/////////////////////////////////////////////////////////////////////////////
BGGame::BGGame( int game_number, QString password, unsigned int n_rounds )
  : my_game_number( game_number ), my_password( password ),
    my_n_rounds( n_rounds ), i_started( false )
{
  my_players[ BG::WHITE ] = NULL;
  my_players[ BG::BLACK ] = NULL;
}

/////////////////////////////////////////////////////////////////////////////
/// Transmits the board to the clients.
/////////////////////////////////////////////////////////////////////////////
void BGGame::transmit_board( void )
{
  transmit_board_to_player( BG::WHITE );
  transmit_board_to_player( BG::BLACK );
}

/////////////////////////////////////////////////////////////////////////////
/// Transmits the board to the player \a which.
/////////////////////////////////////////////////////////////////////////////
void BGGame::transmit_board_to_player( BG::Player which )
{
  QString buffer = "BOARD:";

  lock_arrays();

  if( which == BG::WHITE )
    {
      buffer += QString::number( get_beared_off()[ BG::WHITE ] ) + " ";
      for( short int i = 23; i >= 0; i-- )
        buffer += QString::number( get_points()[ i ] ) + " ";
      buffer += QString::number( get_on_bar()[ BG::WHITE ] ) + " "
                + QString::number( -1 * get_beared_off()[ BG::BLACK ] ) + " "
                + QString::number( -1 * get_on_bar()[ BG::BLACK ] );
    }
  else
    {
      buffer += QString::number( get_beared_off()[ BG::BLACK ] ) + " ";
      for( short int i = 0; i <= 23; i++ )
        buffer += QString::number( -1 * get_points()[ i ] ) + " ";
      buffer += QString::number( get_on_bar()[ BG::BLACK ] ) + " "
                + QString::number( -1 * get_beared_off()[ BG::WHITE ] ) + " "
                + QString::number( -1 * get_on_bar()[ BG::WHITE ] );
    }

  unlock_arrays();

  buffer += "\r\n";
  my_players[ which ]->write( buffer );
}

/////////////////////////////////////////////////////////////////////////////
/// Transmits the dice to the clients.
/////////////////////////////////////////////////////////////////////////////
void BGGame::transmit_dice( void )
{
  QString dice;
  for( unsigned short int i = 0; i < 4; i++ )
    {
      if( i > 0 )
        dice += " ";
      dice += QString::number( get_dice()[ i ] );
    }

  if( get_act_player() == BG::WHITE )
    {
      my_players[ BG::WHITE ]->write( "DICE:" + dice + "\r\n" );
      my_players[ BG::BLACK ]->write( "INFO:DICE;" + dice + "\r\n" );
    }
  else
    {
      my_players[ BG::BLACK ]->write( "DICE:" + dice + "\r\n" );
      my_players[ BG::WHITE ]->write( "INFO:DICE;" + dice + "\r\n" );
    }
}
