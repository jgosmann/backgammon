/*!
 * \file chatwidget.h
 * \brief Deklaration der Klasse \ref ChatWidget.
 * \date Mo Jan 29 2007
 * \author Jan Gosmann (jan@hyper-world.de)
 *
 * \b Copyright: Copyright (C) 2007 Jan Gosmann
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

#ifndef CHATWIDGET_H
#define CHATWIDGET_H 1

#include <QAbstractSocket>
#include <QObject>
#include <QWidget>

#include "netbackgammon.h"

#include "ui_chatwidget.h"

/*< \label{ChatWidget} >*/
/////////////////////////////////////////////////////////////////////////////
/// \brief Widget, dass einen einfachen Chat bereitstellt.
///
/// Stellt ein Widget bereit, dass Meldungen eines Backgammon-Server anzeigt
/// und mit dem Chat-Nachrichten versendet werden können.
/////////////////////////////////////////////////////////////////////////////
class ChatWidget : public QWidget, public Ui::ChatWidget
{
  Q_OBJECT

  public:
    ChatWidget( NetBackgammonConnection *connection,
                bool passive_mode = false, QWidget *parent = 0,
                Qt::WindowFlags f = 0 );

    inline bool get_passive_mode( void ) const { return m_passive_mode; };

  public slots:
    void update( void ); ///< \brief Aktualisiert die Bedienelemente.

    inline void set_passive_mode( bool val ) { m_passive_mode = val; };

    void output_chat_msg( QString msg ); ///< \brief Gibt eine ausgehende
                                         ///< Chat-Nachricht aus.
    void output_net_connection_state( QAbstractSocket::SocketState state );
        ///< \brief Gibt den übergebenen Verbindungsstatus aus.
    void output_net_error( QAbstractSocket::SocketError error );
        ///< \brief Gibt einen Fehler aus.
    void process_srv_msg( NetBackgammonMsg msg );
        ///< \brief Gibt eine Server-Nachricht aus.
    void output_raw_data( QString data ); ///< \brief Gibt Daten ohne
                                          ///< Verarbeitung aus.

    void on_button_send_clicked( void );

  signals:
    void chat( QString msg ); ///< \brief Wird gesendet, wenn eine
                              ///< Chat-Nachricht versendet wird. Diese wird
                              ///< mit \a msg übergeben.

  private:
    NetBackgammonConnection *m_connection; ///< \brief Verbindung zum
                                           ///< Backgammon-Server
    bool m_passive_mode; ///< \brief Wurde des passive Modus aktiviert?
};

#endif
