/*!
 * \file aithread.h
 * \brief Deklaration der Klasse f�r die KI.
 * \date Sa Dec 23 2006
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

#ifndef AITHREAD_H
#define AITHREAD_H 1

#include <vector>

#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QThread>

#include "backgammon.h"

/*< \label{AIThread} >*/
/////////////////////////////////////////////////////////////////////////////
/// Diese Klasse implementiert eine KI f�r Backgammon. Sie startet einen
/// extra Thread, um das Programm nicht zu blockieren und wird �ber spezielle
/// Steuerfunktionen bedient.
/////////////////////////////////////////////////////////////////////////////
class AIThread : public QThread
{
  Q_OBJECT

  public:
    /// \brief Enumeration zur Beschreibung der Zust�nde eines AIThreads.
    ///
    /// Diese Enumeration dient zur Beschreibung der Zust�nde eines AIThreads
    enum StatusFlag
      {
        SLEEPING,       ///< F�hrt gerade keine Aktion aus.
        SEARCHING_MOVE, ///< Sucht beste Zugm�glichkeit.
        TERMINATING,    ///< AIThread wird beendet.
        TERMINATED      ///< AIThread wurde beendet.
      };

    /// \brief Enumeration zur Codierung von Aufforderungen in einen anderen
    /// Zustand �berzugehen.
    ///
    /// Enumeration zur Codierung von Aufforderungen an einen AI-Thread in
    /// einen anderen Zustand �berzugehen.
    enum RequestFlag
      {
        SLEEP,     ///< Ausf�hrung pausieren.
        FIND_MOVE, ///< Zug suchen.
        TERMINATE  ///< Ausf�hrung terminieren.
      };

    /*< \label{AIThread::RatingFactors} >*/
    /// \brief Enumration mit Indizes f�r \ref m_rating_factors.
    ///
    /// Enumration mit Indizes f�r \ref m_rating_factors.
    enum RatingFactors
      {
        PROB_CHECKER_IS_HIT = 0,      ///< Faktor f�r die Wahrscheinlichkeit,
                                      ///< dass ein eigener Spielstein
                                      ///< geschlagen wird.
        PROB_CANNOT_MOVE = 1,         ///< Faktor f�r die Wahrscheinlichkeit,
                                      ///< dass die KI im n�chsten Zug
                                      ///< zugunf�hig ist.
        PROB_OP_CANNOT_MOVE = 2,      ///< Faktor f�r die Wahrscheinlichkeit,
                                      ///< dass der Gegner zugunf�hig ist.
        PROB_OP_CANNOT_MOVE_BAR = 3,  ///< Faktor f�r die Wahrscheinlichkeit,
                                      ///< dass der Gegner nicht von der Bar
                                      ///< ziehen kann.
        N_POINTS_WITH_CHECKERS = 4,   ///< Zahl der "Zungen" mit mindestens
                                      ///< zwei eigenen Spielsteinen.
        BEARED_OFF = 5,               ///< Faktor f�r die Zahl der
                                      ///< ausgew�rfelten Spielsteine.
        DISTANCE_FROM_HOMEBOARD = 6,  ///< Durchschnittliche Entfernung vom
                                      ///< Homeboard.
        CHECKERS_IN_OP_HOMEBOARD = 7, ///< Zahl der Spielsteine, die sich
                                      ///< im gegnerischen Homeboard oder auf
                                      ///< der Bar befinden.
        OP_DISTANCE_FROM_OFF_GAME = 8 ///< Faktor f�r die durchschnittliche
                                      ///< Entfernung zum Ausw�rfeln eines
                                      ///< Spielsteines.
      };
    static const unsigned int NUM_RATING_FACTORS = 9;
        ///< Gr��e von \ref m_rating_factors.

    AIThread( BG::Backgammon *game, QObject *parent = 0 );

    /*< \label{inline:void:AIThread::request(RequestFlag)} >*/
    inline void request( RequestFlag request_flag )
        { QMutexLocker lock( &m_request_mutex ); m_request = request_flag; }
        ///< \brief Sendet eine Aufforderung zur Status�nderung an einen
        ///< AI-Thread.
    inline void set_rating_factor( RatingFactors factor, double value )
        { QMutexLocker lock( &m_rating_factors_mutex );
          m_rating_factors[ factor ] = value; };
        ///< \brief Setzt \ref m_rating_factors.
    inline void set_moves_set( bool val )
        { QMutexLocker lock( &m_moves_set_mutex ); m_moves_set = val; };
        ///< \brief Setzt \ref m_moves_set.

    inline std::vector< BG::BackgammonMove > & get_best_move( void )
        { return m_best_move; }
        ///< \brief Gibt den besten gefunden Zug zur�ck.
        ///<
        ///< Gibt den besten gefundenen Zug zur�ck.
        ///< \attention Vor Verwendung der Funktion muss lock_best_move()
        ///< aufgerufen werden. Nach Verwendung unlock_best_move().
    inline void lock_best_move( void ) {  m_best_move_mutex.lock(); }
        ///< \brief Sperrt \ref m_best_move, so dass die Klasse bzw. der
        ///< Thread nicht mehr darauf zugreifen kann. Muss vor
        ///< get_best_move() aufgerufen werden.
    inline void unlock_best_move( void ) {  m_best_move_mutex.unlock(); }
        ///< \brief Erlaubt der Klasse wieder auf \ref m_best_move zu
        ///< zugreifen. Muss nach get_best_move() aufgerufen werden.
    inline StatusFlag get_status( void ) const
        { QMutexLocker lock( &m_status_mutex ); return m_status; };
    inline RequestFlag get_request( void ) const
        { QMutexLocker lock( &m_request_mutex ); return m_request; };
    inline double get_rating_factor( RatingFactors factor ) const
        { QMutexLocker lock( &m_rating_factors_mutex );
          return m_rating_factors[ factor ]; };
    inline bool get_moves_set( void ) const
        { QMutexLocker lock( &m_moves_set_mutex ); return m_moves_set; }

