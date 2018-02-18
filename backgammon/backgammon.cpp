/*!
 * \file backgammon.cpp
 * \brief Implementation der Backgammon-Klasse, welche Funktionen zur
 *        Verwaltung eines Backgammon-Spieles bereitstellt.
 * \date Mi Nov 22 2006
 * \author Jan Gosmann (jan@hyper-world.de)
 *
 * \b Copyright: Copyright (C) 2006 - 2007 Jan Gosmann
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

#include <cassert>
#include <cstdlib>

#include <vector>

#include <QMutex>
#include <QMutexLocker>
#include <QObject>

#include "backgammon.h"

using namespace BG;

//===========================================================================
// BackgammonMove
//===========================================================================
/////////////////////////////////////////////////////////////////////////////
/// Konstruktor. \a from gibt den Zacken an, von dem der Spielstein gezogen
/// wird, \a distance gibt an, um wie viele Felder der Spielstein gezogen
/// wird. Für \a distance kann in vielen Fällen auch die zusammengerechnete
/// Augenzahl von beiden Würfeln verwendet werden.
/////////////////////////////////////////////////////////////////////////////
BackgammonMove::BackgammonMove( short int from, short int distance ) :
  m_from( from ), m_distance( distance )
{
  assert( (from >= 0 && from <= 23) || from == BAR );
}

//===========================================================================
// BackgammonTurn
//===========================================================================
/////////////////////////////////////////////////////////////////////////////
/// Konstruktor, der \ref m_player mit \a player, \ref m_dice[0] mit \a d1
/// und \ref m_dice[1] mit \a d2 initialisiert.
/////////////////////////////////////////////////////////////////////////////
BackgammonTurn::BackgammonTurn( Player player, short int d1, short int d2 ) :
  m_player( player )
{
  m_dice[ 0 ] = d1;
  m_dice[ 1 ] = d2;
}

/////////////////////////////////////////////////////////////////////////////
/// Fügt den Zug \a move zu den in der Runde ausgeführten Zügen hinzu. Dabei
/// gibt \a hit_checker an, ob mit \a move ein gegnerischer Spielstein
/// geschlagen wurde.
/////////////////////////////////////////////////////////////////////////////
void BackgammonTurn::add_move( BackgammonMove move, bool hit_checker )
{
  m_move_list.push_back( move );
  m_hit_checker.push_back( hit_checker );
}

//===========================================================================
// Backgammon
//===========================================================================
/////////////////////////////////////////////////////////////////////////////
/// Konstruktor. Wenn \a is_auto_dice_roll_enabled = true, dann würfelt die
/// Klasse automatisch, wenn der ziehende Spieler wechselt. Das
/// Würfelergebnis kann mit get_dice() abgerufen werden. Ansonsten muss das
/// Würfelergebnis manuell mit set_dice() gesetzt werden.
/////////////////////////////////////////////////////////////////////////////
Backgammon::Backgammon( bool is_auto_dice_roll_enabled, QObject *parent ) :
  QObject( parent ), m_var_mutex( QMutex::Recursive ), m_array_mutex(
      QMutex::Recursive ), m_is_auto_dice_roll_enabled(
      is_auto_dice_roll_enabled )
{
  reset();
}

/*< \label{bool:BG::Backgammon::is_valid_move(const:BackgammonMove&,IllegalMove*,bool*,bool)const} >*/
/////////////////////////////////////////////////////////////////////////////
/// Prüft, ob der in \a move übergebene Zug gültig ist. Wenn \a reason nicht
/// NULL ist, wird in der übergebenen Variable gespeichert, wieso der Zug
/// nicht möglich ist, falls dies der Fall ist. Wenn eine Variable für
/// \a dice_used (welche ein Array mit vier Elementen sein muss) übergeben
/// wird, wird in diesem Array gespeichert, welche der geworfenen Augenzahlen
/// für diesen Zug verwendet werden müssten. Siehe dazu auch \ref m_dice.
/// \a check_bar gibt an, ob geprüft werden soll, ob noch Spielsteine auf der
/// Bar sind, die erst wieder zurück ins Spiel gebracht werden müssen.
/// \retval true  Der Zug ist gültig.
/// \retval false Der Zug ist ungültig.
/// \sa are_valid_moves(), is_able_to_move(), move()
/////////////////////////////////////////////////////////////////////////////
bool Backgammon::is_valid_move( const BackgammonMove &move,
    IllegalMove *reason, bool *dice_used, bool check_bar ) const
{
  QMutexLocker var_lock( &m_var_mutex ), array_lock( &m_array_mutex );

  const short int PLAYER_IND = (m_act_player == WHITE) ? 1 : -1;
  // Um if-Abfragen etc. übersichtlicher zu gestalten.
  short int distance_left; // Zahl der Felder, die noch gesetzt werden muss.
  const short int START_POSITION = (move.get_from() != BAR) ? move.get_from()
      : (m_act_player == WHITE) ? -1 : 24;
  // Feld, von dem gezogen wird, für Berechnungszwecke.
  const short int END_POSITION = PLAYER_IND * move.get_distance()
      + START_POSITION;
  // Feld zu dem gezogen werden soll.
  unsigned int n_dice_usable; // Gibt an für wie viele Würfel noch nicht
  // gesetzt wurde.
  short int tmp_position[ 2 ]; // Zum temporären Speichern von Positionen.
  BackgammonMove tmp_move; // Zum temporären Speichern von Zügen.
  unsigned int i;

  for( i = 0, n_dice_usable = 0; i < 4; i++ )
    {
      if( m_dice[ i ] > 0 )
        n_dice_usable++;
    }

  /* Korrekte Zugrichtung? */
  if( move.get_distance() <= 0 )
    {
      if( reason )
        *reason = WRONG_DIRECTION;
      return false;
    }

  /* Müssen noch Spielsteine zurück ins Spiel gebracht werden? */
  if( m_on_bar[ m_act_player ] > 0 && move.get_from() != BAR && check_bar )
    {
      if( reason )
        *reason = CHECKER_LEFT_ON_BAR;
      return false;
    }

  /* Befindet sich ein eigener Spielstein auf dem Feld, von dem gezogen
   * werden soll? */
  if( (move.get_from() != BAR && PLAYER_IND * m_points[ move.get_from() ] <= 0)
      || (move.get_from() == BAR && m_on_bar[ m_act_player ] <= 0) )
    {
      if( reason )
        *reason = NO_CHECKER_TO_MOVE;
      return false;
    }

  /***** Eigentliche Überprüfung auf Gültigkeit des Zuges. *****/
  /* Falls versucht wird einen Spielstein auszuwürfeln... */
  if( (m_act_player == WHITE && END_POSITION > 23) || (m_act_player == BLACK
      && END_POSITION < 0) )
    {
      if( !m_are_all_in_homeboard[ m_act_player ] )
        {
          if( reason )
            *reason = NOT_ALL_IN_HOMEBOARD;
          return false;
        }

      /* Wenn die Augenzahl nicht passend ist... (Weiß am Zug) */
      if( m_act_player == WHITE && END_POSITION > 24 )
        {
          /* Bei folgender for-Schleife ist es nicht nötig zu prüfen, ob
           * move.get_from() == BAR, da in diesem Fall END_POSITION
           * höchstens 23 sein könnte, end_position aber größer ist. */
          for( i = move.get_from() - 1; i >= 18; i-- )
            {
              if( m_points[ i ] > 0 )
                {
                  if( reason )
                    *reason = ANOTHER_CHECKER_HAS_TO_BE_MOVED;
                  return false;
                }
            }
        }
      /* Wenn die Augenzahl nicht passend ist... (Schwarz am Zug) */
      else if( m_act_player == BLACK && END_POSITION < -1 )
        {
          for( i = move.get_from() + 1; i <= 5; i++ )
            {
              if( m_points[ i ] < 0 )
                {
                  if( reason )
                    *reason = ANOTHER_CHECKER_HAS_TO_BE_MOVED;
                  return false;
                }
            }
        }
    }

  /* Wenn Zielfeld schon belegt ist... */
  if( END_POSITION >= 0 && END_POSITION <= 23 && PLAYER_IND
      * m_points[ END_POSITION ] < -1 )
    {
      if( reason )
        *reason = BLOCKED;
      return false;
    }

  /* Prüfen, ob die entsprechende Entfernung mit dem Würfelergebnis gesetzt
   * werden kann.
   */
  if( dice_used )
    {
      for( i = 0; i < 4; i++ )
        dice_used[ i ] = false;
    }

  /* Würfel 1 */
  if( m_dice[ 0 ] == move.get_distance() )
    {
      if( (m_dice[ 0 ] > m_dice[ 1 ] && !m_dice_result_has_to_be_used[ 0 ])
          || (m_dice[ 0 ] < m_dice[ 1 ] && !m_dice_result_has_to_be_used[ 1 ]) )
        {
          if( reason )
            *reason = DICE_VALUE;
          return false;
        }

      /* Mit Würfel 1 wäre der Zug möglich, kann danach aber noch für den
       * anderen Würfel gesetzt werden, wenn dies aufgrund von Zugzwang
       * nötig ist? */
      if( (m_dice[ 0 ] > m_dice[ 1 ] && m_dice_result_has_to_be_used[ 1 ])
          || (m_dice[ 0 ] < m_dice[ 1 ] && m_dice_result_has_to_be_used[ 0 ]) )
        {
          tmp_move = BackgammonMove( BAR, m_dice[ 1 ] );
          if( is_able_to_move( &tmp_move, &move ) )
            {
              if( dice_used )
                dice_used[ 0 ] = true;
              return true;
            }
          for( i = 0; i <= 23; i++ )
            {
              tmp_move = BackgammonMove( i, m_dice[ 1 ] );
              if( is_able_to_move( &tmp_move, &move ) )
                {
                  if( dice_used )
                    dice_used[ 0 ] = true;
                  return true;
                }
            }
          if( m_beared_off[ m_act_player ] < 14 )
            {
              if( reason )
                *reason = DICE_VALUE;
              return false;
            }
        }
      if( dice_used )
        dice_used[ 0 ] = true;
      return true;
    }

  /* Würfel 2 */
  if( m_dice[ 1 ] == move.get_distance() && m_dice[ 0 ] != m_dice[ 1 ] )
    {
      if( (m_dice[ 1 ] > m_dice[ 0 ] && !m_dice_result_has_to_be_used[ 0 ])
          || (m_dice[ 1 ] < m_dice[ 0 ] && !m_dice_result_has_to_be_used[ 1 ]) )
        {
          if( reason )
            *reason = DICE_VALUE;
          return false;
        }

      /* Mit Würfel 2 wäre der Zug möglich, kann danach aber noch für den
       * anderen Würfel gesetzt werden, wenn dies aufgrund von Zugzwang
       * nötig ist? */
      if( (m_dice[ 1 ] > m_dice[ 0 ] && m_dice_result_has_to_be_used[ 1 ])
          || (m_dice[ 1 ] < m_dice[ 0 ] && m_dice_result_has_to_be_used[ 0 ]) )
        {
          tmp_move = BackgammonMove( BAR, m_dice[ 0 ] );
          if( is_able_to_move( &tmp_move, &move ) )
            {
              if( dice_used )
                dice_used[ 1 ] = true;
              return true;
            }
          for( i = 0; i <= 23; i++ )
            {
              tmp_move = BackgammonMove( i, m_dice[ 0 ] );
              if( is_able_to_move( &tmp_move, &move ) )
                {
                  if( dice_used )
                    dice_used[ 1 ] = true;
                  return true;
                }
            }
          if( m_beared_off[ m_act_player ] < 14 )
            {
              if( reason )
                *reason = DICE_VALUE;
              return false;
            }
        }
      if( dice_used )
        dice_used[ 1 ] = true;
      return true;
    }

  /* Falls nach dem Zug noch Spielsteine auf der Bar sind, darf nur für eine
   * Augenzahl gesetzt werden. */
  if( m_on_bar[ m_act_player ] > 1 )
    {
      if( reason )
        *reason = DICE_VALUE;
      return false;
    }

  /* Da der Zug mit der Augenzahl eines einzelnen Würfels nicht möglich ist,
   * wird nun die Kombination beider Augenzahlen geprüft. */
  tmp_position[ 0 ] = PLAYER_IND * m_dice[ 0 ] + START_POSITION;
  if( tmp_position[ 0 ] < 0 || tmp_position[ 0 ] > 23 )
    tmp_position[ 0 ] = OUT_OF_GAME;
  tmp_position[ 1 ] = PLAYER_IND * m_dice[ 1 ] + START_POSITION;
  if( tmp_position[ 1 ] < 0 || tmp_position[ 1 ] > 23 )
    tmp_position[ 1 ] = OUT_OF_GAME;

  if( tmp_position[ 0 ] == OUT_OF_GAME && tmp_position[ 1 ] == OUT_OF_GAME )
    {
      if( reason )
        *reason = DICE_VALUE;
      return false;
    }

  if( m_dice[ 0 ] + m_dice[ 1 ] == move.get_distance() && ((tmp_position[ 0 ]
      != OUT_OF_GAME && PLAYER_IND * m_points[ tmp_position[ 0 ] ] >= -1)
      || (tmp_position[ 1 ] != OUT_OF_GAME && PLAYER_IND
          * m_points[ tmp_position[ 1 ] ] >= -1)) )
    {
      if( dice_used )
        dice_used[ 0 ] = dice_used[ 1 ] = true;
      return true;
    }

  /* Wenn auch die Kombination beider Augenzahlen den Zug nicht ermöglicht,
   * wird im folgenden geprüft, ob eine Augenzahl mehr als zweimal verwendet
   * werden muss, wie dies bei einem Pasch der Fall sein kann. */
  distance_left = move.get_distance();
  for( i = 0; i < 4; i++ )
    {
      if( m_dice[ i ] <= 0 )
        continue;

      tmp_position[ 0 ] = PLAYER_IND * m_dice[ i ] + START_POSITION
          + PLAYER_IND * (move.get_distance() - distance_left);
      if( tmp_position[ 0 ] < 0 || tmp_position[ 0 ] > 23 )
        tmp_position[ 0 ] = OUT_OF_GAME;

      if( tmp_position[ 0 ] == OUT_OF_GAME || PLAYER_IND
          * m_points[ tmp_position[ 0 ] ] >= -1 )
        {
          if( dice_used )
            dice_used[ i ] = true;
          distance_left -= m_dice[ i ];
          if( distance_left == 0 )
            return true;
          else if( tmp_position[ 0 ] == OUT_OF_GAME )
            {
              if( reason )
                *reason = DICE_VALUE;
              return false;
            }
        }
    }

  if( reason )
    *reason = DICE_VALUE;
  return false;
}

