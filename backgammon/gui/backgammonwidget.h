/*!
 * \file backgammonwidget.h
 * \brief Deklaration der BackgammonWidget-Klasse, welche zur Anzeige eines
 *        Backgammon-Spieles und der Eingabe von Zügen dient.
 * \date Mi Nov 15 2006
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

#ifndef BACKGAMMONWIDGET_H
#define BACKGAMMONWIDGET_H 1

#include <QMouseEvent>
#include <QPicture>
#include <QWidget>

#include "backgammon.h"

/*< \label{BackgammonWidget} >*/
/////////////////////////////////////////////////////////////////////////////
/// Zeigt ein Backgammon-Spiel an und ermöglicht die Eingabe von Zügen.
/////////////////////////////////////////////////////////////////////////////
class BackgammonWidget : public QWidget
{
  Q_OBJECT

  public:
    BackgammonWidget( BG::Backgammon *game, QWidget *parent = 0 );
    ~BackgammonWidget( void );

    BG::Position conv_mouse_pos( int x, int y );
        ///< \brief Konvertiert eine Mausposition in eine Position.

    inline bool get_is_rotated( void ) const { return m_is_rotated; }
    inline QColor get_col_white( void ) const { return m_col_white; }
    inline QColor get_col_black( void ) const { return m_col_black; }
    inline QColor get_col_pen( void ) const { return m_col_pen; }
    inline QColor get_col_bar( void ) const { return m_col_bar; }
    inline QColor get_col_bg( void ) const { return m_col_bg; }

  public slots:
    void set_is_rotated( bool value ); ///< \brief Setzt \ref m_is_rotated.
    void set_col_white( QColor color ); ///< \brief Setzt \ref m_col_white.
    void set_col_black( QColor color ); ///< \brief Setzt \ref m_col_black.
    void set_col_pen( QColor color ); ///< \brief Setzt \ref m_col_pen.
    void set_col_bar( QColor color ); ///< \brief Setzt \ref m_col_bar.
    void set_col_bg( QColor color ); ///< \brief Setzt \ref m_col_bg.

  signals:
    void illegal_move( int reason );
        ///< \brief Wird ein nicht möglicher Zug eingeben, wird dieses Signal
        ///< gesendet.

    void is_rotated_changed( bool value );
    void col_white_changed( QColor color );
    void col_black_changed( QColor color );
    void col_pen_changed( QColor color );
    void col_bar_changed( QColor color );
    void col_bg_changed( QColor color );

  protected:
    QPicture draw_checkers( unsigned short int n, double size, QColor fg,
                            QColor bg, bool invert = false );
        ///< \brief Zeichnet eine Reihe von Spielsteinen.

    virtual void dragEnterEvent( QDragEnterEvent *event );
    virtual void dropEvent( QDropEvent *event );
    virtual void mousePressEvent( QMouseEvent *event );
    virtual void paintEvent( QPaintEvent *event );
    virtual void resizeEvent( QResizeEvent *event );

  private:
    BG::Backgammon *m_game; ///< \brief Das Backgammon-Spiel, dass
                            ///< dargestellt werden soll.
    bool m_is_rotated; ///< \brief Gibt an, ob das Spielfeld um 180° gedreht
                       ///< dargestellt wird.

    QPixmap *m_drag_icon; ///< \brief Grafik eines Spielsteines für
                          ///< Drag & Drop-Aktionen.
    BG::Position m_drag_start; ///< \brief Position, von der ein Spielstein
                               ///< gezogen wird.

    QColor m_col_white; ///< \brief Farbe für weiße Spielsteine und Zacken.
    QColor m_col_black; ///< \brief Farbe für scharze Spielsteine und Zacken.
    QColor m_col_pen; ///< \brief Standard Umrandungsfarbe.
    QColor m_col_bar; ///< \brief Farbe für die Bar.
    QColor m_col_bg; ///< \brief Hintergrundsfarbe.
};

#endif
