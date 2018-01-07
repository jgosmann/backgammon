/*!
 * \file serveroptions.h
 * \brief Declarations of the ServerOptions Class implementing the UI for
 *        setting up a server.
 * \date 23.05.2009
 * \author Jan Gosmann (jan@hyper-world.de)
 *
 * \b Copyright: Copyright (C) 2009 Jan Gosmann
 */

#ifndef SERVEROPTIONS_H_
#define SERVEROPTIONS_H_

#include "ui_serveroptions.h"

#include <stdexcept>

#include <QDialog>
#include <QFileDialog>
#include <QHashIterator>
#include <QMessageBox>

#include "../bgserver/serversettings.h"

/////////////////////////////////////////////////////////////////////////////
/// \brief Dialog with the server options.
///
/// Dialog for setting the same options you could set in the server
/// configuration file. It has also the possibility to save and load
/// configuration files.
/////////////////////////////////////////////////////////////////////////////
class ServerOptions : public QDialog, public Ui::ServerOptions
{
    Q_OBJECT

public:
    ServerOptions( QWidget *parent = 0, Qt::WindowFlags f = 0 );

    inline ServerSettings & get_settings( void )
      { return mySettings; };

public slots:
    void readFromXml( QString file );
    void writeToXml( QString file );
    void updateGui( void );
    void updateSettings( void );

    void on_load_clicked( void );
    void on_save_clicked( void );
    void on_users_cellChanged( int row, int column );

private:
    ServerSettings mySettings;
};

#endif /* SERVEROPTIONS_H_ */