/*< \label{bool:BG::Backgammon::are_valid_moves(const:std::vector:BackgammonMove&,IllegalMove*,bool**)const} >*/
/////////////////////////////////////////////////////////////////////////////
/// Prüft, ob die in \a moves übergebenen Züge gültig sind und sich alle in
/// zusammen in dieser Runde ausführen lassen. Dabei muss move.player bei
/// jedem übergebenen Zug dem Spieler entsprechen, der gerade am Zug ist.
/// Wenn \a reason nicht NULL ist, wird in der übergebenen Variable
/// gespeichert, wieso der Zug nicht möglich ist, falls dies der Fall ist.
/// Wenn eine Variable für \a dice_used (welche ein zweidimensionales Array
/// mit n mal vier Elementen sein muss, wobei n die \a moves.size() sein
/// muss) übergeben wird, wird in diesem Array gespeichert, welche der
/// geworfenen Augenzahlen für diesen Zug verwendet werden müssen. Siehe dazu
/// auch \ref m_dice.
/// \retval true  Der Züge sind gültig und zusammen möglich.
/// \retval false Mindestens einer der Züge ist ungültig oder sie sind nicht
///               in dieser Kombination möglich.
/// \sa is_valid_move(), is_able_to_move(), move()
/////////////////////////////////////////////////////////////////////////////
bool Backgammon::are_valid_moves( const std::vector< BackgammonMove > &moves,
    IllegalMove *reason, bool **dice_used ) const
{
  Backgammon game_copy; // Kopie des Spiels, um Züge durchzuprobieren
  bool move_applied_in_cycle; // Konnte ein Zug im letzten Schleifendurchlauf
  // ausgeführt werden?
  bool *moved = new bool[ moves.size() ]; // Wurde der entsprechende Zug in
  // moves bereits ausgeführt?
  short int dice_before_move[ 4 ]; // Noch zu verwendende Würfel vor ziehen
  // des nächsten Zuges in moves.
  bool dice_used_tmp[ 4 ];
  unsigned int i, j, k, l;

  /* Variablen initialisieren */
  for( i = 0; i < moves.size(); i++ )
    {
      moved[ i ] = false;
      for( j = 0; dice_used && j < 4; j++ )
        dice_used[ i ][ j ] = false;
    }
  for( i = 0; i < 4; i++ )
    dice_used_tmp[ i ] = false;
  game_copy.copy_without_turn_list( *this );

  /* Züge durchprobieren */
  do
    {
      move_applied_in_cycle = false;
      for( i = 0; i < moves.size(); i++ )
        {
          if( moved[ i ] )
            continue;

          for( j = 0; j < 4; j++ )
            dice_before_move[ j ] = game_copy.get_dice()[ j ];

          if( game_copy.move( moves[ i ], reason, dice_used_tmp ) )
            {
              /* Falls moves[i] gesetzt werden konnte, sicherstellen, dass
               * keine Augenzahl eines Würfels mehrfach verwendet wird. */
              for( j = 0; dice_used && j < 4; j++ )
                {
                  if( !dice_used_tmp[ j ] )
                    continue;

                  m_array_mutex.lock();
                  for( k = 0; k < 4; k++ )
                    {
                      if( dice_before_move[ j ] != m_dice[ k ] )
                        continue;

                      dice_used[ i ][ k ] = true;
                      for( l = 0; l < moves.size(); l++ )
                        {
                          if( dice_used[ l ][ k ] && l != i )
                            dice_used[ i ][ k ] = false;
                        }
                      if( dice_used[ i ][ k ] )
                        break;
                    }
                  m_array_mutex.unlock();
                }

              moved[ i ] = true;
              move_applied_in_cycle = true;
            }
        }
    } while( move_applied_in_cycle );

  /* Aufräumen */
  for( i = 0; i < moves.size(); i++ )
    {
      if( !moved[ i ] )
        {
          delete[] moved;
          return false;
        }
    }

  delete[] moved;
  return true;
}