  signals:
    void best_move_found( void );

  protected:
    void run( void ); ///< \brief Startet die Ausf�hrung der KI.

    void find_move( short int rec_depth = 0 );
        ///< \brief Hauptfunktion der KI, die den besten Zug sucht.
    double rate_game_situation( const BG::Backgammon *game,
                                BG::Player player )
        const; ///< \brief Bewertet die Spielsituation.

    double distance_from_homeboard( const BG::Backgammon *game,
                                    BG::Player player ) const;
        ///< \brief Durchschnittliche Entfernung zum Homeboard.
    double distance_from_off_game( const BG::Backgammon *game,
                                   BG::Player player ) const;
        ///< \brief Durchschnittliche Entfernung zum Ausw�feln.
    unsigned short int n_checkers_in_op_homeboard(
        const BG::Backgammon *game, BG::Player player ) const;
        ///< \brief Anzahl eigner Steine im gegnerischen Homeboard.
    unsigned short int n_points_with_checkers( const BG::Backgammon *game,
                                               BG::Player player ) const;
        ///< \brief Anzahl an "Zungen" mit eigenen Spielsteinen.
    double prob_get_checker_back_into_game( const BG::Backgammon *game,
                                            BG::Player player ) const;
        ///< \brief Wahrscheinlichkeit einen Spielstein wieder einzuw�rfeln.
    double prob_player_cannot_move_at_pos( BG::Position pos,
                                           const BG::Backgammon *game,
                                           BG::Player player ) const;
        ///< \brief Wahrscheinlichkeit, dass ein Spieler die Spielsteine
        ///< \brief auf einer bestimmten Position nicht setzen kann.
    double prob_position_is_reached( const BG::Backgammon *game,
                                     short int position,
                                     BG::Player player ) const;
        ///< \brief Wahrscheinlichkeit eine Position zu erreichen.

    inline void set_status( StatusFlag status_flag )
        { QMutexLocker lock( &m_status_mutex ); m_status = status_flag; }

  private:
    BG::Backgammon *m_game; ///< \brief Zeiger auf das Backgammon-Spiel f�r
                            ///< das die KI verwendet wird.

    mutable QMutex m_best_move_mutex;
    /*< \label{AIThread::m_best_move} >*/
    std::vector< BG::BackgammonMove > m_best_move;
        ///< \brief Beste gefundene Zugm�lichkeit.
    double m_best_move_rating; ///< Bewertung der besten gefundenen
                               ///< Zugm�glichkeit.

    mutable QMutex m_status_mutex;
    StatusFlag m_status; ///< \brief Aktueller Status des AI-Threads
    mutable QMutex m_request_mutex;
    RequestFlag m_request; ///< \brief Anforderung f�r Statuswechsel.

    mutable QMutex m_rating_factors_mutex;
    /*< \label{AIThread::m_rating_factors} >*/
    double m_rating_factors[ NUM_RATING_FACTORS ];
        ///< \brief Faktoren f�r die Bewertungsfunktion.

    mutable QMutex m_moves_set_mutex;
    bool m_moves_set; ///< \brief Wurde bereits eine Zugkombination in
                      ///< \ref m_moves gespeichert? Diese Variable muss
                      ///< mit set_moves_set() auf false gesetzt werden,
                      ///< bevor m_request auf FIND_MOVE gesetzt wird. Denn
                      ///< die Klasse tut dies nicht. W�rde diese Variable
                      ///< nur von der Klasse auf false gesetzt k�nnte es
                      ///< zu einer race condition kommen.

    /* Folgende Variablen werden von find_move() verwendet. */
    /*< \label{AIThread::m_game_copy} >*/
    BG::Backgammon m_game_copy; ///< \brief Kopie von \ref m_game, verwendet
                                ///< von find_move().
    /*< \label{AIThread::m_dice} >*/
    short int m_dice[ 4 ]; ///< \brief Von find_move() verwendete Variable
                           ///< mit den Augenzahlen f�r die noch gesetzt
                           ///< werden muss
    /*< \label{AIThread::m_changes} >*/
    short int m_changes[ 25 ];
        ///< \brief Tempor�re �nderungen an den Positionen der Spielsteine
        ///< in find_move().
        ///<
        ///< Diese Variable speichert tempor�re �nderungen der Positionen an
        ///< den Spielsteinen des Backgammon-Spiels in \ref m_game anstatt
        ///< tats�chlich in diesem. \ref m_changes[24] steht f�r die Bar.
    /*< \label{AIThread::m_moves} >*/
    std::vector< BG::BackgammonMove > m_moves;
        ///< \brief Aktuelle in find_move() gepr�fte Zugkombination.
};

#endif
