/*!
 * \file backgammonwidget.cpp
 * \brief Implementation der BackgammonWidget-Klasse, welche zur Anzeige
 *        eines Backgammon-Spieles und der Eingabe von Zügen dient.
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

#include <cmath>

#include <QDrag>
#include <QMouseEvent>
#include <QPainter>
#include <QPicture>
#include <QPixmap>
#include <QSettings>
#include <QWidget>

#include "backgammon.h"
#include "settings.h"

#include "backgammonwidget.h"

/////////////////////////////////////////////////////////////////////////////
/// Konstruktor, in \a game muss das darzustellende Spiel übergeben werden.
/////////////////////////////////////////////////////////////////////////////
BackgammonWidget::BackgammonWidget( BG::Backgammon *game, QWidget *parent )
    : QWidget( parent ), m_game( game ), m_is_rotated( false ),
      m_drag_icon( NULL )
{
  QSettings settings;

  setAcceptDrops( true );

  m_col_white = load_color( "BackgammonWidget/m_col_white",
                            QColor( 255, 255, 255 ) );
  m_col_black = load_color( "BackgammonWidget/m_col_black",
                            QColor( 0, 0, 0 ) );
  m_col_pen = load_color( "BackgammonWidget/m_col_pen",
                          QColor( 0, 0, 0 ) );
  m_col_bar = load_color( "BackgammonWidget/m_col_bar",
                          QColor( 240, 240, 184 ) );
  m_col_bg = load_color( "BackgammonWidget/m_col_bg",
                         QColor( 255, 255, 204 ) );
}

/////////////////////////////////////////////////////////////////////////////
/// Destruktor
/////////////////////////////////////////////////////////////////////////////
BackgammonWidget::~BackgammonWidget( void )
{
  QSettings settings;

  if( m_drag_icon )
    delete m_drag_icon;

  save_color( "BackgammonWidget/m_col_white", m_col_white );
  save_color( "BackgammonWidget/m_col_black", m_col_black );
  save_color( "BackgammonWidget/m_col_pen", m_col_pen );
  save_color( "BackgammonWidget/m_col_bar", m_col_bar );
  save_color( "BackgammonWidget/m_col_bg", m_col_bg );
}

/*< \label{BG::Position:BackgammonWidget::conv_mouse_pos(int,int)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Konvertiert die Mausposition mit den Koordinaten \a x und \a y (relativ
/// zum Widget) in eine Position des Backgammon Spieles, welche zurück
/// gegeben wird.
/////////////////////////////////////////////////////////////////////////////
BG::Position BackgammonWidget::conv_mouse_pos( int x, int y )
{
  short int position; // Position, die zurückgegeben wird.

  position = static_cast< short int >( floor( x / ( width() / 15.0 ) ) );
  if( position <= 0 || position >= 14 )
    return BG::OUT_OF_GAME;
  if( position == 7 )
    return BG::BAR;

  if( position > 7 )
    position--;
  position--;
  if( y > height() / 2 )
    position = 23 - position;
  if( m_is_rotated )
    {
      if( y <= height() / 2 )
        position = 12 + position;
      else
        position = position - 12;
    }
  return BG::Position( position );
}

/////////////////////////////////////////////////////////////////////////////
/// Setzt \ref m_is_rotated.
/////////////////////////////////////////////////////////////////////////////
void BackgammonWidget::set_is_rotated( bool value )
{
  if( m_is_rotated == value )
    return;
  m_is_rotated = value;
  update();
  emit is_rotated_changed( m_is_rotated );
}

/////////////////////////////////////////////////////////////////////////////
/// Setzt \ref m_col_white.
/////////////////////////////////////////////////////////////////////////////
void BackgammonWidget::set_col_white( QColor color )
{
  if( m_col_white == color )
    return;
  m_col_white = color;
  update();
  emit col_white_changed( m_col_white );
}

/////////////////////////////////////////////////////////////////////////////
/// Setzt \ref m_col_black.
/////////////////////////////////////////////////////////////////////////////
void BackgammonWidget::set_col_black( QColor color )
{
  if( m_col_black == color )
    return;
  m_col_black = color;
  update();
  emit col_black_changed( m_col_black );
}

/////////////////////////////////////////////////////////////////////////////
/// Setzt \ref m_col_pen.
/////////////////////////////////////////////////////////////////////////////
void BackgammonWidget::set_col_pen( QColor color )
{
  if( m_col_pen == color )
    return;
  m_col_pen = color;
  update();
  emit col_pen_changed( m_col_pen );
}

/////////////////////////////////////////////////////////////////////////////
/// Setzt \ref m_col_bar.
/////////////////////////////////////////////////////////////////////////////
void BackgammonWidget::set_col_bar( QColor color )
{
  if( m_col_bar == color )
    return;
  m_col_bar = color;
  update();
  emit col_bar_changed( m_col_bar );
}

/////////////////////////////////////////////////////////////////////////////
/// Setzt \ref m_col_bg.
/////////////////////////////////////////////////////////////////////////////
void BackgammonWidget::set_col_bg( QColor color )
{
  if( m_col_bg == color )
    return;
  m_col_bg = color;
  update();
  emit col_bg_changed( m_col_bg );
}

/*< \label{QPicture:BackgammonWidget::draw_checkers(unsigned:short:int,double,QColor,QColor,bool)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Zeichnet \a n Spielsteine (von unten nach oben, wobei bei mehr als 6
/// Steinen, diese sich überlappen). \a size gibt die Breite und Höhe eines
/// Spielsteines an. \a fg gibt die Umrandungsfarbe und \a bg die Füllfarbe
/// an. Wenn \a invert true ist wird nicht von oben nach unten, sonder von
/// unten nach oben gezeichnet.
/////////////////////////////////////////////////////////////////////////////
QPicture BackgammonWidget::draw_checkers( unsigned short int n, double size,
                                          QColor fg, QColor bg, bool invert )
{
  QPicture pic;
  QPainter painter( &pic );
  unsigned int i;

  painter.setPen( fg );
  painter.setBrush( bg );

  for( i = 0; i < n && i < 5; i++ )
    painter.drawEllipse( QRectF( 0, ( invert ) ? 5.0 * size - (i + 1) * size
                                               : i * size,
                                 size, size ) );
  for( ; i < n && i < 9; i++ )
    painter.drawEllipse( QRectF( 0, ( invert ) ? 4.5 * size - (i - 4) * size
                                               : (i - 5) * size + 0.5 * size,
                                 size, size ) );
  for( ; i < n && i < 12; i++ )
    painter.drawEllipse( QRectF( 0, ( invert ) ? 4.0 * size - (i - 8) * size
                                               : (i - 9) * size + 1.0 * size,
                                 size, size ) );
  for( ; i < n && i < 14; i++ )
    painter.drawEllipse( QRectF( 0, ( invert ) ? 3.5 * size - (i - 11) * size
                                               : (i - 12) * size
                                                 + 1.5 * size,
                                 size, size ) );
  if( i < n )
    painter.drawEllipse( QRectF( 0, 2.0 * size, size, size ) );

  return pic;
}

/////////////////////////////////////////////////////////////////////////////
/// DragEnterEvent, sorgt dafür, dass nur Spielsteine "gedroppt" werden
/// können.
/////////////////////////////////////////////////////////////////////////////
void BackgammonWidget::dragEnterEvent( QDragEnterEvent *event )
{
  if( event->mimeData()->hasFormat( "application/backgammon-move" ) )
    event->acceptProposedAction();
}

/*< \label{void:BackgammonWidget::dropEvent(QDropEvent)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Übergibt den so eben eingegebenen Zug der Instanz der Backgammon-Klasse,
/// um den Zug auszuführen.
/////////////////////////////////////////////////////////////////////////////
void BackgammonWidget::dropEvent( QDropEvent *event )
{
  const short int START_POSITION =
      ( m_drag_start != BG::BAR )
        ? m_drag_start : (( m_game->get_act_player() == BG::WHITE ) ? -1
                                                                    : 24);
  BG::Position end_position; // Position von zu der gezogen wird.
  short int distance; // Distanz, die gezogen wird.
  BG::IllegalMove reason; // Grund, warum Zug nicht möglich (wenn zutreffend)
  unsigned int i;

  if( !event->mimeData()->hasFormat( "application/backgammon-move" ) )
    return;

  /* end_position bestimmen */
  end_position = conv_mouse_pos( event->pos().x(), event->pos().y() );
  if( end_position == BG::BAR )
    return;

  /* Distanz bestimmen */
  m_game->lock_arrays();
  if( end_position == BG::OUT_OF_GAME )
    {
      distance = ( m_game->get_act_player() == BG::WHITE )
                 ? 24 - START_POSITION : 1 + START_POSITION;
      if( m_game->get_dice()[ 0 ] <= m_game->get_dice()[ 1 ] )
        {
          if( m_game->get_dice()[ 0 ] >= distance )
            distance = m_game->get_dice()[ 0 ];
          else if( m_game->get_dice()[ 1 ] >= distance )
            distance = m_game->get_dice()[ 1 ];
          else
            {
              distance = 0;
              for( i = 0; i < 4 && distance < (( m_game->get_act_player()
                                                 == BG::WHITE )
                                               ? 24 - START_POSITION
                                               : 1 + START_POSITION);
                   i++ )
                distance += m_game->get_dice()[ i ];
            }
        }
      else
        {
          if( m_game->get_dice()[ 1 ] >= distance )
            distance = m_game->get_dice()[ 1 ];
          else if( m_game->get_dice()[ 0 ] >= distance )
            distance = m_game->get_dice()[ 0 ];
          else
            {
              distance = 0;
              for( i = 0; i < 4 && distance < (( m_game->get_act_player()
                                                 == BG::WHITE )
                                               ? 24 - START_POSITION
                                               : 1 + START_POSITION);
                   i++ )
                distance += m_game->get_dice()[ i ];
            }
        }

      /* Wenn die gewürfelten Augenzahlen nicht reichen den Spielstein
       * auszuwürfeln, Distanz wieder zurücksetzen, damit angezeigt wird,
       * dass der Zug ungültig ist */
      if( distance < ( ( m_game->get_act_player() == BG::WHITE )
                         ? 24 - START_POSITION : 1 + START_POSITION ) )
        distance = ( m_game->get_act_player() == BG::WHITE )
                     ? 24 - START_POSITION : 1 + START_POSITION;
    }
  else
    distance = (end_position - START_POSITION)
               * (( m_game->get_act_player() == BG::WHITE ) ? 1 : -1 );
  m_game->unlock_arrays();

  BG::BackgammonMove move( m_drag_start, distance );
  if( !m_game->move( move, &reason ) )
    emit illegal_move( reason );

  update();
  event->acceptProposedAction();
}