/*< \label{bool:BG::Backgammon::is_valid_move_possible(BG::BackgammonMove*)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Prüft, ob es einen gültigen Zug gibt, der ausgeführt werden kann. Wenn
/// \a move nicht NULL ist, wird in \a move der erste gefundene Zug, der
/// möglich ist gespeichert
///
/// Diese Funktion sollte, so fern sie nicht von dieser Klasse selber
/// aufgerufen wird immer true zurückgeben, da andernfalls automatisch zum
/// nächsten Spieler übergegangen werden sollte.
/// \retval true Es gibt einen gültigen Zug.
/// \retval false Es gibt keinen gültigen Zug.
/////////////////////////////////////////////////////////////////////////////
bool Backgammon::is_valid_move_possible( BackgammonMove *move )
{
  QMutexLocker var_lock( &m_var_mutex ), array_lock( &m_array_mutex );

  const short int DICE[ 2 ] = { (m_dice[ 0 ] > m_dice[ 1 ]) ? m_dice[ 0 ]
      : m_dice[ 1 ], (m_dice[ 0 ] > m_dice[ 1 ]) ? m_dice[ 1 ] : m_dice[ 0 ] };
  // Die beiden gewürfelten Zahlen, nach Größe sortiert.
  short int can_be_set_at_pos[ 2 ]; // Gibt jeweils eine mögliche Position an
  // von der DICE[0] bzw. DICE[1] gezogen
  // werden kann. Die Bar wird nicht be-
  // achtet.
  bool is_move_set = false; // Wurde bereits ein Zug in move gespeichert?
  BackgammonMove tmp_move[ 2 ]; // Temporäre Speicherung von Zügen.
  int i, j;

  m_dice_result_has_to_be_used[ 0 ] = false;
  m_dice_result_has_to_be_used[ 1 ] = false;

  /***** Bereits für alle Augenzahlen gesetzt? *****/
  if( DICE[ 0 ] <= 0 )
    return false;

  /***** Muss nur noch für genau eine Augenzahl gesetzt werden? *****/
  if( DICE[ 1 ] <= 0 )
    {
      /* Noch Steine auf der Bar? */
      if( m_on_bar[ m_act_player ] >= 1 )
        {
          if( (m_act_player == WHITE && m_points[ DICE[ 0 ] - 1 ] >= -1)
              || (m_act_player == BLACK && m_points[ 24 - DICE[ 0 ] ] <= 1) )
            {
              if( move )
                {
                  move->set_from( BAR );
                  move->set_distance( DICE[ 0 ] );
                }
              m_dice_result_has_to_be_used[ 0 ] = true;
              return true;
            }
          return false;
        }

      /* Wenn keine Steine mehr auf der Bar sind... */
      for( i = 0; i <= 23; i++ )
        {
          tmp_move[ 0 ] = BackgammonMove( i, DICE[ 0 ] );
          if( is_able_to_move( &tmp_move[ 0 ] ) )
            {
              if( move )
                {
                  move->set_from( i );
                  move->set_distance( DICE[ 0 ] );
                }
              m_dice_result_has_to_be_used[ 0 ] = true;
              return true;
            }
        }
      return false;
    }

  /***** Es muss noch für beide Würfel gesetzt werden. *****/
  /* Falls noch min. 2 Steine auf der Bar... */
  if( m_on_bar[ m_act_player ] >= 2 )
    {
      if( move )
        move->set_from( BAR );

      for( i = 0; i < 2; i++ )
        {
          if( (m_act_player == WHITE && m_points[ DICE[ i ] - 1 ] >= -1)
              || (m_act_player == BLACK && m_points[ 24 - DICE[ i ] ] <= 1) )
            {
              if( move )
                move->set_distance( DICE[ i ] );
              m_dice_result_has_to_be_used[ i ] = true;
            }
        }
      return m_dice_result_has_to_be_used[ 0 ]
          | m_dice_result_has_to_be_used[ 1 ];
    }

  /* Mögliche "Zungen" finden, von denen die Augenzahlen gesetzt werden
   * könnten. */
  can_be_set_at_pos[ 0 ] = can_be_set_at_pos[ 1 ] = -1;
  for( i = 0; i <= 23 && (can_be_set_at_pos[ 0 ] < 0 || can_be_set_at_pos[ 1 ]
      < 0); i++ )
    {
      tmp_move[ 0 ] = BackgammonMove( i, DICE[ 0 ] );

      for( j = 0; j <= 23 && (can_be_set_at_pos[ 0 ] < 0
          || can_be_set_at_pos[ 1 ] < 0); j++ )
        {
          tmp_move[ 1 ] = BackgammonMove( j, DICE[ 1 ] );

          if( is_able_to_move( &tmp_move[ 0 ] ) )
            {
              if( is_able_to_move( &tmp_move[ 1 ], &tmp_move[ 0 ] )
                  && m_on_bar[ m_act_player ] <= 0 )
                {
                  can_be_set_at_pos[ 0 ] = i;
                  can_be_set_at_pos[ 1 ] = j;
                }
              else if( can_be_set_at_pos[ 0 ] < 0 )
                {
                  can_be_set_at_pos[ 0 ] = i;
                  can_be_set_at_pos[ 1 ] = -1;
                }
            }
          else if( is_able_to_move( &tmp_move[ 1 ] ) )
            {
              if( is_able_to_move( &tmp_move[ 0 ], &tmp_move[ 1 ] )
                  && m_on_bar[ m_act_player ] <= 0 )
                {
                  can_be_set_at_pos[ 0 ] = i;
                  can_be_set_at_pos[ 1 ] = j;
                }
              else if( can_be_set_at_pos[ 0 ] < 0 && can_be_set_at_pos[ 1 ] < 0 )
                can_be_set_at_pos[ 1 ] = j;
            }
        }
    }

  /* Falls noch genau ein Stein auf der Bar ist... */
  if( m_on_bar[ m_act_player ] == 1 )
    {
      /* ...und dieser mit der größeren gewürfelten Augenzahl zurück ins
       * Spiel gebracht werden kann. */
      if( (m_act_player == WHITE && m_points[ DICE[ 0 ] - 1 ] >= -1)
          || (m_act_player == BLACK && m_points[ 24 - DICE[ 0 ] ] <= 1) )
        {
          if( move )
            {
              move->set_from( BAR );
              move->set_distance( DICE[ 0 ] );
            }
          is_move_set = true;
          m_dice_result_has_to_be_used[ 0 ] = true;

          /* Kann anschließend auch noch die kleinere gewürfelte Augenzahl
           * gesetzt werden? */
          tmp_move[ 0 ] = BackgammonMove( (m_act_player == WHITE) ? DICE[ 0 ]
              - 1 : 24 - DICE[ 0 ], DICE[ 1 ] );
          tmp_move[ 1 ] = BackgammonMove( BAR, DICE[ 0 ] );
          if( can_be_set_at_pos[ 1 ] >= 0 || is_able_to_move( &tmp_move[ 0 ],
              &tmp_move[ 1 ] ) )
            {
              m_dice_result_has_to_be_used[ 1 ] = true;
              return true;
            }
        }
      /* ...und dieser mit der kleineren gewürfelten Augenzahl zurück ins
       * Spiel gebracht werden kann. */
      if( (m_act_player == WHITE && m_points[ DICE[ 1 ] - 1 ] >= -1)
          || (m_act_player == BLACK && m_points[ 24 - DICE[ 1 ] ] <= 1) )
        {
          /* Falls anschließend noch die größere Augenzahl gesetzt werden
           * kann... */
          tmp_move[ 0 ] = BackgammonMove( (m_act_player == WHITE) ? DICE[ 1 ]
              - 1 : 24 - DICE[ 1 ], DICE[ 0 ] );
          tmp_move[ 1 ] = BackgammonMove( BAR, DICE[ 1 ] );
          if( can_be_set_at_pos[ 0 ] >= 0 || is_able_to_move( &tmp_move[ 0 ],
              &tmp_move[ 1 ] ) )
            {
              if( move )
                {
                  move->set_from( BAR );
                  move->set_distance( DICE[ 1 ] );
                }
              m_dice_result_has_to_be_used[ 0 ] = true;
              m_dice_result_has_to_be_used[ 1 ] = true;
              return true;
            }

          /* Wenn die größere Augenzahl nicht mehr gesetzt werden kann und
           * es damit auch nicht möglich ist den Spielstein zurück ins
           * Spiel zu bringen... */
          if( move && !is_move_set )
            {
              move->set_from( BAR );
              move->set_distance( DICE[ 1 ] );
            }
          if( !is_move_set )
            m_dice_result_has_to_be_used[ 1 ] = true;
          return true;
        }
      if( is_move_set )
        return true;
      return false;
    }

  /* Falls keine Steine mehr auf der Bar sind... */
  if( can_be_set_at_pos[ 0 ] >= 0 )
    {
      m_dice_result_has_to_be_used[ 0 ] = true;
      if( can_be_set_at_pos[ 1 ] >= 0 || m_beared_off[ m_act_player ] >= 14 )
        m_dice_result_has_to_be_used[ 1 ] = true;

      if( move )
        {
          move->set_from( can_be_set_at_pos[ 0 ] );
          move->set_distance( DICE[ 0 ] );
          if( !is_able_to_move( move ) )
            {
              move->set_from( can_be_set_at_pos[ 1 ] );
              move->set_distance( DICE[ 1 ] );
            }
        }
      return true;
    }
  if( can_be_set_at_pos[ 1 ] >= 0 )
    {
      m_dice_result_has_to_be_used[ 1 ] = true;

      if( move )
        {
          move->set_from( can_be_set_at_pos[ 1 ] );
          move->set_distance( DICE[ 1 ] );
        }
      return true;
    }

  return false;
}

