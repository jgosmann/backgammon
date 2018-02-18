/*!
 * \file backgammon.h
 * \brief Deklaration der Backgammon-Klasse, welche Funktionen zur Verwaltung
 *        eines Backgammon-Spieles bereitstellt.
 * \date Tu Nov 21 2006
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

#ifndef BACKGAMMON_H
#define BACKGAMMON_H 1

#include <cassert>

#include <vector>

#include <QMutex>
#include <QMutexLocker>
#include <QObject>

/// \brief Klassen und Funktionen zur Implementation von Backgammon-Spielen.
namespace BG
{

/*< \label{BG::Player} >*/
/////////////////////////////////////////////////////////////////////////////
/// \brief Enumeration für die Spielerfarben.
///
/// Enumeration für die Spielerfarben.
/////////////////////////////////////////////////////////////////////////////
enum Player
{
  WHITE = 0,  ///< Weißer Spieler. *< \label{BG::WHITE} >*
  BLACK = 1,  ///< Schwarzer Spieler. *< \label{BG::BLACK} >*
  NOT_DEFINED ///< Nicht festgelegt.
};

/////////////////////////////////////////////////////////////////////////////
/// \brief Enumeration mit Werten zur Angabe des Spielstatus.
///
/// Enumeration mit Werten zur Angabe des Spielstatus.
/////////////////////////////////////////////////////////////////////////////
enum GameStatus
{
  FIRST_ROUND,     ///< 1. Runde des Spiels.
  GAME_IS_RUNNING, ///< Mitte des Spiels, keine Besonderheiten.
  BEARING_OFF,     ///< Die Spielsteine der beiden Spieler sind
                   ///< aneinander vorbei (d.h. es kann keiner mehr
                   ///< geschlagen werden), sind aber evtl. noch nicht
                   ///< alle im Homeboard. Trotzdem geht es in dieser
                   ///< Situation nur noch darum, möglichst schnell
                   ///< alle Spielsteine auszuwürfeln.
  GAME_FINISHED    ///< Spiel wurde beendet.
};

/////////////////////////////////////////////////////////////////////////////
/// \brief Enumeration für die Gewinnhöhe.
///
/// Enumeration für die Gewinnhöhe.
/////////////////////////////////////////////////////////////////////////////
enum WinHeight
{
  NOWIN = 0,     ///< Kein Punkt, weil Spiel z.B. nicht beendet wurde.
  NORMAL = 1,    ///< Normaler Gewinn mit einem Punkt.
  GAMMON = 2,    ///< Gewinn mit zwei Punkten, ein Gammon.
  BACKGAMMON = 3 ///< Gewinn mit drei Punkten, ein Backgammon.
};

/*< \label{BG::Position} >*/
/////////////////////////////////////////////////////////////////////////////
/// \brief Enumeration mit Werten zur Angabe von Spezialfeldern.
///
/// Enumeration mit Werten zur Angabe von Spezialfeldern
/////////////////////////////////////////////////////////////////////////////
enum Position
{
  BAR = 24,         ///< Bar *< \label{BG::BAR} >*
  OUT_OF_GAME = 25, ///< Ausgewürfelt *< \label{BG::OUT_OF_GAME} >*
};

/////////////////////////////////////////////////////////////////////////////
/// \brief Enumeration zur Bezeichnung von illegalen Zügen.
///
/// Enumeration zur Bezeichnung von illegalen Zügen.
/////////////////////////////////////////////////////////////////////////////
enum IllegalMove
{
  WRONG_PLAYER,                    ///< Der andere Spieler ist gerade am Zug.
  WRONG_DIRECTION,                 ///< Falsche Zugrichtung.
  CHECKER_LEFT_ON_BAR,             ///< Es müssen noch Spielsteine zurück ins
                                   ///< Spiel gebracht werden.
  NO_CHECKER_TO_MOVE,              ///< Auf dem angegebenen Feld ist kein
                                   ///< eigener Spielstein, der gesetzt
                                   ///< werden könnte.
  NOT_ALL_IN_HOMEBOARD,            ///< Es sind noch nicht alle Spielsteine
                                   ///< im Homeboard. Daher ist ein
                                   ///< Auswürfeln noch nicht möglich.
  ANOTHER_CHECKER_HAS_TO_BE_MOVED, ///< Es muss ein anderer Checker bewegt
                                   ///< werden, welcher Vorrang hat.
  BLOCKED,                         ///< Zielfeld ist bereits belegt (von mehr
                                   ///< als einem gegnerischem Stein).
  DICE_VALUE,                      ///< Position ist mit dem Würfelergebnis
                                   ///< nicht erreichbar oder es muss ein
                                   ///< anderer Stein gesetzt werden, damit
                                   ///< beide oder zumindestens die höhere
                                   ///< Augenzahl gesetzt wird.
  OTHER                            ///< Sonstiges.
};