/*< \label{void:BackgammonWidget::mousePressEvent(QMouseEvent)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Startet eine Drag & Drop-Operation zur Eingabe eines Zuges.
/////////////////////////////////////////////////////////////////////////////
void BackgammonWidget::mousePressEvent( QMouseEvent *event )
{
  QPixmap alpha_channel( width() / 15, width() / 15 ); // Alpha-Channel für
                                                       // das Drag-Icon.
  QPainter alpha_channel_painter( &alpha_channel );

  if( event->button() != Qt::LeftButton )
    return;

  /* position bestimmen */
  m_drag_start = conv_mouse_pos( event->x(), event->y() );
  if( m_drag_start == BG::OUT_OF_GAME )
    return;

  /* Prüfen, ob vom der entsprechenden Position gezogen werden kann */
  m_game->lock_arrays();
  if( m_drag_start == BG::BAR
      && m_game->get_on_bar()[ m_game->get_act_player() ] <= 0 )
    {
      m_game->unlock_arrays();
      return;
    }
  if( m_drag_start != BG::BAR
      && m_game->get_points()[ m_drag_start ]
         * (( m_game->get_act_player() == BG::WHITE ) ? 1 : -1) <= 0 )
    {
      m_game->unlock_arrays();
      return;
    }
  m_game->unlock_arrays();

  /* Drag starten und Icon setzen */
  QDrag *drag = new QDrag( this );

  QMimeData *mimeData = new QMimeData;
  mimeData->setData( "application/backgammon-move", NULL );
  drag->setMimeData( mimeData );

  if( m_drag_icon )
    delete m_drag_icon;
  m_drag_icon = new QPixmap( width() / 15, width() / 15 );
  QPainter painter( m_drag_icon );
  painter.setRenderHint( QPainter::Antialiasing );
  if( m_game->get_act_player() == BG::WHITE )
    {
      painter.setPen( m_col_black );
      painter.setBrush( m_col_white );
    }
  else
    {
      painter.setPen( m_col_white );
      painter.setBrush( m_col_black );
    }
  painter.fillRect( 0, 0, width() / 15, width() / 15,
                    QBrush( QColor( 255, 255, 255 ) ) );
  painter.drawEllipse( 0, 0, width() / 15 - 1, width() / 15 - 1 );
  painter.end();

  alpha_channel_painter.fillRect( 0, 0, width() / 15, width() / 15,
                                  QBrush( QColor( 0, 0, 0 ) ) );
  alpha_channel_painter.setRenderHint( QPainter::Antialiasing );
  alpha_channel_painter.setPen( QColor ( 255, 255, 255 ) );
  alpha_channel_painter.setBrush( QColor ( 255, 255, 255 ) );
  alpha_channel_painter.drawEllipse( 0, 0, width() / 15 - 1,
  width() / 15 - 1 );
  alpha_channel_painter.end();
  m_drag_icon->setAlphaChannel( alpha_channel );

  drag->setPixmap( *m_drag_icon );
  drag->setHotSpot( QPoint( width() / 30, width() / 30 ) );

  drag->start( Qt::MoveAction );
}