/*< \label{bool:BG::Backgammon::move(void)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Führt keinen Zug aus, wechselt aber zum nächsten Spieler, wenn der
/// aktuelle Zugunfähig ist.
/// \retval true  Der aktuelle Spieler ist zugunfähig.
/// \retval false Der aktuelle Spieler ist nicht zugunfähig.
/////////////////////////////////////////////////////////////////////////////
bool Backgammon::move( void )
{
  if( is_valid_move_possible() || get_game_status() == GAME_FINISHED )
    return false;

  refresh();
  end_turn();
  return true;
}

/*< \label{bool:BG::Backgammon::move(const:BackgammonMove&,IllegalMove*,bool*)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Prüft, ob der in \a move übergebene Zug gültig ist und führt ihn aus
/// (wenn er gültig ist). Dabei muss \a move.m_player dem Spieler
/// entsprechen, der gerade am Zug ist. Wenn \a reason nicht NULL ist, wird
/// in der übergebenen Variable gespeichert, wieso der Zug nicht möglich ist,
/// falls dies der Fall ist. Wenn eine Variable für \a dice_used (welche ein
/// Array mit vier Elementen sein muss) übergeben wird, wird in diesem Array
/// gespeichert, welche der geworfenen Augenzahlen für diesen Zug verwendet
/// wurden. Siehe dazu auch \ref m_dice.
/// \retval true  Der Zug wurde ausgeführt.
/// \retval false Der Zug ist ungültig und wurde nicht ausgeführt.
/// \sa apply_move(), is_valid_move(), are_valid_moves()
/////////////////////////////////////////////////////////////////////////////
bool Backgammon::move( const BackgammonMove &move, IllegalMove *reason,
    bool *dice_used )
{
  bool my_dice_used[ 4 ];
  unsigned int i, j;

  if( !is_valid_move( move, reason, my_dice_used ) )
    return false;

  apply_move( move, my_dice_used );
  m_array_mutex.lock();
  for( i = 0; i < 3; i++ )
    {
      for( j = 3; m_dice[ i ] <= 0 && j > i; j-- )
        {
          if( m_dice[ j ] > 0 )
            {
              m_dice[ i ] = m_dice[ j ];
              m_dice[ j ] = 0;
              break;
            }
        }
    }
  m_array_mutex.unlock();

  refresh();
  if( !is_valid_move_possible() && get_game_status() != GAME_FINISHED )
    end_turn();

  if( dice_used )
    {
      for( i = 0; i < 4; i++ )
        dice_used[ i ] = my_dice_used[ i ];
    }
  return true;
}

/*< \label{bool:BG::Backgammon::move(const:std::vector:BackgammonMove&,IllegalMove*,bool*)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Prüft, ob die in \a moves übergebenen Züge gültig sind und führt sie aus
/// (wenn sie gültig sind). Dabei muss \a move[i].m_player dem Spieler
/// entsprechen, der gerade am Zug ist. Wenn \a reason nicht NULL ist, wird
/// in der übergebenen Variable gespeichert, wieso dier Züge nicht möglich
/// sind, falls dies der Fall ist. Wenn eine Variable für \a dice_used
/// (welche ein Array mit vier Elementen sein muss) übergeben wird, wird in
/// diesem Array gespeichert, welche der geworfenen Augenzahlen für diesen
/// Zug verwendet wurden. Siehe dazu auch \ref m_dice.
/// \retval true  Der Züge wurden ausgeführt.
/// \retval false Deie Züge sind ungültig und wurden nicht ausgeführt.
/// \sa apply_move(), is_valid_move(), are_valid_moves()
/////////////////////////////////////////////////////////////////////////////
bool Backgammon::move( const std::vector< BackgammonMove > &moves,
    IllegalMove *reason, bool *dice_used )
{
  bool **my_dice_used;
  unsigned int i, j;

  my_dice_used = new bool *[ moves.size() ];
  for( i = 0; i < moves.size(); i++ )
    my_dice_used[ i ] = new bool[ 4 ];

  if( !are_valid_moves( moves, reason, my_dice_used ) )
    {
      for( i = 0; i < moves.size(); i++ )
        delete[] my_dice_used[ i ];
      delete[] my_dice_used;
      return false;
    }

  if( dice_used )
    {
      for( i = 0; i < 4; i++ )
        {
          for( j = 0; j < moves.size(); j++ )
            dice_used[ i ] |= my_dice_used[ j ][ i ];
        }
    }

  for( i = 0; i < moves.size(); i++ )
    apply_move( moves[ i ], my_dice_used[ i ] );
  m_array_mutex.lock();
  for( i = 0; i < 3; i++ )
    {
      for( j = 3; m_dice[ i ] <= 0 && j > i; j-- )
        {
          if( m_dice[ j ] > 0 )
            {
              m_dice[ i ] = m_dice[ j ];
              m_dice[ j ] = 0;
              break;
            }
        }
    }
  m_array_mutex.unlock();

  refresh();
  if( !is_valid_move_possible() && get_game_status() != GAME_FINISHED )
    end_turn();

  for( i = 0; i < moves.size(); i++ )
    delete[] my_dice_used[ i ];
  delete[] my_dice_used;
  return true;
}

/////////////////////////////////////////////////////////////////////////////
/// Kopiert \a source in die aufrufende Klasse und gibt diese zurück. Die
/// Zugliste (\ref m_turn_list ) wird dabei ausgelassen und zurückgesetzt.
/// Auch wird \ref m_is_auto_dice_roll_enabled nicht kopiert.
/////////////////////////////////////////////////////////////////////////////
Backgammon & Backgammon::copy_without_turn_list( const Backgammon &source )
{
  QMutexLocker var_lock( &m_var_mutex ), array_lock( &m_array_mutex );
  unsigned int i;

  source.lock_arrays();

  for( i = 0; i < 24; i++ )
    m_points[ i ] = source.get_points()[ i ];

  m_beared_off[ 0 ] = source.get_beared_off()[ 0 ];
  m_beared_off[ 1 ] = source.get_beared_off()[ 1 ];

  m_on_bar[ 0 ] = source.get_on_bar()[ 0 ];
  m_on_bar[ 1 ] = source.get_on_bar()[ 1 ];

  m_are_all_in_homeboard[ 0 ] = source.get_are_all_in_homeboard()[ 0 ];
  m_are_all_in_homeboard[ 1 ] = source.get_are_all_in_homeboard()[ 1 ];

  m_act_player = source.get_act_player();

  for( i = 0; i < 4; i++ )
    m_dice[ i ] = source.get_dice()[ i ];

  m_dice_result_has_to_be_used[ 0 ]
      = source.get_dice_result_has_to_be_used()[ 0 ];
  m_dice_result_has_to_be_used[ 1 ]
      = source.get_dice_result_has_to_be_used()[ 1 ];

  m_turn_number = source.get_turn_number();
  m_game_status = source.get_game_status();

  m_turn_list.clear();
  m_turn_list.push_back(
      BackgammonTurn( m_act_player, m_dice[ 0 ], m_dice[ 1 ] ) );

  source.unlock_arrays();

  var_lock.unlock();
  array_lock.unlock();
  emit act_player_changed( m_act_player );
  emit dice_changed( m_dice[ 0 ], m_dice[ 1 ] );
  emit game_status_changed( m_game_status );
  emit turn_list_changed();
  var_lock.relock();
  array_lock.relock();

  return *this;
}

/////////////////////////////////////////////////////////////////////////////
/// Nimmt den letzten Zug im Backgammon-Spiel zurück.
/////////////////////////////////////////////////////////////////////////////
void Backgammon::undo_last_turn( void )
{
  QMutexLocker var_lock( &m_var_mutex ), array_lock( &m_array_mutex );
  short int PLAYER_IND = (m_act_player == WHITE) ? 1 : -1;

  if( m_turn_list.size() <= 0 || get_game_status() == GAME_FINISHED
      || ( m_turn_list.size() == 1
           && ( m_turn_list.front().get_move_list().size() <= 0
                || m_turn_list.front().get_dice()[ 0 ] == 0
                || m_turn_list.front().get_dice()[ 1 ] == 0 ) ) )
    return;

  BackgammonTurn last_turn = m_turn_list.back();

  /* Spieler am Zug */
  if( last_turn.get_move_list().size() <= 0 )
    {
      m_turn_list.erase( --m_turn_list.end() );
      emit turn_list_changed();

      m_act_player = m_turn_list.back().get_player();
      PLAYER_IND = (m_act_player == WHITE) ? 1 : -1;
      emit act_player_changed( m_act_player );

      m_turn_number--;
      emit turn_number_changed( m_turn_number );

      last_turn = m_turn_list.back();
    }

  std::vector< BackgammonMove > move_list = last_turn.get_move_list();
  std::vector< BackgammonMove >::reverse_iterator iter = move_list.rbegin();
  std::vector< bool > hit_list = last_turn.get_hit_checker();
  std::vector< bool >::reverse_iterator hit_iter = hit_list.rbegin();
  for( ; iter != move_list.rend()
          && hit_iter != hit_list.rend();
      iter++, hit_iter++ )
    {
      /* Positionen der Spielsteine */
      assert( iter->get_from() != OUT_OF_GAME );
      if( iter->get_from() == BAR )
        m_on_bar[ m_act_player ]++;
      else
        m_points[ iter->get_from() ] += PLAYER_IND;

      short int move_to = iter->get_from() + PLAYER_IND * iter->get_distance();
      if( move_to < 0 || move_to > 23 )
        m_beared_off[ m_act_player ]--;
      else
        m_points[ move_to ] -= PLAYER_IND;

      /* Geschlagene Spielsteine */
      if( *hit_iter )
        {
          assert( move_to >= 0 && move_to <= 23 );
          assert( m_points[ move_to ] == 0 );
          m_points[ move_to ] = -1 * PLAYER_IND;
          m_on_bar[ (m_act_player == WHITE) ? BLACK : WHITE ]--;
        }
    }

  /* Würfelwurf */
  if( last_turn.get_dice()[ 0 ] == last_turn.get_dice()[ 1 ] )
    m_dice[ 0 ] = m_dice[ 1 ] = m_dice[ 2 ] = m_dice[ 3 ]
        = last_turn.get_dice()[ 0 ];
  else
    {
      m_dice[ 0 ] = last_turn.get_dice()[ 0 ];
      m_dice[ 1 ] = last_turn.get_dice()[ 1 ];
      m_dice[ 2 ] = m_dice[ 3 ] = 0;
    }
  emit dice_changed( m_dice[ 0 ], m_dice[ 1 ] );

  /* Zugliste verkürzen */
  m_turn_list.erase( --m_turn_list.end() );

  /* m_game_status setzen */
  if( m_turn_list.size() <= 0 )
    {
      m_game_status = FIRST_ROUND;
      emit game_status_changed( get_game_status() );
    }

  m_turn_list.push_back( BackgammonTurn( m_act_player, m_dice[ 0 ],
                                                       m_dice[ 1 ] ) );
  emit turn_list_changed();

  is_valid_move_possible( NULL ); // Sicherstellen, dass
                                  // m_dice_result_has_to_be_used aktualisiert
                                  // wird.

}

