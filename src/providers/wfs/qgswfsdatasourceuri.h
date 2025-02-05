/***************************************************************************
    qgswfsdatasourceuri.h
    ---------------------
    begin                : February 2016
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWFSDATASOURCEURI_H
#define QGSWFSDATASOURCEURI_H

#include "qgsauthmanager.h"
#include "qgsdatasourceuri.h"
#include "qgsrectangle.h"
#include "qgsapplication.h"

#include <QNetworkRequest>
#include <QString>

// TODO: merge with QgsWmsAuthorization?
struct QgsWFSAuthorization
{
  QgsWFSAuthorization( const QString &userName = QString(), const QString &password = QString(), const QString &authcfg = QString() )
    : mUserName( userName )
    , mPassword( password )
    , mAuthCfg( authcfg )
  {}

  //! update authorization for request
  bool setAuthorization( QNetworkRequest &request ) const
  {
    if ( !mAuthCfg.isEmpty() ) // must be non-empty value
    {
      return QgsApplication::authManager()->updateNetworkRequest( request, mAuthCfg );
    }
    else if ( !mUserName.isNull() || !mPassword.isNull() ) // allow empty values
    {
      request.setRawHeader( "Authorization", "Basic " + QStringLiteral( "%1:%2" ).arg( mUserName, mPassword ).toLatin1().toBase64() );
    }
    return true;
  }

  //! update authorization for reply
  bool setAuthorizationReply( QNetworkReply *reply ) const
  {
    if ( !mAuthCfg.isEmpty() )
    {
      return QgsApplication::authManager()->updateNetworkReply( reply, mAuthCfg );
    }
    return true;
  }

  //! Username for basic http authentication
  QString mUserName;

  //! Password for basic http authentication
  QString mPassword;

  //! Authentication configuration ID
  QString mAuthCfg;
};

/**
 * Utility class that wraps a QgsDataSourceUri with conveniency
 * methods with the parameters used for a WFS URI.
 */
class QgsWFSDataSourceURI
{
  public:

    //! Http method for DCP URIs
    enum Method
    {
      Get,
      Post
    };

    explicit QgsWFSDataSourceURI( const QString &uri );

    //! Returns the URI, avoiding expansion of authentication configuration, which is handled during network access
    const QString uri() const;

    //! Returns base URL (with SERVICE=WFS parameter if bIncludeServiceWFS=true)
    QUrl baseURL( bool bIncludeServiceWFS = true ) const;

    //! Returns request URL (with SERVICE=WFS parameter)
    QUrl requestUrl( const QString &request, const Method &method = Method::Get ) const;

    //! Gets WFS version. Can be auto, 1.0.0, 1.1.0 or 2.0.0.
    QString version() const;

    //! Returns user defined limit of features to download. 0=no limitation
    int maxNumFeatures() const;

    //! Sets user defined limit of features to download
    void setMaxNumFeatures( int maxNumFeatures );

    //! Returns user defined limit page size. 0=server udefault
    int pageSize() const;

    //! Returns whether paging is enabled.
    bool pagingEnabled() const;

    //! Gets typename (with prefix)
    QString typeName() const;

    //! Sets typename (with prefix)
    void setTypeName( const QString &typeName );

    //! Gets SRS name (in the normalized form EPSG:xxxx)
    QString SRSName() const;

    //! Sets SRS name (in the normalized form EPSG:xxxx)
    void setSRSName( const QString &crsString );

    //! Sets version
    void setVersion( const QString &versionString );

    //! Gets OGC filter xml or a QGIS expression
    QString filter() const;

    //! Sets OGC filter xml or a QGIS expression
    void setFilter( const QString &filterIn );

    //! Gets SQL query
    QString sql() const;

    //! Sets SQL query
    void setSql( const QString &sql );

    //! Gets GetFeature output format
    QString outputFormat() const;

    //! Sets GetFeature output format
    void setOutputFormat( const QString &outputFormat );

    //! Returns whether GetFeature request should include the request bounding box. Defaults to false
    bool isRestrictedToRequestBBOX() const;

    //! Returns whether axis orientation should be ignored (for WFS >= 1.1). Defaults to false
    bool ignoreAxisOrientation() const;

    //! Returns whether axis orientation should be inverted. Defaults to false
    bool invertAxisOrientation() const;

    //! For debug purposes. Checks that functions used in sql match functions declared by the server. Defaults to false
    bool validateSqlFunctions() const;

    //! Whether to hide download progress dialog in QGIS main app. Defaults to false
    bool hideDownloadProgressDialog() const;

    //! Returns authorization parameters
    QgsWFSAuthorization &auth() { return mAuth; }

    //! Builds a derived uri from a base uri
    static QString build( const QString &uri,
                          const QString &typeName,
                          const QString &crsString = QString(),
                          const QString &sql = QString(),
                          bool restrictToCurrentViewExtent = false );

    //! Sets Get DCP endpoints
    void setGetEndpoints( const QgsStringMap &map );

    //! Sets Post DCP endpoints
    void setPostEndpoints( const QgsStringMap &map );

  private:
    QgsDataSourceUri    mURI;
    QgsWFSAuthorization mAuth;
    QgsStringMap mGetEndpoints;
    QgsStringMap mPostEndpoints;
};


#endif // QGSWFSDATASOURCEURI_H