/*< \label{BG::BackgammonMove} >*/
/////////////////////////////////////////////////////////////////////////////
/// \brief Klasse zur Angabe eines Zuges in einem Backgammon-Spiel.
///
/// Diese Klasse ermöglicht die Angabe eines Zuges mit einem Spielstein in
/// einem Backgammon-Spielstein. Dabei kann die Augenzahl beider Würfel
/// bereits zusammengerechnet sein. Werden mehrere Spielsteine in einem
/// Zug gesetzt, sind mehrere Instanzen dieser Struktur nötig.
/////////////////////////////////////////////////////////////////////////////
class BackgammonMove
{
  public:
    BackgammonMove( short int from = 0, short int distance = 1 );

    inline short int get_from( void ) const { return m_from; }
    inline short int get_distance( void ) const { return m_distance; }

    inline void set_from( short int value )
      {
        assert( (m_from >= 0 && m_from <= 23) || m_from == BAR );
        m_from = value;
      }
    inline void set_distance( short int value ) { m_distance = value; }

  private:
    short int m_from; ///< \brief Feld von dem gezogen wird.
    short int m_distance; ///< \brief Zahl der Felder die gezogen wird.
};

/*< \label{BG::BackgammonTurn} >*/
/////////////////////////////////////////////////////////////////////////////
/// \brief Klasse zur Angabe aller Züge in einer Runde in einem
/// Backgammon-Spiel
///
/// Diese Klasse dient dazu alle Züge, die ein Spieler in einer Runde
/// ausgeführt hat, zu speichern
/////////////////////////////////////////////////////////////////////////////
class BackgammonTurn
{
  public:
    BackgammonTurn( Player player = NOT_DEFINED, short int d1 = 0,
                    short int d2 = 0 );

    inline void set_player( Player player ) { m_player = player; }
    inline void set_dice( short int d1, short int d2 )
        { m_dice[ 0 ] = d1; m_dice[ 1 ] = d2; }
    void add_move( BackgammonMove move, bool hit_checker );
        ///< \brief Zug zur Runde hinzufügen.

    inline Player get_player( void ) const { return m_player; }
    inline const short int * get_dice( void ) const { return m_dice; }
    inline const std::vector< BackgammonMove > & get_move_list( void ) const
        { return m_move_list; }
    inline const std::vector< bool > & get_hit_checker( void ) const
        { return m_hit_checker; }

  private:
    Player m_player; ///< \brief Spieler, der gesetzt hat.
    short int m_dice[ 2 ]; ///< \brief Würfelergebnis
    std::vector< BackgammonMove > m_move_list; ///< \brief Ausgeführte Züge.
    std::vector< bool > m_hit_checker; ///< \brief Ob mit dem jeweiligen Zug
                                       ///< in \ref m_move_list ein
                                       ///< gegnerischer Stein geschlagen
                                       ///< wurde.
};

/*< \label{BG::Backgammon} >*/
/////////////////////////////////////////////////////////////////////////////
/// \brief Klasse zur Verwaltung eines Backgammon-Spiels.
///
/// Diese Klasse verwaltet ein Backgammon-Spiel, indem sie den aktuellen
/// Spielstand speichert und Funktionen bereitstellt, die prüfen, ob
/// bestimmte Züge gültig sind.
///
/// Diese Klasse ist Thread-Safe.
/////////////////////////////////////////////////////////////////////////////
class Backgammon : public QObject
{
  Q_OBJECT

  public:
    Backgammon( bool is_auto_dice_roll_enabled = true, QObject *parent = 0 );

    bool is_valid_move( const BackgammonMove &move,
                        IllegalMove *reason = NULL, bool *dice_used = NULL,
                        bool check_bar = true )
        const; ///< \brief \a move gültiger Zug?
    bool are_valid_moves( const std::vector< BackgammonMove > &moves,
                          IllegalMove *reason = NULL,
                          bool **dice_used = NULL ) const;
        ///< \brief Ist \a moves eine gültige Kombination von Zügen?
    bool is_valid_move_possible( BackgammonMove *move = NULL );
        ///< \brief Ist es möglich einen gültigen Zug zu machen?