/*< \label{void:BG::Backgammon::reset(void)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Startet ein neues Backgammon-Spiel, indem alle Werte auf die Startwerte
/// zurückgesetzt werden.
///
/// \attention Dieser Slot darf nicht mit den Signalen turn_number_changed(),
/// game_status_changed() oder turn_list_changed() verbunden werden.
/////////////////////////////////////////////////////////////////////////////
void Backgammon::reset( void )
{
  QMutexLocker var_lock( &m_var_mutex ), array_lock( &m_array_mutex );
  unsigned int i;

  /* Spielfeld initialisieren */
  for( i = 0; i < 24; i++ )
    m_points[ i ] = 0;

  m_points[ 0 ] = 2;
  m_points[ 11 ] = 5;
  m_points[ 16 ] = 3;
  m_points[ 18 ] = 5;

  m_points[ 23 ] = -2;
  m_points[ 12 ] = -5;
  m_points[ 7 ] = -3;
  m_points[ 5 ] = -5;

  m_beared_off[ WHITE ] = m_beared_off[ BLACK ] = 0;
  m_on_bar[ WHITE ] = m_on_bar[ BLACK ] = 0;
  m_are_all_in_homeboard[ WHITE ] = m_are_all_in_homeboard[ BLACK ] = false;

  /* Würfel */
  for( i = 0; i < 4; i++ )
    m_dice[ i ] = 0;

  m_act_player = WHITE; // Damit m_act_player in jedem Fall einen gültigen
  // Wert hat.
  if( m_is_auto_dice_roll_enabled )
    {
      while( m_dice[ 0 ] == m_dice[ 1 ] )
        {
          m_dice[ 0 ] = (rand() % 6) + 1;
          m_dice[ 1 ] = (rand() % 6) + 1;
        }

      /* Startspieler */
      if( m_dice[ 0 ] > m_dice[ 1 ] )
        m_act_player = WHITE;
      else
        m_act_player = BLACK;
    }

  m_dice_result_has_to_be_used[ 0 ] = true;
  m_dice_result_has_to_be_used[ 1 ] = true;

  /* Spiel-Status etc. */
  m_turn_number = 1;
  m_game_status = FIRST_ROUND;
  m_winner = NOT_DEFINED;
  m_win_height = NOWIN;
  m_turn_list.clear();
  m_turn_list.push_back(
      BackgammonTurn( m_act_player, m_dice[ 0 ], m_dice[ 1 ] ) );

  var_lock.unlock();
  array_lock.unlock();
  emit turn_number_changed( m_turn_number );
  emit game_status_changed( m_game_status );
  emit turn_list_changed();
  var_lock.relock();
  array_lock.relock();
}

