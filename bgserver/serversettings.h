/*!
 * \file serversetttings.h
 * \brief Contains the declaration of the \ref ServerSettings class.
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

#ifndef SERVERSETTINGS_H
#define SERVERSETTINGS_H 1

#include <QHash>
#include <QString>
#include <QXmlDefaultHandler>

/////////////////////////////////////////////////////////////////////////////
/// This class reads the server configuration file and provides access to
/// these settings.
/////////////////////////////////////////////////////////////////////////////
class ServerSettings : QXmlDefaultHandler
{
  public:
    ServerSettings( void );
    ServerSettings( QString conffile );

    void readFromXml( QString conffile );
    void writeToXml( QString conffile );

    void readFrom( QIODevice *buf );
    void writeTo( QIODevice *buf );

    inline const QHash< QString, QString > & get_user_table( void ) const
      { return my_users; };
    inline bool user_exists( QString user ) const
      { return my_users.contains( user ); };
    inline QString get_password( QString user ) const
      { return my_users.value( user ); };
    inline bool unregistered_allowed( void ) const
      { return i_allow_unregistered; };
    inline quint16 get_port( void) const { return my_port; };
    inline unsigned int get_max_connections( void ) const
      { return my_max_connections; };
    inline QString get_motd( void ) const { return my_motd; };

    inline void clear_users( void )
      { my_users.clear(); };
    inline void insert_user( QString name, QString pass )
      { my_users.insert( name, pass ); };
    inline void set_unregistered_allowed( bool value )
      { i_allow_unregistered = value; };
    inline void set_port( quint16 value )
      { my_port = value; };
    inline void set_max_connections( unsigned int value )
      { my_max_connections = value; };
    inline void set_motd( QString value )
      { my_motd = value; };

  protected:
    virtual bool characters ( const QString &ch );
    virtual bool endElement( const QString &namespaceURI,
                             const QString &localName,
                             const QString &qName );
    virtual bool startElement( const QString &namespaceURI,
                               const QString &localName,
                               const QString &qName,
                               const QXmlAttributes &atts );

  private:
    QHash< QString, QString > my_users; ///< User/password pairs.
    bool i_allow_unregistered;
    quint16 my_port;
    unsigned int my_max_connections;
    QString my_motd; ///< Message of the Day

    bool i_read_motd;
};

#endif // SERVERSETTINGS_H