/*< \label{void:BackgammonWidget::paintEvent(QPaintEvent*)} >*/
/////////////////////////////////////////////////////////////////////////////
/// Zeichnet das Widget.
/////////////////////////////////////////////////////////////////////////////
void BackgammonWidget::paintEvent( QPaintEvent *event )
{
  QPainter painter( this );
  const double point_width = width() / 15.0; // Breite einer "Zunge"
  const double point_height = 5.0 * point_width; // Höhe einer "Zunge"
  QPointF triangle_coords[ 3 ]; // Speicherung der "Zungen"-Koordinaten zum
                                // Zeichnen.
  short int n_checkers_to_draw;
  BG::Player player;
  QString str; // Nur für temporäre Speicherung.
  unsigned int i;

  /* Hintergrund */
  painter.fillRect( QRect( 0, 0, width(), height() ), m_col_bg );

  painter.translate( point_width, 0 );
  /* Drehung */
  if( m_is_rotated )
    {
      painter.translate( width(), height() );
      painter.rotate( 180 );
      painter.translate( 2.0 * point_width, 0 );
    }

  painter.setRenderHint( QPainter::Antialiasing );

  /* Bar und Ränder */
  painter.setRenderHint( QPainter::Antialiasing, false );
  painter.setPen( m_col_pen );
  painter.setBrush( m_col_bar );
  painter.drawRect( QRectF( 6.0 * point_width, point_width / 2.0,
                            point_width - 1,
                            2.0 * point_height + point_width ) );
  painter.drawLine( QLineF( 0, point_width / 2.0, 0,
                            2.0 * point_height + 1.5 * point_width ) );
  painter.drawLine( QLineF( 13.0 * point_width, point_width / 2.0,
                            13.0 * point_width,
                            2.0 * point_height + 1.5 * point_width ) );

  m_game->lock_arrays();
  painter.setRenderHint( QPainter::Antialiasing );
  painter.setPen( m_col_black );
  painter.setBrush( m_col_white );
  painter.drawPicture( QPointF( 6.0 * point_width, point_width / 2.0 ),
          draw_checkers( m_game->get_on_bar()[ BG::WHITE ], point_width,
                         m_col_black, m_col_white ) );
  painter.drawPicture( QPointF( -1.0 * point_width, point_height
                                                    + 1.5 * point_width ),
          draw_checkers( m_game->get_beared_off()[ BG::WHITE ], point_width,
                         m_col_black, m_col_white, true ) );
  painter.setPen( m_col_white );
  painter.setBrush( m_col_black );
  painter.drawPicture( QPointF( 6.0 * point_width, point_height
                                                   + 1.5 * point_width ),
          draw_checkers( m_game->get_on_bar()[ BG::BLACK ], point_width,
                         m_col_white, m_col_black, true ) );
  painter.drawPicture( QPointF( -1.0 * point_width, point_width / 2.0 ),
          draw_checkers( m_game->get_beared_off()[ BG::BLACK ], point_width,
                         m_col_white, m_col_black ) );
  m_game->unlock_arrays();

  /* Obere Hälfte */
  triangle_coords[ 0 ].setY( point_width / 2.0 );
  triangle_coords[ 1 ].setY( point_width / 2.0 );
  triangle_coords[ 2 ].setY( point_width / 2.0 + point_height );
  for( i = 0; i < 13; i++ )
    {
      painter.setPen( m_col_pen );
      if( i == 6 )
        continue;
      if( i % 2 == ( i < 6 ) ? 0 : 1 )
        painter.setBrush( m_col_white );
      else
        painter.setBrush( m_col_black );

      triangle_coords[ 0 ].setX( i * point_width );
      triangle_coords[ 1 ].setX( (i + 1) * point_width - 1 );
      triangle_coords[ 2 ].setX( (i * point_width + (i + 1) * point_width)
                                 / 2.0 );
      painter.drawConvexPolygon( triangle_coords, 3 );

      m_game->lock_arrays();
      n_checkers_to_draw = m_game->get_points()[ ( i < 6 ) ? i : i - 1 ];
      m_game->unlock_arrays();
      if( n_checkers_to_draw >= 0 )
        player = BG::WHITE;
      else
        {
          n_checkers_to_draw *= -1;
          player = BG::BLACK;
        }
      painter.setPen( ( player == BG::WHITE ) ? m_col_black : m_col_white );
      painter.setBrush( ( player == BG::WHITE ) ? m_col_white
                                                : m_col_black );
      painter.drawPicture( QPointF( i * point_width, point_width / 2.0 ),
          draw_checkers( n_checkers_to_draw, point_width,
                         ( player == BG::WHITE ) ? m_col_black
                                                 : m_col_white,
                         ( player == BG::WHITE ) ? m_col_white
                                                 : m_col_black ) );
    }

  /* Untere Hälfte */
  triangle_coords[ 0 ].setY( 2.0 * point_height + 1.5 * point_width );
  triangle_coords[ 1 ].setY( 2.0 * point_height + 1.5 * point_width );
  triangle_coords[ 2 ].setY( point_height + 1.5 * point_width );
  for( i = 0; i < 13; i++ )
    {
      painter.setPen( m_col_pen );
      if( i == 6 )
        continue;
      if( i % 2 == ( i < 6 ) ? 0 : 1 )
        painter.setBrush( m_col_black );
      else
        painter.setBrush( m_col_white );

      triangle_coords[ 0 ].setX( i * point_width );
      triangle_coords[ 1 ].setX( (i + 1) * point_width - 1 );
      triangle_coords[ 2 ].setX( (i * point_width + (i + 1) * point_width)
                                 / 2.0 );
      painter.drawConvexPolygon( triangle_coords, 3 );

      m_game->lock_arrays();
      n_checkers_to_draw = m_game->get_points()[ ( i < 6 ) ? 23 - i
                                                           : 24 - i ];
      m_game->unlock_arrays();
      if( n_checkers_to_draw >= 0 )
        player = BG::WHITE;
      else
        {
          n_checkers_to_draw *= -1;
          player = BG::BLACK;
        }
      painter.setPen( ( player == BG::WHITE ) ? m_col_black : m_col_white );
      painter.setBrush( ( player == BG::WHITE ) ? m_col_white
                                                : m_col_black );
      painter.drawPicture( QPointF( i * point_width, point_height
                                                     + 1.5 * point_width ),
          draw_checkers( n_checkers_to_draw, point_width,
                         ( player == BG::WHITE ) ? m_col_black
                                                 : m_col_white,
                         ( player == BG::WHITE ) ? m_col_white
                                                 : m_col_black,
                         true ) );
    }

  /* Drehung */
  if( m_is_rotated )
    {
      painter.translate( -2.0 * point_width, 0 );
      painter.rotate( -180 );
      painter.translate( -width(), -height() );
    }

  /* Nummerierung */
  painter.setRenderHint( QPainter::Antialiasing, false );
  painter.fillRect( QRectF( 0, 0, 13.0 * point_width, point_width / 4.0 ),
                    m_col_white );
  painter.fillRect( QRectF( 0, point_width / 4.0, 13.0 * point_width,
                            point_width / 4.0 ),
                    m_col_black );
  painter.fillRect( QRectF( 0, 1.5 * point_width + 2.0 * point_height,
                            13.0 * point_width, point_width / 4.0 ),
                    m_col_black );
  painter.fillRect( QRectF( 0, 1.75 * point_width + 2.0 * point_height,
                            13.0 * point_width, point_width / 4.0 ),
                    m_col_white );

  for( i = 0; i < 13; i++ )
    {
      if( i == 6 )
        continue;

      if( !m_is_rotated )
        str.setNum( ( i < 6 ) ?  24 - i : 25 - i );
      else
        str.setNum( ( i < 6 ) ? 12 - i : 13 - i );
      painter.setPen( m_col_black );
      painter.drawText( QRectF( i * point_width, 0, point_width,
                                point_width / 4.0 ),
                        Qt::AlignCenter, str );

      if( !m_is_rotated )
        str.setNum( ( i < 6 ) ?  i + 1: i );
      else
        str.setNum( ( i < 6 ) ? 13 + i : 12 + i );
      painter.setPen( m_col_white );
      painter.drawText( QRectF( i * point_width, point_width / 4.0,
                                point_width, point_width / 4.0 ),
                        Qt::AlignCenter, str );
    }
  for( i = 0; i < 13; i++ )
    {
      if( i == 6 )
        continue;

      if( !m_is_rotated )
        str.setNum( ( i < 6 ) ?  24 - i : 25 - i );
      else
        str.setNum( ( i < 6 ) ? 12 - i : 13 - i );
      painter.setPen( m_col_white );
      painter.drawText( QRectF( i * point_width, 1.5 * point_width
                                                 + 2.0 * point_height,
                                point_width, point_width / 4.0 ),
                        Qt::AlignCenter, str );

      if( !m_is_rotated )
        str.setNum( ( i < 6 ) ?  i + 1: i );
      else
        str.setNum( ( i < 6 ) ? 13 + i : 12 + i );
      painter.setPen( m_col_black );
      painter.drawText( QRectF( i * point_width, 1.75 * point_width
                                                 + 2.0 * point_height,
                                point_width, point_width / 4.0 ),
                        Qt::AlignCenter, str );
    }
}

/////////////////////////////////////////////////////////////////////////////
/// Sorgt dafür, dass das Widget bei einer Größenänderung das richtige
/// Seitenverhältnis behält.
/////////////////////////////////////////////////////////////////////////////
void BackgammonWidget::resizeEvent( QResizeEvent *event )
{
  const double square_width = width() / 15.0;
  const double square_height = height() / 12.0;

  if ( round( square_width ) == round( square_height ) )
    return;

  if( square_width > square_height )
    resize( static_cast< int > (square_height * 15.0), height() );
  else if( square_width < square_height )
    resize( width(), static_cast< int > (square_width * 12.0) );
}
