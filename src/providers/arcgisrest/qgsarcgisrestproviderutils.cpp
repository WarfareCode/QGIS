/***************************************************************************
    qgsarcgisrestproviderutils.cpp
    ----------------------------
    begin                : Nov 25, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : manisandro@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsarcgisrestproviderutils.h"
#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"

#include <QEventLoop>
#include <QNetworkRequest>

QgsArcGisAsyncQuery::QgsArcGisAsyncQuery( QObject *parent )
  : QObject( parent )
{
}

QgsArcGisAsyncQuery::~QgsArcGisAsyncQuery()
{
  if ( mReply )
    mReply->deleteLater();
}

void QgsArcGisAsyncQuery::start( const QUrl &url, const QString &authCfg, QByteArray *result, bool allowCache, const QgsStringMap &headers )
{
  mResult = result;
  QNetworkRequest request( url );

  for ( auto it = headers.constBegin(); it != headers.constEnd(); ++it )
  {
    request.setRawHeader( it.key().toUtf8(), it.value().toUtf8() );
  }

  if ( !authCfg.isEmpty() &&  !QgsApplication::authManager()->updateNetworkRequest( request, authCfg ) )
  {
    const QString error = tr( "network request update failed for authentication config" );
    emit failed( QStringLiteral( "Network" ), error );
    return;
  }

  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsArcGisAsyncQuery" ) );
  if ( allowCache )
  {
    request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
    request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
  }
  mReply = QgsNetworkAccessManager::instance()->get( request );
  connect( mReply, &QNetworkReply::finished, this, &QgsArcGisAsyncQuery::handleReply );
}

void QgsArcGisAsyncQuery::handleReply()
{
  mReply->deleteLater();
  // Handle network errors
  if ( mReply->error() != QNetworkReply::NoError )
  {
    QgsDebugMsg( QStringLiteral( "Network error: %1" ).arg( mReply->errorString() ) );
    emit failed( QStringLiteral( "Network error" ), mReply->errorString() );
    return;
  }

  // Handle HTTP redirects
  QVariant redirect = mReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
  if ( !redirect.isNull() )
  {
    QNetworkRequest request = mReply->request();
    QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsArcGisAsyncQuery" ) );
    QgsDebugMsg( "redirecting to " + redirect.toUrl().toString() );
    request.setUrl( redirect.toUrl() );
    mReply = QgsNetworkAccessManager::instance()->get( request );
    connect( mReply, &QNetworkReply::finished, this, &QgsArcGisAsyncQuery::handleReply );
    return;
  }

  *mResult = mReply->readAll();
  mResult = nullptr;
  emit finished();
}

///////////////////////////////////////////////////////////////////////////////

QgsArcGisAsyncParallelQuery::QgsArcGisAsyncParallelQuery( const QString &authcfg, const QgsStringMap &requestHeaders, QObject *parent )
  : QObject( parent )
  , mAuthCfg( authcfg )
  , mRequestHeaders( requestHeaders )
{
}

void QgsArcGisAsyncParallelQuery::start( const QVector<QUrl> &urls, QVector<QByteArray> *results, bool allowCache )
{
  Q_ASSERT( results->size() == urls.size() );
  mResults = results;
  mPendingRequests = mResults->size();
  for ( int i = 0, n = urls.size(); i < n; ++i )
  {
    QNetworkRequest request( urls[i] );
    QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsArcGisAsyncParallelQuery" ) );
    QgsSetRequestInitiatorId( request, QString::number( i ) );

    for ( auto it = mRequestHeaders.constBegin(); it != mRequestHeaders.constEnd(); ++it )
    {
      request.setRawHeader( it.key().toUtf8(), it.value().toUtf8() );
    }
    if ( !mAuthCfg.isEmpty() && !QgsApplication::authManager()->updateNetworkRequest( request, mAuthCfg ) )
    {
      const QString error = tr( "network request update failed for authentication config" );
      mErrors.append( error );
      QgsMessageLog::logMessage( error, tr( "Network" ) );
      continue;
    }

    request.setAttribute( QNetworkRequest::HttpPipeliningAllowedAttribute, true );
    if ( allowCache )
    {
      request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
      request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
      request.setRawHeader( "Connection", "keep-alive" );
    }
    QNetworkReply *reply = QgsNetworkAccessManager::instance()->get( request );
    reply->setProperty( "idx", i );
    connect( reply, &QNetworkReply::finished, this, &QgsArcGisAsyncParallelQuery::handleReply );
  }
}

void QgsArcGisAsyncParallelQuery::handleReply()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply *>( QObject::sender() );
  QVariant redirect = reply->attribute( QNetworkRequest::RedirectionTargetAttribute );
  int idx = reply->property( "idx" ).toInt();
  reply->deleteLater();
  if ( reply->error() != QNetworkReply::NoError )
  {
    // Handle network errors
    mErrors.append( reply->errorString() );
    --mPendingRequests;
  }
  else if ( !redirect.isNull() )
  {
    // Handle HTTP redirects
    QNetworkRequest request = reply->request();
    QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsArcGisAsyncParallelQuery" ) );
    QgsDebugMsg( "redirecting to " + redirect.toUrl().toString() );
    request.setUrl( redirect.toUrl() );
    reply = QgsNetworkAccessManager::instance()->get( request );
    reply->setProperty( "idx", idx );
    connect( reply, &QNetworkReply::finished, this, &QgsArcGisAsyncParallelQuery::handleReply );
  }
  else
  {
    // All OK
    ( *mResults )[idx] = reply->readAll();
    --mPendingRequests;
  }
  if ( mPendingRequests == 0 )
  {
    emit finished( mErrors );
    mResults = nullptr;
    mErrors.clear();
  }
}