    bool move( void ); ///< \brief Zum nächsten Spieler wechseln, wenn der
                       ///< aktulle zugunfähig ist.
    bool move( const BackgammonMove &move, IllegalMove *reason = NULL,
               bool *dice_used = NULL); ///< \brief Zug ausführen.
    bool move( const std::vector< BackgammonMove > &moves,
               IllegalMove *reason = NULL, bool *dice_used = NULL);
        ///< \brief Züge ausführen.

    inline void lock_arrays( void ) const { m_array_mutex.lock(); };
        ///< \brief Sperrt die Memberarrays.
        ///<
        ///< Sperrt die Memberarrays der Klasse. Muss vor einem Zugriff auf
        ///< diese aufgerufen werden, sofern es zu einem Zugriff von
        ///< verschiedenen Threads kommen kann. Anschließend muss die
        ///< Sperrung mit unlock_arrays() aufgehoben werden.
    inline void unlock_arrays( void ) const { m_array_mutex.unlock(); };
        ///< \brief Entsperrt die Memberarrays.
        ///<
        ///< Entsperrt die Memberarrays der Klasse.
        ///< \sa lock_arrays()

    inline const short int * get_points( void ) const { return m_points; };
        ///< \attention lock_arrays() muss zuerst aufgerufen werden.
    inline const short int * get_beared_off( void ) const
        { return m_beared_off; };
        ///< \attention lock_arrays() muss zuerst aufgerufen werden.
    inline const short int * get_on_bar( void ) const { return m_on_bar; };
        ///< \attention lock_arrays() muss zuerst aufgerufen werden.
    inline const bool * get_are_all_in_homeboard( void ) const
        { return m_are_all_in_homeboard; };
        ///< \attention lock_arrays() muss zuerst aufgerufen werden.
    inline Player get_act_player( void ) const
        { QMutexLocker lock( &m_var_mutex ); return m_act_player; };
    inline const short int * get_dice( void ) const { return m_dice; };
        ///< \attention lock_arrays() muss zuerst aufgerufen werden.
    inline const bool * get_dice_result_has_to_be_used( void ) const
        { return m_dice_result_has_to_be_used; };
        ///< \attention lock_arrays() muss zuerst aufgerufen werden.
    inline bool is_auto_dice_roll_enabled( void ) const
        { QMutexLocker lock( &m_var_mutex );
          return m_is_auto_dice_roll_enabled; };
    inline unsigned int get_turn_number( void ) const
        { QMutexLocker lock( &m_var_mutex ); return m_turn_number; };
    inline GameStatus get_game_status( void ) const
        { QMutexLocker lock( &m_var_mutex ); return m_game_status; };
    inline Player get_winner( void ) const
        { QMutexLocker lock( &m_var_mutex ); return m_winner; };
    inline WinHeight get_win_height( void ) const
        { QMutexLocker lock( &m_var_mutex ); return m_win_height; };
    inline const std::vector< BackgammonTurn > & get_turn_list( void ) const
        { QMutexLocker lock( &m_var_mutex ); return m_turn_list; };

    Backgammon & copy_without_turn_list( const Backgammon &source );
        ///< \brief Kopiert \a source ohne die Zugliste.

  public slots:
    void undo_last_turn( void );
    void reset( void ); ///< \brief Startet ein neues Spiel.

    void set_dice( short int d1, short int d2 );
        ///< \brief Setzt \ref m_dice.
    void set_is_auto_dice_roll_enabled( bool value );
        ///< \brief Setzt \ref m_is_auto_dice_roll_enabled.

  signals:
    void next_player( short int player ); ///< \brief Nächster Spieler ist an
                                          ///< die Reihe gekommen.
    void game_ended( void ); ///< \brief Das Spiel ist zu Ende.
    void act_player_changed( int player );

    void dice_changed( short int d1, short int d2 );
    void is_auto_dice_roll_enabled_changed( bool value );
    void turn_number_changed( unsigned int value );
    void game_status_changed( int value );
    void turn_list_changed( void );

  protected:
    void add_move_to_turn_list( BackgammonMove move );
        ///< \brief Fügt einen Zug \ref m_turn_list hinzu.
    void apply_move( BackgammonMove move, bool dice_used[ 4 ] );
        ///< \brief Einen Zug ausführen und zu \ref m_turn_list hinzufügen.
    bool is_able_to_move( const BackgammonMove *move,
                          const BackgammonMove *before = NULL ) const;
        ///< \brief Ist \a move ein gültiger Zug?

    WinHeight calc_win_height( void ); ///< \brief Berechnet die Gewinnhöhe.

  protected slots:
    void clear_dice_silent(); ///< \brief Setzt alle Elemente von \ref m_dice
                              ///< auf 0.
    void end_turn( void ); ///< \brief Beendet den aktuellen Zug und der
                           ///< nächste Spieler kommt an die Reihe.
    void refresh( void ); ///< \brief Aktualisiert alle Statusvariablen.