/////////////////////////////////////////////////////////////////////////////
/// Setzt \ref m_dice. \a d1 gibt die Augenzahl des ersten, \a d2 die des
/// zweiten Würfels an. Ist \a d1 = \a d2, dann wird das gesamte Array
/// \ref m_dice auf den gleichen Wert gesetzt, ansonsten werden
/// \ref m_dice[2] und \ref m_dice[3] gleich 0 gesetzt. Ist der Spieler nach
/// setzten der Augenzahlen zugunfähig, wird direkt zum nächsten Spieler
/// übergegangen.
/////////////////////////////////////////////////////////////////////////////
void Backgammon::set_dice( short int d1, short int d2 )
{
  QMutexLocker var_lock( &m_var_mutex ), array_lock( &m_array_mutex );
  const Player old_player = m_act_player;

  assert( d1 >= 1 && d1 <= 6 && d2 >= 1 && d2 <= 6 );
  if( d1 < 1 || d1 > 6 || d2 < 1 || d2 > 6 || (m_dice[ 0 ] == d1 && m_dice[ 1 ]
      == d2) )
    return;

  m_dice[ 0 ] = d1;
  m_dice[ 1 ] = d2;
  if( d1 == d2 )
    m_dice[ 2 ] = m_dice[ 3 ] = d1;
  else
    m_dice[ 2 ] = m_dice[ 3 ] = 0;

  if( m_game_status == FIRST_ROUND )
    {
      if( m_dice[ 0 ] > m_dice[ 1 ] )
        m_act_player = WHITE;
      else if( m_dice[ 0 ] < m_dice[ 1 ] )
        m_act_player = BLACK;
      if( m_act_player != old_player )
        {
          var_lock.unlock();
          array_lock.unlock();
          emit act_player_changed( m_act_player );
          var_lock.relock();
          array_lock.relock();
        }
    }

  m_turn_list.back().set_player( m_act_player );
  m_turn_list.back().set_dice( d1, d2 );

  var_lock.unlock();
  array_lock.unlock();

  emit turn_list_changed();
  refresh();
  emit dice_changed( d1, d2 );

  var_lock.relock();
  array_lock.relock();
}

/////////////////////////////////////////////////////////////////////////////
/// Setzt \ref m_is_auto_dice_roll_enabled auf \a value.
/////////////////////////////////////////////////////////////////////////////
void Backgammon::set_is_auto_dice_roll_enabled( bool value )
{
  QMutexLocker var_lock( &m_var_mutex );

  if( value == m_is_auto_dice_roll_enabled )
    return;

  m_is_auto_dice_roll_enabled = value;
  var_lock.unlock();
  emit is_auto_dice_roll_enabled_changed( value );
  var_lock.relock();
}

/////////////////////////////////////////////////////////////////////////////
/// Fügt \a move als letzten Zug in \ref m_turn_list ein. Es wird davon
/// ausgegangen, dass der aktuelle Spieler diesen Zug ausführt.
/////////////////////////////////////////////////////////////////////////////
void Backgammon::add_move_to_turn_list( BackgammonMove move )
{
  QMutexLocker var_lock( &m_var_mutex );
  const short int PLAYER_IND = (m_act_player == WHITE) ? 1 : -1;
  // Ermöglicht es die Zahl der if-Abfragen zu reduzieren.
  const short int END_POSITION = move.get_from() + PLAYER_IND
      * move.get_distance();
  // Endposition des Zuges.
  bool hit_checker = false; // Wird ein gegnerischer Spielstein geschlagen?
  unsigned int i;

  /* Prüfen, ob ein Spielstein geschlagen wird... */
  if( END_POSITION >= 0 && END_POSITION <= 23 && m_points[ END_POSITION ]
      * PLAYER_IND == -1 )
    {
      hit_checker = true;
      /* Sicherstellen, dass der Spielstein nicht bereits geschlagen worden
       * wäre. */
      for( i = 0; i < m_turn_list.back().get_move_list().size(); i++ )
        {
          if( m_turn_list.back().get_move_list()[ i ].get_from()
              + m_turn_list.back().get_move_list()[ i ].get_distance()
                  * PLAYER_IND == END_POSITION )
            hit_checker = false;
        }
    }

  m_turn_list.back().add_move( move, hit_checker );
  var_lock.unlock();
  emit turn_list_changed();
  var_lock.relock();
}

