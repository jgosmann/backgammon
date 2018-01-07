/*!
 * \file bggame.h
 * \brief Contains the declaration of the \ref BGGame class.
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

#ifndef BGGAME_H
#define BGGAME_H 1

#include <QString>

#include "backgammon.h"

class BGGame;

#include "bgconnection.h"

/////////////////////////////////////////////////////////////////////////////
/// This class manages one single game at the server
/////////////////////////////////////////////////////////////////////////////
class BGGame : public BG::Backgammon
{
  Q_OBJECT

  public:
    BGGame( int game_number, QString password = "",
            unsigned int n_rounds = 0 );

    void transmit_board( void );
    void transmit_board_to_player( BG::Player which );
    void transmit_dice( void );

    inline int get_game_number( void ) const { return my_game_number; };
    inline QString get_password( void ) const { return my_password; };
    inline unsigned int get_n_rounds( void ) const { return my_n_rounds; };
    inline BGConnection * get_player( BG::Player which )
      { return my_players[ which ]; };
    inline bool started( void ) const { return i_started; };

    inline void dec_rounds( void ) { my_n_rounds--; };
    inline void set_player( BG::Player which, BGConnection *player )
      { my_players[ which ] = player; };
    inline void set_started( bool value ) { i_started = value; };

  private:
    int my_game_number;
    QString my_password;
    unsigned int my_n_rounds;
    BGConnection *my_players[ 2 ];
    bool i_started;
};

#endif // BGGAME_H