    void set_act_player( short int player ); ///< \brief Setzt den aktuellen
                                             ///< Spieler.
    void set_point( unsigned short int pos, short int value );
        ///< \brief Setzt \ref m_points.
    void set_beared_off( unsigned short int player,
                         unsigned short int value );
        ///< \brief Setzt \ref m_beared_off.
    void set_on_bar( unsigned short int player, unsigned short int value );
        ///< \brief Setzt \ref m_on_bar.
    void set_game_status( short int value ); ///< \brief Setzt
                                             ///< \ref m_game_status.
    void set_winner( unsigned short int player ); ///< \brief Setzt
                                                  ///< \ref m_winner.
    void set_win_height( unsigned short int value ); ///< \brief Setzt
                                                     ///< \ref m_win_height.

  private:
    mutable QMutex m_var_mutex; ///< \brief Mutex für alle Membervariablen,
                                ///< die kein Array sind.
    mutable QMutex m_array_mutex; ///< \brief Mutex für sämtliche
                                  ///< Memberarrays.

    /*< \label{BG::Backgammon::m_points} >*/
    /// \brief Die Anzahl der Spielsteine auf den 24 points bzw. "Zungen".
    ///
    /// Die Anzahl der Spielsteine auf den 24 points bzw. "Zungen". Eine
    /// positive Zahl steht für die Spielsteine des weißen Spielers. Dieser
    /// zieht von 0 zu 23. Negative Zahlen geben die Zahl der Spielsteine des
    /// schwarzen Spielers an, welcher folglich von 23 zu 0 zieht.
    short int m_points[ 24 ];

    /*< \label{BG::Backgammon::m_beared_off} >*/
    short int m_beared_off[ 2 ]; ///< \brief Zahl der ausgewürfelten
                                 ///< Spielsteine der Spieler.
    /*< \label{BG::Backgammon::m_on_bar} >*/
    short int m_on_bar[ 2 ]; ///< \brief Spielsteine der Spieler auf der Bar.
    bool m_are_all_in_homeboard[ 2 ]; ///< \brief Sind alle Spielsteine eines
                                      ///< Spielers im Homeboard?

    /*< \label{BG::Backgammon::m_act_player} >*/
    Player m_act_player; ///< \brief Spieler, der gerade am Zug ist.

    /*< \label{BG::Backgammon::m_dice} >*/
    /// \brief Würfelwurf
    ///
    /// Würfelergebnis bzw. Augenzahlen die gesetzt werden müssen. -1 steht
    /// für eine bereits gesezte Augenzahl. Es wird davon ausgegangen, dass
    /// nach dem ersten Wert <= 0 keine weiteren zu setzenden Augenzahlen
    /// mehr folgen. Die Array-Größe beträgt 4, da bei einem Pasch viermal
    /// gesetzt werden muss.
    short int m_dice[ 4 ];

    /*< \label{BG::Backgammon::m_dice_result_has_to_be_used} >*/
    bool m_dice_result_has_to_be_used[ 2 ];
        ///< \brief Gibt an, ob welche der beiden gewürfelten Augenzahlen
        ///< gesetzt werden muss (wegen Zugzwang). Dabei ist
        ///< \ref m_dice_result_has_to_be_used[0] die höhere und entsprechend
        ///< \ref m_dice_result_has_to_be_used[1] die niedrigere.
        ///< Die Anordnung kann daher von der in \ref m_dice abweichen!
        ///< Bei einem Pasch sind beide Array-Elemente true oder, sofern nur
        ///< eine Augenzahl gesetzt werden kann, nur
        ///< \ref m_dice_result_has_to_be_used[0].

    /// \brief Automtisches Würfeln aktivieren?
    ///
    /// Ob die Instanz der Klasse automatisch würfelt. Ansonsten muss
    /// \ref m_dice manuell gesetzt werden.
    bool m_is_auto_dice_roll_enabled;

    unsigned int m_turn_number; ///< \brief Zugnummer
    /*< \label{BG::Backgammon::game_status} >*/
    GameStatus m_game_status; ///< \brief Status des Spiels.
    Player m_winner; ///< \brief Spieler, der das Spiel gewonnen hat.
    WinHeight m_win_height; ///< \brief Gewinnhöhe des letzten Spiels.

    /*< \label{BG::Backgammon::m_turn_list} >*/
    std::vector< BackgammonTurn > m_turn_list; ///< \brief Zugliste
};

} // Namespace BG

#endif