/*< \label{void:BG::Backgammon::apply_move(BackgammonMove,bool[4])} >*/
/////////////////////////////////////////////////////////////////////////////
/// Führt \a move aus und aktualisiert auch \ref m_turn_list. Dabei wird
/// \a move gegebenenfalls in mehrere einzelne Züge aufgeteilt, falls mehrere
/// Würfel für den Zug verwendet wurden. Welche Würfel verwendet wurden, muss
/// in \a dice_used übergeben werden. Es wird nicht geprüft, ob es sich um
/// einen gültigen Zug handelt.
/// \sa move(), is_valid_move(), are_valid_moves()
/////////////////////////////////////////////////////////////////////////////
void Backgammon::apply_move( BackgammonMove move, bool dice_used[ 4 ] )
{
  QMutexLocker var_lock( &m_var_mutex ), array_lock( &m_array_mutex );
  short unsigned int number_of_dice_used = 0; // Zahl der benutzten Würfel.
  const short int PLAYER_IND = (m_act_player == WHITE) ? 1 : -1;
  // Um die Zahl der if-Abragen zu reduzieren.
  short int start_position; // Feld von dem gezogen wird, für
  // Berechnungszwecke.
  short int end_position; // Feld zu dem gezogen werden soll.
  bool hit_checker; // Spielstein geschlagen?
  unsigned int i;

  assert( move.get_distance()> 0 );

  for( i = 0; i < 4; i++ )
    {
      if( dice_used[ i ] )
        number_of_dice_used++;
    }

  while( number_of_dice_used > 0 && m_beared_off[ m_act_player ] < 15 )
    {
      for( i = 0; i < 4 && m_beared_off[ m_act_player ] < 15; i++ )
        {
          if( m_dice[ i ] <= 0 || !dice_used[ i ] )
            continue;

          /* Start- und Endpostion bestimmen. */
          start_position = (move.get_from() != BAR) ? move.get_from()
              : (m_act_player == WHITE) ? -1 : 24;
          end_position = PLAYER_IND * m_dice[ i ] + start_position;
          hit_checker = false;
          if( end_position < 0 || end_position > 23 )
            end_position = OUT_OF_GAME;

          /* Kann zu dem Zielfeld gezogen werden? */
          if( end_position == OUT_OF_GAME || PLAYER_IND
              * m_points[ end_position ] >= -1 )
            {
              /* Wert des Startfeldes entsprechend ändern. */
              if( move.get_from() != BAR )
                m_points[ move.get_from() ] -= PLAYER_IND;
              else
                {
                  --m_on_bar[ m_act_player ];
                }

              /* Wert des Zielfeldes entsprechedn ändern. */
              if( end_position != OUT_OF_GAME )
                {
                  if( PLAYER_IND * m_points[ end_position ] == -1 )
                    {
                      ++m_on_bar[ (m_act_player == WHITE) ? BLACK : WHITE ];
                      m_are_all_in_homeboard[ (m_act_player == WHITE) ? BLACK
                          : WHITE ] = false;
                      m_points[ end_position ] = 0;
                      hit_checker = true;
                    }
                  m_points[ end_position ] += PLAYER_IND;
                }
              else
                ++m_beared_off[ m_act_player ];

              /* Zugliste aktualisieren. */
              assert( m_turn_list.size()> 0 );
              m_turn_list.back().add_move( BackgammonMove( move.get_from(),
                  m_dice[ i ] ), hit_checker );

              /* Bereits zurückgelegte und noch zurückzulegenede Strecke
               * in move setzen. */
              move.set_from( PLAYER_IND * m_dice[ i ] + start_position );
              move.set_distance( move.get_distance() - m_dice[ i ] );

              m_dice[ i ] = 0;
              number_of_dice_used--;
            }
        }
    }

  var_lock.unlock();
  array_lock.unlock();
  emit turn_list_changed();
  var_lock.relock();
  array_lock.relock();
}

/*< \label{bool:BG::Backgammon::is_able_to_move(const:BackgammonMove*,const:BackgammonMove*)const} >*/
/////////////////////////////////////////////////////////////////////////////
/// Prüft, ob \a move gesetzt werden kann und gibt entsprechend true oder
/// false zurück. Wird mit \a before ein weiterer Zug übergeben, wird dies
/// für den Fall geprüft, dass vorher der in \a before übergebene Zug gesetzt
/// wurde. Es wird vorrausgesetzt, dass \a before kein regelwidriger Zug ist.
///
/// Diese Funktion achtet nicht auf das Würfelergebnis, sondern nur auf
/// den Spieler, ob die "Zungen" frei sind, ob evtl. andere Spielsteine
/// gesetzt werden müssen (beim Auswürfeln, aber nicht ob von der Bar gezogen
/// werden muss) und ob überhaupt ein eigener Spielstein zum Setzen vorhanden
/// ist.
///
/// \attention Diese Funktion sperrt keine Mutexes.
/////////////////////////////////////////////////////////////////////////////
bool Backgammon::is_able_to_move( const BackgammonMove *move,
    const BackgammonMove *before ) const
{
  const short int PLAYER_IND = (m_act_player == WHITE) ? 1 : -1;
  // Um if-Abfragen etc. zu erleichtern.
  short int end_position; // Endposition des in move übergebenen Zuges.
  short int end_position_before; // Endposition des in before übergebenen
  // Zuges.
  bool are_all_in_homeboard = m_are_all_in_homeboard[ m_act_player ];
  // Durch before kann sich dies ändern, daher extra Variable nötig.
  unsigned short int checkers_in_homeboard; // Anzahl der Spielsteine im
  // Homeboard und der
  // ausgewürfleten.
  int i;

  assert( move->get_from() == BAR
      || (move->get_from() >= 0 && move->get_from() <= 23) );

  /* Falls von der Bar gezogen wird... */
  if( move->get_from() == BAR )
    {
      if( m_act_player == WHITE && m_points[ move->get_distance() - 1 ] >= -1 )
        return true;
      if( m_act_player == BLACK && m_points[ 24 - move->get_distance() ] <= 1 )
        return true;
      return false;
    }

  /* Endpositionen bestimmen */
  end_position = move->get_from() + PLAYER_IND * move->get_distance();
  if( before )
    {
      if( before->get_from() == BAR )
        end_position_before = (m_act_player == WHITE) ? -1 : 24;
      else
        end_position_before = before->get_from();
      end_position_before += PLAYER_IND * before->get_distance();
    }

  /* Kein Stein vorhanden, der gezogen werden kann? */
  if( (PLAYER_IND * m_points[ move->get_from() ] <= 0 || (before
      && move->get_from() == before->get_from() && PLAYER_IND
      * m_points[ move->get_from() ] <= 1)) && (!before || end_position_before
      != move->get_from()) )
    return false;

  /* Nach setzen von before alle im Homeboard? */
  if( before && !are_all_in_homeboard )
    {
      checkers_in_homeboard = m_beared_off[ m_act_player ];
      for( i = (m_act_player == WHITE) ? 18 : 5; i >= 0 && i <= 23; i
          += PLAYER_IND )
        {
          if( PLAYER_IND * m_points[ i ] > 0 )
            checkers_in_homeboard += PLAYER_IND * m_points[ i ];
        }
      if( (m_act_player == WHITE && end_position_before >= 18
          && before->get_from() < 18) || (m_act_player == BLACK
          && end_position_before <= 5 && before->get_from() > 5) )
        checkers_in_homeboard++;
      if( checkers_in_homeboard >= 15 )
        are_all_in_homeboard = true;
    }

  /* Zug zu Zielfeld möglich? */
  if( end_position >= 0 && end_position <= 23 )
    {
      if( PLAYER_IND * m_points[ end_position ] >= -1 )
        return true;
    }
  else if( are_all_in_homeboard )
    {
      if( end_position == -1 || end_position == 24 )
        return true;

      if( before && PLAYER_IND * end_position_before < PLAYER_IND
          * move->get_from() )
        return false;

      for( i = move->get_from() - PLAYER_IND; i <= 5 || i >= 18; i
          -= PLAYER_IND )
        {
          if( !(PLAYER_IND * m_points[ i ] <= 0 || (before && PLAYER_IND
              * m_points[ i ] <= 1 && before->get_from() == i)) )
            return false;
        }

      return true;
    }

  return false;
}

/*< \label{WinHeight:BG::Backgammon::calc_win_height(void)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Berechnet die Gewinnhöhe und gibt diese zurück. Sollte das Spiel noch
/// nicht beendet sein, wird \ref NOWIN zurückgegeben.
/////////////////////////////////////////////////////////////////////////////
WinHeight Backgammon::calc_win_height( void )
{
  QMutexLocker var_lock( &m_var_mutex ), array_lock( &m_array_mutex );
  short int player_ind; // Um if-Abfragen etc. übersichtlicher zu gestalten.
  Player opponent; // Gegnerischer Spieler
  int i;

  if( m_game_status != GAME_FINISHED )
    return NOWIN;

  if( m_beared_off[ WHITE ] >= 15 )
    opponent = BLACK;
  else
    opponent = WHITE;
  player_ind = (opponent == BLACK) ? 1 : -1;

  if( m_beared_off[ opponent ] <= 0 )
    {
      if( m_on_bar[ opponent ] > 0 )
        return BACKGAMMON;
      for( i = ((opponent == WHITE) ? 5 : 18); i >= 0 && i <= 23; i
          += player_ind )
        {
          if( m_points[ i ] * player_ind < 0 )
            return BACKGAMMON;
        }
      return GAMMON;
    }
  return NORMAL;
}

/////////////////////////////////////////////////////////////////////////////
/// Setzt alle Elemente von \ref m_dice auf 0. Es wird in keinem Fall das
/// Signal \ref dice_changed() gesendet.
/////////////////////////////////////////////////////////////////////////////
void Backgammon::clear_dice_silent()
{
  unsigned short int i;

  for( i = 0; i < 4; i++ )
    m_dice[ i ] = 0;
}

/*< \label{void:BG::Backgammon::end_turn(void)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Beendet den aktuellen Zug und der nächste Spieler kommt an die Reihe.
/////////////////////////////////////////////////////////////////////////////
void Backgammon::end_turn( void )
{
  if( get_game_status() == GAME_FINISHED )
    return;

  /* Gegebenenfalls automatisch würfeln... */
  m_var_mutex.lock();
  if( m_is_auto_dice_roll_enabled )
    {
      m_array_mutex.lock();
      m_dice[ 0 ] = (rand() % 6) + 1;
      m_dice[ 1 ] = (rand() % 6) + 1;
      if( m_dice[ 0 ] == m_dice[ 1 ] )
        m_dice[ 2 ] = m_dice[ 3 ] = m_dice[ 0 ];
      else
        m_dice[ 2 ] = m_dice[ 3 ] = 0;
      m_array_mutex.unlock();
      m_var_mutex.unlock();
      emit dice_changed( m_dice[ 0 ], m_dice[ 1 ] );
      m_var_mutex.lock();
    }
  m_var_mutex.unlock();

  /* m_game_status setzen */
  if( get_game_status() == FIRST_ROUND )
    {
      m_var_mutex.lock();
      m_game_status = GAME_IS_RUNNING;
      m_var_mutex.unlock();
      emit game_status_changed( get_game_status() );
    }

  /* Zugliste und Zugnummer */
  m_var_mutex.lock();
  m_act_player = (m_act_player == WHITE) ? BLACK : WHITE;
  m_turn_number++;
  m_var_mutex.unlock();
  emit turn_number_changed( get_turn_number() );

  m_var_mutex.lock();
  if( m_is_auto_dice_roll_enabled )
    m_turn_list.push_back( BackgammonTurn( m_act_player, m_dice[ 0 ],
        m_dice[ 1 ] ) );
  else
    m_turn_list.push_back( BackgammonTurn( m_act_player, 0, 0 ) );
  m_var_mutex.unlock();
  emit turn_list_changed();

  emit act_player_changed( m_act_player );

  is_valid_move_possible( NULL ); // Sicherstellen, dass
  // m_dice_result_has_to_be_used
  // aktualisiert wird.
  emit next_player( m_act_player );
}

/*< \label{void:BG::Backgammon::refresh(void)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Aktualisiert alle Status-Variablen der Instanz der Klasse.
/////////////////////////////////////////////////////////////////////////////
void Backgammon::refresh( void )
{
  short int lowest_pos_white = 23, highest_pos_black = 0;
  // Niedrigste bzw. höchste Position an der ein weißer bzw. schwarzer
  // Spielstein ist.
  int i;

  m_array_mutex.lock();

  /* Sind alle Steine eines Spieles im Homeboard? */
  m_are_all_in_homeboard[ WHITE ] = (m_on_bar[ WHITE ] > 0) ? false : true;
  for( i = 0; i <= 23; i++ )
    {
      if( m_points[ i ] > 0 )
        {
          lowest_pos_white = i;
          if( i < 18 )
            m_are_all_in_homeboard[ WHITE ] = false;
          break;
        }
    }

  m_are_all_in_homeboard[ BLACK ] = (m_on_bar[ BLACK ] > 0) ? false : true;
  for( i = 23; i >= 0; i-- )
    {
      if( m_points[ i ] < 0 )
        {
          highest_pos_black = i;
          if( i > 5 )
            m_are_all_in_homeboard[ BLACK ] = false;
          break;
        }
    }

  /* m_game_status setzen */
  if( m_beared_off[ WHITE ] >= 15 || m_beared_off[ BLACK ] >= 15 )
    {
      if( get_game_status() != GAME_FINISHED )
        {
          m_var_mutex.lock();
          m_game_status = GAME_FINISHED;
          if( m_beared_off[ WHITE ] >= 15 )
            m_winner = WHITE;
          else
            m_winner = BLACK;
          m_win_height = calc_win_height();
          m_var_mutex.unlock();
          m_array_mutex.unlock();
          emit game_status_changed( get_game_status() );
          emit game_ended();
          return;
        }
    }

  m_array_mutex.unlock();

  if( lowest_pos_white > highest_pos_black )
    {
      if( get_game_status() != BEARING_OFF )
        {
          m_var_mutex.lock();
          m_game_status = BEARING_OFF;
          m_var_mutex.unlock();
          emit game_status_changed( get_game_status() );
        }
    }

  is_valid_move_possible( NULL ); // Sicherstellen, dass
  // m_dice_result_has_to_be_used
  // aktualisiert wird.
}

/////////////////////////////////////////////////////////////////////////////
/// Setzt \ref m_act_player auf \a player.
/////////////////////////////////////////////////////////////////////////////
void Backgammon::set_act_player( short int player )
{
  QMutexLocker var_lock( &m_var_mutex );

  assert( player == BG::WHITE || player == BG::BLACK );

  if( m_act_player != player )
    {
      m_act_player = Player( player );
      var_lock.unlock();
      emit act_player_changed( player );
      var_lock.relock();
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Setzt \ref m_points[ \a pos ] auf \a value.
/////////////////////////////////////////////////////////////////////////////
void Backgammon::set_point( unsigned short int pos, short int value )
{
  QMutexLocker array_lock( &m_array_mutex );

  assert( pos < 24 );
  assert( value >= -15 && value <= 15 );
  m_points[ pos ] = value;
}

/////////////////////////////////////////////////////////////////////////////
/// Setzt \ref m_beared_off[ \a player ] auf \a value.
/////////////////////////////////////////////////////////////////////////////
void Backgammon::set_beared_off( unsigned short int player,
    unsigned short int value )
{
  QMutexLocker var_lock( &m_var_mutex );

  assert( player < 2 );
  assert( value <= 15 );
  m_beared_off[ player ] = value;
}

/////////////////////////////////////////////////////////////////////////////
/// Setzt \ref m_on_bar[ \a player ] auf \a value.
/////////////////////////////////////////////////////////////////////////////
void Backgammon::set_on_bar( unsigned short int player,
    unsigned short int value )
{
  QMutexLocker array_lock( &m_array_mutex );

  assert( player < 2 );
  assert( value <= 15 );
  m_on_bar[ player ] = value;
}

/////////////////////////////////////////////////////////////////////////////
/// Setzt \ref m_game_status auf \a value.
/////////////////////////////////////////////////////////////////////////////
void Backgammon::set_game_status( short int value )
{
  QMutexLocker var_lock( &m_var_mutex );

  if( value == m_game_status )
    return;
  m_game_status = GameStatus( value );
  var_lock.unlock();
  emit game_status_changed( value );
  var_lock.relock();
}

/////////////////////////////////////////////////////////////////////////////
/// Setzt \ref m_winner auf \a player.
/////////////////////////////////////////////////////////////////////////////
void Backgammon::set_winner( unsigned short int player )
{
  QMutexLocker lock( &m_var_mutex );
  assert( player < 3 );
  m_winner = Player( player );
}

/////////////////////////////////////////////////////////////////////////////
/// Setzt \ref m_win_height auf \a value.
/////////////////////////////////////////////////////////////////////////////
void Backgammon::set_win_height( unsigned short int value )
{
  QMutexLocker var_lock( &m_var_mutex );
  assert( value < 3 );
  m_win_height = WinHeight( value );
}
